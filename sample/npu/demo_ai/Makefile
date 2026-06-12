ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
    SDK_DIR := $(shell  cd $(CURDIR)/../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk
include $(SAMPLE_DIR)/sample_base.mk
#$(info '$(TOOLCHAIN)' )

DMS_OPENCV = 0

TARGET := sample_svp

LIBS :=  -lxmedia_svp $(SAMPLE_LIBS) $(SAMPLE_COMMON_LIB) -llive_rtsp

INCLUDES := $(SAMPLE_INCLUDES)
ifeq ($(DMS_OPENCV),1)
	INCLUDES += -I./common/lib/opencv/include
endif
INCLUDES += -I./common/lib/ffmpeg/include
INCLUDES += -I./common
INCLUDES += -I./common/font
INCLUDES += -I./common/data_base
INCLUDES += -I./common/data_base/include

ifeq ($(TOOLCHAIN),arm-gcc12.2.0-linux)
	LIBS += -L./common/lib/ffmpeg/arm-gcc12.2.0-linux-gnueabi -lavformat -lavcodec -lavutil -lswscale -lswresample
	LIBS += -L./common/lib/cjson/arm-gcc12.2.0-linux-gnueabi -lcjson
	LIBS += -L./data_base/SQLite/$(TOOLCHAIN) -lsqlite3
endif

ifeq ($(TOOLCHAIN),arm-gcc12.2.0-linux-uclibceabi)
	ifeq ($(DMS_OPENCV),1)
		LIBS += -L./common/lib/opencv/arm-gcc12.2.0-linux-uclibceabi -lopencv_calib3d -lopencv_imgproc \
		-lopencv_features2d -lopencv_core -lopencv_flann -lzlib -ltegra_hal -littnotify
	endif
	LIBS += -L./common/lib/ffmpeg/arm-gcc12.2.0-linux-uclibceabi -lavformat -lavcodec -lavutil -lswscale
	LIBS += -L./common/lib/cjson/arm-gcc12.2.0-linux-uclibceabi -lcJSON
	LIBS += -L./common/lib/SQLite/$(TOOLCHAIN) -lsqlite3
endif
#live_libs += -Wl,-Bstatic,--start-group -latomic -Wl,--end-group
CFLAGS := $(SAMPLE_CFLAGS) $(LIBS) $(INCLUDES) #$(live_libs)
#CFLAGS += -latomic
#CFLAGS += -DUSE_OSD
#CFLAGS += -DREAD_YUV
SRCS := $(wildcard ./*.c)
SRCS += $(wildcard ./common/data_base/*.c)
SRCS += $(wildcard ./common/font/*.c)
OBJS := $(patsubst %.c, %.o, $(SRCS))
ifeq ($(DMS_OPENCV),1)
    CPP_SRCS := $(wildcard ./common/*.cpp)
    CPP_OBJS := $(patsubst %.cpp, %.o, $(CPP_SRCS))
    CPP := $(TOOLCHAIN)-g++
    CFLAGS += -DUSE_OSD
    CFLAGS += -DDMS_OPENCV
endif
.PHONY: all clean

all: $(OBJS) $(CPP_OBJS)
	$(AT)$(CC) -o $(TARGET) $^ $(CFLAGS)

$(OBJS) : %.o: %.c
	$(AT)$(CC) -c -o $@ $< $(CFLAGS)

ifeq ($(DMS_OPENCV),1)
$(CPP_OBJS) : %.o: %.cpp
	$(AT)$(CPP) -c -o $@ $< $(CFLAGS)
endif

clean:
	$(AT)rm -rf $(OBJS) $(CPP_OBJS) $(TARGET)


