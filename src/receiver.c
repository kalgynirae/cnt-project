#include "receiver.h"

/* read a message from the socked descriptor sockfd
 * return the type of the message, or MSG_INVALID if an error occurs
 * point length to the length of the payload (0 if no payload)
 * read the payload into the supplied buffer
 */
message_t recv_msg(int sockfd, unsigned int *payload_len,
                   unsigned char *payload)
{
    int nbytes;     //# bytes received
    unsigned char header[HEADER_SIZE];

	if ((nbytes = recv(sockfd, header, HEADER_SIZE, 0)) != HEADER_SIZE) {
        fprintf(stderr, "recv_msg(): could not recv incoming message header\n");
    }

    //is it a handshake?
    if (strncmp((char*)header, HS_GREETING, HEADER_SIZE) == 0)
    {
        //recv padding
        char padding[HS_PADDING_LEN];
        if ((nbytes = recv(sockfd, padding, HS_PADDING_LEN, 0)) != HS_PADDING_LEN)
        {
            fprintf(stderr, "could not recv incoming handshake padding\n");
        }

        //get payload (sender id)
        *payload_len = PEER_ID_LEN;
        if (recv_payload(sockfd, payload, PEER_ID_LEN) != PEER_ID_LEN) { 
            return INVALID_MSG; 
        }

        return HANDSHAKE;
    }

    //must be a normal message
    //extract type field
    message_t type = header[MSG_TYPE_POS];
    //extract length subtract size of type field to get payload length
    *payload_len = unpack_int(header + MSG_LEN_POS) - MSG_TYPE_LEN;

    if (*payload_len == 0)          //no content beyond type field
    {
        payload = NULL;
    }
    else
    {
        recv_payload(sockfd, payload, *payload_len);
    }

    return type;       //return message type
}

//extract int from payload
unsigned int unpack_int(unsigned char bytes[4])
{
    int i;
    memcpy(&i, bytes, 4);
    return ntohl(i);
}

//extract bitfield from payload
bitfield_t unpack_bitfield(unsigned char bytes[1])
{
    return 0;   //TODO
}

//extract and save content from piece payload
void extract_and_save_piece(unsigned int len, unsigned char payload[], int my_id)
{
    unsigned int idx = unpack_int(payload);         //piece index
    int content_len = len - PIECE_IDX_LEN;          //content length 
    char *content = (char*)payload + PIECE_IDX_LEN; //ptr to first byte of content

    write_piece(idx, content_len, content, my_id);
}

//receive the payload of a message
int recv_payload(int sockfd, unsigned char *buf, int length)
{
    int nbytes, so_far = 0;
    while (so_far < length)
    {
        if ((nbytes = recv(sockfd, buf + so_far, length - so_far, 0)) < 0) {
            fprintf(stderr, "could not receive payload of incoming message\n");
            return so_far;
        }
        so_far += nbytes;
    }
    return so_far;
}
