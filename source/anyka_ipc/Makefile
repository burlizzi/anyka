include config.mk

LIBROOT_DIR = ../../../librootfs
LIBMEDIA_DIR = ../../rootfs
USE_CPP := 0

LOCAL_SRC_CFILES := \
	app/src/audio_play.c \
	app/src/audio_record.c \
	app/src/record_replay.c \
	app/src/anyka_ftp_update.c \
	app/src/ap_opt.c \
	app/src/video_record.c \
	app/src/anyka_interface.c \
	app/src/anyka_time.c	\
	app/src/audio_talk.c		\
	app/src/anyka_ini.c		\
	app/src/ipc_main.c              \
	app/src/anyka_config.c              \
	app/src/anyka_dection.c             \
	app/src/video_net.c           \
	app/src/anyka_photo.c           \
	device/src/audio_hd.c              \
	device/src/camera.c              \
	device/src/PTZControl.c              \
	device/src/SDcard.c              \
	mid/src/video_demux.c 		\
	mid/src/anyka_fs.c              \
	mid/src/anyka_queue.c              \
	mid/src/audio_ctrl.c              \
	mid/src/audio_decode.c              \
	mid/src/audio_enc.c              \
	mid/src/font_display.c              \
	mid/src/font_lib.c              \
	mid/src/muxer.c              \
	mid/src/video_ctrl.c              \
	mid/src/video_encode.c              \
	mid/src/video_fs_manage.c              \
	mid/src/video_fs_list_manage.c              \
	mid/src/anyka_com.c              \
	mid/src/anyka_monitor.c              \
	mid/src/voice_tips.c			\
	mid/src/convert.c   \
	device/src/isp_conf_file.c

LOCAL_SRC_CXXFILES 	:=

ifeq ($(CONFIG_DANA_SUPPORT), y)
LOCAL_DANA_SRC_CFILES := \
	cloud/dana/src/dana_init.c              \
	cloud/dana/src/dana_interface.c              
endif

ifeq ($(CONFIG_RTSP_SUPPORT), y)
LOCAL_RTSP_DIR := cloud/rtsp

LOCAL_RTSP_SRC_CFILES := $(LOCAL_RTSP_DIR)/src/ak_rtsp.c 

LOCAL_RTSP_SRC_CXXFILES := \
	$(LOCAL_RTSP_DIR)/src/rtsp_main.cpp \
	$(LOCAL_RTSP_DIR)/src/mid_rtsp.cpp
	
LOCAL_RTSP_INCLUDES := \
	$(LOCAL_RTSP_DIR)/include/akliveMedia/include/				\
	$(LOCAL_RTSP_DIR)/include/BasicUsageEnvironment/include/	\
	$(LOCAL_RTSP_DIR)/include/groupsock/include/				\
	$(LOCAL_RTSP_DIR)/include/liveMedia/include/				\
	$(LOCAL_RTSP_DIR)/include/UsageEnvironment/include/			\
	$(LOCAL_RTSP_DIR)/include/									\

LOCAL_RTSP_LIBS	:= \
	$(LOCAL_RTSP_DIR)/lib/libakliveMedia.a 	\
	$(LOCAL_RTSP_DIR)/lib/libliveMedia.a	\
	$(LOCAL_RTSP_DIR)/lib/libgroupsock.a 	\
	$(LOCAL_RTSP_DIR)/lib/libBasicUsageEnvironment.a  \
	$(LOCAL_RTSP_DIR)/lib/libUsageEnvironment.a	
endif

ifeq ($(CONFIG_ONVIF_SUPPORT), y)
LOCAL_ONVIF_SRC_CFILES := \
	cloud/onvif/src/onvif_init.c    \
	cloud/onvif/src/ak_onvif.c

LOCAL_ONVIF_LIBS :=	\
	cloud/onvif/libonvif.a	
endif

ifeq ($(CONFIG_TENCENT_SUPPORT), y)
LOCAL_TENCENT_SRC_CFILES := \
		cloud/tencent/src/ak_tencent.c        \
		cloud/tencent/src/tencent_init.c
endif

LOCAL_MODULE := anyka_ipc

LOCAL_CFLAGS := -D__LINUX__

PREFIX := arm-linux-gnueabi-

#????????????????????????????libs????????????????
#LOCAL_SHARED_LIBRARIES :=  akuio akmedialib akaudiocodec akaudiofilter dl  pthread
LOCAL_SHARED_LIBRARIES :=  akuio akmedialib akaudiocodec akaudiofilter akstreamenclib dl akmotiondetectlib pthread 

#????????????????????????????????????????????????????????????
LOCAL_C_INCLUDES := $(LIBMEDIA_DIR)/akmedialib/include/akmedialib \
					$(LIBMEDIA_DIR)/akmedialib/include \
					$(LIBMEDIA_DIR)/akuiolib/include \
					./app/include \
					./cloud/dana/include \
					./cloud/onvif/include \
					./cloud/tencent/include \
					./device/include \
					./mid/include \
					./include \
					./ \
#??????????????????????????????????????lib.
LOCAL_LDFLAGS := -L$(LIBMEDIA_DIR)/akmedialib/usr/lib \
				-L$(LIBMEDIA_DIR)/akuiolib/usr/lib \
				-L$(LIBMEDIA_DIR)/rootfs_tencent/lib \
				-L$(LIBMEDIA_DIR)/rootfs_tencent/usr/lib \
				-L./ \
				-L./app \


LOCAL_LDFLAGS += ./app/libpcm.a	
LOCAL_LDFLAGS += ./device/libispsdk.a	

		
ifeq ($(CONFIG_DANA_SUPPORT), y)
	LOCAL_SRC_CFILES += $(LOCAL_DANA_SRC_CFILES)
	LOCAL_LDFLAGS += ./cloud/dana/libdana_video.a -lresolv	
	LOCAL_SHARED_LIBRARIES += iconv 
	LOCAL_CFLAGS += -DCONFIG_DANA_SUPPORT=1
endif
		
ifeq ($(CONFIG_TENCENT_SUPPORT), y)
	LOCAL_SRC_CFILES += $(LOCAL_TENCENT_SRC_CFILES)
	LOCAL_SHARED_LIBRARIES += iconv txdevicesdk stdc++ 
	LOCAL_CFLAGS += -DCONFIG_TENCENT_SUPPORT=1
endif
	
ifeq ($(CONFIG_FTP_UPDATE_SUPPORT), y)
	LOCAL_CFLAGS += -DCONFIG_FTP_UPDATE_SUPPORT=1
endif

ifeq ($(CONFIG_ONVIF_SUPPORT), y)
	LOCAL_SRC_CFILES += $(LOCAL_ONVIF_SRC_CFILES)
	LOCAL_LDFLAGS += $(LOCAL_ONVIF_LIBS)
	LOCAL_CFLAGS += -DCONFIG_ONVIF_SUPPORT=1
endif


ifeq ($(CONFIG_RTSP_SUPPORT), y)
USE_CPP += 1
	LOCAL_SRC_CFILES 	+= $(LOCAL_RTSP_SRC_CFILES)
	LOCAL_SRC_CXXFILES 	+= $(LOCAL_RTSP_SRC_CXXFILES)
	LOCAL_C_INCLUDES 	+= $(LOCAL_RTSP_INCLUDES)
	LOCAL_LDFLAGS 		+= $(LOCAL_RTSP_LIBS)
	LOCAL_CFLAGS 		+= -DCONFIG_RTSP_SUPPORT=1
endif

include exec_rules.mk
