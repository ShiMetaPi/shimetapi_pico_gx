/*
 * Lightweight HTTP server for GK7206 projects.
 * Provides: static file serving, JSON API routing, and MJPEG streaming.
 *
 * Usage: Each project provides a route handler for custom API requests.
 */

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#define WEB_LISTEN_PORT     80
#define WEB_BACKLOG         8
#define WEB_REQ_BUF_SIZE    2048
#define WEB_SCRIPT_BUF_SIZE 4096
#define WEB_WWW_ROOT        "/www"

/* Route handler callback: return 0 if handled, -1 if not */
typedef int (*web_route_handler)(int client_fd, const char *path);
typedef void (*web_mjpeg_handler)(int client_fd);
typedef void (*web_stop_handler)(void);
typedef void (*web_snapshot_handler)(int client_fd);

/* Register optional project-specific streaming hooks. */
void web_server_set_mjpeg_handler(web_mjpeg_handler mjpeg_handler, web_stop_handler stop_handler);
void web_server_set_snapshot_handler(web_snapshot_handler snapshot_handler);

/* Start the web server (blocking loop). Returns on signal or g_stop. */
int web_server_run(web_route_handler route_handler);

/* Stop the web server */
void web_server_stop(void);

/* Send HTTP response helpers */
void web_send_response(int fd, const char *status, const char *ctype, const char *body);
void web_send_header(int fd, const char *status, const char *ctype, size_t len);
void web_send_all(int fd, const char *data, size_t len);
void web_send_file(int fd, const char *request_path);
void web_send_json_ok(int fd, const char *json_body);

/* MJPEG streaming */
void web_mjpeg_send_stream(int fd);
void web_mjpeg_request_stop(void);

#endif /* WEB_SERVER_H */
