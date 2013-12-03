#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include "init.h"
#include "sender.h"
#include "receiver.h"
#include "peer_log.h"
#include "socket.h"
#include "peer.h"
#include "peer_log.h"

#define COMMON_CFG_PATH "config/Common.cfg"
#define PEER_CFG_PATH "config/PeerInfo.cfg"
#define RECEIVE_PORT "9007"

//global configuration options
extern struct common_cfg g_config;

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <peer_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct peer_info *peers = NULL;
    bitfield_t our_bitfield;

    int num_peers;
    int i;

    read_cfg(COMMON_CFG_PATH);

    printf("NumberOfPreferredNeighbors = %d\n", g_config.n_preferred_neighbors);
    printf("UnchokingInterval = %d\n", g_config.unchoke_interval);
    printf("OptimisticUnchokingInterval = %d\n", 
            g_config.optimistic_unchoke_interval);
    printf("FileName = %s\n", g_config.file_name);
    printf("FileSize = %d\n", g_config.file_size);
    printf("PieceSize = %d\n", g_config.piece_size);

    int our_peer_id = atoi(argv[1]);
    peers = read_peers(PEER_CFG_PATH, &num_peers, our_peer_id);
    if (peers == NULL)
    {
        fprintf(stderr, "Exiting after read_peers() error.\n");
        exit(EXIT_FAILURE);
    }

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

    /*
     * Test the various log message functions
     */
    log_connect(1000, 1001);
    int preferred[3] = {1003, 1004, 1005};
    log_change_preferred(1000, 3, preferred);
    log_optimistic_unchoke(1000, 1001);
    log_unchoked_by(1000, 1001);
    log_receive_choke(1000, 1001);
    log_received_have(1000, 1001);
    log_received_interested(1000, 1001);
    log_received_not_interested(1000, 1001);
    log_downloaded_piece(1000, 1001);
    log_downloaded_file(1000);

    /*
     * Open a socket from which to receive things
     */
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
    unsigned char payload[BUFSIZ];
    int payload_len;

    // Var for storing the number of the peer each time we receive data
    int peer_n;

    /*
     * select() loop
     *
     * Each iteration of this loop does the following:
     *   * calls select()
     *   * loops through all open sockets and checks with select() whether each
     *     socket can be read from, and if yes:
     *       * recv()s from the socket into a buffer
     *       * figures out which peer the socket belongs to
     *       * calls peer_handle_data() passing that peer's peer_info
     *   * calls peer_handle_periodic() for every peer
     *
     * A lot of this code is straight from the Wikipedia page for select().
     */
    for (;;)
    {
        read_fds = master;

        // We want to timeout after five seconds of waiting. This has to be
        // reset each time because select() modifies it.
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // Dunno why this is necessary, but we've confirmed that it is.
        memcpy(&read_fds, &master, sizeof(master));

        // Call select()
        int nready = select(max_fd + 1, &read_fds, NULL, NULL, &tv);
        if (nready == -1)
        {
            perror("select()");
            exit(EXIT_FAILURE);
        }

        /*
         * Loop through sockets (by file descriptor) and see if each is ready
         * to be recv()'d from (or, in the case of the listen_socket_fd,
         * accept()'d from).
         */
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

                    // Add new socket to the set and increase max_fd
                    FD_SET(new_fd, &master);
                    if (max_fd < new_fd)
                    {
                        max_fd = new_fd;
                    }
                }
                else
                {
                    // Let's figure out which peer this socket belongs to.
                    for (peer_n = 0; peer_n < num_peers; peer_n++)
                    {
                        if (i == peers[peer_n].socket_fd)
                            break;
                    }
                    if (peer_n < num_peers)
                    {
                        fprintf(stderr, "Receiving data from peer %d\n", peer_n);

                        message_t type = recv_msg(i, &payload_len, payload);
                        if (type == INVALID_MSG)
                        {
                            fprintf(stderr, "recv_msg() failed\n");
                            break; // Move on to the next socket
                        }

                        peer_handle_data(&peers[peer_n], type, payload,
                                         payload_len, our_bitfield, peers,
                                         num_peers, our_peer_id);
                    }
                    else
                    {
                        fprintf(stderr, "Receiving data to a socket not yet "
                                        "assigned to a peer\n");

                        // TODO: Receive a handshake message, extract the peer
                        // ID, and save `i` as that peer's socket_fd
                    }
                }
            }
        }

        // Call the periodic handler for every peer
        for (i = 0; i < num_peers; i++)
        {
            peer_handle_periodic(&peers[i], our_peer_id, our_bitfield, peers, num_peers);
        }
    }
    return 0;
}
