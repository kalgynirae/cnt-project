#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include "init.h"
#include "message.h"
#include "peer_log.h"
#include "socket.h"

#define COMMON_CFG_PATH "config/Common.cfg"
#define PEER_CFG_PATH "config/PeerInfo.cfg"
#define RECEIVE_PORT "9007"

int main(int argc, char *argv[])
{
    struct common_cfg cfg;
    struct peer_info *peers = NULL;
    int num_peers;
    int i;

    cfg = read_cfg(COMMON_CFG_PATH);

    printf("NumberOfPreferredNeighbors = %d\n", cfg.n_preferred_neighbors);
    printf("UnchokingInterval = %d\n", cfg.unchoke_interval);
    printf("OptimisticUnchokingInterval = %d\n", cfg.optimistic_unchoke_interval);
    printf("FileName = %s\n", cfg.file_name);
    printf("FileSize = %d\n", cfg.file_size);
    printf("PieceSize = %d\n", cfg.piece_size);

    peers = read_peers(PEER_CFG_PATH, &num_peers);

    for (i = 0 ; i < num_peers ; i++)
    {
        struct peer_info info = peers[i];

        printf("peer_id: %d\n", info.peer_id);
        printf("hostname: %s\n", info.hostname);
        printf("port: %d\n", info.port);
        printf("has_file: %d\n", info.has_file);
        printf("state: %d\n", info.state);
        printf("socket_fd: %d\n", info.socket_fd);
    }

    log_connect(1000, 1001);

    int preferred[3] = {1003, 1004, 1005};
    log_change_preferred(1000, 3, preferred);

    log_optimistic_unchoke(1000, 1001);

    log_unchoked_by(1000, 1001);

    log_recieve_choke(1000, 1001);

    log_recieved_have(1000, 1001);

    log_recieved_interested(1000, 1001);

    log_recieved_not_interested(1000, 1001);

    log_downloaded_piece(1000, 1001);

    log_downloaded_file(1000);

    // Open a socket from which to receive things
    int listen_socket_fd = open_socket_and_listen(RECEIVE_PORT);
    if (listen_socket_fd == -1)
    {
        fprintf(stderr, "open_socket_and_listen() failure\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Set up parameters for select()
     */
    // Construct the lists of file descriptors
    fd_set master, read_fds;
    FD_ZERO(&master);
    FD_ZERO(&read_fds);
    FD_SET(listen_socket_fd, &master);
    int max_fd = listen_socket_fd;

    // Allocate a buffer to store data read from socket
    char buffer[BUFSIZ];

    /*
     * select() loop
     *
     * This stuff is basically straight from the Wikipedia page for select()
     */
    for (;;)
    {
        read_fds = master;

        // We want to timeout after five seconds of waiting. This has to be
        // reset each time because select() modifies it.
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // Dunno why this is necessary
        memcpy(&read_fds, &master, sizeof(master));

        // Call select()
        int nready = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        fprintf(stderr, "select() returned, woo!\n");
        if (nready == -1)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }

        for (i = 0; i <= max_fd; i++)
        {
            if (FD_ISSET(i, &read_fds))
            {
                if (i == listen_socket_fd)
                {
                    // This is the listening socket, so we're going to call
                    // accept() to open a new socket.
                    int new_fd = accept(listen_socket_fd, NULL, NULL);
                    if (new_fd == -1)
                    {
                        perror("accept()");
                        exit(EXIT_FAILURE);
                    }

                    // Not sure why we call fcntl() here
                    //if (-1 == fcntl(new_fd, F_SETFD, O_NONBLOCK))
                    //{
                    //    perror("fcntl()");
                    //    exit(EXIT_FAILURE);
                    //}

                    // Add new socket to the set and increase max_fd
                    FD_SET(new_fd, &master);
                    if (max_fd < new_fd)
                    {
                        max_fd = new_fd;
                    }
                }
                else
                {
                    // This is not the listening socket, so we'll receive data
                    // from it and print it.
                    int nbytes = recv(i, buffer, sizeof(buffer), 0);
                    if (nbytes <= 0)
                    {
                        if (nbytes != 0)
                        {
                            perror("recv()");
                            exit(EXIT_FAILURE);
                        }
                        close(i);
                        FD_CLR(i, &master);
                    }
                    else
                    {
                        // Print the data
                        buffer[nbytes] = '\0';
                        printf("%s", buffer);
                    }
                }
            }
        }

        // Figure out which peer
        //struct peer_info *the_peer = ...;

        // Handle the request

    }
    return 0;
}
