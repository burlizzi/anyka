#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>

#include <easy_setup.h>
#include <scan.h>

#include <cooee.h>
#include <neeze.h>
#include <akiss.h>

#define ES_TARGET_VERSION "v2.0.0"

#define QUERY_TIMEOUT_MS (30*1000)
#define QUERY_INTERVAL_MS (200)

#define WLAN_IFACE "wlan0"

#define PARAM_MAX_LEN (256)
#define RESULT_MAX_LEN (256)

#define DEFAULT_PROTOCOL_MASK (0x1) /* cooee */
uint16 g_protocol_mask = 0; /* default no protocols enabled */

/* Linux network driver ioctl encoding */
typedef struct wl_ioctl {
    uint cmd;        /* common ioctl definition */
    void *buf;       /* pointer to user buffer */
    uint len;        /* length of user buffer */
    unsigned char set;  /* 1=set IOCTL; 0=query IOCTL */
    uint used;       /* bytes read or written (optional) */
    uint needed;     /* bytes needed (optional) */
} wl_ioctl_t;

typedef struct {      
    uint8 enable;    /* START/STOP/RESTART easy setup */ 
    uint16 protocol_mask;  /* set parameters to which protocol */ 
    tlv_t param; /* param blocks for each protocol */
} easy_setup_param_t;

static int g_ioc_fd = -1; /* socket fd */
static easy_setup_result_t g_result;
static uint8 g_protocol = 0;

extern int g_sniffer_running;

int easy_setup_start() {
    int i;
    int ret = 0;

    LOGE("Easy setup target library %s\n", ES_TARGET_VERSION);

    int found = 0;
    for (i=0; i<EASY_SETUP_PROTO_MAX; i++) {
        if (g_protocol_mask & (1<<i)) {
            found = 1;
            break;
        }
    }

    if (!found) {
        g_protocol_mask = DEFAULT_PROTOCOL_MASK;
    }

    /* open socket to kernel */
    if ((g_ioc_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        LOGE("create control socket failed: %d(%s)\n", errno, strerror(errno));
        return -1;
    }

    easy_setup_param_t* p = (easy_setup_param_t*) malloc(PARAM_MAX_LEN);
    memset(p, 0, PARAM_MAX_LEN);
    p->enable = WLC_EASY_SETUP_START;
    p->protocol_mask = g_protocol_mask;

    int param_len = 0;
    param_len += ((uint8*)&p->param-(uint8*)p);

    tlv_t* t = &p->param;
    i=0; 
    for (i=0; i<EASY_SETUP_PROTO_MAX; i++) {
        if (g_protocol_mask & (1<<i)) {
            t->type = i;

            if (i==EASY_SETUP_PROTO_COOEE) {
                t->length = sizeof(cooee_param_t);
                cooee_get_param(t->value);
            } else if (i==EASY_SETUP_PROTO_NEEZE) {
                t->length = sizeof(neeze_param_t);
                neeze_get_param(t->value);
            } else if (i==EASY_SETUP_PROTO_AKISS) {
                t->length = sizeof(akiss_param_t);
                akiss_get_param(t->value);
            } else {
                LOGE("invalid protocol %d\n", i);
                free(p);
                return -1;
            }

            t = (tlv_t*) (t->value + t->length);
        }
    }

    if ((ret = easy_setup_ioctl(WLC_SET_EASY_SETUP, 1, p, (uint8*) t - (uint8*) p)) < 0) {
        LOGE("easy setup start failed: %d(%s)\n", 
                errno, strerror(errno));
    }

    return ret;
}

int easy_setup_stop() {
    int ret = 0;

    if (g_ioc_fd < 0) {
        LOGE("easy setup query: control socket not initialized.\n");
        return -1;
    }

    easy_setup_param_t param;
    memset(&param, 0, sizeof(param));
    param.enable = WLC_EASY_SETUP_STOP;
    param.protocol_mask = 0;

    if ((ret = easy_setup_ioctl(WLC_SET_EASY_SETUP, 1, &param, sizeof(param))) < 0) {
        LOGE("easy setup stop failed: %d(%s)\n", 
                errno, strerror(errno));
    }

    close(g_ioc_fd);

    return ret;
}

/* query result is of tlv format
 * type = protocol shot
 * length = result length
 * value = cooee_result_t/akiss_result_t ... */
int easy_setup_query() {
    int ret = 0;
    tlv_t* query = NULL;

    if (g_ioc_fd < 0) {
        LOGE("easy setup query: control socket not initialized.\n");
        return -1;
    }

    if (!query) {
        query = (tlv_t*) malloc(RESULT_MAX_LEN);
    }

    if (!query) {
        LOGE("failed allocating result buffer.\n");
        return -1;
    }
    
    uint last_state = 0;
 //  int tries = QUERY_TIMEOUT_MS/QUERY_INTERVAL_MS;
    while (g_sniffer_running) { 
//    while (tries--) {
        ret = easy_setup_ioctl(WLC_GET_EASY_SETUP, 0, query, RESULT_MAX_LEN);
        if (ret < 0) {
            LOGE("easy setup query failed: %d(%s)\n", errno, strerror(errno));
            return -1;
        } else {
  //          int i;

            /* copy common part of result to easy_setup_result_t */
            memcpy(&g_result, query->value, sizeof(g_result));
            if (last_state != g_result.state) {
                LOGD("state: %d --> %d\n", last_state, g_result.state);
                last_state = g_result.state;
                if (last_state == EASY_SETUP_STATE_DONE) {
                    uint8 protocol = query->type;
                    g_protocol = protocol;
                    if (protocol == EASY_SETUP_PROTO_COOEE) {
                        cooee_set_result(query->value);
                    } else if (protocol == EASY_SETUP_PROTO_NEEZE) {
                        neeze_set_result(query->value);
                    } else if (protocol == EASY_SETUP_PROTO_AKISS) {
                        akiss_set_result(query->value);
                    } else {
                        LOGE("result returned for unknown protocol %d\n", protocol);
                    }

                    break;
                }
            }
        }

        usleep(QUERY_INTERVAL_MS * 1000);
    }

 //   if (tries <= 0) {
    if (!g_sniffer_running) { 
        LOGE("easy setup query timed out.\n");
        return -1;
    }

    free(query);

    return 0;
}

int easy_setup_ioctl(int cmd, int set, void* param, int size) {
    struct ifreq ifr;
    wl_ioctl_t ioc;
    int ret = 0;

    if (g_ioc_fd < 0) {
        LOGE("easy setup query: control socket not initialized.\n");
        return -1;
    }

    void* ptr = param;
    ioc.cmd = cmd;
    ioc.buf = ptr;
    ioc.len = size;
    ioc.set = set;

    strncpy(ifr.ifr_name, WLAN_IFACE, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ-1] = 0;
    ifr.ifr_data = (caddr_t) &ioc;

    if ((ret = ioctl(g_ioc_fd, SIOCDEVPRIVATE, &ifr)) < 0) {
        LOGD("easy setup ioctl(cmd=%d) failed: %d(%s)\n", 
                cmd, errno, strerror(errno));
        return -1;
    }

    return 0;
}

int easy_setup_get_ssid(char buff[], int buff_len) {
    int i;
    easy_setup_result_t* r = &g_result;

    if (r->state != EASY_SETUP_STATE_DONE) {
        LOGE("easy setup data unavailable\n");
        return -1;
    }

    if (buff_len < r->ap_ssid.len+1) {
        LOGE("insufficient buffer provided: %d < %d\n", buff_len, r->ap_ssid.len+1);
        return -1;
    }

    for (i=0; i<r->ap_ssid.len; i++) {
        buff[i] = r->ap_ssid.val[i];
    }
    buff[i] = 0;

    return 0;
}

int easy_setup_get_password(char buff[], int buff_len) {
    int i;
    easy_setup_result_t* r = &g_result;

    if (r->state != EASY_SETUP_STATE_DONE) {
        LOGE("easy setup data unavailable\n");
        return -1;
    }

    if (buff_len < r->security_key_length+1) {
        LOGE("insufficient buffer provided: %d < %d\n", buff_len, r->security_key_length+1);
        return -1;
    }

    for (i=0; i<r->security_key_length; i++) {
        buff[i] = r->security_key[i];
    }
    buff[i] = 0;

    return 0;
}

void easy_setup_enable_cooee() {
    g_protocol_mask |= (1<<EASY_SETUP_PROTO_COOEE);
}

void easy_setup_enable_neeze() {
    g_protocol_mask |= (1<<EASY_SETUP_PROTO_NEEZE);
}

void easy_setup_enable_akiss() {
    g_protocol_mask |= (1<<EASY_SETUP_PROTO_AKISS);
}

void easy_setup_enable_qqcon() {
    g_protocol_mask |= (1<<EASY_SETUP_PROTO_COOEE);
    cooee_enable_qqconnect();
}

void easy_setup_enable_protocols(uint16 proto_mask) {
    if (proto_mask & (1<<EASY_SETUP_PROTO_COOEE))
        easy_setup_enable_cooee();
    if (proto_mask & (1<<EASY_SETUP_PROTO_NEEZE))
        easy_setup_enable_neeze();
    if (proto_mask & (1<<EASY_SETUP_PROTO_AKISS))
        easy_setup_enable_akiss();
}

int easy_setup_get_protocol(uint16* protocol) {
//    int i;
    easy_setup_result_t* r = &g_result;

    if (r->state != EASY_SETUP_STATE_DONE) {
        LOGE("easy setup data unavailable\n");
        return -1;
    }

    *protocol = g_protocol;

    return 0;
}

int easy_setup_get_security() {
    if (g_result.state != EASY_SETUP_STATE_DONE) {
        LOGE("easy setup data unavailable\n");
        return -1;
    }

    return scan_and_get_security(0, g_result.ap_bssid.octet);
}
