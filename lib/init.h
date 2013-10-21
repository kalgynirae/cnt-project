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

//read config file and return struct containing options
struct common_cfg read_cfg(char* cfg_file_name);
//read config file and return struct containing options
void parse_cfg_line(char *line, struct common_cfg *cfg);
#endif
