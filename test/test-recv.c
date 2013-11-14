/*
** modified version of client.c from beeg networking tutorial
** modify receive test to change message parsing behavior
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include "message.h"
#include "receiver.h"

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 80 // max number of bytes we can get at once 

void receive_test(unsigned char msg_in[], int nbytes)
{   //place test code for parsing received message here
    printf("message received:\n");
    int i;
    for (i = 0 ; i < nbytes ; i++)
    {   //print message as hex
        printf("%x", msg_in[i]);
    }
    printf("\n");
    printf("\nLength: %d", unpack_int(msg_in + MSG_LEN_POS));
    printf("\nType: %x", msg_in[MSG_TYPE_POS]);
    printf("\nContent: %s\n", msg_in + MSG_CONTENT_POS);
}

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	unsigned char buf[MAXDATASIZE];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];

	if (argc != 2) {
	    fprintf(stderr,"usage: client hostname\n");
	    exit(1);
	}

	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}

	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
				p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}

		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
			s, sizeof s);
	printf("client: connecting to %s\n", s);

	freeaddrinfo(servinfo); // all done with this structure

    memset(buf, 0, MAXDATASIZE);

	if ((numbytes = recv(sockfd, buf, MAXDATASIZE, 0)) == -1) {
	    perror("recv");
	    exit(1);
	}

    receive_test(buf, numbytes);

	close(sockfd);

	return 0;
}
