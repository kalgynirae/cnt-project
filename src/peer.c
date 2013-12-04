#include "peer.h"

extern int g_bitfield_len;
struct common_cfg g_config;
struct bitfield_seg
{
    char byte;          //section of bitfield
    int idx;            //index of section
};

int peer_handle_data(struct peer_info *peer, message_t msg_type, 
        unsigned char *payload, int nbytes, bitfield_t our_bitfield,
        struct peer_info *peers, int num_peers, int our_peer_id)
{
    fprintf(stderr, "peer_handle_data(%d)\n", peer->peer_id);
    int sender;
    if (msg_type == HAVE)
    {
        // update peer->bitfield based on the HAVE received
        unsigned int piece_idx = unpack_int(payload);
        fprintf(stderr, "\tHAVE: %d\n", piece_idx);
        bitfield_set(peer->bitfield, piece_idx);
        // send out not/interesting
        int interesting = find_interesting_piece(our_bitfield, peer->bitfield);
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
        fprintf(stderr, "\tNOT_INTERESTED\n");
        log_received_not_interested(our_peer_id, peer->peer_id);
    }
    else if (msg_type == INTERESTED)
    {
        fprintf(stderr, "\tINTERESTED\n");
        log_received_interested(our_peer_id, peer->peer_id);
    }
    else if (msg_type == UNCHOKE)
    {
        fprintf(stderr, "\tUNCHOKE\n");
        log_unchoked_by(our_peer_id, peer->peer_id);
    }
    else if (msg_type == HANDSHAKE)
    {
        fprintf(stderr, "\tHANDSHAKE\n");
        // Check the handshake for validity
        if ((sender = unpack_int(payload)) >= 0)
        {
            // Send a return handshake if we didn't initiate the connection
            if (peer->state == PEER_NOT_CONNECTED)
            {
                send_handshake(peer->socket_fd, our_peer_id);
            }
            // Send our bitfield; wait for their bitfield
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
        fprintf(stderr, "\tBITFIELD\n");
        // update peer's bitfield in peer_info
        memcpy(peer->bitfield, payload, g_bitfield_len);

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
            fprintf(stderr, "\tCHOKE\n");
            peer->state = PEER_CHOKED;
            log_receive_choke(our_peer_id, peer->peer_id);
        }
        else if (msg_type == PIECE)
        {
            fprintf(stderr, "\tPIECE\n");
            // write the payload to disc
            unsigned int piece_idx = unpack_int(payload); // TODO: does this break?
            fprintf(stderr, "\tidx: %d\n", piece_idx);
            extract_and_save_piece(nbytes, payload, our_peer_id);     
            fprintf(stderr, "\textracted and saved%d\n", piece_idx);
            // update our_bitfield
            bitfield_set(our_bitfield, piece_idx);
            fprintf(stderr, "\tnew bitfield: ");
            print_bitfield(stderr, our_bitfield);
            // increment pieces_this_interval field of peer_info
            peer->pieces_this_interval++;

            // check if we downloaded the entire file, write appropriate log messages
            // TODO: WHAT IS HAPPENING HERE?????
            int i; // counter for everything in this branch
            if (bitfield_filled(our_bitfield)) // meaning we just got the last piece
            {   //are we setting the other peer's has file based on our bitfield?
                peer->has_file = 1;     //is this right?
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
                unsigned int rand_idx = find_interesting_piece(our_bitfield, peer->bitfield);
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
            send_request(peer->socket_fd, next_idx);
            // send haves to all peers
            for (i = 0; i < num_peers; i++)
            {
                send_have(peers[i].socket_fd, piece_idx);
            }
        }
        else if (msg_type == REQUEST)
        {
            fprintf(stderr, "\tREQUEST\n");
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

int peer_handle_periodic(struct peer_info *peer, int our_peer_id, bitfield_t our_bitfield,
        struct peer_info *peers, int num_peers)
{
    fprintf(stderr, "peer_handle_periodic(%d)\n", peer->peer_id);
    // No FD will trigger when the Peer is not connected
    if (peer->state == PEER_NOT_CONNECTED)
    {
        fprintf(stderr, "\tstate=PEER_NOT_CONNECTED\n");
        // Only send handshake if our peer id is less than theirs
        if (our_peer_id < peer->peer_id)
        {
            int s = make_socket_to_peer(peer);
            if (s == -1)
            {
                fprintf(stderr, "peer_handle_periodic(): error making socket\n");
            }
            else
            {
                send_handshake(peer->socket_fd, our_peer_id);
                // Start a timer and attach it to the peer_info struct
                peer->time_last_message_sent = time(NULL);
                peer->state = PEER_WAIT_FOR_HANDSHAKE;
            }
        }
    }
    else if (peer->state == PEER_WAIT_FOR_HANDSHAKE)
    {
        fprintf(stderr, "\tstate=PEER_WAIT_FOR_HANDSHAKE\n");
        // Self-edge when timeout occurs, re-send handshake
        if (time(NULL) - peer->time_last_message_sent >= HANDSHAKE_TIMEOUT_TIME)
        {
            fprintf(stderr, "\t\ttimeout on handshake to %d\n", peer->peer_id);
            // Reset back to NOT_CONNECTED
            peer->state = PEER_NOT_CONNECTED;
        }
    }
    else if (peer->state == PEER_WAIT_FOR_BITFIELD)
    {
        fprintf(stderr, "\tstate=PEER_WAIT_FOR_BITFIELD\n");
        // In the event of a timeout, go back to state 0, implying that no
        // bitfield was sent because the peer has no interesting pieces.
        if (time(NULL) - peer->time_last_message_sent >= BITFIELD_TIMEOUT_TIME)
        {
            fprintf(stderr, "\t\ttimeout on bitfield to %d\n", peer->peer_id);
            peer->time_last_message_sent = time(NULL);
            peer->state = PEER_NOT_CONNECTED;
        }
    }
    else if (peer->state == PEER_CHOKED)
    {
        fprintf(stderr, "\tstate=CHOKED\n");
        // Do nothing?
    }
    //fprintf(stderr, "peer_handle_periodic returning\n");
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

int bitfield_get(bitfield_t bitfield, int idx)
{
    char section = bitfield[idx / 8];  //byte containing desired bit
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

void init_bitfield(bitfield_t bitfield, int has_file)
{
    int val = has_file ? 0xFF : 0x00;
    memset(bitfield, val, g_bitfield_len);
}

int bitfield_filled(bitfield_t bitfield)
{
    int i;
    for (i = 0 ; i < g_bitfield_len - 1; i++)
    {
        if (bitfield[i] != 0xFF)
        {   //not filled - piece missing
            return 0;
        }
    }
    for(i = 0 ; i < (g_config.file_size / g_config.piece_size) % 8; i++)
    {   //check last byte of bitfield, not necessarily full
        if ((bitfield[g_bitfield_len - 1] & (0x1 << i)) == 0)
        {
            return 0;
        }
    }
    
    return 1;
}

//print bitfield
void print_bitfield(FILE *stream, bitfield_t bitfield)
{
    if (bitfield == NULL) { fprintf(stderr, "bitfield null! abort! abort!\n"); }
    int j;
    for (j = 0 ; j < g_bitfield_len ; j++) {
        fprintf(stream, "%x ", bitfield[j] & 0xFF);
    }
    fprintf(stream, "\n");
}
