#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

int socket_open(const char* hostname, int port) {
    struct hostent *server;
    int sockfd;
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"ERROR, no such host\n");
        exit(0);
    }

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
    serv_addr.sin_port = htons(port);

    if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR connecting");
	return sockfd;
}

int main(int argc, char *argv[])
{
	int portno = 0, n;
	char buffer[256];

	// parse command line
    if (argc < 4) {
       fprintf(stderr,"usage %s hostname port message\n", argv[0]);
       exit(0);
    }
    portno = atoi(argv[2]);

    // open socket
    int sockfd = socket_open(argv[1], portno);

    // write to socket
    fprintf(stdout, "Sending the message: %s\n", argv[3]);
    n = write(sockfd,argv[3],strlen(argv[3]));
    if (n < 0)
         error("ERROR writing to socket");

    // read from socket
    bzero(buffer,256);
    n = read(sockfd,buffer,255);
    if (n < 0)
         error("ERROR reading from socket");

    // display results
    printf("Read %d bytes from buffer. The first byte is %02x\n", n, buffer[0]);
    printf("Result: %s\n", buffer);

    // cleanup
    close(sockfd);

    return 0;
}
