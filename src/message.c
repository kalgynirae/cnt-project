#include "message.h"

struct mess_handshake write_handshake(int p_id){//generate handshake message with peer ID

	struct mess_handshake hs;//handshake message struct to be returned
	
	hs.header = "HELLO";//handshake header
	
	hs.zeros = "00000000000000000000000";//23 zeros
	
	hs.peer_id = p_id;//4 byte peer ID
	
	return hs;//return generated handshake message

}