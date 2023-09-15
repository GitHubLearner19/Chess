#include "bitscan.h"

int popCount(uint64_t bitboard) {
    int count = 0;
    while (bitboard) {
        count ++;
        bitboard &= bitboard - 1; // reset LS1B
    }
    return count;
}

int ls1bTable[64] = {
    0,  1, 48,  2, 57, 49, 28,  3,
   61, 58, 50, 42, 38, 29, 17,  4,
   62, 55, 59, 36, 53, 51, 43, 22,
   45, 39, 33, 30, 24, 18, 12,  5,
   63, 47, 56, 27, 60, 41, 37, 16,
   54, 35, 52, 21, 44, 32, 23, 11,
   46, 26, 40, 15, 34, 20, 31, 10,
   25, 14, 19,  9, 13,  8,  7,  6
};

/**
 * get least significant set bit
 * @author Martin Läuter (1997)
 *         Charles E. Leiserson
 *         Harald Prokop
 *         Keith H. Randall
 */
int bitscan_forward(uint64_t bitboard) {
   uint64_t debruijn64 = 0x03f79d71b4cb0a89LL;
   return ls1bTable[((bitboard & -bitboard) * debruijn64) >> 58];
}

int ms1bTable[256];

void setup_ms1b_table() {
    for (int i = 0; i < 256; i ++) {
        if (i > 127) {
            ms1bTable[i] = 7;
        } else if (i > 63) {
            ms1bTable[i] = 6;
        } else if (i > 31) {
            ms1bTable[i] = 5;
        } else if (i > 15) {
            ms1bTable[i] = 4;
        } else if (i > 7) {
            ms1bTable[i] = 3;
        } else if (i > 3) {
            ms1bTable[i] = 2;
        } else if (i > 1) {
            ms1bTable[i] = 1;
        } else {
            ms1bTable[i] = 0;
        }
    }
}

/**
 * get most significant set bit
 * @author Eugene Nalimov
 */
int bitscan_reverse(uint64_t bitboard) {
    int result = 0;

    if (bitboard > 0xFFFFFFFF) {
        bitboard >>= 32;
        result = 32;
    }
    if (bitboard > 0xFFFF) {
        bitboard >>= 16;
        result += 16;
    }
    if (bitboard > 0xFF) {
        bitboard >>= 8;
        result += 8;
    }
    
    return result + ms1bTable[bitboard];
}

