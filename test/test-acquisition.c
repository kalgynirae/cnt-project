#include <stdlib.h>
#include "peer.h"
#define N_PEERS 4

void print_bits(bitfield_t bitfield);
void print_bytes(bitfield_t bitfield);
void test_bit(int idx, bitfield_t bitfield);
void test_interesting(bitfield_t my_bitfield, bitfield_t other_bitfield, int reps);
void test_set(int idx, bitfield_t bitfield);

extern int g_bitfield_len;
extern int g_num_pieces;

struct peer_info peers[N_PEERS];

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <n_pieces>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    srand(time(NULL));
    //testing only! don't set this outside of init
    g_num_pieces = atoi(argv[1]);     
    g_bitfield_len = g_num_pieces / 8;    
    g_bitfield_len += ((g_num_pieces % 8 == 0) ? 0 : 1);
    printf("g_num_pieces: %d, g_bitfield_len: %d\n", g_num_pieces, 
            g_bitfield_len);
    unsigned char b1[g_bitfield_len];
    unsigned char b2[g_bitfield_len];
    init_bitfield(b1,0);
    init_bitfield(b2,1);
    //mock up peers
    peers[0].requested = -1;
    peers[1].requested = -1;
    peers[2].requested = -1;
    peers[3].requested = -1;

    printf("start: ");
    print_bytes(b1);
    printf("\n");
    printf("goal:  ");
    print_bytes(b2);
    printf("\n");

    int requested[g_num_pieces * 8];
    memset(requested, 0, g_num_pieces * 8);

    int idx = 0, i = 0;
    for (i = 0 ; i < g_num_pieces ; i++)
    {
        if ((idx = find_interesting_piece(b1, b2, peers, N_PEERS)) < 0)
        {
            printf("Failed to find interesting piece\n");
        }
        if (requested[idx])
        {
            printf("asking for %d again!!\n", idx);
        }
        printf("getting %d\n", idx);
        bitfield_set(b1, idx);
        requested[idx] = 1;
        print_bytes(b1);
    }

    return 0;
}

void print_bits(bitfield_t bitfield)
{
    int i, j;
    putchar('<');
    for (i = g_bitfield_len - 1 ; i >= 0 ; i--)
    {
        for (j = 7 ; j >= 0; j--)
        {
            putchar('0' + ((bitfield[i] >> j) & 0x1));
        }
        putchar(' ');
    }
    putchar('>');
}

void print_bytes(bitfield_t bitfield)
{
    int i;
    putchar('<');
    for (i = g_bitfield_len - 1 ; i >= 0 ; i--)
    {
        printf(" %02X", bitfield[i] & 0xFF);
    }
    printf(">\n");
}

void test_bit(int idx, bitfield_t bitfield)
{
    printf("bitfield ");
    print_bits(bitfield);
    if (bitfield_get(bitfield, idx))
    {
        printf(" has piece %d\n", idx);
    }
    else if (!bitfield_get(bitfield, idx))
    {
        printf(" doesn't have piece %d\n", idx);
    }
    else
    {
        printf("ERROR");
    }
}

void test_set(int idx, bitfield_t bitfield)
{
    printf("Before:\n");
    test_bit(idx, bitfield);
    bitfield_set(bitfield, idx);
    printf("After setting %d:\n", idx);
    test_bit(idx, bitfield);
}


void test_interesting(bitfield_t my_bitfield, bitfield_t other_bitfield, int reps)
{
    int idx;
    while (reps-- > 0)
    {
        idx = find_interesting_piece(my_bitfield, other_bitfield, peers, N_PEERS);
        if (idx < 0)
        {
            print_bits(my_bitfield);
            printf(" needs nothing from ");
            print_bits(other_bitfield);
        }
        else
        {
            bitfield_set(my_bitfield, idx);
            print_bits(my_bitfield);
            printf("\n");
            int spaces = (9 * g_bitfield_len) - (1 + idx + idx / 8);
            while (spaces-- > 0) { putchar(' '); }
            printf("| piece %d is interesting\n", idx);
            print_bits(other_bitfield);
        }
        printf("\n");
        printf("\n");
    }
    printf("\n");
}
