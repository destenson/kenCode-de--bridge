/*
 * Testing of websocket connectivity
 */
#include "utils/websocket.h"

int test_connect() {
	char* host = "127.0.0.1:8090";
	char* service = "blah";

	// setup callbacks
	struct wslay_event_callbacks callbacks = {
		recv_callback,
		send_callback,
		genmask_callback,
		NULL, /* on_frame_recv_start_callback */
		NULL, /* on_frame_recv_callback */
		NULL, /* on_frame_recv_end_callback */
		on_msg_recv_callback
	};
	// connect to service
	// do http handshake
	// subscribe
	// process events
	// disconnect
	return 1;
}
