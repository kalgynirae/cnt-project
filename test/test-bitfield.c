#include "peer.h"

void print_bits(bitfield_t bitfield);
void test_bit(int idx, bitfield_t bitfield);
void test_interesting(bitfield_t my_bitfield, bitfield_t other_bitfield, int reps);

extern int g_bitfield_len;

int main()
{
    printf("--------------------Testing has_piece--------------------\n");
    g_bitfield_len = 4;     //testing only! don't set this outside of init
    unsigned char b1[4] = { 0x3, 0x0, 0x3};
    unsigned char b2[4] = { 0x8D, 0xC, 0x81};

    test_bit(0, b1);
    test_bit(1, b1);
    test_bit(2, b1);
    test_bit(7, b1);
    test_bit(16, b1);
    test_bit(23, b1);

    test_bit(0, b2);
    test_bit(1, b2);
    test_bit(2, b2);
    test_bit(7, b2);
    test_bit(16, b2);
    test_bit(23, b1);

    printf("--------------------Testing has_piece--------------------\n");
    printf("should randomly select different pieces\n");
    test_interesting(b1, b2, 4);
    test_interesting(b2, b1, 4);
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

void test_bit(int idx, bitfield_t bitfield)
{
    printf("bitfield ");
    print_bits(bitfield);
    if (has_piece(idx, bitfield))
    {
        printf(" has piece %d\n", idx);
    }
    else
    {
        printf(" doesn't have piece %d\n", idx);
    }
}

void test_interesting(bitfield_t my_bitfield, bitfield_t other_bitfield, int reps)
{
    int idx;
    while (reps-- > 0)
    {
        if ((idx = find_interesting_piece(my_bitfield, other_bitfield)) < 0)
        {
            print_bits(my_bitfield);
            printf(" needs nothing from ");
            print_bits(other_bitfield);
            printf("\n");
        }
        else
        {
            print_bits(my_bitfield);
            printf(" wants %d from ", idx);
            print_bits(other_bitfield);
            printf("\n");
        }
    }
}
