#include "receiver.h"

/* Try to parse data received from a message as a handshake
 * If successful, return id of peer who sent handshake
 * Else, return -1
 */
int parse_handshake_msg(char message[], int n_bytes)
{
    if (n_bytes != 32 || strncmp(message, "HELLO", 5) != 0)
    {
        return -1;
    }
    int i;
    for (i = 5 ; i < 28 ; i++)
    {
        if (message[i] != 0)
        {
            return -1;
        }
    }
    char sender_id[5];
    strncpy(sender_id, message + 28, 4);
    sender_id[4] = '\0';

    return atoi(sender_id);
}

 /*
  * Try to parse data received from a message as a normal message
  * return 1 on success, 0 on failure
  * place parsed message in mess
 */
int parse_normal_msg(char message[], int n_bytes, struct mess_normal *mess)
{
    return -1;
}


/* Check a received BITFIELD message for an interesting piece.
 * Return the index of the first interesting piece, or -1 if none is found.
 * returns -2 if bitfield_msg.type != BITFIELD
 */
int find_interesting_piece(bitfield_t self_bits, struct mess_normal bitfield_msg)
{
    return -1;
}
