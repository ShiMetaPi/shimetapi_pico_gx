
SAMPLE_LIB_MODE := static

SAMPLE_INCLUDES := -I$(SDK_DIR)/source/gmp/include \
		   -I$(SDK_DIR)/source/gmp/modules/osal/include \
		   -I$(SDK_DIR)/source/gmp/usr/include \
		   -I$(SDK_DIR)/sample/common \
		   -I$(SDK_DIR)/source/gmp/usr/common/include \
		   -I$(SDK_DIR)/source/gmp/drv/osal/include \
		   -I$(SDK_DIR)/source/gmp/drv/include \
		   -I$(SDK_DIR)/source/gmp/drv/misc/include

SYS_LIBS := -lpthread -lm -lc -lrt -ldl -lstdc++

# gmp base
GMP_LIBS := -lxmedia_venc -lxmedia_vpss -lxmedia_region -lxmedia_vi -lxmedia_vo -lxmedia_vgs -lxmedia_common -lxmedia_tde -lxmedia_cl -lxmedia_npu -lxmedia_svp -lxmedia_pm
GMP_LIBS += -lxmedia_audio -lxmedia_audio_vqe -lxmedia_audio_vqe_bcd -lxmedia_audio_vqe_sv2_3a -lxmedia_audio_vqe_sv3_3a -lxmedia_audio_vqe_sv2_bf -lxmedia_audio_vqe_sv1_3a -lxmedia_audio_vqe_sv_vad -lxmedia_audio_vqe_sv_hpf -lxmedia_audio_vqe_sv_gain -lxmedia_audio_vqe_sv_wns -lxmedia_audio_vqe_sv_eq -lxmedia_audio_vqe_sv_ssl -lxmedia_audio_vqe_sv_ved -lxmedia_audio_vqe_sv_npu_gbd -lxmedia_audio_vqe_sv_dereverb -lxmedia_audio_vqe_res -lxmedia_audio_vqe_sv_common \
-lxmedia_audio_aac_common -lxmedia_audio_aac_dec -lxmedia_audio_aac_enc -lxmedia_audio_mp3_dec -lxmedia_audio_mp3_enc -lxmedia_audio_adpcm_codec -lxmedia_audio_g711_codec -lxmedia_audio_g726_codec \
-lxmedia_isp -lxmedia_ae -lxmedia_awb -lxmedia_flatcc -lxmedia_audio_lpcm_codec -lxmedia_cipher -lxmedia_ive -lxmedia_md
GMP_LIBS += -lsns_sc8238 -lsns_gc2053 -lsns_os08a20 -lsns_os04a10 -lsns_os04c10 -lsns_os04d10 -lsns_os05a10 -lsns_sc850sl -lsns_sc530ai -lsns_sc200ai -lsns_sc500ai -lsns_sc533hai -lsns_sc450ai -lsns_gc4663 -lsns_cv4003 \
-lsns_gc8613 -lsns_os02k10 -lsns_imx678 -lsns_sc3338 -lsns_imx307 -lsns_gc5603 -lsns_ox03f10 -lsns_sc4336 -lsns_ox05b1s -lsns_imx415 -lsns_os04j10 -lsns_sc485sl -lsns_sc231hai -lsns_sc465sl -lsns_sc235hai -lsns_mis40h1 -lsns_sc2337p -lsns_mis20s1 -lsns_gc4653 -lsns_sc4336p \
-lsns_cv2005 -lsns_ov4689  -lsns_imx179 -lsns_gc2083 -lsns_gc20c3 -lsns_gc6603 -lsns_sc285sl -lsns_sc431hai -lsns_sc435hai


GMP_LIBS += -lsample_common -llive_rtsp

ifeq ($(SAMPLE_LIB_MODE), static)
SAMPLE_LIBS += -L$(OUT_DIR)/lib/static
SAMPLE_LIBS += -Wl,-Bstatic,--start-group $(GMP_LIBS) -Wl,--end-group,-Bdynamic $(SYS_LIBS)

else ifeq ($(SAMPLE_LIB_MODE), share)
SAMPLE_LIBS += -L$(OUT_DIR)/lib/share
SAMPLE_LIBS += -Wl,--start-group $(GMP_LIBS) $(SYS_LIBS) -Wl,--end-group
endif

SAMPLE_COMMON_LIB := -L$(SDK_DIR)/sample/common

SAMPLE_CFLAGS := $(SDK_USR_CFLAGS)
SAMPLE_CFLAGS += -I $(SAMPLE_DIR)/live_rtsp_server/include
SAMPLE_CFLAGS += -DUSER_BIT_32 -DKERNEL_BIT_32 -Wno-date-time -D_GNU_SOURCE
SAMPLE_CFLAGS += -DACODEC_TYPE_INNER -DVQE_USE_STATIC_MODULE_REGISTER -DAAC_USE_STATIC_MODULE_REGISTER -DAAC_HAVE_SBR_LIB -DCHIPSET=$(CHIPSET)

SENSOR0_TYPE := SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT
SENSOR1_TYPE := SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT
SENSOR2_TYPE := SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT
SENSOR3_TYPE := SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT
SENSOR4_TYPE := SMART_SC465SL_MIPI_4M_4LANE_30FPS_12BIT

SAMPLE_CFLAGS += -DSENSOR0_TYPE=$(SENSOR0_TYPE)
SAMPLE_CFLAGS += -DSENSOR1_TYPE=$(SENSOR1_TYPE)
SAMPLE_CFLAGS += -DSENSOR2_TYPE=$(SENSOR2_TYPE)
SAMPLE_CFLAGS += -DSENSOR3_TYPE=$(SENSOR3_TYPE)
SAMPLE_CFLAGS += -DSENSOR4_TYPE=$(SENSOR4_TYPE)

SCREEN0_TYPE := MICROTECH_MTF050_LCD_800X480
SAMPLE_CFLAGS += -DSCREEN0_TYPE=$(SCREEN0_TYPE) -DSCREEN1_TYPE=$(SCREEN1_TYPE) -DSCREEN2_TYPE=$(SCREEN2_TYPE)

