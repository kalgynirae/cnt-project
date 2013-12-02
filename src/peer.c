#include "peer.h"

extern int g_bitfield_len;
struct common_cfg g_config;
struct bitfield_seg
{
    char byte;          //section of bitfield
    int idx;            //index of sectiongg
};

int peer_handle_data(struct peer_info *peer, message_t msg_type, 
        unsigned char *payload, int nbytes, bitfield_t our_bitfield,
        struct peer_info *peers, int num_peers, int our_peer_id)
{
    int sender;
    if (msg_type == HAVE)
    {
        // update peer->bitfield based on the HAVE received
        unsigned int piece_idx = unpack_int(payload);
        bitfield_set(peer->bitfield, piece_idx);
        // send out not/interesting
        int interesting = find_interesting_piece(our_bitfield, other_bitfield);
        if (interesting == INCORRECT_MSG_TYPE)
        {
            fprintf(stderr, "peer_handle_data(): incompatible message type\n");
        }
        else if (interesting == NO_INTERESTING_PIECE)
        {
            send_not_interested(peer->socket_fd);
        }
        else
        {
            send_interested(peer->socket_fd);
        }
        log_received_have(our_peer_id, peer->peer_id);
    }
    else if (msg_type == NOT_INTERESTED)
    {
        log_received_not_interested(our_peer_id, peer->peer_id);
    }
    else if (msg_type == INTERESTED)
    {
        log_received_interested(our_peer_id, peer->peer_id);
    }
    else if (msg_type == UNCHOKE)
    {
        log_unchoked_by(our_peer_id, peer->peer_id);
    }
    else if (peer->state == PEER_WAIT_FOR_HANDSHAKE && msg_type == HANDSHAKE)
    {   
        // Transition to bitfield if rcv'd handshake and handshake is valid
        if ((sender = unpack_int(payload)) >= 0)
        {
            send_bitfield(peer->socket_fd, our_bitfield);
            peer->time_last_message_sent = time(NULL);
            peer->state = PEER_WAIT_FOR_BITFIELD;
            log_connect(our_peer_id, peer->peer_id);
        }
        else // In lieu of writing unsuccessful attempts to the log
        {
            fprintf(stderr, "peer_handle_data(): received invalid handshake\n");
        }
    }
    else if (peer->state == PEER_WAIT_FOR_BITFIELD && msg_type == BITFIELD)
    {
        // update peer's bitfield in peer_info
        if (unpack_bitfield(payload, peer->bitfield) < 0)
        {
            fprintf(stderr, "peer_handle_data(): error unpacking bitfield\n");
        }
        // send out not/interesting
        int interesting = find_interesting_piece(our_bitfield, peer->bitfield);
        if (interesting == INCORRECT_MSG_TYPE)
        {
            fprintf(stderr, "peer_handle_data(): incompatible message type\n");
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
    else if (peer->state == PEER_WAIT_UNCHOKED)
    {
        if (msg_type == CHOKE)
        {
            peer->state = PEER_CHOKED;
            log_receive_choke(our_peer_id, peer->peer_id);
        }
        else if (msg_type == PIECE)
        {
            // write the payload to disc
            unsigned int piece_idx = unpack_int(payload); // TODO: does this break?
            if (write_piece(piece_idx, payload[4], nbytes) <= 0)
            {
                fprintf(stderr, "peer_handle_data(): file piece not written\n");
            }
            // update our_bitfield
            bitfield_set(our_bitfield, piece_idx);
            // check if we downloaded the entire file, 
            // write appropriate log messages
            int i; // counter for everything in this branch
            for (i = 0; i < g_bitfield_len; i++)
            {
                if (!has_piece(i, our_bitfield))
                {
                    break;
                }
            }
            if (i == g_bitfield_len) // meaning we just got the last piece
            {
                log_downloaded_file(our_peer_id);
            }
            else
            {
                log_downloaded_piece(our_peer_id, piece_idx);
            }
            // send new request
            unsigned int next_idx;
            for (;;)
            {
                unsigned int rand_idx = rand() % (nbytes*8);
                if (has_piece(rand_idx, peer->bitfield) && // They have it
                        !has_piece(rand_idx, our_bitfield)) // We don't have it
                {
                    for (i = 0; i < num_peers; i++)
                    {
                        if (peers[i].requested == rand_idx) // We have asked for it
                        {
                            break;
                        }
                    }
                    if (i == num_peers) // Oh good we haven't asked for it
                    {
                        next_idx = rand_idx;
                    }
                }
            }
            send_request(peer->socket_fd, next_idx);
            // send haves to all peers
            for (i = 0; i < num_peers; i++)
            {
                send_have(peers[i].socket_fd, piece_idx);
            }
        }
        else if (msg_type == REQUEST)
        {
            unsigned int requested_idx = unpack_int(payload);
            send_piece(peer->socket_fd, requested_idx, g_config.piece_size, peer->peer_id);
        }
        else
        {
            fprintf(stderr, "peer_handle_data(): incompatible message type\n");
        }
    }
    else
    {
        // Check what kind of message we've got and print an error
        fprintf(stderr, "peer_handle_data(): peer %d select()'d with state: "
                        "%d, message_type: %d\n",
                peer->peer_id, peer->state, msg_type);
    }
    return 0;
}

int peer_handle_periodic(struct peer_info *peer, int our_peer_id)
{
    // No FD will trigger when the Peer is not connected
    if (peer->state == PEER_NOT_CONNECTED)
    {
        // Only send handshake if our peer id is less than theirs
        if (our_peer_id < peer->peer_id)
        {
            send_handshake(peer->socket_fd, our_peer_id);
        }
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
            peer->time_last_message_sent = time();
            peer->state = PEER_NOT_CONNECTED;
        }
    }
    else if (peer->state == PEER_CHOKED)
    {   // TODO: do this part
        // Calculate new preferred peers
        if (0/*selected_as_preferred_peer*/)
        {
            // send choke to weakest peer
            // send unchoke
            //log_change_preferred(our_peer_id, num_preferred, preferred);
        }
        // Calculate new optimistic peer
        if (0/*selected_as_optimistic_peer*/)
        {
            // send choke to weakest peer
            // send unchoke
            //log_optimistic_unchoke(our_peer_id, new_peer);
        }
    }
    return 0;
}

int find_interesting_piece(bitfield_t my_bitfield, bitfield_t other_bitfield)
{
    //find interesting byte of bitfield
    int i, j = 0;
    char interesting;   //pieces in segment other has that I don't
    struct bitfield_seg all_interesting[g_bitfield_len]; 
    for (i = 0 ; i < g_bitfield_len ; i++)
    {
        interesting = (my_bitfield[i] ^ other_bitfield[i]) & other_bitfield[i];
        if (interesting != 0)
        {   //segment has piece of interest. store interesting bits and index
            all_interesting[j].idx = i;
            all_interesting[j++].byte = interesting;
        }
    }

    if (j == 0) { return -1; }  //nothing interesting

    //randomly select byte
    struct bitfield_seg segment = all_interesting[random() % j];
    int bits[8];
    j = 0;      //number of interesting bits
    for (i = 0 ; i < 8 ; i++)   //bit pointer
    {
        if ((segment.byte >> i) & 0x1)
        {   //bit is a 1 - interesting!
            bits[j++] = i;          //save interesting bit position
        }
    }
    //select random bit and map to overall bitfield position
    return (bits[random() % j] + 8 * segment.idx);
}

int has_piece(int idx, bitfield_t my_bitfield)
{
    char section = my_bitfield[idx / 8];  //byte containing desired bit
    char mask = 0x1 << (idx % 8);         //mask for desired bit
    return (section & mask);
}

// This function sets the bit in bitfield at idx to 1
int bitfield_set(bitfield_t bitfield, int idx)
{   
    char section = bitfield[idx / 8];  //byte containing desired bit
    char mask = 0x1 << (idx % 8);         //mask for desired bit
    bitfield[idx / 8] = (section | mask);
    return 0;
}
