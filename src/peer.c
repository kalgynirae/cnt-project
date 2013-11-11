#include "peer.h"

#define has_piece(idx, bitfield) ((bitfield & (0x1 << idx)) !=0)

int peer_handle_data(struct peer_info *peer, struct peer_info *me, char *data, int nbytes)
{
    // incoming normal message. handshake needs to be handled separately
    // probably need to read in as bytestream, then cast to correct message
    // struct after checking if first 5 bytes == "HELLO"
    struct mess_normal msg_in;

    int sender;
    if (peer->state == PEER_WAIT_FOR_HANDSHAKE)
    {
        // Transition to bitfield if rcv'd handshake and handshake is valid
        if ((sender = parse_handshake_msg(data, nbytes)) >= 0)
        {
            send_bitfield(peer->socket_fd, me->bitfield);
            peer->time_last_message_sent = time(NULL);
            peer->state = PEER_WAIT_FOR_BITFIELD;
        }
    }
    else if (parse_normal_msg(data, nbytes, &msg_in))
    {
        // Do these regardless of state:
        if (msg_in.type == HAVE)
        {
            // update bitfield, parse_have()?
            piece_n = parse_have(/*whatever goes here*/);
            if (has_piece(piece_n, peer->bitfield))
            {
                send_interested(peer->socket_fd);
            }
            else
            {
                send_not_interested(peer->socket_fd);
            }
            log_receieved_have(me->peer_id, peer->peer_id);
        }
        if (msg_in.type == INTERESTED)
        {
            log_received_interested(me->peer_id, peer->peer_id);
        }
        if (msg_in.type == NOT_INTERESTED)
        {
            log_received_not_interested(me->peer_id, peer->peer_id);
        }
        // Rcv'd bitfield
        if (peer->state == PEER_WAIT_FOR_BITFIELD && msg_in.type == BITFIELD)
        {
            int interesting = find_interesting_piece(me->bitfield, msg_in);
            if (interesting == INCORRECT_MSG_TYPE)
            {
                fprintf(stderr, "incompatible message type");
            }
            else if (interesting == NO_INTERESTING_PIECE)
            {
                send_not_interested(peer->socket_fd);
                peer->state = PEER_CHOKED;
            }
            else
            {
                send_interested(peer->socket_fd);
                peer->state = PEER_CHOKED;
            }
        }
    }
    else
    {
        fprintf(stderr, "Failed to parse normal message");
    }
}

int peer_handle_timeout(struct peer_info *peer)
{
    // No FD will trigger when the Peer is not connected
    if (peer->state == PEER_NOT_CONNECTED)
    {
        send_handshake(peer->socket_fd, peer->peer_id);
        // Start a timer and attach it to the peer_info struct
        peer->time_last_message_sent = time(NULL);
        peer->state = PEER_WAIT_FOR_HANDSHAKE;
    }
    else if (peer->state == PEER_WAIT_FOR_HANDSHAKE)
    {
        // Self-edge when timeout occurs, re-send handshake
        if (time(NULL) - peer->time_last_message_sent >= HANDSHAKE_TIMEOUT_TIME)
        {
            send_handshake(peer->socket_fd, peer->peer_id);
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
            peer->state = PEER_NOT_CONNECTED;
        }
    }
}
