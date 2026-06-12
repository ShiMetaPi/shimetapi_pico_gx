#
# Copyright (c) Lotus. All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause
#



LOTUS_PLAT		:=	plat/lotus
LOTUS_PLAT_SOC		:=	${LOTUS_PLAT}/${PLAT}

PLAT_INCLUDES		:=	-I${LOTUS_PLAT}/				\
				-I${LOTUS_PLAT}/include/			\
				-Iinclude/plat/common/				\
				-Iinclude/common/				\
				-Iinclude/drivers/				\
				-Iinclude/drivers/arm/                          \
				-Iinclude/lib/xlat_tables/			\
				-Iinclude/lib/					\
				-Iinclude/lib/el3_runtime/			\
				-Iinclude/lib/psci/				\
				-I${LOTUS_PLAT_SOC}/				\
				-I${LOTUS_PLAT_SOC}/include/

# Include GICv2 driver files
# include drivers/arm/gic/v2/gicv2.mk
LOTUS_GIC_SOURCES	:= 	drivers/arm/gic/common/gic_common.c	\
				drivers/arm/gic/v2/gicv2_main.c		\
				drivers/arm/gic/v2/gicv2_helpers.c	\
				plat/common/plat_gicv2.c		\
				${LOTUS_PLAT}/common/lotus_gicv2.c

PLAT_BL_COMMON_SOURCES	:=	lib/xlat_tables_v2/xlat_tables_context.c	\
				lib/xlat_tables_v2/xlat_tables_utils.c		\
				lib/xlat_tables_v2/xlat_tables_core.c		\
				lib/xlat_tables_v2/aarch64/enable_mmu.S	\
				lib/xlat_tables_v2/aarch64/xlat_tables_arch.c	\
				plat/common/plat_psci_common.c

BL31_SOURCES		+=	$(LOTUS_GIC_SOURCES)				\
				drivers/arm/pl011/aarch64/pl011_console.S \
				drivers/delay_timer/delay_timer.c		\
				lib/cpus/aarch64/aem_generic.S			\
				lib/cpus/aarch64/cortex_a55.S			\
				plat/common/aarch64/platform_mp_stack.S		\
				${LOTUS_PLAT}/common/lotus_bl31_setup.c		\
				${LOTUS_PLAT}/common/lotus_console.c		\
				${LOTUS_PLAT_SOC}/aarch64/plat_helpers.S		\
				${LOTUS_PLAT_SOC}/aarch64/platform_common.c	\
				${LOTUS_PLAT_SOC}/plat_topology.c		\
				${LOTUS_PLAT_SOC}/drivers/pmc/pmc.c		\
				${LOTUS_PLAT_SOC}/bl31_plat_setup.c		\
				${LOTUS_PLAT_SOC}/plat_pm.c			\
				${LOTUS_PLAT_SOC}/plat_delay_timer.c		\
				${LOTUS_PLAT_SOC}/resume_entry_32bit.S	\
				${LOTUS_PLAT_SOC}/sleep.S \
				${LOTUS_PLAT_SOC}/lotus_suspend.S

HW_ASSISTED_COHERENCY   := 1
USE_COHERENT_MEM        := 0
ENABLE_PLAT_COMPAT	:= 0
CTX_INCLUDE_FPREGS	:= 1
NEED_BL33		:= yes
MULTI_CONSOLE_API	:= 1
PROGRAMMABLE_RESET_ADDRESS := 1
CONFIG_XMFALCON	:= 1

ERRATA_ABI_SUPPORT := 1
CORTEX_A55_H_INC   := 1
ERRATA_A55_1530923 := 1

# Do not enable SVE
ENABLE_SVE_FOR_NS	:= 0
#CONFIG_FPGA		:= 0
CONFIG_LOTUS		:= 1
$(eval $(call add_define,CONFIG_LOTUS))
$(eval $(call add_define,CONFIG_BL31_BASE))
#$(eval $(call add_define,CONFIG_FPGA))
$(eval $(call add_define,CONFIG_XMFALCON))
ifeq (${SPD},opteed)
$(eval $(call add_define,CONFIG_LOTUS_OPTEED))
endif
