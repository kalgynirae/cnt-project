#ifndef _peer_h
#define _peer_h

#include <time.h>
#include <stdlib.h>
#include "init.h"
#include "receiver.h"

int peer_handle_data(struct peer_info *peer, message_t msg_type,
        unsigned char *data, int nbytes, bitfield_t bitfield);
                     
int peer_handle_periodic(struct peer_info *peer);

//randomly choose a piece index owned by other peer and not us
//return -1 if no interesting piece
int find_interesting_piece(bitfield_t my_bitfield, bitfield_t other_bitfield);

//returns a non-zero value if my_bitfield has a bit set for the piece idx
int has_piece(int idx, bitfield_t my_bitfield);

#endif
