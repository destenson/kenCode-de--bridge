#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

typedef char* (*function_pointer)(char*);
volatile sig_atomic_t run = 1;

void terminate(int signum) {
	fprintf(stdout, "Terminating...\n");
	run = 0;
}

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

char* get_trading_pairs(char* command_line) {
	char* result = malloc(15);
	strcpy(result, "Hello, World!");
	return result;
}

function_pointer getCommand(char* buffer) {
	if (strncmp(buffer, "trading_pairs", 13) == 0) {
		return get_trading_pairs;
	}
	return NULL;
}

/**
 * Append data to the current buffer
 */
int buffer_append(char** buffer, size_t curr_buffer_length, char* incoming, size_t incoming_length) {
	if (incoming_length > 0) {
		char* new_buffer = (char*)malloc(curr_buffer_length + incoming_length + 1);
		if (new_buffer == NULL)
			return -1;
		memcpy(new_buffer, *buffer, curr_buffer_length);
		memcpy(&new_buffer[curr_buffer_length], incoming, incoming_length);
		new_buffer[curr_buffer_length + incoming_length] = 0;
		free(*buffer);
		*buffer = new_buffer;
	}
	return curr_buffer_length + incoming_length;
}

int do_connect(int sockfd) {
	char *buffer = (char*)malloc(0);
	size_t buffer_length = 0;
	size_t n = 255;
	int retVal = 1;
	// read from the socket
	while (n >= 0 && n == 255) {
		char incoming[255];
		n = read(sockfd,incoming,255);
		if ((buffer_length = buffer_append(&buffer, buffer_length, incoming, n)) == -1)
			break;
		if (buffer_length > 65535)
			n = -1;
	}
	if (n < 0) {
		error("ERROR reading from socket");
		retVal = -1;
	} else {
		if (strncmp(buffer, "EXIT", 4) == 0) {
			int parent = getppid();
			kill(parent, SIGTERM);
			retVal = -3;
		} else {
			// figure out what the user wanted to do
			function_pointer command = getCommand(buffer);
			if (command != NULL) {
				char* result = command(buffer);
				// write back to the socket
				if (result != NULL) {
					n = write(sockfd,result,strlen(result));
					if (n < 0)
						error("ERROR writing to socket");
					free(result);
				} else {
					n = write(sockfd,"No Results",10);
					if (n < 0)
						error("ERROR writing to socket");
				}
			}
		}
	}
	// cleanup
	free(buffer);
	close(sockfd);
	return retVal;
}

int main(int argc, char *argv[])
{
     int sockfd, newsockfd, portno;
     socklen_t clilen;
     struct sockaddr_in serv_addr, cli_addr;

     // handle SIGTERM
     struct sigaction action;
     memset(&action, 0, sizeof(struct sigaction));
     action.sa_handler = terminate;
     sigaction(SIGTERM, &action, NULL);

     // process command line
     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         fprintf(stderr, "Syntax: %s PORTNO\n", argv[0]);
         exit(1);
     }

     // open socket
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0)
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     // bind and listen
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0)
              error("ERROR on binding");
     listen(sockfd,5);
     clilen = sizeof(cli_addr);
     while(run) {
		 // blocks until a connection
		 newsockfd = accept(sockfd,
					 (struct sockaddr *) &cli_addr,
					 &clilen);
		 if (newsockfd < 0)
			  error("ERROR on accept");
		 // TODO: Do this in a thread, not a fork.
		 int pid = fork();
		 if (pid < 0) {
			 error("ERROR on fork");
		 }
		 if (pid == 0) {
			 close(sockfd);
			 do_connect(newsockfd);
			 exit(0);
		 }
		 else {
			 close(newsockfd);
		 }

	 }
     close(sockfd);
     return 0;
}
