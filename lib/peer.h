#ifndef _peer_h
#define _peer_h

#include <time.h>
#include <stdlib.h>
#include "init.h"
#include "receiver.h"
#include "sender.h"
#include "socket.h"
#include "peer_log.h"

enum {
    PEER_NOT_CONNECTED,
    PEER_WAIT_FOR_HANDSHAKE,
    PEER_WAIT_FOR_BITFIELD,
    PEER_CHOKED,
    PEER_WAIT_UNCHOKED
};

void peer_handle_data(struct peer_info *peer, message_t msg_type,
        unsigned char *payload, int nbytes, bitfield_t our_bitfield,
        struct peer_info *peers, int num_peers, int our_peer_id);

void peer_handle_periodic(struct peer_info *peer, int our_peer_id,
        bitfield_t our_bitfield, struct peer_info *peers, int num_peers);

//randomly choose a piece index owned by other peer and not us
//return -1 if no interesting piece
int find_interesting_piece(bitfield_t my_bitfield, bitfield_t other_bitfield,
        struct peer_info *peers, int n_peers);

//returns a non-zero value if bitfield has a bit set for the piece idx
int bitfield_get(bitfield_t bitfield, int idx);

//set the bit at idx in bitfield to 1
int bitfield_set(bitfield_t bitfield, int idx);

//initialize bitfield
void init_bitfield(bitfield_t bitfield, int has_piece);

//return 1 if bitfield full (has all pieces). Else return 0.
int bitfield_filled(bitfield_t bitfield);

//print bitfield
void print_bitfield(FILE *stream, bitfield_t bitfield);

#endif
