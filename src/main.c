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

#define COMMON_CFG_PATH "config/Common.cfg"
#define PEER_CFG_PATH "config/PeerInfo.cfg"
#define RECEIVE_PORT "9007"

int main(int argc, char *argv[])
{
    struct common_cfg cfg;
    struct peer_info *peers = NULL;
    //track pieces that this peer has
    bitfield_t bitfield;

    /* incoming normal message. handshake needs to be handled separately
     * probably need to read in as bytestream, then cast to correct message
     * struct after checking if first 5 bytes == "HELLO"
     */
    struct mess_normal msg_in;

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
    //number of bytes read
    int nbytes;

    // Var for storing the number of the peer each time we receive data
    int peer_n;

    /*
     * select() loop
     *
     * This stuff is basically straight from the Wikipedia page for select()
     */
    for (;;)
    {
        read_fds = master;

        peer_n = -1;

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
                    // Let's figure out which peer this socket belongs to.
                    for (peer_n = 0; peer_n < num_peers; peer_n++)
                    {
                        if (i == peers[peer_n].socket_fd)
                            break;
                    }
                    if (peer_n < num_peers)
                    {
                        fprintf(stderr, "Received data from peer %d",
                                peer_n);
                    }
                    else
                    {
                        fprintf(stderr, "Received data to a non-peer socket");
                        peer_n = -1;
                    }

                    // This is not the listening socket, so we'll receive data
                    // from it and print it.
                    nbytes = recv(i, buffer, sizeof(buffer), 0);
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

        struct peer_info the_peer;
        if (peer_n > 0)
        {
            // Figure out which peer
            the_peer = peers[peer_n];
            //id of peer that sent message
            int sender;

            if (the_peer.state == PEER_WAIT_FOR_HANDSHAKE)
            {
                // Transition to bitfield if rcv'd handshake and handshake is valid
                if ((sender = parse_handshake_msg(buffer, nbytes)) >= 0)
                {
                    // Send bitfield
                    the_peer.time_last_message_sent = time(NULL);
                    the_peer.state = PEER_WAIT_FOR_BITFIELD;
                }
            }
            else if (parse_normal_msg(buffer, nbytes, &msg_in))
            {
                // Rcv'd bitfield 
                if (the_peer.state == PEER_WAIT_FOR_BITFIELD
                        && msg_in.type == BITFIELD)
                {
                    //index of interesting piece
                    int interesting = find_interesting_piece(bitfield, msg_in); 
                    if (interesting == INCORRECT_MSG_TYPE)
                    {
                        fprintf(stderr, "incompatible message type");
                    }
                    else if (interesting == NO_INTERESTING_PIECE)
                    {
                        // Send not interested
                    }
                    else 
                    {
                        // Send interested
                    }
                    the_peer.state = PEER_CHOKED;
                }
            }
        }

////////// Do timeout stuff
        for (i = 0; i < num_peers; i++)
        {
            the_peer = peers[i];

            // No FD will trigger when the Peer is not connected
            if (the_peer.state == PEER_NOT_CONNECTED)
            {
                // Send our handshake message
                // Start a timer and attach it to the peer_info struct
                the_peer.time_last_message_sent = time(NULL);
                the_peer.state = PEER_WAIT_FOR_HANDSHAKE;
            }
            else if (the_peer.state == PEER_WAIT_FOR_HANDSHAKE)
            {
                // Self-edge when timeout occurs, re-send handshake
                if (time(NULL) - the_peer.time_last_message_sent >= HANDSHAKE_TIMEOUT_TIME)
                {
                    // Send our handshake message
                    // Start a timer and attach it to the peer_info struct
                    the_peer.time_last_message_sent = time(NULL);
                }
            }
            else if (the_peer.state == PEER_WAIT_FOR_BITFIELD)
            {
                // In the event of a timeout, go back to state 0, implying that no
                // bitfield was sent because the peer has no interesting pieces.
                if (time(NULL) - the_peer.time_last_message_sent 
                        >= BITFIELD_TIMEOUT_TIME)
                {

                }
            }
        }
    }
    return 0;
}
