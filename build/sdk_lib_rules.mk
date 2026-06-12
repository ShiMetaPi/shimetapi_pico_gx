
# modules should define SRCS LIB_NAME SDK_USR_CFLAGS(if needed)
# by default, libs will install to out/lib dir, modules can define THIS_LIB_STATIC_INSTALL_DIR and THIS_LIB_SHARE_INSTALL_DIR to change it

OBJS := $(SRCS:%.c=%.o) $(CPP_SRCS:%.cpp=%.o) $(ASM_SRCS:%.S=%.o)

OBJ_DIR32 := $(OUT_DIR)/obj/$(subst $(SDK_DIR)/,,$(CURDIR))

OBJS32 := $(addprefix $(OBJ_DIR32)/,$(OBJS))

TARGETS := $(OBJ_DIR32)/$(LIB_NAME)

ifeq ($(THIS_LIB_STATIC_INSTALL_DIR),)
THIS_LIB_STATIC_INSTALL_DIR = $(SDK_STATIC_LIB_DIR)
endif

ifeq ($(THIS_LIB_SHARE_INSTALL_DIR),)
THIS_LIB_SHARE_INSTALL_DIR = $(SDK_SHARE_LIB_DIR)
endif

.PHONY: all clean

all: $(TARGETS)
	$(AT)test -d $(THIS_LIB_STATIC_INSTALL_DIR) || mkdir -p $(THIS_LIB_STATIC_INSTALL_DIR)
	$(AT)cp -rf $(OBJ_DIR32)/$(LIB_NAME).a $(THIS_LIB_STATIC_INSTALL_DIR)
	$(AT)test -d $(THIS_LIB_SHARE_INSTALL_DIR) || mkdir -p $(THIS_LIB_SHARE_INSTALL_DIR)
	$(AT)cp -rf $(OBJ_DIR32)/$(LIB_NAME).so $(THIS_LIB_SHARE_INSTALL_DIR)

clean:
	$(AT)rm -rf $(OBJS32)
	$(AT)rm -rf $(OBJ_DIR32)/$(LIB_NAME).a $(OBJ_DIR32)/$(LIB_NAME).so
	$(AT)rm -f $(THIS_LIB_STATIC_INSTALL_DIR)/$(LIB_NAME).a $(THIS_LIB_SHARE_INSTALL_DIR)/$(LIB_NAME).so

$(OBJ_DIR32)/%.o : %.c
	$(AT)test -d $(dir $@) || mkdir -p $(dir $@)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc $(SDK_USR_CFLAGS) -o $@ -c $^

$(OBJ_DIR32)/%.o : %.cpp
	$(AT)test -d $(dir $@) || mkdir -p $(dir $@)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc $(SDK_USR_CFLAGS) -o $@ -c $^

$(OBJ_DIR32)/%.o : %.S
	$(AT)test -d $(dir $@) || mkdir -p $(dir $@)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc $(SDK_USR_CFLAGS) -o $@ -c $^

$(OBJ_DIR32)/$(LIB_NAME) : $(OBJS32)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-ar -rcs $@.a $(OBJS32)
	$(AT)$(CONFIG_XMEDIA_USR_ARM_TOOLCHAINS)-gcc $(SDK_LD_CFLAGS) -fPIC -shared -o $@.so $(OBJS32)

