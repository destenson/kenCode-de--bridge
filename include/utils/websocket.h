#pragma once

#include "wslay/wslay.h"

struct WebSocketClient {
	  int fd;
	  wslay_event_context_ptr ctx;
	  char* body;
	  size_t body_off;
	  int dev_urand;
};

struct WebSocketClient* websocket_client_new(int fd, struct wslay_event_callbacks* callbacks, const char* body);
void websocket_client_free(struct WebSocketClient* client);
int websocket_client_on_read_event(struct WebSocketClient* ws);
int websocket_client_on_write_event(struct WebSocketClient* ws);
ssize_t websocket_client_send_data(struct WebSocketClient* ws, const uint8_t *data, size_t len, int flags);
ssize_t websocket_client_feed_body(struct WebSocketClient* ws, uint8_t *data, size_t len);
ssize_t websocket_client_recv_data(struct WebSocketClient* ws, uint8_t *data, size_t len, int flags);
int websocket_client_want_read(struct WebSocketClient* ws);
int websocket_client_want_write(struct WebSocketClient* ws);
int websocket_client_fd(struct WebSocketClient* ws);
void websocket_client_get_random(struct WebSocketClient* ws, uint8_t *buf, size_t len);
void websocket_client_set_callbacks(struct WebSocketClient* ws, const struct wslay_event_callbacks *callbacks);
int send_http_handshake(int fd, const char* reqheader);
int recv_http_handshake(int fd, char* resheader);
char* base64(char* in);
char* sha1(char* in);
char* get_random16(char* buf);
char* create_acceptkey(const char* clientkey);
int http_handshake(int fd, const char* host, const char* service, const char* path, char* body);
ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data, size_t len, int flags, void *user_data);
ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data);
ssize_t feed_body_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data);
int genmask_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len, void *user_data);
void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data);
int connect_to(const char *host, const char *service);
int make_non_block(int fd);
void ctl_epollev(int epollfd, int op, struct WebSocketClient* ws);
int communicate(const char *host, const char *service, const char *path,
                const struct wslay_event_callbacks *callbacks);
