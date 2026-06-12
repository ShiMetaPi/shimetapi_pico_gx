/*
 * Lightweight HTTP server for GK7206 projects.
 * Provides: static file serving, JSON API routing, and MJPEG streaming.
 *
 * Usage: Each project provides a route handler for custom API requests.
 */

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "web_server.h"

static volatile sig_atomic_t g_web_running = 1;
static web_route_handler g_route_handler = NULL;
static web_mjpeg_handler g_mjpeg_handler = NULL;
static web_stop_handler g_mjpeg_stop_handler = NULL;
static web_snapshot_handler g_snapshot_handler = NULL;

/* Get content type based on file extension */
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
    if (strcmp(ext, ".jpg") == 0 || strcmp(ext, ".jpeg") == 0) {
        return "image/jpeg";
    }
    if (strcmp(ext, ".png") == 0) {
        return "image/png";
    }

    return "application/octet-stream";
}

/* Check if path is safe (no directory traversal) */
static int is_safe_static_path(const char *path)
{
    if (strstr(path, "..") != NULL) {
        return 0;
    }
    return 1;
}

/* Send all data to socket */
void web_send_all(int fd, const char *data, size_t len)
{
    size_t sent_len = 0;

    while (sent_len < len) {
        ssize_t ret = send(fd, data + sent_len, len - sent_len, 0);
        if (ret <= 0) {
            return;
        }
        sent_len += (size_t)ret;
    }
}

/* Send HTTP header */
void web_send_header(int fd, const char *status, const char *ctype, size_t len)
{
    char header[512];
    int header_len;

    header_len = snprintf(header, sizeof(header),
        "HTTP/1.1 %s\r\n"
        "Content-Type: %s\r\n"
        "Content-Length: %zu\r\n"
        "Cache-Control: no-store\r\n"
        "Connection: close\r\n"
        "\r\n",
        status, ctype, len);

    if (header_len > 0) {
        web_send_all(fd, header, (size_t)header_len);
    }
}

/* Send complete HTTP response */
void web_send_response(int fd, const char *status, const char *ctype, const char *body)
{
    size_t body_len = strlen(body);
    web_send_header(fd, status, ctype, body_len);
    web_send_all(fd, body, body_len);
}

/* Send JSON OK response */
void web_send_json_ok(int fd, const char *json_body)
{
    web_send_response(fd, "200 OK", "application/json; charset=utf-8", json_body);
}

/* Send static file */
void web_send_file(int fd, const char *request_path)
{
    FILE *fp;
    char file_path[512];
    char buffer[1024];
    struct stat st;
    size_t read_len;

    if (!is_safe_static_path(request_path)) {
        web_send_response(fd, "403 Forbidden", "text/plain", "403 Forbidden\n");
        return;
    }

    if (strcmp(request_path, "/") == 0) {
        request_path = "/index.html";
    }

    snprintf(file_path, sizeof(file_path), "%s%s", WEB_WWW_ROOT, request_path);

    if (stat(file_path, &st) < 0 || !S_ISREG(st.st_mode)) {
        web_send_response(fd, "404 Not Found", "text/plain", "404 Not Found\n");
        return;
    }

    fp = fopen(file_path, "rb");
    if (fp == NULL) {
        web_send_response(fd, "500 Internal Server Error", "text/plain", "open file failed\n");
        return;
    }

    web_send_header(fd, "200 OK", get_content_type(file_path), (size_t)st.st_size);
    while ((read_len = fread(buffer, 1, sizeof(buffer), fp)) > 0) {
        web_send_all(fd, buffer, read_len);
    }

    fclose(fp);
}

void web_server_set_mjpeg_handler(web_mjpeg_handler mjpeg_handler, web_stop_handler stop_handler)
{
    g_mjpeg_handler = mjpeg_handler;
    g_mjpeg_stop_handler = stop_handler;
}

void web_server_set_snapshot_handler(web_snapshot_handler snapshot_handler)
{
    g_snapshot_handler = snapshot_handler;
}

/* Request MJPEG stream stop */
void web_mjpeg_request_stop(void)
{
    if (g_mjpeg_stop_handler != NULL) {
        g_mjpeg_stop_handler();
    }
}

void web_mjpeg_send_stream(int fd)
{
    if (g_mjpeg_handler == NULL) {
        web_send_response(fd, "404 Not Found", "text/plain", "mjpeg stream is not available\n");
        return;
    }

    g_mjpeg_handler(fd);
}

/* Route GET request */
static void route_get(int client_fd, const char *path)
{
    /* Check for MJPEG stream / snapshot */
    if (strcmp(path, "/mjpeg") == 0 || strcmp(path, "/stream/mjpeg") == 0) {
        web_mjpeg_send_stream(client_fd);
        return;
    }
    if (strcmp(path, "/snapshot.jpg") == 0 || strcmp(path, "/api/frame.jpg") == 0) {
        if (g_snapshot_handler != NULL) {
            g_snapshot_handler(client_fd);
            return;
        }
        web_send_response(client_fd, "404 Not Found", "text/plain", "snapshot is not available\n");
        return;
    }

    /* Check for project-specific API routes */
    if (g_route_handler != NULL) {
        if (g_route_handler(client_fd, path) == 0) {
            return; /* Handled by project */
        }
    }

    /* Default: serve static file */
    web_send_file(client_fd, path);
}

/* Handle client connection */
static void handle_client(int client_fd)
{
    char request_buffer[WEB_REQ_BUF_SIZE];
    char method[16];
    char path[256];
    ssize_t recv_len;

    memset(request_buffer, 0, sizeof(request_buffer));
    recv_len = recv(client_fd, request_buffer, sizeof(request_buffer) - 1, 0);

    if (recv_len <= 0) {
        close(client_fd);
        return;
    }

    request_buffer[recv_len] = '\0';

    if (sscanf(request_buffer, "%15s %255s", method, path) != 2) {
        web_send_response(client_fd, "400 Bad Request", "text/plain", "400 Bad Request\n");
        close(client_fd);
        return;
    }

    if (strcmp(method, "GET") == 0) {
        char *query = strchr(path, '?');
        if (query != NULL) {
            *query = '\0';
        }
        route_get(client_fd, path);
    } else {
        web_send_response(client_fd, "405 Method Not Allowed", "text/plain", "405 Method Not Allowed\n");
    }

    close(client_fd);
}

/* Client thread main */
static void *client_thread_main(void *arg)
{
    int client_fd = *(int *)arg;
    free(arg);
    handle_client(client_fd);
    return NULL;
}

/* Create server socket */
static int create_server_socket(void)
{
    int server_fd;
    int reuse = 1;
    struct sockaddr_in address;

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return -1;
    }

    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        perror("setsockopt");
        close(server_fd);
        return -1;
    }

    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(WEB_LISTEN_PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind");
        close(server_fd);
        return -1;
    }

    if (listen(server_fd, WEB_BACKLOG) < 0) {
        perror("listen");
        close(server_fd);
        return -1;
    }

    return server_fd;
}

/* Run web server (blocking loop) */
int web_server_run(web_route_handler route_handler)
{
    int server_fd;

    g_route_handler = route_handler;

    signal(SIGPIPE, SIG_IGN);

    server_fd = create_server_socket();
    if (server_fd < 0) {
        return -1;
    }

    printf("HTTP server listening on port %d\n", WEB_LISTEN_PORT);
    printf("Web root: %s\n", WEB_WWW_ROOT);
    printf("Open http://<board-ip>/ in browser\n");
    fflush(stdout);

    while (g_web_running) {
        int client_fd;
        int *client_fd_ptr;
        pthread_t thread_id;

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

        if (pthread_create(&thread_id, NULL, client_thread_main, client_fd_ptr) != 0) {
            close(client_fd);
            free(client_fd_ptr);
            continue;
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}

/* Signal handler for web server stop */
void web_server_stop(void)
{
    g_web_running = 0;
    web_mjpeg_request_stop();
}