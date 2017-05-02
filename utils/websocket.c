#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <pthread.h>

#include "utils/websocket.h"
#include "utils/base64.h"
#include "utils/sha1.h"

struct WebSocketClient* websocket_client_new(int fd, struct wslay_event_callbacks* callbacks, const char* body) {
	struct WebSocketClient* ws = malloc(sizeof(struct WebSocketClient));
	if (ws) {
		ws->fd = fd;
		size_t len = strlen(body) + 1;
		ws->body = malloc(len);
		if (ws->body == NULL) {
			free (ws);
			return NULL;
		}
		wslay_event_context_client_init(&ws->ctx, callbacks, ws);
		strcpy(ws->body, body);
		ws->body_off = 0;
		ws->dev_urand = open("/dev/urandom", O_RDONLY);
		ws->recv_size = 0;
	}
	return ws;
}

void websocket_client_free(struct WebSocketClient* client) {
	pthread_cancel (client->pth);
	wslay_event_context_free(client->ctx);
	shutdown(client->fd, SHUT_WR);
	close(client->dev_urand);
	free(client->body);
	free(client);
}

int websocket_client_on_read_event(struct WebSocketClient* ws) {
	return wslay_event_recv(ws->ctx);
}

int websocket_client_on_write_event(struct WebSocketClient* ws) {
	return wslay_event_send(ws->ctx);
}

ssize_t websocket_client_send_data(struct WebSocketClient* ws, const uint8_t *data, size_t len, int flags) {
	ssize_t r;
	int sflags = 0;
#ifdef MSG_MORE
	if(flags & WSLAY_MSG_MORE) {
		sflags |= MSG_MORE;
	}
#endif // MSG_MORE
	while((r = send(ws->fd, data, len, sflags)) == -1 && errno == EINTR);
	return r;
}

ssize_t websocket_client_feed_body(struct WebSocketClient* ws, uint8_t *data, size_t len)
{
	if(ws->body_off < strlen(ws->body)) {
		size_t wlen = len;
		if (len > strlen(ws->body) - ws->body_off)
			len = strlen(ws->body) - ws->body_off;
		memcpy(data, ws->body, wlen);
		ws->body_off += wlen;
		return wlen;
	} else {
		return 0;
	}
}

ssize_t websocket_client_recv_data(struct WebSocketClient* ws, uint8_t *data, size_t len, int flags)
{
	ssize_t r;
	while((r = recv(ws->fd, data, len, 0)) == -1 && errno == EINTR);
	return r;
}

int websocket_client_want_read(struct WebSocketClient* ws)
{
	return wslay_event_want_read(ws->ctx);
}

int websocket_client_want_write(struct WebSocketClient* ws)
{
	return wslay_event_want_write(ws->ctx);
}

int websocket_client_fd(struct WebSocketClient* ws)
{
	return ws->fd;
}

void websocket_client_get_random(struct WebSocketClient* ws, uint8_t *buf, size_t len)
{
	read(ws->dev_urand, buf, len);
}

void websocket_client_set_callbacks(struct WebSocketClient* ws, const struct wslay_event_callbacks *callbacks)
{
	wslay_event_config_set_callbacks(ws->ctx, callbacks);
}

int send_http_handshake(int fd, const char* reqheader)
{
	size_t off = 0;
	while(off < strlen(reqheader)) {
		ssize_t r;
		size_t len = strlen(reqheader)-off;
		while((r = write(fd, reqheader+off, len)) == -1 && errno == EINTR);
		if(r == -1) {
			perror("write");
			return -1;
		}
		off += r;
	}
	return 0;
}

int recv_http_handshake(int fd, char* resheader)
{
	char buf[4096];
	while(1) {
		ssize_t r;
		while((r = read(fd, buf, sizeof(buf))) == -1 && errno == EINTR);
		if(r <= 0) {
			return -1;
		}
		buf[r] = '\0';

		// append buf
		if (strlen(resheader) + r + 1 > 8192) {
			fprintf(stderr, "Too big response header\n");
			return -1;
		}

		strcat(resheader, buf);

		if (strstr(resheader, "\r\n\r\n") != NULL) {
			break;
		}
	}
	return 0;
}

char* base64(char* in, int len)
{
	unsigned char *out = malloc ((len/3 + 1) * 4 + 1);
	if (out)
		base64_encode((unsigned char*)in, out, len, 0);
	return (char*)out;
}

char* sha1(char* in)
{
	SHA1_CTX sha;
	char *out = malloc(SHA1_DIGEST_SIZE);

	if (!out)
		return NULL;

	SHA1_Init   (&sha);
	SHA1_Update (&sha, (uint8_t *)in, strlen(in));
	SHA1_Final  (&sha, (uint8_t *)out);

	return out;
}

char* get_random16(char* buf)
{
	int fd = open("/dev/urandom", O_RDONLY);
	char *p;
	if (fd != -1) {
		read(fd, buf, 16);
		// Make sure 16 bytes are different from null terminator.
		for (p = memchr(buf, '\0', 16);
		     p;
		     p = memchr(buf, '\0', 16)) {
			read(fd, p, 1);
		}
		close(fd);
	}
	buf[16] = '\0';
	return buf;
}

char* websocket_create_acceptkey(const char* clientkey)
{
	size_t len = strlen(clientkey) + 40;
	char* str = malloc(len);
	if (!str)
		return NULL;
	snprintf(str, len, "%s%s", clientkey, "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
	char* sha = sha1(str);
	free(str);
	if (!sha)
		return NULL;
	char* results = base64(sha, SHA1_DIGEST_SIZE);
	free(sha);
	return results;
}

int http_handshake(int fd, const char* host, const char* service, const char* path, char* body) {
	char reqheader[4096];
	char r16[17];
	char* client_key = base64(get_random16(r16), 16);
	snprintf(reqheader, 4096,
	         "GET %s HTTP/1.1\r\n"
	         "Host: %s:%s\r\n"
	         "Upgrade: websocket\r\n"
	         "Connection: Upgrade\r\n"
	         "Sec-WebSocket-Key: %s\r\n"
	         "Sec-WebSocket-Version: 13\r\n"
	         "\r\n",
	         path, host, service, client_key);
	if(send_http_handshake(fd, reqheader) == -1) {
		return -1;
	}
	// Allocate 8k which is the maximum used in the recv_http_handshake call.
	char* resheader = malloc(8192);
	if (!resheader) {
		return -1;
	}
	*resheader = '\0'; // Initialize memory with an empty string.
	if(recv_http_handshake(fd, resheader) == -1) {
		free(resheader);
		return -1;
	}
	char* keyhdstart = strstr(resheader, "Sec-WebSocket-Accept: ");
	if(keyhdstart == NULL) {
		fprintf(stderr, "http_upgrade: missing required headers\n");
		free(resheader);
		return -1;
	}
	keyhdstart += 22;
	char* keyhdend = strstr(keyhdstart, "\r\n");
	if(!keyhdend) {
		free(resheader);
		return -1;
	}
	char* accept_key = malloc(keyhdend - keyhdstart + 1);
	if(!accept_key) {
		free(resheader);
		return -1;
	}
	memcpy(accept_key, keyhdstart, keyhdend-keyhdstart);
	accept_key[keyhdend-keyhdstart] = 0;
	int retVal = -1;
	char *p = websocket_create_acceptkey(client_key);
	if (p) {
		if(strcmp (accept_key, p) == 0) {
			char* body_pos = strstr(resheader, "\r\n\r\n");
			if (body_pos != NULL) {
				body_pos += 4;
				strcpy(body, body_pos);
				retVal = 0;
			}
		}
		free (p);
	}
	free(client_key);
	free(accept_key);
	free(resheader);
	return retVal;
}

ssize_t send_callback(wslay_event_context_ptr ctx, const uint8_t *data, size_t len, int flags, void *user_data) {
	struct WebSocketClient *ws = (struct WebSocketClient*)user_data;
	ssize_t r = websocket_client_send_data(ws, data, len, flags);
	if(r == -1) {
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
		} else {
			wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
		}
	}
	return r;
}

ssize_t recv_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data) {
	struct WebSocketClient *ws = (struct WebSocketClient*)user_data;
	ssize_t r = websocket_client_recv_data(ws, data, len, flags);
	if(r == -1) {
		if(errno == EAGAIN || errno == EWOULDBLOCK) {
			wslay_event_set_error(ctx, WSLAY_ERR_WOULDBLOCK);
		} else {
			wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
		}
	} else if(r == 0) {
		wslay_event_set_error(ctx, WSLAY_ERR_CALLBACK_FAILURE);
		r = -1;
	}
	return r;
}

ssize_t feed_body_callback(wslay_event_context_ptr ctx, uint8_t *data, size_t len, int flags, void *user_data) {
	struct WebSocketClient *ws = (struct WebSocketClient*)user_data;
	return websocket_client_feed_body(ws, data, len);
}

int genmask_callback(wslay_event_context_ptr ctx, uint8_t *buf, size_t len, void *user_data) {
	struct WebSocketClient *ws = (struct WebSocketClient*)user_data;
	websocket_client_get_random(ws, buf, len);
	return 0;
}

void on_msg_recv_callback(wslay_event_context_ptr ctx,
                          const struct wslay_event_on_msg_recv_arg *arg,
                          void *user_data)
{
	struct WebSocketClient *ws = (struct WebSocketClient*)user_data;
	int size;

	if(!wslay_is_ctrl_frame(arg->opcode)) {
		for(;;) {
			size = sizeof (ws->recv_buf) - ws->recv_size;
			if (size >= arg->msg_length)
				break;
			sleep (1);
		}
		memcpy (ws->recv_buf + ws->recv_size, arg->msg, arg->msg_length);
		ws->recv_size += arg->msg_length;
	}
}

int websocket_recv(struct WebSocketClient *ws, char *data, int size)
{
	int len = ws->recv_size;
	if (len > 0) {
		if (len > size) {
			len = size;
			memcpy(data, ws->recv_buf, len);
			memmove(ws->recv_buf, ws->recv_buf + len, ws->recv_size - len);
			ws->recv_size -= len;
		} else {
			memcpy(data, ws->recv_buf, len);
			ws->recv_size = 0;
		}
	}
	return len;
}

int websocket_send(struct WebSocketClient *ws, char *data, int size)
{
	struct wslay_event_msg msgarg = {
		WSLAY_BINARY_FRAME, (uint8_t*)data, size
	};
	wslay_event_queue_msg(ws->ctx, &msgarg);
	return size;
}

int connect_to(const char *host, const char *service)
{
	struct addrinfo hints, *rp;
	int fd = -1;
	int r;
	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *res;
	r = getaddrinfo(host, service, &hints, &res);
	if(r != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(r));
		return -1;
	}
	for(rp = res; rp; rp = rp->ai_next) {
		fd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
		if(fd == -1) {
			continue;
		}
		while((r = connect(fd, rp->ai_addr, rp->ai_addrlen)) == -1 &&
		      errno == EINTR);
		if(r == 0) {
			break;
		}
		close(fd);
		fd = -1;
	}
	freeaddrinfo(res);
	return fd;
}

int make_non_block(int fd)
{
	int flags, r;
	while((flags = fcntl(fd, F_GETFL, 0)) == -1 && errno == EINTR);
	if(flags == -1) {
		return -1;
	}
	while((r = fcntl(fd, F_SETFL, flags | O_NONBLOCK)) == -1 && errno == EINTR);
	if(r == -1) {
		return -1;
	}
	return 0;
}

void ctl_epollev(int epollfd, int op, struct WebSocketClient* ws)
{
	struct epoll_event ev;
	memset(&ev, 0, sizeof(ev));
	if(websocket_client_want_read(ws)) {
		ev.events |= EPOLLIN;
	}
	if(websocket_client_want_write(ws)) {
		ev.events |= EPOLLOUT;
	}
	if(epoll_ctl(epollfd, op, ws->fd, &ev) == -1) {
		perror("epoll_ctl");
		exit(EXIT_FAILURE);
	}
}

void *websockets_thread (void *ptr)
{
	struct WebSocketClient* ws = ptr;
	int epollfd = epoll_create(1);
	if(epollfd == -1) {
		perror("epoll_create");
		return NULL;
	}
	ctl_epollev(epollfd, EPOLL_CTL_ADD, ws);
	static const size_t MAX_EVENTS = 1;
	struct epoll_event events[MAX_EVENTS];
	int ok = 1;
	while(websocket_client_want_read(ws) || websocket_client_want_write(ws)) {
		int n, nfds = epoll_wait(epollfd, events, MAX_EVENTS, 100);
		if(nfds == -1) {
			perror("epoll_wait");
			return NULL;
		}
		for(n = 0; n < nfds; ++n) {
			if(((events[n].events & EPOLLIN) && websocket_client_on_read_event(ws) != 0) ||
			   ((events[n].events & EPOLLOUT) && websocket_client_on_write_event(ws) != 0)) {
				ok = 0;
				break;
			}
		}
		if(!ok) {
			break;
		}
		ctl_epollev(epollfd, EPOLL_CTL_MOD, ws);
	}
	return NULL;
}

/**
 * Communicate with the websocket
 * @param host the host
 * @param service the service
 * @param path the url path
 * @param callbacks the callbacks to use
 * @returns 0 on success, -1 if not
 */
struct WebSocketClient*  communicate(const char *host, const char *service, const char *path,
					const struct wslay_event_callbacks *callbacks)
{
	struct wslay_event_callbacks cb = *callbacks;
	cb.recv_callback = feed_body_callback;
	int fd = connect_to(host, service);
	if(fd == -1) {
		fprintf(stderr, "Could not connect to the host.\n");
		return NULL;
	}
	char body[8192];
	if(http_handshake(fd, host, service, path, body) == -1) {
		fprintf(stderr, "Failed handshake\n");
		close(fd);
		return NULL;
	}
	make_non_block(fd);
	int val = 1;
	if(setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &val, (socklen_t)sizeof(val))
	   == -1) {
		perror("setsockopt: TCP_NODELAY");
		return NULL;
	}
	struct WebSocketClient* ws = websocket_client_new(fd, &cb, body);
	if(websocket_client_on_read_event(ws) == -1) {
		return NULL;
	}
	cb.recv_callback = callbacks->recv_callback;
	websocket_client_set_callbacks(ws, &cb);
	if (pthread_create(&ws->pth, NULL, websockets_thread, ws)) {
		websocket_client_free (ws);
		return NULL;
	}
	return ws;
}
