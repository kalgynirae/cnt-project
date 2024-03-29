#ifndef _init_h
#define _init_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "message.h"

#define HANDSHAKE_TIMEOUT_TIME 5

#define HOSTNAME_LEN 512      //# of chars in host name
#define PORT_DIGITS 6

// common configuration options for p2p session
struct common_cfg
{
    int n_preferred_neighbors; 
    int unchoke_interval;
    int optimistic_unchoke_interval;
    char* file_name;
    int file_size;
    int piece_size;
};

struct peer_info
{
    int peer_id;        // unique identifier of peer
    char hostname[HOSTNAME_LEN];     // host name
    char port[PORT_DIGITS];           // port on which peer listens
    int has_file;       // 1 if peer has complete file, else 0
    int state;          // current state of peer
    int to_fd;          // descriptor of open socket for sending to peer
    int from_fd;        // descriptor of open socket for receiving from peer
    int time_last_message_sent; // time in seconds since THE EPOCH, using time()
    bitfield_t bitfield;
    int requested;     // piece index requested from this peer
    int pieces_this_interval;   // # of pieces completed this preferred unchoking interval
    int optimistic_flag;        // 1 if this peer was optimistically unchoked, 0 otherwise
    int choked_by_us;           // whether the peer is choked by us
    int interested_in_us;
};

// read config file and return struct containing options
void read_cfg(char* cfg_file_name);

/* read config file and assign array of peer_info structs to result
 * returns number of peers
 */
struct peer_info* read_peers(char* cfg_file_name, int* num_peers,
                             int our_peer_id, int *we_have_file, char *our_port);
                             

// parse a line of the common config file
void parse_cfg_line(char *line, struct common_cfg *cfg);
// parse a line of the peer info file
struct peer_info parse_peer_line(char *line);

//initialize bitfield based on whether or not piece is owned
void init_bitfield(bitfield_t bitfield, int has_file);
#endif
