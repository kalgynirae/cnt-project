#ifndef _sender_h
#define _sender_h

#include "message.h"

/* helper functions for sending messages
 * All message sending functions take sock_fd,
 * which is the socket file descriptor of the desired recipient
 */

/* send a handshake message
 * sender_id: peer id of peer sending handshake */
int send_handshake(int sock_fd, int sender_id);

/* Normal Messages */

/* deselect a preferred neighbor. No payload. */
int send_choke(int sock_fd);

/* select a preferred neighbor. No payload. */
int send_unchoke(int sock_fd);

/* notify a neighbor it has pieces that the sender lacks. No payload. */
int send_interested(int sock_fd);

/* notify a neighbor it has no pieces that the sender lacks. No payload. */
int send_not_interested(int sock_fd);

/* advertise ownership of piece. Payload: 4-byte piece index
 * piece_idx is the index of the advertised piece*/
int send_have(int sock_fd, int piece_idx);

/* first message after connection established
 * each bit of bitfield is 0 for a missing piece or 1 for an owned piece*/
int send_bitfield(int sock_fd, bitfield_t bitfield);

/* request piece. Payload: 4-byte index of desired piece */
int send_request(int sock_fd, int piece_idx);

/* send content. Payload: 4-byte piece index + content */
int send_piece(int sock_fd, int piece_idx, unsigned char content[]);

#endif
