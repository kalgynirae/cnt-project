#include "peer.h"

extern int g_bitfield_len;
struct bitfield_seg
{
    char byte;          //section of bitfield
    int idx;            //index of section
};

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

int peer_handle_periodic(struct peer_info *peer)
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
    printf("found %d interesting segments\n", j);

    //randomly select byte
    struct bitfield_seg segment = all_interesting[random() % j];
    int bits[8];
    j = 0;      //number of interesting bits
    for (i = 0 ; i < 8 ; i++)   //bit pointer
    {
        if ((segment.byte >> i) && 0x1 != 0)
        {   //bit is a 1 - interesting!
            bits[j++] = i;          //save interesting bit position
        }
    }
    printf("found %d interesting bits\n", j);
    //select random bit and map to overall bitfield position
    return (bits[random() % j] + 8 * segment.idx);
}

int has_piece(int idx, bitfield_t my_bitfield)
{
    char section = my_bitfield[idx / 8];  //byte containing desired bit
    char mask = 0x1 << (idx % 8);         //mask for desired bit
    return (section & mask);
}
