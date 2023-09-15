#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "bitscan.h"

enum pieceColor {
    White, Black
};

enum enumSquare {
    a1, b1, c1, d1, e1, f1, g1, h1,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a8, b8, c8, d8, e8, f8, g8, h8
};

void print_bitboard(uint64_t bitboard) {
    for (int row = 7; row >= 0; row --) {
        for (int col = 0; col < 8; col ++) {
            if (bitboard >> (row * 8 + col) & 1) {
                putchar('1');
            } else {
                putchar('.');
            }
        }
        putchar('\n');
    }
}

enum direction {
    Nort, NoEa, East, SoEa, Sout, SoWe, West, NoWe
};

// attacks from each square in each direction
uint64_t rayAttacks[64][8];

// shift bitboard east one
uint64_t east_one(uint64_t bitboard) {
    uint64_t notA = 0xfefefefefefefefeLL;
    return (bitboard << 1) & notA;
};

// shift bitboard west one
uint64_t west_one(uint64_t bitboard) {
    uint64_t notH = 0x7f7f7f7f7f7f7f7fLL;
    return (bitboard >> 1) & notH;
};

// setup ray attacks
// based on https://www.chessprogramming.org/On_an_empty_Board#Initialization
void setup_ray_attacks() {
    uint64_t nort = 0x101010101010100LL;
    for (int i = 0; i < 64; i ++) {
        rayAttacks[i][Nort] = nort;
        nort <<= 1;
    }

    uint64_t noea = 0x8040201008040200LL;
    for (int col = 0; col < 8; col ++) {
        uint64_t ne = noea;
        for (int row = 0; row < 8; row ++) {
            rayAttacks[row * 8 + col][NoEa] = ne;
            ne <<= 8;
        }
        noea = east_one(noea);
    }

    uint64_t east = 0xfeLL;
    for (int col = 0; col < 8; col ++) {
        uint64_t ea = east;
        for (int row = 0; row < 8; row ++) {
            rayAttacks[row * 8 + col][East] = ea;
            ea <<= 8;
        }
        east = east_one(east);
    }

    uint64_t soea = 0x2040810204080LL;
    for (int col = 0; col < 8; col ++) {
        uint64_t se = soea;
        for (int row = 7; row >= 0; row --) {
            rayAttacks[row * 8 + col][SoEa] = se;
            se >>= 8;
        }
        soea = east_one(soea);
    }

    uint64_t sout = 0x80808080808080LL;
    for (int i = 63; i >= 0; i--) {
        rayAttacks[i][Sout] = sout;
        sout >>= 1;
    }

    uint64_t sowe = 0x40201008040201LL;
    for (int col = 7; col >= 0; col --) {
        uint64_t sw = sowe;
        for (int row = 7; row >= 0; row --) {
            rayAttacks[row * 8 + col][SoWe] = sw;
            sw >>= 8;
        }
        sowe = west_one(sowe);
    }

    uint64_t west = 0x7fLL;
    for (int col = 7; col >= 0; col --) {
        uint64_t we = west;
        for (int row = 0; row < 8; row ++) {
            rayAttacks[row * 8 + col][West] = we;
            we <<= 8;
        }
        west = west_one(west);
    }

    uint64_t nowe = 0x102040810204000LL;
    for (int col = 7; col >= 0; col --) {
        uint64_t nw = nowe;
        for (int row = 0; row < 8; row ++) {
            rayAttacks[row * 8 + col][NoWe] = nw;
            nw <<= 8;
        }
        nowe = west_one(nowe);
    }
}

// possible squares for blockers for each piece at each square
uint64_t rookMasks[64];
uint64_t bishopMasks[64];

// get bitboard of all non-edge squares
uint64_t get_not_edge(int square) {
    uint64_t result = 0x7e7e7e7e7e7e00LL;

    if (square % 8 == 0) {
        result |= 0x1010101010100LL;
    } else if (square % 8 == 7) {
        result |= 0x80808080808000LL;
    }

    if (square < 8) {
        result |= 0x7eLL;
    } else if (square >= 56) {
        result |= 0x7e00000000000000LL;
    }

    return result;
}

// intialize masks
void setup_piece_masks() {
    for (int i = 0; i < 64; i ++) {
        rookMasks[i] = rayAttacks[i][Nort] | rayAttacks[i][East] | 
        rayAttacks[i][Sout] | rayAttacks[i][West];
        rookMasks[i] &= get_not_edge(i);
    }

    for (int i = 0; i < 64; i ++) {
        bishopMasks[i] = rayAttacks[i][NoEa] | rayAttacks[i][SoEa] | 
        rayAttacks[i][SoWe] | rayAttacks[i][NoWe];
        bishopMasks[i] &= get_not_edge(i);
    }
}

// get ray attacks in positive direction on occupied board
uint64_t get_positive_ray_attacks(uint64_t occupied, int square, enum direction dir) {
    uint64_t attacks = rayAttacks[square][dir];
    uint64_t blockers = attacks & occupied;
    if (blockers) {
        attacks ^= rayAttacks[bitscan_forward(blockers)][dir];
    }
    return attacks;
}

// get ray attacks in negative direction on occupied board
uint64_t get_negative_ray_attacks(uint64_t occupied, int square, enum direction dir) {
    uint64_t attacks = rayAttacks[square][dir];
    uint64_t blockers = attacks & occupied;
    if (blockers) {
        attacks ^= rayAttacks[bitscan_reverse(blockers)][dir];
    }
    return attacks;
}

uint64_t get_bishop_attacks_classical(uint64_t occupied, int square) {
    return get_positive_ray_attacks(occupied, square, NoEa) | get_negative_ray_attacks(occupied, square, SoEa) | 
    get_negative_ray_attacks(occupied, square, SoWe) |
    get_positive_ray_attacks(occupied, square, NoWe);
}

uint64_t get_rook_attacks_classical(uint64_t occupied, int square) {
    return get_positive_ray_attacks(occupied, square, Nort) | get_positive_ray_attacks(occupied, square, East) | 
    get_negative_ray_attacks(occupied, square, Sout) |
    get_negative_ray_attacks(occupied, square, West);
}

typedef enum enumPiece enumPiece;

enum enumPiece {
    Bishop, Rook
};

// number of set bits in rook masks from each square
int rookBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

// number of set bits in bishop masks from each square
int bishopBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

// attacks for sliding pieces from each square on occupied board
// hash table for fancy magic bitboards
uint64_t attackTable[107648];

typedef struct SMagic SMagic;

// magic for a square
struct SMagic {
    uint64_t* ptr; // pointer to index of square's attack table
    uint64_t mask; // possible blockers
    uint64_t magic;
    int shift; // 64 - number of set bits in mask
};

SMagic bishopMagics[64];
SMagic rookMagics[64];

uint64_t bishop_magic_hash(uint64_t occupied, int square) {
    occupied &= bishopMagics[square].mask;
    occupied *= bishopMagics[square].magic;
    occupied >>= bishopMagics[square].shift;
    return occupied;
}

uint64_t get_bishop_attacks_magic(uint64_t occupied, int square) {
    uint64_t* all_attacks = bishopMagics[square].ptr;
    return all_attacks[bishop_magic_hash(occupied, square)];
}

uint64_t rook_magic_hash(uint64_t occupied, int square) {
    occupied &= rookMagics[square].mask;
    occupied *= rookMagics[square].magic;
    occupied >>= rookMagics[square].shift;
    return occupied;
}

uint64_t get_rook_attacks_magic(uint64_t occupied, int square) {
    uint64_t* all_attacks = rookMagics[square].ptr;
    return all_attacks[rook_magic_hash(occupied, square)];
}

uint64_t random_uint64() {
    uint64_t u1 = (uint64_t)(random()) & 0xFFFF; 
    uint64_t u2 = (uint64_t)(random()) & 0xFFFF;
    uint64_t u3 = (uint64_t)(random()) & 0xFFFF; 
    uint64_t u4 = (uint64_t)(random()) & 0xFFFF;
    return u1 | (u2 << 16) | (u3 << 32) | (u4 << 48);
}

uint64_t random_magic() {
    return random_uint64() & random_uint64() & random_uint64();
}

void clear_attacks(uint64_t* attacks, int len) {
    for (int i = 0; i < len; i ++) {
        attacks[i] = 0;
    }
}

/* 
 * finds magic number given square and piece
 * returns the number of spaces used in the attack table 
 */
int find_magic(int square, enumPiece piece) {
    SMagic* magics = piece == Bishop ? bishopMagics : rookMagics;
    uint64_t* allAttacks = magics[square].ptr;
    magics[square].mask = piece == Bishop ? bishopMasks[square] : rookMasks[square];
    uint64_t indexBits = piece == Bishop ? bishopBits[square] : 
    rookBits[square]; // number of set bits in the mask
    magics[square].shift = 64 - indexBits;
    int totalNumSubsets = 1 << indexBits; // number of blocker bitboards

    // calculate attacks classical approach
    uint64_t classicalAttacks[totalNumSubsets]; 
    uint64_t subset = 0;
    for (int i = 0; i < totalNumSubsets; i ++) { // loop over all blockers
        subset = (subset - magics[square].mask) & magics[square].mask;
        classicalAttacks[i] = piece == Bishop ? get_bishop_attacks_classical(subset, square) : get_rook_attacks_classical(subset, square);
    }

    // test magic numbers until one perfectly maps all blocker bitboards to corresponding attack bitboards
    int i = 0;
    while (i < totalNumSubsets) {
        magics[square].magic = random_magic();
        clear_attacks(allAttacks, totalNumSubsets);
        subset = 0;
        uint64_t magicAttacks = 0;
        i = 0;
        while (i < totalNumSubsets) { // loop over all blockers
            subset = (subset - magics[square].mask) & magics[square].mask;
            uint64_t magicHash = piece == Bishop ? bishop_magic_hash(subset, square) : rook_magic_hash(subset, square);
            if (magicHash >= totalNumSubsets) {
                break; // index exceeds table size; start over
            }
            magicAttacks = allAttacks[magicHash];
            if (magicAttacks == 0) {
                allAttacks[magicHash] = classicalAttacks[i]; // add new entry
            } else if (magicHash != classicalAttacks[i]) {
                break; // different entry already exists; start over
            }
            i ++;
        }
    }

    return totalNumSubsets;
}

/*
 * get magic number for all squares
 * start indicates where to add entries into attack table
 */
void find_bishop_magics(int start) {
    uint64_t* lastPointer = attackTable;
    int lastSize = start;
    for (int i = 0; i < 64; i ++) {
        bishopMagics[i].ptr = lastPointer + lastSize;
        lastSize = find_magic(i, Bishop);
        lastPointer = bishopMagics[i].ptr;
    }
}

/*
 * get magic number for all squares
 * start indicates where to add entries into attack table
 */
void find_rook_magics(int start) {
    uint64_t* lastPointer = attackTable;
    int lastSize = start;
    for (int i = 0; i < 64; i ++) {
        rookMagics[i].ptr = lastPointer + lastSize;
        lastSize = find_magic(i, Rook);
        lastPointer = rookMagics[i].ptr;
    }
}

int setup() {
    setup_ms1b_table();
    setup_ray_attacks();
    setup_piece_masks();
    return 0;
}

int main(int argc, char* argv[]) {
    setup();
    find_bishop_magics(0);
    find_rook_magics(5248);
    printf("Bishop magics:\n");
    for (int i = 0; i < 64; i ++) {
        printf("0x%llxLL, ", bishopMagics[i].magic);
    }
    printf("\nRook magics:\n");
    for (int i = 0; i < 64; i ++) {
        printf("0x%llxLL, ", rookMagics[i].magic);
    }
    return 0;
}
