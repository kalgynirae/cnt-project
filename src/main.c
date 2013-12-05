#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
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
extern int g_bitfield_len;

void ensure_peer_dir_exists(int id);

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <peer_id>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct peer_info *peers = NULL;

    int num_peers;
    int i;

    read_cfg(COMMON_CFG_PATH);

    printf("========== CONFIG ========== \n");
    printf("NumberOfPreferredNeighbors = %d\n", g_config.n_preferred_neighbors);
    printf("UnchokingInterval = %d\n", g_config.unchoke_interval);
    printf("OptimisticUnchokingInterval = %d\n", 
            g_config.optimistic_unchoke_interval);
    printf("FileName = %s\n", g_config.file_name);
    printf("FileSize = %d\n", g_config.file_size);
    printf("PieceSize = %d\n", g_config.piece_size);

    int our_peer_id = atoi(argv[1]);
    //make sure runtime/peer_{id} exists
    ensure_peer_dir_exists(our_peer_id);
    
    int we_have_file;
    char our_port[PORT_DIGITS];
    peers = read_peers(PEER_CFG_PATH, &num_peers, our_peer_id, &we_have_file, 
            our_port);
    if (peers == NULL)
    {
        fprintf(stderr, "Exiting after read_peers() error.\n");
        exit(EXIT_FAILURE);
    }

    // Initialize peer bitfields
    for (i = 0 ; i < num_peers ; i++)
    {
        peers[i].bitfield = malloc(g_bitfield_len);
        init_bitfield(peers[i].bitfield, 0);
    }

    // Split file into pieces if necessary
    if (we_have_file)
    {   //divide file into segments, save to peer directory
        file_split(g_config.file_name, 
                g_config.file_size, 
                g_config.piece_size, 
                our_peer_id);
    }

    // Print our info
    printf("========== US ==========\n");
    printf("our_peer_id: %d\n", our_peer_id);
    printf("our_port: %s\n", our_port);
    printf("we_have_file: %d\n", we_have_file);

    // Print peer info
    for (i = 0 ; i < num_peers ; i++)
    {
        struct peer_info info = peers[i];

        printf("========== PEERS ==========\n");
        printf("peer_id: %d\n", info.peer_id);
        printf("  hostname: %s\n", info.hostname);
        printf("  port: %s\n", info.port);
        printf("  has_file: %d\n", info.has_file);
        printf("  state: %d\n", info.state);
        printf("  to_fd: %d\n", info.to_fd);
        printf("  from_fd: %d\n", info.from_fd);
    }

    char bagels[g_bitfield_len];
    bitfield_t our_bitfield = (bitfield_t) bagels;
    init_bitfield(our_bitfield, we_have_file);

    /*
     * Open a socket from which to receive things
     */
    int listen_socket_fd = open_socket_and_listen(our_port);
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
    unsigned char payload[g_config.piece_size];
    unsigned int payload_len;

    // Var for storing the number of the peer each time we receive data
    int peer_n;

    // Info for doing peer choking/unchoking
    time_t last_unchoke_time = time(NULL);
    int last_unchoke_index = -1;
    time_t last_optimistic = time(NULL);
    int last_optimistic_peer = -1;

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
        // We want to timeout after one second of waiting. This has to be
        // reset each time because select() modifies it.
        struct timeval tv;
        tv.tv_sec = 1;
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
        int socket;
        for (socket = 0; socket <= max_fd; socket++)
        {
            if (FD_ISSET(socket, &read_fds))
            {
                fprintf(stderr, "main: socket %d is flagged\n", socket);

                if (socket == listen_socket_fd)
                {
                    // This is the listening socket, so we're going to call
                    // accept() to open a new socket.
                    int new_fd = accept(listen_socket_fd, NULL, NULL);
                    if (new_fd == -1)
                    {
                        perror("accept()");
                        exit(EXIT_FAILURE);
                    }
                    fprintf(stderr, "main: accept()'d to new socket %d\n",
                            new_fd);

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
                        if (socket == peers[peer_n].from_fd)
                            break;
                    }
                    if (peer_n < num_peers)
                    {
                        fprintf(stderr, "main: Receiving data from peer %d\n",
                                peer_n);

                        message_t type = recv_msg(socket, &payload_len, payload);
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
                        message_t type = recv_msg(socket, &payload_len, payload);
                        if (type != HANDSHAKE)
                        {
                            fprintf(stderr, "main: Received a non-handshake to "
                                            "a new socket. I can't do anything "
                                            "with this.\n");
                            continue;
                        }

                        unsigned int new_peer_id = unpack_int(payload);

                        fprintf(stderr, "main: Received handshake from peer %d\n",
                                new_peer_id);

                        for (i = 0; i < num_peers; i++)
                        {
                            if (peers[i].peer_id == new_peer_id)
                            {
                                break;
                            }
                        }
                        if (i == num_peers)
                        {
                            fprintf(stderr, "%d is not a valid peer id; "
                                            "ignoring\n", i);
                            continue;
                        }

                        fprintf(stderr, "set from_fd=%d for peer %d\n",
                                socket, new_peer_id);
                        peers[i].from_fd = socket;

                        peer_handle_data(&peers[i], type, payload,
                                         payload_len, our_bitfield, peers,
                                         num_peers, our_peer_id);
                    }
                }
            }
        }

        // Call the periodic handler for every peer
        for (i = 0; i < num_peers; i++)
        {
            peer_handle_periodic(&peers[i], our_peer_id, our_bitfield, peers, num_peers);
        }

        if (time(NULL) - last_unchoke_time > g_config.unchoke_interval)
        {
            if (we_have_file)
            {
                // Pick random preferred neighbors (this isn't random; it just
                // cycles, but it's probably close enough)
                last_unchoke_time = time(NULL);
                last_unchoke_index = (last_unchoke_index + 1) % num_peers;
                for (i = 0; i < g_config.n_preferred_neighbors; i++)
                {
                    send_unchoke(peers[(last_unchoke_index + i) % num_peers].to_fd);
                }
                for (i = g_config.n_preferred_neighbors; i < num_peers; i++)
                {
                    send_choke(peers[(last_unchoke_index + i) % num_peers].to_fd);
                }
            }
            else
            {
                // TODO: Calculate preferred neighbors
                last_unchoke_time = time(NULL);
                last_unchoke_index = (last_unchoke_index + 1) % num_peers;
                for (i = 0; i < g_config.n_preferred_neighbors; i++)
                {
                    send_unchoke(peers[(last_unchoke_index + i) % num_peers].to_fd);
                }
                for (i = g_config.n_preferred_neighbors; i < num_peers; i++)
                {
                    send_choke(peers[(last_unchoke_index + i) % num_peers].to_fd);
                }
            }
        }

    } // End of select() loop

    //free bitfields
    for (i = 0 ; i < num_peers ; i++)
    {
        free(peers[i].bitfield);
    }
    return 0;
}

void ensure_peer_dir_exists(int id)
{
    struct stat st = {0};

    char dir[32];
    sprintf(dir, "runtime/peer_%d", id);
    if (stat(dir, &st) == -1) {
        mkdir(dir, 0700);
    }
}
