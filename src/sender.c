#include "sender.h"

//helper method for use within sender.c
int send_msg(int sock_fd, message_t msg_type, unsigned char content[])
{
    return 0;
}

int send_handshake(int sock_fd, int sender_id)
{
}

int send_choke(int sock_fd)
{
}

int send_unchoke(int sock_fd)
{
}

int send_interested(int sock_fd)
{
}

int send_not_interested(int sock_fd)
{
}

int send_have(int sock_fd, int piece_idx)
{
}

int send_bitfield(int sock_fd, bitfield_t bitfield)
{
}

int send_request(int sock_fd, int piece_idx)
{
}
