SDK_DIR ?= $(shell cd $(CURDIR)/../../../../../../.. && /bin/pwd)
include $(SDK_DIR)/build/base.mk

CROSS_COMP := $(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)
LIBRARY = lib/libwarp_affine.a

CC = $(CROSS_COMP)-gcc
CPP = $(CROSS_COMP)-g++
AR = $(CROSS_COMP)-ar

CSRCS = $(wildcard *.c)
CPPSRCS = $(wildcard *.cpp)

COBJS := $(CSRCS:.c=.o)
CPPOBJS := $(CPPSRCS:.cpp=.o)

PORTING_CFLAGS := -O3 -fPIC -std=c++0x -D__LINUX__ -w -mcpu=cortex-a7 -mfloat-abi=hard -mfpu=neon-vfpv4 -fno-aggressive-loop-optimizations -ldl -ffunction-sections -fdata-sections -fstack-protector-strong -DUSER_BIT_32 -DKERNEL_BIT_32 -Wno-date-time -D_GNU_SOURCE

.PHONY: all clean

all:$(LIBRARY)

$(COBJS) : %.o: %.c
	$(CC) $(INCLUDE) $(PORTING_CFLAGS) -c $< -o $@
$(CPPOBJS) : %.o: %.cpp
	$(CPP) $(INCLUDE) $(PORTING_CFLAGS) -c $< -o $@

$(LIBRARY) :$(COBJS) $(CPPOBJS)
	$(AR) rcs $(LIBRARY) $(COBJS) $(CPPOBJS);

clean:
	rm -rfv $(COBJS) $(CPPOBJS);


