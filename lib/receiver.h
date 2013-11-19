#ifndef _receiver_h
#define _receiver_h

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include "message.h"

/* helper functions for receiving messages
 */

//minimum number of bytes to recieve at a time
#define MIN_RECV_SIZE 32
#define RECV_FAIL -1

//error codes
#define NO_INTERESTING_PIECE -1
#define INCORRECT_MSG_TYPE -2

/* read a message from the socked descriptor sockfd
 * return the type of the message, or MSG_INVALID if an error occurs
 * point length to the length of the payload (0 if no payload)
 * point payload to a malloc'd buffer containng the payload (or NULL if no payload)
 * remember to free payload after use!!
 */
message_t recv_msg(int sockfd, unsigned int *payload_len, unsigned char **payload);

//extract int from payload
unsigned int unpack_int(unsigned char bytes[4]);

//extract bitfield from payload
bitfield_t unpack_bitfield(unsigned char bytes[1]);

//extract and save content from piece payload
void extract_and_save_piece(unsigned int len, char payload[]);

#endif
