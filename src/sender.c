#include "sender.h"

//helper methods for use within sender.c
int send_msg(int sock_fd, message_t msg_type, unsigned char content[]);
void pack_int(unsigned int val, char bytes[4]);

int send_handshake(int sock_fd, int sender_id)
{
    char msg[32];

    strncpy(msg, HS_GREETING, HS_GREETING_LEN);      //prefix with greeting
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
int send_msg(int sock_fd, message_t msg_type, unsigned char content[])
{
    struct mess_normal msg;
    msg.type = msg_type;
    //msg.payload = malloc(sizeof(content));
    //msg.payload = content;
    msg.length = sizeof(msg) - sizeof(msg.length);  //don't count length field

    int rval = send(sock_fd, (void*)&msg, sizeof(msg), 0);

    if (rval < 0) {
        fprintf(stderr, "send_msg to sock_fd %d failed.\n", sock_fd);
    }

    return rval;
}

void pack_int(unsigned int val, char bytes[4])
{
    bytes[0] = (val >> 24) & 0xFF;
    bytes[1] = (val >> 16) & 0xFF;
    bytes[2] = (val >> 8) & 0xFF;
    bytes[3] = val & 0xFF;
}
