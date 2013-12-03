#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sender.h"
#include "init.h"
#include "socket.h"

#define PORT "3490"  // the port users will be connecting to
#define COMMON_CFG_PATH "config/Common.cfg"

extern int g_bitfield_len;

void send_messages(int sock_fd)
{
    //if (send_handshake(sock_fd, 1001) == -1) { perror("send"); }

    if (send_choke(sock_fd) == -1) { perror("send"); }

    if (send_unchoke(sock_fd) == -1) { perror("send"); }

    if (send_interested(sock_fd) == -1) { perror("send"); }

    if (send_not_interested(sock_fd) == -1) { perror("send"); }

    if (send_have(sock_fd, 125) == -1) { perror("send"); }

    if (send_request(sock_fd, 294) == -1) { perror("send"); }

    char bitfield[g_bitfield_len];
    memset(bitfield, 0xA, g_bitfield_len);
    if (send_bitfield(sock_fd, (bitfield_t)bitfield) == -1) { perror("send"); }

    if (send_piece(sock_fd, 5, 10, 1) == -1) { perror("send"); }
}

int main(void)
{
    read_cfg(COMMON_CFG_PATH);      //to find bitfield length
    int sock_fd = open_socket_and_listen(PORT);
    if (sock_fd < 0) {
        fprintf(stderr, "server: open_socket() failed");
        exit(1);
    }
    int new_fd;  // new connection on new_fd

    printf("server: waiting for a connection...\n");

    new_fd = accept(sock_fd, NULL, NULL);
    if (new_fd == -1) {
        perror("accept");
        exit(1);
    }

    close(sock_fd);
    send_messages(new_fd);
    close(new_fd);

    return 0;
}
