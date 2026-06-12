
SAMPLE_LIB_MODE := static

SAMPLE_INCLUDES := -I$(SDK_DIR)/source/gmp/include \
		   -I$(SDK_DIR)/source/gmp/modules/osal/include \
		   -I$(SDK_DIR)/source/gmp/usr/include \
		   -I$(SDK_DIR)/sample/common \
		   -I$(SDK_DIR)/source/gmp/usr/common/include \
		   -I$(SDK_DIR)/source/gmp/drv/osal/include

SYS_LIBS := -lpthread -lm -lc -lrt -ldl -lstdc++

# gmp base
GMP_LIBS := -lxmedia_venc -lxmedia_vpss -lxmedia_region -lxmedia_vi -lxmedia_vgs -lxmedia_common -lxmedia_tde -lxmedia_cl -lxmedia_npu -lxmedia_svp -lxmedia_ive
GMP_LIBS += -lxmedia_isp -lxmedia_ae -lxmedia_awb -lxmedia_flatcc -lxmedia_pm
GMP_LIBS += -lsns_sc485sl
#GMP_LIBS += -lsns_sc235hai

SAMPLE_LIBS += -L$(OUT_DIR)/lib/static
SAMPLE_LIBS += -static -Wl,-Bstatic,--start-group $(GMP_LIBS) $(SYS_LIBS) -Wl,--end-group

SAMPLE_CFLAGS := $(SDK_USR_CFLAGS)
SAMPLE_CFLAGS += -DUSER_BIT_32 -DKERNEL_BIT_32 -Wno-date-time -D_GNU_SOURCE
SAMPLE_CFLAGS += -DACODEC_TYPE_INNER -DVQE_USE_STATIC_MODULE_REGISTER -DAAC_USE_STATIC_MODULE_REGISTER -DAAC_HAVE_SBR_LIB -DCHIPSET=$(CHIPSET)

#SENSOR0_TYPE := SMART_SC235HAI_MIPI_2M_2LANE_30FPS_10BIT
SENSOR0_TYPE := SMART_SC485SL_MIPI_4M_30FPS_2LANE_12BIT

SAMPLE_CFLAGS += -D$(SENSOR0_TYPE)

#### reserve double sensor cfg
###SENSOR1_TYPE := SMART_SC3338_MIPI_3M_30FPS_10BIT
###SAMPLE_CFLAGS += -D$(SENSOR1_TYPE)

BOARD_TYPE := IPC_RB_B
SAMPLE_CFLAGS += -DBOARD_TYPE=$(BOARD_TYPE)
