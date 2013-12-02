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

//returns a non-zero value if bitfield has a bit set for the piece idx
int bitfield_get(bitfield_t bitfield, int idx);

//set the bit at idx in bitfield to 1
int bitfield_set(bitfield_t bitfield, int idx);

#endif
