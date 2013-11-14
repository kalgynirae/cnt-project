#include "sender.h"

//helper methods for use within sender.c
//send a normal message
int norm_send(int sock_fd, 
             message_t msg_type, 
             unsigned char content[], 
             size_t content_size);
//pack an integer into the 4-byte array passed in
void pack_int(unsigned int val, unsigned char bytes[4]);

int send_handshake(int sock_fd, int sender_id)
{
    unsigned char msg[32];

    strncpy((char*)msg, HS_GREETING, HS_GREETING_LEN);      //prefix with greeting
    memset(msg + HS_PADDING_POS, 0, HS_PADDING_LEN); //pad with zeroes
    pack_int(sender_id, msg + HS_ID_POS);    //place sender_id in last 4 bytes

    return send(sock_fd, msg, 32, 0);           //send message
}

int send_choke(int sock_fd)
{
    return 0;
}

int send_unchoke(int sock_fd)
{
    return 0;
}

int send_interested(int sock_fd)
{
    return 0;
}

int send_not_interested(int sock_fd)
{
    return 0;
}

int send_have(int sock_fd, int piece_idx)
{
    return 0;
}

int send_bitfield(int sock_fd, bitfield_t bitfield)
{
    return 0;
}

int send_request(int sock_fd, int piece_idx)
{
    return 0;
}

int send_piece(int sock_fd, int piece_idx, unsigned char content[])
{
    return 0;
}

//helper methods for use within sender.c
int norm_send(int sock_fd, 
             message_t msg_type, 
             unsigned char content[], 
             size_t content_size)
{
    unsigned char msg[MSG_TYPE_LEN + MSG_LEN_LEN + content_size];
    int msg_size = content_size + sizeof(message_t) + sizeof(int);

    //write length field to message
    pack_int(sizeof(message_t) + content_size, msg);
    //write type field to message
    msg[MSG_TYPE_POS] = msg_type;
    //write content to message
    memcpy(msg + MSG_CONTENT_POS, content, content_size);

    //send message
    int rval = send(sock_fd, (void*)&msg, msg_size, 0);

    if (rval < 0) {
        fprintf(stderr, "send_msg to sock_fd %d failed.\n", sock_fd);
    }

    unsigned char *c;
    for (c = msg ; c < msg + MSG_TYPE_LEN + MSG_LEN_LEN + content_size ; c++)
    {
        printf("%x ", *c);
    }
    printf("\n");

    return rval;
}

void pack_int(unsigned int val, unsigned char bytes[4])
{
    //network byte order is MSB first
    bytes[0] = (val >> 24) & 0xFF;
    bytes[1] = (val >> 16) & 0xFF;
    bytes[2] = (val >> 8) & 0xFF;
    bytes[3] = val & 0xFF;
}
