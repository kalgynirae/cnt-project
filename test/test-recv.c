#include <stdio.h>
#include <stdlib.h>

#include "init.h"
#include "message.h"
#include "receiver.h"
#include "socket.h"

#define PORT 3490
#define COMMON_CFG_PATH "config/Common.cfg"

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
            extract_and_save_piece(nbytes, content, 4);
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr,"usage: client hostname\n");
        exit(1);
    }

    read_cfg(COMMON_CFG_PATH);      //to find bitfield length

    struct peer_info info;
    info.hostname = argv[1];
    info.port = PORT;

    int s = make_socket_to_peer(&info);
    if (s == -1) {
        fprintf(stderr, "client: make_socket_to_peer() failed");
        exit(1);
    }

    unsigned int payload_len;
    message_t type;
    unsigned char payload[5000];

    int i;
    for (i = 0 ; i < 8 ; i++)
    {
        type = recv_msg(info.socket_fd, &payload_len, payload);
        process_msg(payload, payload_len, type);
        printf("\n");
    }

    close(info.socket_fd);

    return 0;
}
