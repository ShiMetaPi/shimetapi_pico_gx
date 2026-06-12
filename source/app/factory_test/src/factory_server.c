#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>

#define LISTEN_PORT 80
#define BACKLOG 8
#define REQ_BUF_SIZE 2048   
#define SCRIPT_OUTPUT_SIZE 4096
#define WWW_ROOT "/www" // 静态网页根目录
#define FACTORY_BIN_DIR "/usr/factory/bin" // 测试脚本目录
#define RESULT_FILE "/etc/factory_test_result.json"
#define RESULT_FILE_TMP "/tmp/factory_test_result.json"
#define FIRST_BOOT_FLAG_FILE "/etc/.factory_server_first_boot_done"
#define MAX_RESULT_ITEMS 16

void factory_mjpeg_send_stream(int client_fd);
void factory_mjpeg_request_stop(void);
void factory_mjpeg_shutdown(void);

static volatile sig_atomic_t g_running = 1;
static pthread_mutex_t g_result_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    char item[32];
    char name[64];
    char status[16];
    char message[256];
    int duration_ms;
    int in_use;
} factory_result_item;

static factory_result_item g_result_items[MAX_RESULT_ITEMS];
static int g_test_finished = 0;

struct api_route {
    const char *path;
    const char *script;
};

// 脚本映射
static const struct api_route g_api_routes[] = {
    {"/api/test/rtc", FACTORY_BIN_DIR "/test_rtc.sh"},
    {"/api/test/led", FACTORY_BIN_DIR "/test_led.sh"},
    {"/api/test/sdcard", FACTORY_BIN_DIR "/test_sdcard.sh"},
    {"/api/test/spi_flash", FACTORY_BIN_DIR "/test_spi_flash.sh"},
    {"/api/test/sensor", FACTORY_BIN_DIR "/test_sensor.sh"},
    {"/api/test/mjpeg", FACTORY_BIN_DIR "/test_mjpeg.sh"},
};

static int enable_sensor(void)
{
    int ret;

    ret = system("xmmm 0x100C0044 0x00001000");
    if (ret != 0) {
        perror("xmmm 0x100C0044 0x00001000");
        return -1;
    }

    ret = system("sh -c 'if [ ! -e /sys/class/gpio/gpio46 ]; then echo 46 > /sys/class/gpio/export; fi'");
    if (ret != 0) {
        perror("echo 46 > /sys/class/gpio/export");
        return -1;
    }

    ret = system("sh -c 'echo out > /sys/class/gpio/gpio46/direction'");
    if (ret != 0) {
        perror("echo out > /sys/class/gpio/gpio46/direction");
        return -1;
    }

    ret = system("sh -c 'echo 1 > /sys/class/gpio/gpio46/value'");
    if (ret != 0) {
        perror("echo 1 > /sys/class/gpio/gpio46/value");
        return -1;
    }

    return 0;
}

static void handle_signal(int signal_number)
{
    (void)signal_number;
    g_running = 0;
}


// 创建服务器socket并监听80端口
static int create_server_socket(void)
{
    int server_fd;// 服务器socket文件描述符
    int reuse = 1;// 重用socket地址
    struct sockaddr_in address;// 服务器地址

    server_fd = socket(AF_INET, SOCK_STREAM, 0);// 创建服务器socket
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    // 设置socket地址重用,允许在TIME_WAIT状态下重用socket地址
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    // 初始化服务器地址,告诉操作系统：凡是到达这台机器的某个 IP + 端口的连接请求，都交给我这个程序
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);//本机所有网卡的IP地址
    address.sin_port = htons(LISTEN_PORT);//监听端口

    // 将socket绑定到服务器地址和端口
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    // 开始监听,允许最多BACKLOG个连接请求排队
    if (listen(server_fd, BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}


static void send_all(int client_fd, const char *data, size_t len)
{
    size_t sent_len = 0;

    while (sent_len < len) {
        ssize_t ret = send(client_fd, data + sent_len, len - sent_len, 0);
        if (ret <= 0) {
            return;
        }
        sent_len += (size_t)ret;
    }
}

static void send_header(int client_fd, const char *status, const char *content_type, size_t content_length)
{
    char header[256];
    int header_len;

    header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, content_type, content_length);

    if (header_len > 0) {
        send_all(client_fd, header, (size_t)header_len);
    }
}

static void send_response(int client_fd, const char *status, const char *content_type, const char *body)
{
    size_t body_len = strlen(body);

    send_header(client_fd, status, content_type, body_len);
    send_all(client_fd, body, body_len);
}

static const char *get_content_type(const char *path)
{
    const char *ext = strrchr(path, '.');

    if (ext == NULL) {
        return "application/octet-stream";
    }
    if (strcmp(ext, ".html") == 0) {
        return "text/html; charset=utf-8";
    }
    if (strcmp(ext, ".css") == 0) {
        return "text/css; charset=utf-8";
    }
    if (strcmp(ext, ".js") == 0) {
        return "application/javascript; charset=utf-8";
    }
    if (strcmp(ext, ".json") == 0) {
        return "application/json; charset=utf-8";
    }
    if (strcmp(ext, ".txt") == 0) {
        return "text/plain; charset=utf-8";
    }

    return "application/octet-stream";
}

static int is_safe_static_path(const char *path)
{
    if (strstr(path, "..") != NULL) {
        return 0;
    }
    return 1;
}

static void send_file(int client_fd, const char *request_path)
{
    FILE *fp;
    char file_path[512];
    char buffer[1024];
    struct stat st;
    size_t read_len;

    if (!is_safe_static_path(request_path)) {
        send_response(client_fd, "403 Forbidden", "text/plain; charset=utf-8", "403 Forbidden\n");
        return;
    }

    if (strcmp(request_path, "/") == 0) {
        request_path = "/index.html";
    }

    (void)snprintf(file_path, sizeof(file_path), "%s%s", WWW_ROOT, request_path);

    if (stat(file_path, &st) < 0 || !S_ISREG(st.st_mode)) {
        send_response(client_fd, "404 Not Found", "text/plain; charset=utf-8", "404 Not Found\n");
        return;
    }

    fp = fopen(file_path, "rb");
    if (fp == NULL) {
        send_response(client_fd, "500 Internal Server Error", "text/plain; charset=utf-8", "open static file failed\n");
        return;
    }

    send_header(client_fd, "200 OK", get_content_type(file_path), (size_t)st.st_size);
    while ((read_len = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        send_all(client_fd, buffer, read_len);
    }

    (void)fclose(fp);
}

static void get_current_time_string(char *buf, size_t buf_size)
{
    time_t now;
    struct tm tm_now;

    now = time(NULL);
    if (localtime_r(&now, &tm_now) == NULL) {
        snprintf(buf, buf_size, "unknown");
        return;
    }

    (void)strftime(buf, buf_size, "%Y-%m-%d %H:%M:%S", &tm_now);
}

static void send_info(int client_fd)
{
    char time_buf[32];
    char body[512];

    get_current_time_string(time_buf, sizeof(time_buf));
    (void)snprintf(body, sizeof(body),
        "{\n"
        "  \"board\": \"GK7206\",\n"
        "  \"app\": \"factory_server\",\n"
        "  \"version\": \"0.4.0\",\n"
        "  \"web_root\": \"%s\",\n"
        "  \"script_dir\": \"%s\",\n"
        "  \"result_file\": \"%s\",\n"
        "  \"server_time\": \"%s\",\n"
        "  \"features\": [\"mjpeg\", \"autostart\", \"end_test\"]\n"
        "}\n",
        WWW_ROOT, FACTORY_BIN_DIR, RESULT_FILE, time_buf);

    send_response(client_fd, "200 OK", "application/json; charset=utf-8", body);
}

static int run_script_capture(const char *script_path, char *output, size_t output_size)
{
    FILE *fp;
    char command[512];
    size_t used = 0;
    int status;

    if (access(script_path, X_OK) != 0) {
        (void)snprintf(output, output_size,
            "{\n"
            "  \"item\": \"unknown\",\n"
            "  \"name\": \"Unknown\",\n"
            "  \"status\": \"FAIL\",\n"
            "  \"message\": \"script not executable: %s\",\n"
            "  \"duration_ms\": 0\n"
            "}\n",
            script_path);
        return -1;
    }

    (void)snprintf(command, sizeof(command), "%s 2>&1", script_path);
    fp = popen(command, "r");
    if (fp == NULL) {
        (void)snprintf(output, output_size,
            "{\"item\":\"unknown\",\"name\":\"Unknown\",\"status\":\"FAIL\",\"message\":\"popen failed\",\"duration_ms\":0}\n");
        return -1;
    }

    while (used + 1 < output_size) {
        size_t ret = fread(output + used, 1, output_size - used - 1, fp);
        if (ret == 0) {
            break;
        }
        used += ret;
    }
    output[used] = '\0';

    status = pclose(fp);
    if (used == 0) {
        (void)snprintf(output, output_size,
            "{\"item\":\"unknown\",\"name\":\"Unknown\",\"status\":\"FAIL\",\"message\":\"empty script output\",\"duration_ms\":0}\n");
        return -1;
    }

    if (status == -1) {
        return -1;
    }

    if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
    }

    return -1;
}

static const char *find_script_by_path(const char *path)
{
    size_t i;

    for (i = 0; i < sizeof(g_api_routes) / sizeof(g_api_routes[0]); ++i) {
        if (strcmp(path, g_api_routes[i].path) == 0) {
            return g_api_routes[i].script;
        }
    }

    return NULL;
}

static void stop_mjpeg_stream(void)
{
    factory_mjpeg_request_stop();
    factory_mjpeg_shutdown();
}

static void json_escape_copy(char *dst, size_t dst_size, const char *src)
{
    size_t di = 0;
    size_t si = 0;

    if (dst_size == 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    while (src[si] != '\0' && di + 1 < dst_size) {
        unsigned char ch = (unsigned char)src[si++];

        if (ch == '\\' || ch == '"') {
            if (di + 2 >= dst_size) {
                break;
            }
            dst[di++] = '\\';
            dst[di++] = (char)ch;
        } else if (ch == '\n' || ch == '\r') {
            if (di + 2 >= dst_size) {
                break;
            }
            dst[di++] = '\\';
            dst[di++] = 'n';
        } else if (ch == '\t') {
            if (di + 2 >= dst_size) {
                break;
            }
            dst[di++] = '\\';
            dst[di++] = 't';
        } else if (ch < 0x20) {
            continue;
        } else {
            dst[di++] = (char)ch;
        }
    }

    dst[di] = '\0';
}

static void trim_json_string_value(char *dst, size_t dst_size, const char *src)
{
    size_t len;

    if (dst_size == 0) {
        return;
    }

    if (src == NULL) {
        dst[0] = '\0';
        return;
    }

    while (*src == ' ' || *src == '\t' || *src == '\n' || *src == '\r') {
        ++src;
    }

    len = strlen(src);
    while (len > 0 && (src[len - 1] == ' ' || src[len - 1] == '\t' || src[len - 1] == '\n' || src[len - 1] == '\r')) {
        --len;
    }

    if (len >= dst_size) {
        len = dst_size - 1;
    }

    memcpy(dst, src, len);
    dst[len] = '\0';
}

static int extract_json_string_field(const char *json, const char *key, char *out, size_t out_size)
{
    char pattern[64];
    const char *pos;
    const char *start;
    const char *end;
    size_t len;

    if (json == NULL || key == NULL || out == NULL || out_size == 0) {
        return 0;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    pos = strstr(json, pattern);
    if (pos == NULL) {
        return 0;
    }

    pos = strchr(pos + strlen(pattern), ':');
    if (pos == NULL) {
        return 0;
    }

    start = strchr(pos, '"');
    if (start == NULL) {
        return 0;
    }
    ++start;

    end = start;
    while (*end != '\0') {
        if (*end == '"' && *(end - 1) != '\\') {
            break;
        }
        ++end;
    }
    if (*end != '"') {
        return 0;
    }

    len = (size_t)(end - start);
    if (len >= out_size) {
        len = out_size - 1;
    }
    memcpy(out, start, len);
    out[len] = '\0';
    return 1;
}

static int extract_json_int_field(const char *json, const char *key, int default_value)
{
    char pattern[64];
    const char *pos;
    char *endptr;
    long value;

    if (json == NULL || key == NULL) {
        return default_value;
    }

    snprintf(pattern, sizeof(pattern), "\"%s\"", key);
    pos = strstr(json, pattern);
    if (pos == NULL) {
        return default_value;
    }

    pos = strchr(pos + strlen(pattern), ':');
    if (pos == NULL) {
        return default_value;
    }

    ++pos;
    while (*pos == ' ' || *pos == '\t' || *pos == '\n' || *pos == '\r') {
        ++pos;
    }

    value = strtol(pos, &endptr, 10);
    if (endptr == pos) {
        return default_value;
    }

    return (int)value;
}

static int find_result_slot(const char *item)
{
    int i;
    int free_slot = -1;

    for (i = 0; i < MAX_RESULT_ITEMS; ++i) {
        if (g_result_items[i].in_use) {
            if (strcmp(g_result_items[i].item, item) == 0) {
                return i;
            }
        } else if (free_slot < 0) {
            free_slot = i;
        }
    }

    return free_slot;
}

static void persist_result_file_to_path_locked(const char *path)
{
    FILE *fp;
    int i;
    int first = 1;

    fp = fopen(path, "w");
    if (fp == NULL) {
        return;
    }

    fputs("{\n  \"test_finished\": ", fp);
    fputs(g_test_finished ? "true" : "false", fp);
    fputs(",\n  \"results\": [\n", fp);

    for (i = 0; i < MAX_RESULT_ITEMS; ++i) {
        char esc_name[128];
        char esc_status[32];
        char esc_message[512];

        if (!g_result_items[i].in_use) {
            continue;
        }

        json_escape_copy(esc_name, sizeof(esc_name), g_result_items[i].name);
        json_escape_copy(esc_status, sizeof(esc_status), g_result_items[i].status);
        json_escape_copy(esc_message, sizeof(esc_message), g_result_items[i].message);

        if (!first) {
            fputs(",\n", fp);
        }
        first = 0;

        fprintf(fp,
            "    {\"item\":\"%s\",\"name\":\"%s\",\"status\":\"%s\",\"message\":\"%s\",\"duration_ms\":%d}",
            g_result_items[i].item,
            esc_name,
            esc_status,
            esc_message,
            g_result_items[i].duration_ms);
    }

    fputs("\n  ]\n}\n", fp);
    fclose(fp);
}

static void persist_result_file_locked(void)
{
    persist_result_file_to_path_locked(RESULT_FILE);
    persist_result_file_to_path_locked(RESULT_FILE_TMP);
}

static void clear_result_items_locked(void)
{
    memset(g_result_items, 0, sizeof(g_result_items));
    g_test_finished = 0;
}

static void initialize_result_file(void)
{
    pthread_mutex_lock(&g_result_lock);
    clear_result_items_locked();
    persist_result_file_locked();
    pthread_mutex_unlock(&g_result_lock);
}

static int load_result_finished_flag(void)
{
    FILE *fp;
    char buf[1024];
    size_t len;

    fp = fopen(RESULT_FILE, "r");
    if (fp == NULL) {
        fp = fopen(RESULT_FILE_TMP, "r");
        if (fp == NULL) {
            return 0;
        }
    }

    len = fread(buf, 1, sizeof(buf) - 1, fp);
    fclose(fp);
    buf[len] = '\0';

    return (strstr(buf, "\"test_finished\": true") != NULL) ||
           (strstr(buf, "\"test_finished\":true") != NULL);
}

static void update_result_record(const char *json)
{
    factory_result_item item = {{0}};
    int slot;

    if (json == NULL) {
        return;
    }

    if (!extract_json_string_field(json, "item", item.item, sizeof(item.item))) {
        return;
    }

    extract_json_string_field(json, "name", item.name, sizeof(item.name));
    extract_json_string_field(json, "status", item.status, sizeof(item.status));
    extract_json_string_field(json, "message", item.message, sizeof(item.message));
    item.duration_ms = extract_json_int_field(json, "duration_ms", 0);
    item.in_use = 1;

    trim_json_string_value(item.item, sizeof(item.item), item.item);
    trim_json_string_value(item.name, sizeof(item.name), item.name);
    trim_json_string_value(item.status, sizeof(item.status), item.status);
    trim_json_string_value(item.message, sizeof(item.message), item.message);

    pthread_mutex_lock(&g_result_lock);
    slot = find_result_slot(item.item);
    if (slot >= 0) {
        g_result_items[slot] = item;
        persist_result_file_locked();
    }
    pthread_mutex_unlock(&g_result_lock);
}

static void finish_test_results(void)
{
    pthread_mutex_lock(&g_result_lock);
    g_test_finished = 1;
    persist_result_file_locked();
    pthread_mutex_unlock(&g_result_lock);
}

static void send_test_result(int client_fd, const char *path)
{
    const char *script_path;
    char output[SCRIPT_OUTPUT_SIZE];

    script_path = find_script_by_path(path);
    if (script_path == NULL) {
        send_response(client_fd, "404 Not Found", "application/json; charset=utf-8",
            "{\"status\":\"FAIL\",\"message\":\"unknown api\"}\n");
        return;
    }

    if (strcmp(path, "/api/test/mjpeg") == 0) {
        (void)snprintf(output, sizeof(output),
            "{\n"
            "  \"item\": \"mjpeg\",\n"
            "  \"name\": \"MJPEG 图传\",\n"
            "  \"status\": \"PASS\",\n"
            "  \"message\": \"浏览器自动拉起实时流\",\n"
            "  \"duration_ms\": 0\n"
            "}\n");
        update_result_record(output);
        send_response(client_fd, "200 OK", "application/json; charset=utf-8", output);
        return;
    }

    if (strcmp(path, "/api/test/sensor") == 0) {
        (void)snprintf(output, sizeof(output),
            "{\n"
            "  \"item\": \"sensor\",\n"
            "  \"name\": \"Sensor 出图\",\n"
            "  \"status\": \"PASS\",\n"
            "  \"message\": \"进入图像链路测试\",\n"
            "  \"duration_ms\": 0\n"
            "}\n");
        update_result_record(output);
        send_response(client_fd, "200 OK", "application/json; charset=utf-8", output);
        return;
    }

    (void)run_script_capture(script_path, output, sizeof(output));
    update_result_record(output);
    send_response(client_fd, "200 OK", "application/json; charset=utf-8", output);
}

static void route_request(int client_fd, const char *request)
{
    char method[16];// 请求方法
    char path[256];// 请求路径
    char *query;// 请求查询字符串

    if (sscanf(request, "%15s %255s", method, path) != 2) {// 解析请求行,提取请求方法和路径
        send_response(client_fd, "400 Bad Request", "text/plain; charset=utf-8", "400 Bad Request\n");// 发送400 Bad Request响应
        return;// 返回
    }

    query = strchr(path, '?');// 获取请求查询字符串 如果存在查询字符串,则截取到查询字符串结束符
    if (query != NULL) {// 如果存在查询字符串,则截取到查询字符串结束符
        *query = '\0';// 截取到查询字符串结束符
    }

    if (strcmp(method, "GET") != 0) {// 如果请求方法不是GET,则发送405 Method Not Allowed响应
        send_response(client_fd, "405 Method Not Allowed", "text/plain; charset=utf-8", "405 Method Not Allowed\n");// 发送405 Method Not Allowed响应
        return;// 返回
    }

    if (strcmp(path, "/api/info") == 0) {// 如果请求路径是/api/info,则发送信息响应  
        send_info(client_fd);
    } else if (strcmp(path, "/api/end_test") == 0) {
        finish_test_results();
        close(creat(FIRST_BOOT_FLAG_FILE, 0644));
        send_response(client_fd, "200 OK", "application/json; charset=utf-8",
            "{\"status\":\"PASS\",\"message\":\"测试已结束\",\"test_finished\":true}\n");
        g_running = 0;
    } else if (strcmp(path, "/api/result") == 0) {
        char body[8192];
        FILE *fp;
        size_t len;

        pthread_mutex_lock(&g_result_lock);
        persist_result_file_to_path_locked(RESULT_FILE);
        fp = fopen(RESULT_FILE, "r");
        if (fp != NULL) {
            len = fread(body, 1, sizeof(body) - 1, fp);
            body[len] = '\0';
            fclose(fp);
            pthread_mutex_unlock(&g_result_lock);
            send_response(client_fd, "200 OK", "application/json; charset=utf-8", body);
        } else {
            pthread_mutex_unlock(&g_result_lock);
            send_response(client_fd, "500 Internal Server Error", "text/plain; charset=utf-8", "open result file failed\n");
        }
    } else if (strcmp(path, "/mjpeg") == 0 || strcmp(path, "/stream/mjpeg") == 0) {
        factory_mjpeg_send_stream(client_fd);
    } else if (find_script_by_path(path) != NULL) {// 如果请求路径是测试脚本,则发送测试结果响应
        send_test_result(client_fd, path);
    } else {// 如果请求路径是静态文件,则发送静态文件响应
        send_file(client_fd, path);
    }
}

static void handle_client_connection(int client_fd)
{
    char request_buffer[REQ_BUF_SIZE];
    ssize_t recv_len;

    memset(request_buffer, 0, sizeof(request_buffer));

    recv_len = recv(client_fd, request_buffer, sizeof(request_buffer) - 1, 0);
    if (recv_len > 0) {
        request_buffer[recv_len] = '\0';
        route_request(client_fd, request_buffer);
    }

    close(client_fd);
}

static void *client_thread_main(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);
    handle_client_connection(client_fd);
    return NULL;
}

int main(void)
{
    int server_fd;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);
    signal(SIGPIPE, SIG_IGN);
    atexit(stop_mjpeg_stream);

    if (enable_sensor() != 0) {
        fprintf(stderr, "Failed to enable sensor before server start\n");
        return EXIT_FAILURE;
    }

    if (load_result_finished_flag()) {
        pthread_mutex_lock(&g_result_lock);
        clear_result_items_locked();
        g_test_finished = 1;
        persist_result_file_locked();
        pthread_mutex_unlock(&g_result_lock);
    } else {
        initialize_result_file();
    }

    server_fd = create_server_socket();// 创建服务器socket并监听80端口
    if (server_fd < 0) {
        return EXIT_FAILURE;
    }

    printf("Factory HTTP server listening on port %d\n", LISTEN_PORT); // http服务器监听端口
    printf("Static web root: %s\n", WWW_ROOT); // 静态网页根目录
    printf("Test script dir: %s\n", FACTORY_BIN_DIR); // 测试脚本目录
    printf("Open http://<board-ip>/ in browser\n"); // 在浏览器中打开http://<board-ip>/
    fflush(stdout); // 刷新标准输出流,确保输出立即显示

    while (g_running) {
        int client_fd;
        int *client_fd_ptr;
        pthread_t thread_id;
        int create_ret;

        client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EINTR) {
                continue;
            }
            perror("accept");
            break;
        }

        client_fd_ptr = (int *)malloc(sizeof(*client_fd_ptr));
        if (client_fd_ptr == NULL) {
            close(client_fd);
            continue;
        }
        *client_fd_ptr = client_fd;

        create_ret = pthread_create(&thread_id, NULL, client_thread_main, client_fd_ptr);
        if (create_ret != 0) {
            close(client_fd);
            free(client_fd_ptr);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_fd);// 关闭服务器socket
    stop_mjpeg_stream();
    return EXIT_SUCCESS;// 退出程序
}
