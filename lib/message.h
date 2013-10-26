#ifndef _message_h
#define _message_h

enum {
    CHOKE,          //deselect preferred neighbors. No payload.
    UNCHOKE,        //select preferred neighbors. No payload.
    INTERESTED,     //notify a neighbor it has pieces that the sender lacks
    NOT_INTERESTED, //notify a neighbor it has no pieces that the sender lacks
    HAVE,           //advertise ownership of piece. Payload: 4-byte piece index
    BITFIELD,       //first message after connection established
    REQUEST,        //request piece. Payload: 4-byte index of desired piece
    PIECE           //send content. Payload: 4-byte piece index + content
};

typedef unsigned char message_t;    //unsigned char is a byte

struct mess_handshake {
    char header[5];     //5-byte string "HELLO"
    char zeros[23];     //23 bytes of zeroes
    int peer_id;        //4-byte peer id
};

struct mess_normal {
    int length;                 //message length NOT INCLUDING length field itself
    message_t type;             //type of message
    unsigned char payload[];    //variable sized payload
};

/* NOTES
 * When a peer recives a HAVE or BITFIELD message, it sends and INTERESTED 
 * method if the sender has a piece it is missing.
 * When a peer recieves a complete piece, it checks the bitfields of neighbors
 * and sends NOT_INTERESTED to those not possesing any pieces it does not have.
 */

#endif
