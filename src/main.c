#include <stdio.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
#include "init.h"
#include "message.h"
#include "peer_log.h"
#include "peer.h"

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
    int receiving_socket_fd = open_socket_and_listen(RECEIVE_PORT);

    while (true) {
        /*
         * Set up parameters for select()
         */
        // We want to timeout after five seconds of waiting
        struct timeval tv;
        tv.tv_sec = 5;
        tv.tv_usec = 0;

        // Construct the list of file descriptors
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN, &read_fds);

        // Call select()
        select(...);

        // Figure out which peer
        peer_info *the_peer = ...;

        // Handle the request

    }
    return 0;
}
