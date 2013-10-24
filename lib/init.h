#ifndef _init_h
#define _init_h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//common configuration options for p2p session
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
    int peer_id;        //unique identifier of peer
    char* hostname;     //host name
    int port;           //port on which peer listens
    int has_file;       //1 if peer has complete file, else 0
    int state;          //current state of peer
    int socket_fd;      //descriptor of open socket for sending to peer
};

//read config file and return struct containing options
struct common_cfg read_cfg(char* cfg_file_name);

/* read config file and assign array of peer_info structs to result
 * returns number of peers
 */
struct peer_info* read_peers(char* cfg_file_name, int* num_peers);

//parse a line of the common config file
void parse_cfg_line(char *line, struct common_cfg *cfg);
//parse a line of the peer info file
struct peer_info parse_peer_line(char *line);
#endif
