#include "sender.h"

//helper method for use within sender.c
int send_msg(int sock_fd, message_t msg_type, unsigned char content[])
{
    return 0;
}

int send_handshake(int sock_fd, int sender_id)
{
    return 0;
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
