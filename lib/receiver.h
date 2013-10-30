#ifndef _receiver_h
#define _receiver_h

#include "message.h"

/* helper functions for sending messages
 */

//error codes
#define NO_INTERESTING_PIECE -1
#define INCORRECT_MSG_TYPE -2

/* Try to parse data received from a message as a handshake
 * If successful, return id of peer who sent handshake
 * Else, return -1
 */
int parse_handshake_msg(char message[], int n_bytes);

 /*
  * Try to parse data received from a message as a normal message
  * return 1 on success, 0 on failure
  * place parsed message in mess
 */
int parse_normal_msg(char message[], int n_bytes, struct mess_normal *mess);


/* Check a received BITFIELD message for an interesting piece.
 * Return the index of the first interesting piece, or -1 if none is found.
 * returns -2 if bitfield_msg.type != BITFIELD
 */
int find_interesting_piece(bitfield_t self_bits, struct mess_normal bitfield_msg);


#endif
