#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <time.h>

#include "web_server.h"

static volatile int g_stop = 0;

void detector_get_status(int *motion_count, int *motion_threshold, unsigned long *last_frame_size,
    time_t *last_frame_time, time_t *last_motion_time);
void detector_set_motion_threshold(int threshold);
void detector_record_browser_motion(void);
void detector_reset_motion_count(void);
void rpi_detector_mjpeg_request_stop(void);
void rpi_detector_mjpeg_shutdown(void);
void rpi_detector_mjpeg_send_stream(int client_fd);
void rpi_detector_mjpeg_send_snapshot(int client_fd);

static int enable_sensor_gpio(void)
{
    int ret;

    ret = system("xmmm 0x100C0044 0x00001000");
    if (ret != 0) {
        printf("[init] xmmm sensor power config failed: %d\n", ret);
        return -1;
    }

    ret = system("sh -c 'if [ ! -e /sys/class/gpio/gpio46 ]; then echo 46 > /sys/class/gpio/export; fi'");
    if (ret != 0) {
        printf("[init] gpio46 export failed: %d\n", ret);
        return -1;
    }

    ret = system("sh -c 'echo out > /sys/class/gpio/gpio46/direction'");
    if (ret != 0) {
        printf("[init] gpio46 direction failed: %d\n", ret);
        return -1;
    }

    ret = system("sh -c 'echo 1 > /sys/class/gpio/gpio46/value'");
    if (ret != 0) {
        printf("[init] gpio46 value failed: %d\n", ret);
        return -1;
    }

    return 0;
}

static void sig_handler(int sig)
{
    (void)sig;
    g_stop = 1;
    web_server_stop();
    rpi_detector_mjpeg_request_stop();
}

static void format_time_json(time_t ts, char *buf, size_t size)
{
    struct tm tm_val;

    if (ts <= 0 || localtime_r(&ts, &tm_val) == NULL) {
        if (size > 0) {
            buf[0] = '\0';
        }
        return;
    }

    strftime(buf, size, "%Y-%m-%d %H:%M:%S", &tm_val);
}

static int rpi_detector_route_get(int client_fd, const char *path)
{
    char json_buf[768];
    int motion_count = 0;
    int motion_threshold = 20;
    unsigned long last_frame_size = 0;
    time_t last_frame_time = 0;
    time_t last_motion_time = 0;
    char last_frame_buf[32];
    char last_motion_buf[32];

    detector_get_status(&motion_count, &motion_threshold, &last_frame_size, &last_frame_time, &last_motion_time);
    format_time_json(last_frame_time, last_frame_buf, sizeof(last_frame_buf));
    format_time_json(last_motion_time, last_motion_buf, sizeof(last_motion_buf));

    if (strcmp(path, "/api/status") == 0) {
        snprintf(json_buf, sizeof(json_buf),
            "{\n"
            "  \"running\": true,\n"
            "  \"fps\": 30,\n"
            "  \"motion_threshold\": %d,\n"
            "  \"motion_count\": %d,\n"
            "  \"last_motion_time\": \"%s\",\n"
            "  \"last_frame_time\": \"%s\",\n"
            "  \"stream_url\": \"/mjpeg\",\n"
            "  \"has_frame\": %s,\n"
            "  \"frame_size\": %lu,\n"
            "  \"frame_mtime\": %ld\n"
            "}\n",
            motion_threshold, motion_count, last_motion_buf, last_frame_buf,
            last_frame_time > 0 ? "true" : "false", last_frame_size, (long)last_frame_time);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strncmp(path, "/api/motion/threshold", 21) == 0) {
        const char *val = strstr(path, "val=");
        if (val != NULL) {
            detector_set_motion_threshold(atoi(val + 4));
            detector_get_status(&motion_count, &motion_threshold, &last_frame_size, &last_frame_time, &last_motion_time);
        }
        snprintf(json_buf, sizeof(json_buf), "{\"threshold\":%d,\"status\":\"ok\"}\n", motion_threshold);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strncmp(path, "/api/motion/trigger", 19) == 0) {
        detector_record_browser_motion();
        detector_get_status(&motion_count, &motion_threshold, &last_frame_size, &last_frame_time, &last_motion_time);
        format_time_json(last_motion_time, last_motion_buf, sizeof(last_motion_buf));
        snprintf(json_buf, sizeof(json_buf),
            "{\"motion_count\":%d,\"last_motion_time\":\"%s\",\"status\":\"ok\"}\n",
            motion_count, last_motion_buf);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strcmp(path, "/api/motion/reset") == 0) {
        detector_reset_motion_count();
        detector_get_status(&motion_count, &motion_threshold, &last_frame_size, &last_frame_time, &last_motion_time);
        snprintf(json_buf, sizeof(json_buf), "{\"motion_count\":%d,\"status\":\"ok\"}\n", motion_count);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strcmp(path, "/api/alerts") == 0) {
        snprintf(json_buf, sizeof(json_buf),
            "{\n"
            "  \"total\": %d,\n"
            "  \"last_motion_time\": \"%s\",\n"
            "  \"alerts\": []\n"
            "}\n", motion_count, last_motion_buf);
        web_send_json_ok(client_fd, json_buf);
        return 0;
    }

    if (strcmp(path, "/api/snapshot") == 0 || strcmp(path, "/snapshot.jpg") == 0 || strcmp(path, "/api/frame.jpg") == 0) {
        rpi_detector_mjpeg_send_snapshot(client_fd);
        return 0;
    }

    return -1;
}

int main(int argc, char *argv[])
{
    int initial_threshold = 20;

    signal(SIGINT, sig_handler);
    signal(SIGTERM, sig_handler);

    printf("=== rpi-detector: Motion Detector for GK7206 ===\n");
    printf("Web UI available at http://<board-ip>/\n");

    if (argc > 1) {
        initial_threshold = atoi(argv[1]);
    }
    detector_set_motion_threshold(initial_threshold);
    printf("Motion threshold: %d\n", initial_threshold);

    if (enable_sensor_gpio() != 0) {
        printf("Sensor GPIO init failed\n");
        return EXIT_FAILURE;
    }

    web_server_set_mjpeg_handler(rpi_detector_mjpeg_send_stream, rpi_detector_mjpeg_request_stop);

    printf("Starting web server... Camera pipeline will start when browser opens /mjpeg.\n");
    web_server_run(rpi_detector_route_get);

    rpi_detector_mjpeg_shutdown();
    printf("Exiting rpi-detector.\n");
    (void)g_stop;
    return EXIT_SUCCESS;
}
