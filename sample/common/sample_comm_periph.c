#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/mman.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <sys/select.h>
#include <sys/socket.h>

#include "sample_comm.h"
#include "sample_comm_periph.h"
#include "xmedia_sys.h"


// 最大设备名长度
#define MAX_DEVICE_NAME_LENGTH 256

#define MAX_NL_BUF_SIZE (1024 * 16)
#define MAX_SELECT_TIMEOUT (2 * 1000 * 1000)

// 查找设备的分区（返回分区号，如 p1）
static xmedia_s32 sdcard_find_partition(const xmedia_char *device_name, xmedia_char *partition)
{
    DIR *dir;
    struct dirent *entry;
    xmedia_char path[MAX_DEVICE_NAME_LENGTH + 32] = {0};
    xmedia_s32 found = 0;

    snprintf(path, sizeof(path), "/sys/block/%s", device_name);

    dir = opendir(path);
    if (!dir) {
        return XMEDIA_FAILURE;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strncmp(entry->d_name, device_name, strlen(device_name)) == 0 &&
            strstr(entry->d_name, "p") != NULL) {
            strncpy(partition, entry->d_name, MAX_DEVICE_NAME_LENGTH);
            found = 1;
            break;
        }
    }
    closedir(dir);
    return found ? XMEDIA_SUCCESS : XMEDIA_FAILURE;
}

// 挂载设备
static xmedia_s32 mount_device(const xmedia_char *mount_point, const xmedia_char *device_path)
{
    const xmedia_char *fstypes[] = {"vfat", "ext4", "ntfs", "auto", NULL};

    // 创建挂载点目录（若不存在）
    if (mkdir(mount_point, 0755) && errno != EEXIST) {
        SAMPLE_ERR("mkdir failed: %s\n", strerror(errno));
        return XMEDIA_FAILURE;
    }

    // 尝试挂载常见文件系统类型（按优先级）
    for (xmedia_s32 i = 0; fstypes[i]; i++) {
        if (mount(device_path, mount_point, fstypes[i], MS_NOATIME, NULL) == 0) {
            SAMPLE_PRT("Mounted %s (%s) to %s\n", device_path, fstypes[i], mount_point);
            return XMEDIA_SUCCESS;
        }
    }

    // 所有类型尝试失败
    SAMPLE_ERR("Failed to mount %s: %s\n", device_path, strerror(errno));
    return XMEDIA_FAILURE;
}

static xmedia_bool device_driver_is_bind(const xmedia_char *device, const xmedia_char *driver)
{
    xmedia_char path[256] = {'\0'};
    snprintf(path, 256, "%s/%s", driver, device);
    return (access(path, F_OK) == 0);
}

static xmedia_s32 device_attach_driver(const xmedia_char *device, const xmedia_char *driver)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_char path[256] = {'\0'};
    xmedia_s32 fd = -1;

    snprintf(path, sizeof(path), "%s/bind", driver);
    fd = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        SAMPLE_ERR("[%s()] can't open %s because %s\n", __func__, driver, strerror(errno));
        return XMEDIA_FAILURE;
    }
    //MEDIA_PRT("[%s()] start bind %s to %s\n", __func__, device, driver);
    s32Ret = write(fd, device, strlen(device));
    if (s32Ret < 0) {
        SAMPLE_ERR("[%s()] bind %s to %s failed because %s\n", __func__, device, driver,strerror(errno));
        close(fd);
        return XMEDIA_FAILURE;
    }
    close(fd);
    return XMEDIA_SUCCESS;
}

static xmedia_s32 device_detach_driver(const xmedia_char *device, const xmedia_char *driver)
{
    xmedia_s32 s32Ret = XMEDIA_FAILURE;
    xmedia_char path[256] = {'\0'};
    xmedia_s32 fd = -1;
    snprintf(path, sizeof(path), "%s/unbind", driver);
    fd = open(path, O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        SAMPLE_ERR("[%s()] can't open %s because %s\n", __func__, path, strerror(errno));
        return XMEDIA_FAILURE;
    }
    //MEDIA_PRT("[%s()] start unbind %s from %s\n", __func__, device, driver);
    s32Ret = write(fd, device, strlen(device));
    if (s32Ret < 0) {
        SAMPLE_ERR("[%s()] unbind %s from %s failed because %s\n", __func__, device, driver, strerror(errno));
        close(fd);
        return XMEDIA_FAILURE;
    }
    close(fd);
    return XMEDIA_SUCCESS;
}

xmedia_s32 sample_comm_periph_mount_sdcard(const xmedia_char* mount_path, xmedia_s32 timeout_ms)
{
    DIR *block_dir;
    struct dirent *entry;
    xmedia_u64 start_time = 0;
    xmedia_u64 end_time = 0;
    xmedia_char partition[MAX_DEVICE_NAME_LENGTH] = {0};
    xmedia_char device_path[MAX_DEVICE_NAME_LENGTH + 32] = {0};

    xmedia_sys_get_cur_pts(&start_time);

LOOP_AGAIN:
    block_dir = opendir("/sys/block");
    if (!block_dir) {
        SAMPLE_ERR("Failed to open /sys/block: %s\n", strerror(errno));
        return XMEDIA_FAILURE;
    }

    while ((entry = readdir(block_dir)) != NULL) {
        // 只处理 mmcblk 设备
        if (strncmp(entry->d_name, "mmcblk", 6) != 0) {
            continue;
        }

        // 查找分区
        if (sdcard_find_partition(entry->d_name, partition) == 0) {
            // 挂载分区（如 /dev/mmcblk0p1）
            snprintf(device_path, sizeof(device_path), "/dev/%s", partition);
            if (mount_device(mount_path, device_path) == XMEDIA_SUCCESS) {
                closedir(block_dir);
                return XMEDIA_SUCCESS;
            }
        } else {
            // 无分区，挂载整个设备（如 /dev/mmcblk1）
            snprintf(device_path, sizeof(device_path), "/dev/%s", entry->d_name);
            if (mount_device(mount_path, device_path) == XMEDIA_SUCCESS) {
                closedir(block_dir);
                return XMEDIA_SUCCESS;
            }
        }
    }

    closedir(block_dir);

    xmedia_sys_get_cur_pts(&end_time);
    if(end_time - start_time < timeout_ms * 1000) {
        usleep(50 * 1000);
        SAMPLE_PRT("No SD card found, find again\n");
        goto LOOP_AGAIN;
    }

    SAMPLE_ERR("No SD card found.\n");
    return XMEDIA_FAILURE;
}

xmedia_s32 sample_comm_periph_umount_sdcard(const xmedia_char *mount_path)
{
    const xmedia_s32 max_retries = 30;
    xmedia_s32 retry_count = 0;

    while (retry_count < max_retries) {
        if (umount(mount_path) == 0) {
            return XMEDIA_SUCCESS;
        }

        // 错误处理
        switch (errno) {
            case EBUSY:
                SAMPLE_ERR("ERROR: %s is busy (retry %d/%d)\n",
                        mount_path, retry_count + 1, max_retries);
                break;
            case EINVAL:
                SAMPLE_ERR("Error: %s not mounted or invalid path\n", mount_path);
                return XMEDIA_FAILURE;
            case EPERM:
                SAMPLE_ERR("Error: Requires root privileges\n");
                return XMEDIA_FAILURE;
            default:
                SAMPLE_ERR("Unmount failed: %s\n", strerror(errno));
                return XMEDIA_FAILURE;
        }

        retry_count++;
        usleep(10 * 1000);
    }

    SAMPLE_ERR("Error: Failed to unmount %s after %d attempts\n", mount_path, max_retries);
    return XMEDIA_FAILURE;
}

xmedia_s32 sample_comm_periph_drv_bind(const xmedia_char *device, const xmedia_char *driver, const xmedia_char *notice_str)
{
    xmedia_s32 s32Ret = 0;
    xmedia_s32 fd = -1;
    xmedia_char buf[MAX_NL_BUF_SIZE] = {'\0'};
    fd_set read_set;
    struct timeval timeout;
    struct sockaddr_nl addr;

    if (device_driver_is_bind(device, driver)) {
        SAMPLE_ERR("[%s()] sdcard device already bind!\n", __func__);
        return XMEDIA_SUCCESS;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = NETLINK_KOBJECT_UEVENT;
    addr.nl_pid = 0;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (fd < 0) {
        SAMPLE_ERR("[%s()] Failed to open sdcard netlink because %s\n", __func__, strerror(errno));
        return XMEDIA_FAILURE;
    } else if (bind(fd, (struct sockaddr *)(&addr), sizeof(addr)) != 0) {
        SAMPLE_ERR("[%s()] bind sdcard netlink addr failed because %s\n", __func__, strerror(errno));
        goto FAILED;
    }

    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    timeout.tv_sec = 0;
    timeout.tv_usec = MAX_SELECT_TIMEOUT;

    // bind sdcard
    if (device_attach_driver(device, driver) != XMEDIA_SUCCESS) {
        goto FAILED;
    }

    // wait for bind success
RETRY:
    s32Ret = select(fd + 1, &read_set, NULL, NULL, &timeout);
    if (s32Ret > 0) {
        memset(&buf, 0, sizeof(buf));
        read(fd, buf, sizeof(buf));
        buf[MAX_NL_BUF_SIZE - 1] = '\0';
        if (strncmp(buf, notice_str, strlen(notice_str)) == 0) {
            //SAMPLE_ERR("[%s()] Bind success: %s\n", __func__, buf);
            goto SUCCESS;
        }
        goto RETRY; // drop all message
    } else {
        SAMPLE_ERR("[%s()] select error %s\n", __func__, strerror(errno));
        goto FAILED;
    }
SUCCESS:
    close(fd);
    return XMEDIA_SUCCESS;
FAILED:
    close(fd);
    return XMEDIA_FAILURE;
}

xmedia_s32 sample_comm_periph_drv_unbind(const xmedia_char *device, const xmedia_char *driver, const xmedia_char *notice_str)
{
    xmedia_s32 s32Ret = 0;
    xmedia_s32 fd = -1;
    xmedia_char buf[MAX_NL_BUF_SIZE] = {'\0'};
    fd_set read_set;
    struct timeval timeout;
    struct sockaddr_nl addr;

    if (!device_driver_is_bind(device, driver)) {
        SAMPLE_ERR("[%s()] sdcard device already unbind!\n", __func__);
        return XMEDIA_SUCCESS;
    }

    memset(&addr, 0, sizeof(addr));
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = NETLINK_KOBJECT_UEVENT;
    addr.nl_pid = 0;

    fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_KOBJECT_UEVENT);
    if (fd < 0) {
        SAMPLE_ERR("[%s()] Failed to open sdcard netlink because %s\n", __func__, strerror(errno));
        return XMEDIA_FAILURE;
    } else if (bind(fd, (struct sockaddr *)(&addr), sizeof(addr)) != 0) {
        SAMPLE_ERR("[%s()] bind sdcard netlink addr failed because %s\n", __func__, strerror(errno));
        goto FAILED;
    }

    memset(&buf, 0, sizeof(buf));
    FD_ZERO(&read_set);
    FD_SET(fd, &read_set);
    timeout.tv_sec = 0;
    timeout.tv_usec = MAX_SELECT_TIMEOUT;

    // unbind sdcard
    if (device_detach_driver(device, driver) != XMEDIA_SUCCESS)
        goto FAILED;
    // wait for unbind success
RETRY:
    s32Ret = select(fd + 1, &read_set, NULL, NULL, &timeout);
    if (s32Ret > 0) {
        memset(&buf, 0, sizeof(buf));
        read(fd, buf, sizeof(buf));
        buf[MAX_NL_BUF_SIZE - 1] = '\0';
        if (strcmp(buf, notice_str) == 0) {
            //SAMPLE_ERR("[%s()] Unbind success: %s\n", __func__, buf);
            goto SUCCESS;
        }
        goto RETRY; // drop all message
    } else {
        SAMPLE_ERR("[%s()] select error %s\n", __func__, strerror(errno));
        goto FAILED;
    }
SUCCESS:
    close(fd);
    return XMEDIA_SUCCESS;
FAILED:
    close(fd);
    return XMEDIA_FAILURE;
}
