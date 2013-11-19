#ifndef _peer_h
#define _peer_h

#include <time.h>
#include "init.h"
#include "receiver.h"

int peer_handle_data(struct peer_info *peer, message_t msg_type,
        unsigned char *data, int nbytes, bitfield_t bitfield);
                     
int peer_handle_periodic(struct peer_info *peer);

int find_interesting_piece(bitfield_t my_bitfield, bitfield_t other_bitfield);

#endif
