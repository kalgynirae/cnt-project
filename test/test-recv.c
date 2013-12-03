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

#include "init.h"
#include "message.h"
#include "receiver.h"

#include <arpa/inet.h>

#define PORT "3490" // the port client will be connecting to 
#define COMMON_CFG_PATH "config/Common.cfg"

#define MAXDATASIZE 80 // max number of bytes we can get at once 

//global configuration options
extern int g_bitfield_len;

void process_msg(unsigned char *content, int nbytes, message_t type)
{   //place test code for parsing received message here
    printf("recv'd message: ");

    switch(type)
    {
        case HANDSHAKE:
            printf("Handshake from %d\n", unpack_int(content));
            break;
        case CHOKE:
            printf("CHOKE\n");
            break;
        case UNCHOKE:
            printf("UNCHOKE\n");
            break;
        case INTERESTED:
            printf("INTERESTED\n");
            break;
        case NOT_INTERESTED:
            printf("NOT_INTERESTED\n");
            break;
        case HAVE:
            printf("HAVE piece %d\n", unpack_int(content));
            break;
        case BITFIELD:
            printf("BITFIELD: ");
            int i;
            for (i = 0 ; i < g_bitfield_len ; i++) { printf("%X ", content[i]); }
            break;
        case REQUEST:
            printf("REQUEST piece %d\n", unpack_int(content));
            break;
        case PIECE:
            printf("PIECE \n");
            extract_and_save_piece(nbytes, content);
            break;
        default:
            break;
    }
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
    read_cfg(COMMON_CFG_PATH);      //to find bitfield length
	int sockfd;  
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

    unsigned int payload_len;
    message_t type;
    unsigned char *payload = NULL;

    int i;
    for (i = 0 ; i < 8 ; i++)
    {
        type = recv_msg(sockfd, &payload_len, payload);
        process_msg(payload, payload_len, type);
        if (payload != NULL) { free(payload); }      //Don't forget this!!!
        payload = NULL;
        printf("\n");
    }

	close(sockfd);

	return 0;
}
