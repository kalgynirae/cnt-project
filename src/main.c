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
    peers = read_peers(PEER_CFG_PATH, &num_peers, our_peer_id, &we_have_file);
    if (we_have_file)
    {   //divide file into segments, save to peer directory
        file_split(g_config.file_name, 
                g_config.file_size, 
                g_config.piece_size, 
                our_peer_id);
    }

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

    char bagels[g_bitfield_len];
    bitfield_t our_bitfield = (bitfield_t) bagels;
    init_bitfield(our_bitfield, we_have_file);

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
    unsigned char payload[g_config.piece_size];
    unsigned int payload_len;

    // Var for storing the number of the peer each time we receive data
    int peer_n;

    // Info for doing peer choking/unchoking
    time_t last_p_interval_start = time(NULL);
    time_t last_m_interval_start = time(NULL);
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
        read_fds = master;

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
                        if (socket == peers[peer_n].socket_fd)
                            break;
                    }
                    if (peer_n < num_peers)
                    {
                        fprintf(stderr, "Receiving data from peer %d\n", peer_n);

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
                        fprintf(stderr, "Receiving data to a socket not yet "
                                        "assigned to a peer\n");

                        message_t type = recv_msg(socket, &payload_len, payload);
                        if (type != HANDSHAKE)
                        {
                            fprintf(stderr, "Received a non-handshake to a "
                                            "new socket?!? um...\n");
                        }

                        unsigned int new_peer_id = unpack_int(payload);
                        for (i = 0; i < num_peers; i++)
                        {
                            if (peers[i].peer_id == new_peer_id) {
                                peers[i].socket_fd = socket;
                            }
                        }
                    }
                }
            }
        }

        // Call the periodic handler for every peer
        for (i = 0; i < num_peers; i++)
        {
            peer_handle_periodic(&peers[i], our_peer_id, our_bitfield, peers, num_peers);
        }

        // Calculate new preferred peers
        int k = g_config.n_preferred_neighbors;
        if (num_peers < k)
        {
            k = num_peers;
        }
        int p = g_config.unchoke_interval;
        if ((time(NULL) - last_p_interval_start) >= p) // p time has elapsed
        {
            last_p_interval_start = time(NULL);
            // find the k fastest peers, store in preferred_indexes
            int i, j;
            int preferred_ids[k];
            int preferred_indexes[k];
            memset(preferred_indexes, 0, sizeof(int) * k);
            for (i = 0; i < num_peers; i++)
            {
                /* Check whether we are interested in the ith peer, otherwise,
                 * just continue to the next peer.
                 */
                int interesting = find_interesting_piece(our_bitfield,
                                                         peers[i].bitfield);
                if (interesting == INCORRECT_MSG_TYPE)
                {
                    fprintf(stderr, "peer_handle_periodic(): incompatible "
                                    "message type\n");
                }
                else if (interesting == NO_INTERESTING_PIECE)
                {
                    // Nothing special
                }
                else // they are interesting!
                {
                    // TODO: Fix this. It is broken.
                    for (j = 0; j < k; j++)
                    {
                        if (peers[i].pieces_this_interval >
                                peers[preferred_indexes[j]].pieces_this_interval)
                        {
                            if (j == k-1) // case that this peer is fastest
                            {
                                preferred_indexes[j] = i;
                            }
                        }
                        else // found the first peer we're slower than
                        {
                            if (j > 0)
                            {
                                preferred_indexes[j-1] = i;
                            }
                            break;
                        }
                    }
                }

                peers[i].pieces_this_interval = 0;
            }
            /* now loop through the peers a second time, choking those who are
             * no longer preferred and unchoking those who need to be preferred
             */
            for (i = 0; i < num_peers; i++)
            {
                // find index of peer[i] in preferred_indexes
                for (j = 0; j < k; j++)
                {
                    if (preferred_indexes[j] == i)
                    {
                        break;
                    }
                }
                // j is now the index of the peer in pref_ids if it is, or k if
                // not
                if (peers[i].state == PEER_WAIT_UNCHOKED)
                {
                    if (j == k) // send choke to old preferred peer
                    {
                        send_choke(peers[i].peer_id);
                    }
                }
                else if (peers[i].state == PEER_CHOKED)
                {
                    if (j < k) // send unchoke to new preferred peer
                    {
                        peers[i].optimistic_flag = 0; // mark peer as preferred
                        send_unchoke(peers[i].peer_id);
                    }
                }
            }

            // Generate an array of actual peer ids for logging
            for (i = 0; i < k; i++)
            {
                preferred_ids[i] = peers[preferred_indexes[i]].peer_id;
            }
            log_change_preferred(our_peer_id, k, preferred_ids);
        }

        //// Calculate new optimistic peer
        //int m = g_config.optimistic_unchoke_interval;
        //if ((time(NULL) - last_m_interval_start) >= m)  // m time has elapsed
        //{
        //    last_m_interval_start = time(NULL);
        //    // pick a random index in range(num_peers), check if interested
        //    int rand_index;
        //    for (;;)
        //    {
        //        rand_index = rand() % num_peers;
        //        int interesting = find_interesting_piece(our_bitfield,
        //                peers[rand_index].bitfield);
        //        if (interesting == INCORRECT_MSG_TYPE)
        //        {
        //            fprintf(stderr, "peer_handle_periodic(): incompatible "
        //                            "message type\n");
        //        }
        //        else if (interesting == NO_INTERESTING_PIECE)
        //        {
        //            continue;
        //        }
        //        else // now find a choked peer
        //        {
        //            if (peers[rand_index].state == PEER_CHOKED)
        //            {
        //                // choke last optimistic peer if they're still
        //                // optimistic
        //                if (last_optimistic_peer < 0)
        //                {
        //                    // handle case when we begin
        //                    last_optimistic_peer = rand_index;
        //                }
        //                if (peers[last_optimistic_peer].optimistic_flag == 1)
        //                {
        //                    peers[last_optimistic_peer].optimistic_flag = 0;
        //                    send_choke(last_optimistic_peer);
        //                }
        //                peers[rand_index].optimistic_flag = 1;
        //                last_optimistic_peer = rand_index;
        //                send_unchoke(rand_index);
        //                break;
        //            }
        //            else
        //            {
        //                // don't choose someone already unchoked for
        //                // optimistic peer
        //                continue;
        //            }
        //        }
        //    }
        //    log_optimistic_unchoke(our_peer_id, rand_index);
        //}

    } // End of select() loop
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
