#include "sender.h"

//helper method for use within sender.c
int send_msg(int sock_fd, message_t msg_type, unsigned char[] content)
{
    struct mess_normal msg;
    msg.msg_type = msg_type;
    msg.payload = content;
    msg.length = sizeof(msg) - sizeof(msg.length);  //don't count length field

    int rval = send(sock_fd, msg, sizeof(msg), 0)

    if (rval < 0) {
        fprintf(stderr, "send_msg to sock_fd %d failed.\n", sock_fd);
    }

    return rval;
}

int send_handshake(int sock_fd, int sender_id)
{
	struct mess_handshake hs;               //handshake message to send
	
	hs.header = "HELLO";                    //handshake header
	memset(hs.zeros, 0, sizeof(hs.zeros));  //23 zeros
	hs.peer_id = p_id;                      //4 byte peer ID
	
	return hs;                              //return generated handshake message
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

int send_have(int sock_fd)
{
}

int send_bitfield(int sock_fd)
{
}

int send_request(int sock_fd)
{
}
