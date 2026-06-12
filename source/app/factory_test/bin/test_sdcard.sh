#!/bin/sh

START_SEC=$(date +%s 2>/dev/null)
if [ -z "$START_SEC" ]; then
    START_SEC=0
fi

MOUNT_DIR="/tmp/factory_sdcard"
TEST_FILE="$MOUNT_DIR/factory_sdcard_test.txt"
MOUNT_LOG="/tmp/factory_sdcard_mount.log"
RW_LOG="/tmp/factory_sdcard_rw.log"

status="FAIL"
message="SD card device not found"
mounted_by_script=0

: > "$MOUNT_LOG"
: > "$RW_LOG"

json_escape() {
    printf '%s' "$1" | tr '\n\r\t\"' '    '
}

is_mounted_on_dir() {
    mount | grep -q " $MOUNT_DIR "
}

find_sd_block() {
    for d in /sys/block/mmcblk*; do
        [ -d "$d" ] || continue
        type=$(cat "$d/device/type" 2>/dev/null)
        if [ "$type" = "SD" ]; then
            echo "$(basename "$d")"
            return 0
        fi
    done

    for dev in /dev/mmcblk1 /dev/mmcblk0; do
        [ -b "$dev" ] || continue
        basename "$dev"
        return 0
    done

    return 1
}

find_mount_dev() {
    block="$1"

    # 优先挂载分区。不能直接优先挂载 /dev/mmcblk1，因为整盘设备通常没有文件系统，
    # 会报 Invalid argument；真正可挂载的通常是 /dev/mmcblk1p1。
    for part in /dev/${block}p1 /dev/${block}p2 /dev/${block}p3 /dev/${block}p4; do
        if [ -b "$part" ]; then
            echo "$part"
            return 0
        fi
    done

    # 只有在没有任何分区节点时，才尝试整盘设备。
    if [ -b "/dev/$block" ]; then
        echo "/dev/$block"
        return 0
    fi

    return 1
}

try_mount_dev() {
    dev="$1"

    mount "$dev" "$MOUNT_DIR" >>"$MOUNT_LOG" 2>&1 && return 0
    mount -t vfat "$dev" "$MOUNT_DIR" >>"$MOUNT_LOG" 2>&1 && return 0
    mount -t ext4 "$dev" "$MOUNT_DIR" >>"$MOUNT_LOG" 2>&1 && return 0
    mount -t ext3 "$dev" "$MOUNT_DIR" >>"$MOUNT_LOG" 2>&1 && return 0
    mount -t ext2 "$dev" "$MOUNT_DIR" >>"$MOUNT_LOG" 2>&1 && return 0

    return 1
}

SD_BLOCK=$(find_sd_block)
if [ -n "$SD_BLOCK" ]; then
    SD_DEV=$(find_mount_dev "$SD_BLOCK")
    if [ -n "$SD_DEV" ]; then
        mkdir -p "$MOUNT_DIR"

        if is_mounted_on_dir; then
            mounted_by_script=0
        else
            if try_mount_dev "$SD_DEV"; then
                mounted_by_script=1
            else
                message="mount $SD_DEV failed: $(cat "$MOUNT_LOG" 2>/dev/null)"
            fi
        fi

        if is_mounted_on_dir; then
            echo "factory sdcard test $(date)" > "$TEST_FILE" 2>"$RW_LOG"
            sync
            if [ -f "$TEST_FILE" ] && grep -q "factory sdcard test" "$TEST_FILE" 2>/dev/null; then
                rm -f "$TEST_FILE"
                sync
                status="PASS"
                message="SD read/write ok: $SD_DEV"
            else
                message="SD read/write failed: $(cat "$RW_LOG" 2>/dev/null)"
            fi
        fi

        if [ "$mounted_by_script" = "1" ]; then
            umount "$MOUNT_DIR" >/dev/null 2>&1
        fi
    else
        message="SD block found but no mountable device: $SD_BLOCK"
    fi
fi

END_SEC=$(date +%s 2>/dev/null)
if [ -z "$END_SEC" ]; then
    END_SEC=$START_SEC
fi
DURATION_MS=$(((END_SEC - START_SEC) * 1000))
if [ $DURATION_MS -lt 0 ]; then
    DURATION_MS=0
fi

message_json=$(json_escape "$message")

printf '{\n'
printf '  "item": "sdcard",\n'
printf '  "name": "SD 卡读写",\n'
printf '  "status": "%s",\n' "$status"
printf '  "message": "%s",\n' "$message_json"
printf '  "duration_ms": %s\n' "$DURATION_MS"
printf '}\n'

[ "$status" = "PASS" ]
