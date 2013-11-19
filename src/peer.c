#include "peer.h"

int peer_handle_data(struct peer_info *peer, message_t msg_type, 
        unsigned char *data, int nbytes, bitfield_t bitfield)
{
    int sender;
    if (peer->state == PEER_WAIT_FOR_HANDSHAKE)
    {
        // Transition to bitfield if rcv'd handshake and handshake is valid
        if ((sender = unpack_int(data)) >= 0)
        {
            // Send bitfield
            peer->time_last_message_sent = time(NULL);
            peer->state = PEER_WAIT_FOR_BITFIELD;
        }
    }
    else
    {
        // Rcv'd bitfield
        if (peer->state == PEER_WAIT_FOR_BITFIELD
                && msg_type == BITFIELD)
        {
            bitfield_t other_bitfield = unpack_bitfield(data);
            int interesting = find_interesting_piece(bitfield, other_bitfield);
            if (interesting == INCORRECT_MSG_TYPE)
            {
                fprintf(stderr, "incompatible message type");
            }
            else if (interesting == NO_INTERESTING_PIECE)
            {
                // Send not interested
            }
            else
            {
                // Send interested
            }
            peer->state = PEER_CHOKED;
        }
    }
    return 0;
}

int peer_handle_timeout(struct peer_info *peer)
{
    // No FD will trigger when the Peer is not connected
    if (peer->state == PEER_NOT_CONNECTED)
    {
        // Send our handshake message
        // Start a timer and attach it to the peer_info struct
        peer->time_last_message_sent = time(NULL);
        peer->state = PEER_WAIT_FOR_HANDSHAKE;
    }
    else if (peer->state == PEER_WAIT_FOR_HANDSHAKE)
    {
        // Self-edge when timeout occurs, re-send handshake
        if (time(NULL) - peer->time_last_message_sent >= HANDSHAKE_TIMEOUT_TIME)
        {
            // Send our handshake message
            // Start a timer and attach it to the peer_info struct
            peer->time_last_message_sent = time(NULL);
        }
    }
    else if (peer->state == PEER_WAIT_FOR_BITFIELD)
    {
        // In the event of a timeout, go back to state 0, implying that no
        // bitfield was sent because the peer has no interesting pieces.
        if (time(NULL) - peer->time_last_message_sent >= BITFIELD_TIMEOUT_TIME)
        {

        }
    }
    return 0;
}

int find_interesting_piece(bitfield_t my_bitfield, bitfield_t other_bitfield)
{
    //TODO: bitfield_t will probably need to be a char[]
    return 0;
}
