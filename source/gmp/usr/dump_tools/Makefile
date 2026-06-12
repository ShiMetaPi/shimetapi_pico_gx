ifeq ($(CFG_XMEDIA_EXPORT_FLAG),)
   SDK_DIR := $(shell  cd $(CURDIR)/../../../.. && /bin/pwd)
endif

include $(SDK_DIR)/build/base.mk

SYS_LIBS := -lpthread -lm -lc -lrt -ldl -lstdc++

GMP_LIBS += -lxmedia_vpss   \
            -lxmedia_region \
            -lxmedia_vi     \
            -lxmedia_vgs    \
            -lxmedia_common \
            -lxmedia_tde    \
            -lxmedia_isp    \
            -lxmedia_ae     \
            -lxmedia_venc   \
            -lxmedia_vo     \
            -lxmedia_awb    \
            -lxmedia_svp    \
            -lxmedia_cl     \
            -lxmedia_npu    \
            -lxmedia_flatcc

TOOL_LIBS += -L$(XMEDIA_STATIC_LIB_DIR)
TOOL_LIBS += -Wl,-Bstatic,--start-group $(GMP_LIBS) -Wl,--end-group,-Bdynamic $(SYS_LIBS)

CFLAGS := $(SDK_USR_CFLAGS) $(TOOL_LIBS)
CFLAGS += -I$(SDK_DIR)/source/gmp/include

SRCS := $(wildcard ./*.c)

OBJS := $(patsubst %.c, %.o, $(SRCS))

TARGET := $(OBJS:%.o=%)

.PHONY: all clean

.PHONY : clean all $(SNS)

all: $(TARGET)

$(TARGET):%:%.o
	$(AT)$(CC) -o $@ $^ $(CFLAGS)
	$(AT)rm $^

%.o : %.c
	$(AT)$(CC) -c -o $@ $< $(CFLAGS)

clean:
	$(AT)rm -f $(TARGET)
	$(AT)rm -f $(OBJS)
