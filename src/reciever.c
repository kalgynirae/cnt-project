#include "reciever.h"

/* Try to parse data recieved from a message as a handshake
 * If successful, return id of peer who sent handshake
 * Else, return -1
 */
int parse_handshake_msg(char message[], int n_bytes)
{
    return -1;
}

 /*
  * Try to parse data recieved from a message as a normal message
  * return 1 on success, 0 on failure
  * place parsed message in mess
 */
int parse_normal_msg(char message[], int n_bytes, struct mess_normal *mess)
{
    return -1;
}


/* Check a recieved BITFIELD message for an interesting piece.
 * Return the index of the first interesting piece, or -1 if none is found.
 * returns -2 if bitfield_msg.type != BITFIELD
 */
int find_interesting_piece(bitfield_t self_bits, struct mess_normal bitfield_msg)
{
    return -1;
}
