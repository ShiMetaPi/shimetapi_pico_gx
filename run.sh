#!/bin/bash
#
# GK7206 Linux SDK Build Script
# Version: V100R005C00SPC030
#

set -e

SDK_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SDK_DIR}/build"

usage() {
    cat << EOF
========================================
  GK7206 Linux SDK Build Script
  Version: V100R005C00SPC030
========================================

Usage: ./build.sh <OPTIONS> [-j[N]]

Build Options:
    uboot              Build U-Boot bootloader
    kernel             Build Linux kernel
    rootfs             Build root filesystem
    driver             Build kernel drivers (GMP)
    sysdrv             Build uboot, kernel, rootfs
    osal               Build OSAL adaptation layer
    media              Build media libraries (GMP libs)
    app                Build application samples
    mcu                Build MCU firmware
    all                Build all components

Clean Options:
    clean              Clean all build outputs
    clean uboot        Clean U-Boot
    clean kernel       Clean Linux kernel
    clean driver       Clean kernel drivers
    clean rootfs       Clean root filesystem
    clean sysdrv       Clean uboot/kernel/rootfs
    clean media        Clean media libraries
    clean sample       Clean sample applications
    clean all          Clean everything

Info Options:
    check              Check build environment
    info               Show current build information
    list               List available board configurations
    env                Show environment setup
    lunch              Select board configuration
    help, --help, -h  Show this help message

Parallel Option:
    -j[N]              Allow N parallel jobs; omit N to use all cores

Examples:
    ./build.sh lunch               Select board
    ./build.sh all                 Build everything (single core)
    ./build.sh all -j              Build everything (all cores)
    ./build.sh kernel -j4          Build kernel with 4 jobs
    ./build.sh kernel              Build kernel (single core)
    ./build.sh clean kernel        Clean kernel
    ./build.sh info                Show build info

EOF
}

# source environment
source "${SDK_DIR}/build/env.sh"

# Default: no parallel (single core)
MAKE_J=""
JOBS_DESC="single core"

# Check if last argument is -j[N]
LAST_ARG="${!#}"
case "$LAST_ARG" in
    -j)
        MAKE_J="-j$(nproc)"
        JOBS_DESC="$(nproc) cores"
        set -- "${@:1:$#-1}"  # drop last arg
        ;;
    -j*)
        JOBS="${LAST_ARG#-j}"
        if ! [[ "$JOBS" =~ ^[0-9]+$ ]] || [ "$JOBS" -eq 0 ]; then
            echo "Error: invalid -j value '$JOBS'"
            exit 1
        fi
        MAKE_J="-j${JOBS}"
        JOBS_DESC="${JOBS} cores"
        set -- "${@:1:$#-1}"  # drop last arg
        ;;
esac

# check if argument provided
if [ $# -eq 0 ]; then
    usage
    exit 1
fi

# Parse command
CMD="$1"
shift 2>/dev/null || true

# Support "target clean" as alias for "clean target"
if [ "${1:-}" = "clean" ]; then
    CLEAN_TARGET="$CMD"
    CMD="clean"
    shift
fi

case "$CMD" in
    lunch)
        echo "========================================"
        echo "  Select Board Configuration"
        echo "========================================"

        # Step 1: Select board
        echo ""
        echo "Board:"
        BOARDS=()
        for cfg_dir in "${SDK_DIR}"/configs/*/; do
            [ -d "$cfg_dir" ] || continue
            BOARDS+=("$(basename "$cfg_dir")")
            echo "  ${#BOARDS[@]}. $(basename "$cfg_dir")"
        done
        if [ ${#BOARDS[@]} -eq 0 ]; then
            echo "Error: no board configurations found in configs/"
            exit 1
        fi
        echo ""
        read -p "Select board [1-${#BOARDS[@]}]: " choice
        if ! [[ "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -lt 1 ] || [ "$choice" -gt ${#BOARDS[@]} ]; then
            echo "Error: invalid selection '$choice'"
            exit 1
        fi
        SELECTED_BOARD="${BOARDS[$((choice-1))]}"
        echo "  -> $SELECTED_BOARD"

        # Step 2: Select kernel version + quickstart
        echo ""
        echo "Kernel:"
        KERNELS=()
        PREFIX="${SELECTED_BOARD}evb"
        for kv in linux-5.10 linux-4.9; do
            cfg="${SDK_DIR}/configs/${SELECTED_BOARD}/${PREFIX}_${kv}_cfg.mk"
            if [ -f "$cfg" ]; then
                KERNELS+=("${kv}")
                echo "  ${#KERNELS[@]}. ${kv}"
            fi
            cfg_qs="${SDK_DIR}/configs/${SELECTED_BOARD}/${PREFIX}_${kv}_quickstart_cfg.mk"
            if [ -f "$cfg_qs" ]; then
                KERNELS+=("${kv}-quickstart")
                echo "  ${#KERNELS[@]}. ${kv} (quickstart)"
            fi
        done
        if [ ${#KERNELS[@]} -eq 0 ]; then
            echo "Error: no matching configs for $SELECTED_BOARD"
            exit 1
        fi
        echo ""
        read -p "Select kernel [1-${#KERNELS[@]}]: " choice
        if ! [[ "$choice" =~ ^[0-9]+$ ]] || [ "$choice" -lt 1 ] || [ "$choice" -gt ${#KERNELS[@]} ]; then
            echo "Error: invalid selection '$choice'"
            exit 1
        fi
        SELECTED_KERNEL="${KERNELS[$((choice-1))]}"
        echo "  -> $SELECTED_KERNEL"

        # Build config file path
        if [[ "$SELECTED_KERNEL" == *"-quickstart" ]]; then
            kv="${SELECTED_KERNEL%-quickstart}"
            CFG_FILE="${SDK_DIR}/configs/${SELECTED_BOARD}/${PREFIX}_${kv}_quickstart_cfg.mk"
        else
            CFG_FILE="${SDK_DIR}/configs/${SELECTED_BOARD}/${PREFIX}_${SELECTED_KERNEL}_cfg.mk"
        fi

        if [ ! -f "$CFG_FILE" ]; then
            echo "Error: config file not found: $CFG_FILE"
            exit 1
        fi

        echo ""
        echo "========================================"
        echo "  Board:    $SELECTED_BOARD"
        echo "  Kernel:   $SELECTED_KERNEL"
        echo "========================================"
        cp "$CFG_FILE" "${SDK_DIR}/cfg.mk"
        echo "Configuration written to cfg.mk"
        ;;

    env)
        echo "========================================"
        echo "  Environment Setup"
        echo "========================================"
        echo ""
        echo "Toolchain paths:"
        echo "  ARM GCC (uclibc): ${SDK_DIR}/tools/linux/toolchains/arm-gcc12.2.0-linux-uclibceabi/bin"
        echo "  ARM GCC (glibc):   ${SDK_DIR}/tools/linux/toolchains/arm-gcc12.2.0-linux-gnueabi/bin"
        echo "  RISCV GCC:         ${SDK_DIR}/tools/linux/toolchains/riscv-gcc10.2.0-linux/bin"
        echo ""
        echo "PATH includes toolchain binaries"
        echo ""
        ;;

    uboot)
        echo "Building U-Boot... ($JOBS_DESC)"
        make -C "${SDK_DIR}" uboot ${MAKE_J}
        ;;

    kernel)
        echo "Building Linux kernel... ($JOBS_DESC)"
        make -C "${SDK_DIR}" linux ${MAKE_J}
        ;;

    rootfs)
        echo "Building root filesystem... ($JOBS_DESC)"
        make -C "${SDK_DIR}" rootfs ${MAKE_J}
        echo "Building filesystem images (jffs2/yaffs/ubifs/ext4)... ($JOBS_DESC)"
        make -C "${SDK_DIR}" fs_image ${MAKE_J}
        ;;

    driver)
        echo "Building kernel drivers (GMP)... ($JOBS_DESC)"
        make -C "${SDK_DIR}" gmp ${MAKE_J}
        ;;

    sysdrv)
        echo "Building system drivers (uboot + kernel + rootfs)... ($JOBS_DESC)"
        make -C "${SDK_DIR}" uboot ${MAKE_J}
        make -C "${SDK_DIR}" linux ${MAKE_J}
        make -C "${SDK_DIR}" rootfs ${MAKE_J}
        make -C "${SDK_DIR}" fs_image ${MAKE_J}
        ;;

    osal)
        echo "Building OSAL adaptation layer... ($JOBS_DESC)"
        if [ -d "${SDK_DIR}/source/gmp/drv/osal" ]; then
            make -C "${SDK_DIR}/source/gmp/drv/osal" ${MAKE_J}
        else
            echo "Error: source/gmp/drv/osal directory not found"
            exit 1
        fi
        ;;

    media)
        echo "Building media libraries... ($JOBS_DESC)"
        make -C "${SDK_DIR}" gmp ${MAKE_J}
        ;;

    app)
        echo "Building application samples... ($JOBS_DESC)"
        make -C "${SDK_DIR}" sample ${MAKE_J}
        ;;

    mcu)
        echo "Building MCU firmware... ($JOBS_DESC)"
        make -C "${SDK_DIR}" qs_mcu ${MAKE_J}
        ;;

    all)
        echo "Building all components... ($JOBS_DESC)"
        make -C "${SDK_DIR}" clean
        sleep 1
        make -C "${SDK_DIR}" build ${MAKE_J}
	source build/env.sh
        make fs_image -j40
        ;;

    clean)
        CLEAN_TARGET="${CLEAN_TARGET:-${1:-all}}"
        case "$CLEAN_TARGET" in
            uboot)
                echo "Cleaning U-Boot... ($JOBS_DESC)"
                make -C "${SDK_DIR}" uboot_clean ${MAKE_J}
                ;;
            kernel)
                echo "Cleaning Linux kernel... ($JOBS_DESC)"
                make -C "${SDK_DIR}" linux_clean ${MAKE_J}
                ;;
            rootfs)
                echo "Cleaning root filesystem... ($JOBS_DESC)"
                make -C "${SDK_DIR}" rootfs_clean ${MAKE_J}
                ;;
            driver)
                echo "Cleaning kernel drivers... ($JOBS_DESC)"
                make -C "${SDK_DIR}" gmp_clean ${MAKE_J}
                ;;
            sysdrv)
                echo "Cleaning uboot/kernel/rootfs... ($JOBS_DESC)"
                make -C "${SDK_DIR}" uboot_clean ${MAKE_J}
                make -C "${SDK_DIR}" linux_clean ${MAKE_J}
                make -C "${SDK_DIR}" rootfs_clean ${MAKE_J}
                ;;
            media)
                echo "Cleaning media libraries... ($JOBS_DESC)"
                make -C "${SDK_DIR}" gmp_clean ${MAKE_J}
                ;;
            sample)
                echo "Cleaning sample applications... ($JOBS_DESC)"
                make -C "${SDK_DIR}" sample_clean ${MAKE_J}
                ;;
            all)
                echo "Cleaning all build outputs... ($JOBS_DESC)"
                make -C "${SDK_DIR}" clean ${MAKE_J}
                ;;
            *)
                echo "Unknown clean target: $CLEAN_TARGET"
                echo "Valid targets: uboot, kernel, rootfs, driver, sysdrv, media, sample, all"
                exit 1
                ;;
        esac
        ;;

    check)
        echo "========================================"
        echo "  Checking Build Environment"
        echo "========================================"
        echo ""

        # Check bash
        echo -n "bash: "
        if [ "${BASH_VERSION:-}" ]; then
            echo "OK (${BASH_VERSION})"
        else
            echo "WARNING"
        fi

        # Check required commands
        echo "Checking required commands..."
        for cmd in make gcc arm-gcc12.2.0-linux-uclibceabi-gcc; do
            echo -n "  $cmd: "
            if command -v $cmd &> /dev/null; then
                echo "OK"
            else
                echo "NOT FOUND"
            fi
        done

        # Check toolchains
        echo ""
        echo "Checking toolchains..."
        for tc in arm-gcc12.2.0-linux-uclibceabi riscv-gcc10.2.0-linux; do
            TC_PATH="${SDK_DIR}/tools/linux/toolchains/${tc}/bin"
            echo -n "  ${tc}: "
            if [ -d "$TC_PATH" ]; then
                echo "OK ($TC_PATH)"
            else
                echo "NOT FOUND"
            fi
        done

        # Check fakeroot
        echo ""
        echo -n "fakeroot: "
        if command -v fakeroot &> /dev/null; then
            echo "OK"
        else
            echo "NOT FOUND (required for rootfs image generation)"
        fi

        echo ""
        echo "Environment check complete."
        ;;

    info)
        echo "========================================"
        echo "  Current Build Information"
        echo "========================================"
        echo ""

        # Source cfg.mk to get config
        CONFIG_FILE="${SDK_DIR}/cfg.mk"
        if [ -f "$CONFIG_FILE" ]; then
            source "$CONFIG_FILE"

            echo "Chip Configuration:"
            echo "  Chip Type:    ${CONFIG_XMEDIA_CHIP_TYPE:-unknown}"
            echo "  Chip Arch:    ${CONFIG_XMEDIA_CHIP_ARCH:-unknown}"
            echo ""
            echo "Kernel:"
            echo "  Version:      ${CONFIG_XMEDIA_KERNEL_VERSION:-unknown}"
            echo "  Defconfig:    ${CONFIG_XMEDIA_KERNEL_DEFCONFIG:-unknown}"
            echo "  SMP Support:  ${CONFIG_XMEDIA_KERNEL_SUPPORT_SMP:-n}"
            echo ""
            echo "Bootloader:"
            echo "  U-Boot Ver:   ${CONFIG_XMEDIA_UBOOT_VER:-unknown}"
            echo "  Defconfig:    ${CONFIG_XMEDIA_BOOT_DEFCONFIG:-unknown}"
            echo ""
            echo "Toolchain:"
            echo "  Userspace:    ${CONFIG_XMEDIA_USR_ARM_TOOLCHAINS:-unknown}"
            echo "  Kernel:       ${CONFIG_XMEDIA_KERNEL_ARM_TOOLCHAINS:-unknown}"
            echo ""
            echo "Flash Support:"
            echo "  NAND:         ${CONFIG_XMEDIA_NAND_FLASH_SUPPORT:-n}"
            echo "  SPI NAND:     ${CONFIG_XMEDIA_SPI_NAND_SUPPORT:-n}"
            echo "  SPI NOR:      ${CONFIG_XMEDIA_SPI_SUPPORT:-n}"
            echo "  eMMC:         ${CONFIG_XMEDIA_EMMC_SUPPORT:-n}"
            echo ""
            echo "Build Output:"
            echo "  Directory:    ${SDK_DIR}/out/${CONFIG_XMEDIA_CHIP_TYPE}"
            echo ""
        else
            echo "Error: Configuration file not found: $CONFIG_FILE"
            echo "Please run './build.sh lunch' first to configure the SDK"
            exit 1
        fi
        ;;

    list)
        echo "========================================"
        echo "  Available Board Configurations"
        echo "========================================"
        echo ""
        CURRENT_BOARD=""
        if [ -f "${SDK_DIR}/cfg.mk" ]; then
            CURRENT_BOARD=$(grep '^CONFIG_XMEDIA_CHIP_TYPE=' "${SDK_DIR}/cfg.mk" 2>/dev/null | cut -d'=' -f2)
        fi
        echo "Board configurations in configs/:"
        for cfg in "${SDK_DIR}"/configs/*; do
            if [ -d "$cfg" ]; then
                BOARD=$(basename "$cfg")
                if [ "$BOARD" = "$CURRENT_BOARD" ]; then
                    echo "  - $BOARD  (current)"
                else
                    echo "  - $BOARD"
                fi
            fi
        done
        echo ""
        CURRENT_KERNEL=""
        if [ -f "${SDK_DIR}/cfg.mk" ]; then
            CURRENT_KERNEL=$(grep '^CONFIG_XMEDIA_KERNEL_VERSION=' "${SDK_DIR}/cfg.mk" 2>/dev/null | cut -d'=' -f2)
        fi
        echo "Kernel versions supported:"
        for kv in linux-5.10.y linux-4.9.y; do
            if [ "$kv" = "$CURRENT_KERNEL" ]; then
                echo "  - $kv  (current)"
            else
                echo "  - $kv"
            fi
        done
        echo ""
        echo "To change board configuration, edit cfg.mk or run './build.sh lunch'"
        echo ""
        ;;

    help|--help|-h)
        usage
        ;;

    *)
        echo "Unknown command: $CMD"
        echo ""
        usage
        exit 1
        ;;
esac

echo ""
echo "Done."
