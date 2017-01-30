#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#include "json-c/json_object.h"
#include "bridge/market.h"

void send_error(const char *msg)
{
	fprintf(stdout, "{ \"success\" : false, \"message\" : \"%s\" }", msg);
}

int bridge_connect() {
	int sockfd = -1, portno = 12345;
	struct hostent *server;
	struct sockaddr_in serv_addr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;

	server = gethostbyname("localhost");
	if (server == NULL) {
		return -1;
	}

	bzero( (char*)&serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy( (char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
		return -1;
	}
	return sockfd;
}

struct Market* parse_markets(unsigned char* input, size_t input_length) {
	struct Market* retVal = NULL;
	int success = 0;
	if (market_list_protobuf_decode(input, input_length, &retVal ) == 0)
		goto exit;
	success = 1;
	exit:
	if (success) {
		return retVal;
	}
	return NULL;
}

void format_output(unsigned char* input, size_t input_length) {
	// turn the input back into the structs
	struct Market* head = parse_markets(input, input_length);
	struct Market* current = head;
	struct json_object* parent = json_object_new_array();
	while (current != NULL) {
		struct json_object* pair = json_object_new_object();
		json_object_object_add(pair, "inputCoinType", json_object_new_string(current->base_currency));
		json_object_object_add(pair, "outputCoinType", json_object_new_string(current->market_currency));
		json_object_object_add(pair, "rateFee", json_object_new_double(current->fee));
		json_object_array_add(parent, pair);
		current = current->next;
	}
	// send to caller
	fprintf(stdout, "%s", json_object_to_json_string(parent));
	// clean up
	json_object_put(parent);
	market_free(head);
	return;
}

int main(int argc, char *argv[])
{
	int sockfd;
	size_t n;
	unsigned char buffer[5000];
	const char* bridge_command = "trading_pairs";

	// connect to the bridge
	sockfd = bridge_connect();
    if (sockfd < 0) {
    	send_error("Unable to connect to bridge");
    	exit(0);
    }

    // send the command
    n = write(sockfd, bridge_command, strlen(bridge_command));
    if (n < 0)
         send_error("unable to write to bridge");

    bzero(buffer,5000);
    n = read(sockfd,buffer,5000);
    if (n < 0) {
         send_error("Unable to read from bridge");
         exit(0);
    }

    // send output to the client
    format_output(buffer, n);

    // cleanup
    close(sockfd);
    return 0;
}
