#ifndef _message_h
#define _message_h

//handshake message definitions
#define HS_GREETING "HELLO"                 //header
#define HS_GREETING_LEN sizeof(HS_GREETING) //header length
#define HS_PADDING_POS HS_GREETING_LEN      //starting offset of padding (zeros)
#define HS_PADDING_LEN 23                   //length of padding (# of zeros)
#define HS_ID_POS HS_PADDING_POS + HS_PADDING_LEN    //starting offset of peer_id
#define HS_ID_LEN 4                         //length of peer_id

//normal message definitions
#define MSG_LENGTH_LEN 4                    //# of bytes to store length header
#define MSG_TYPE_POS MSG_LENGTH_LEN         //starting offset of type header
#define MSG_TYPE_LEN 1                      //length of type header
#define MSG_CONTENT_POS MSG_TYPE_POS + MSG_TYPE_LEN      //length of type header

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

//payload types
typedef unsigned long bitfield_t;

/* NOTES
 * When a peer recives a HAVE or BITFIELD message, it sends and INTERESTED 
 * method if the sender has a piece it is missing.
 * When a peer recieves a complete piece, it checks the bitfields of neighbors
 * and sends NOT_INTERESTED to those not possesing any pieces it does not have.
 */

#endif
