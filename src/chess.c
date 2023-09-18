#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "bitscan.h"
#include "shortlist.h"

typedef enum pieceColor pieceColor;

enum pieceColor {
    White, Black
};

typedef enum enumPiece enumPiece;

enum enumPiece {
    BlackPawn, WhitePawn, BlackKnight, WhiteKnight, BlackBishop, WhiteBishop, BlackRook, WhiteRook, BlackQueen, WhiteQueen, BlackKing, WhiteKing
};

typedef struct board chessboard;

struct board {
    uint64_t pieces[12];
    pieceColor turn;
    short castleKing[2];
    short castleQueen[2];
    int epSquare;
    int halfMoveClock;
    int fullMoves;
    shortlist* captureLog;
    shortlist* captureLogTail;
    shortlist* castleKingLog;
    shortlist* castleKingLogTail;
    shortlist* castleQueenLog;
    shortlist* castleQueenLogTail;
    shortlist* epLog;
    shortlist* epLogTail;
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

// get a new chess board from fen
chessboard new_board(char* fen) {
    chessboard board;
    for (int i = 0; i < 12; i ++) {
        board.pieces[i] = 0LL;
    }
    int row = 7; 
    int col = 0;
    int i = 0;
    while (fen[i] != ' ') {
        if (fen[i] == '/') {
            row --;
            col = 0;
        } else if (fen[i] > '0' && fen[i] < '9') {
            col += fen[i] - '0';
        } else {
            uint64_t square = 1LL << (row * 8 + col);
            switch (fen[i]) {
                case 'P':
                    board.pieces[WhitePawn] |= square;
                    break;
                case 'p':
                    board.pieces[BlackPawn] |= square;
                    break;
                case 'R':
                    board.pieces[WhiteRook] |= square;
                    break;
                case 'r':
                    board.pieces[BlackRook] |= square;
                    break;
                case 'B':
                    board.pieces[WhiteBishop] |= square;
                    break;
                case 'b':
                    board.pieces[BlackBishop] |= square;
                    break;
                case 'N':
                    board.pieces[WhiteKnight] |= square;
                    break;
                case 'n':
                    board.pieces[BlackKnight] |= square;
                    break;
                case 'Q':
                    board.pieces[WhiteQueen] |= square;
                    break;
                case 'q':
                    board.pieces[BlackQueen] |= square;
                    break;
                case 'K':
                    board.pieces[WhiteKing] |= square;
                    break;
                case 'k':
                    board.pieces[BlackKing] |= square;
            }
            col ++;
        }
        i ++;
    }

    board.turn = fen[++i] == 'w' ? White : Black;

    i += 2;

    if (fen[i] == '-') {
        board.castleKing[White] = 0;
        board.castleKing[Black] = 0;
        board.castleQueen[White] = 0;
        board.castleQueen[Black] = 0;
        i ++;
    } else {
        while (fen[i] != ' ') {
            switch (fen[i]) {
                case 'K':
                    board.castleKing[White] = 1;
                    break;
                case 'Q':
                    board.castleQueen[White] = 1;
                    break;
                case 'k':
                    board.castleKing[Black] = 1;
                    break;
                case 'q':
                    board.castleQueen[Black] = 1;
            }
            i ++;
        }
    }

    if (fen[++i] == '-') {
        board.epSquare = -1;
    } else {
        board.epSquare = (fen[i] - 'a') + (fen[i + 1] - '1') * 8;
        i ++;
    }

    i += 2;
    board.halfMoveClock = fen[i] - '0';
    i += 2;
    board.fullMoves = fen[i] - '0';

    board.captureLog = NULL;
    board.captureLogTail = NULL;
    board.castleKingLog = NULL;
    board.castleKingLogTail = NULL;
    board.castleQueenLog = NULL;
    board.castleQueenLogTail = NULL;
    board.epLog = NULL;
    board.epLogTail = NULL;
    
    return board;
    return board;
}

// get piece at square on chess board
short get_board_piece(chessboard* board, short square) {
    uint64_t allPieces[12] = {
        board->pieces[BlackPawn], board->pieces[WhitePawn],
        board->pieces[BlackKnight], board->pieces[WhiteKnight],
        board->pieces[BlackBishop], board->pieces[WhiteBishop],
        board->pieces[BlackRook], board->pieces[WhiteRook],
        board->pieces[BlackQueen], board->pieces[WhiteQueen],
        board->pieces[BlackKing], board->pieces[WhiteKing]
    };
    for (int i = 0; i < 12; i ++) {
        if ((allPieces[i] >> square) & 1) {
            return i;
        }
    }
    return -1;
}

// print chess board
void print_board(chessboard* board) {
    char pieceChars[12] = {'p', 'P', 'n', 'N', 'b', 'B', 'r', 'R', 'q', 'Q', 'k', 'K'};
    for (int row = 7; row >= 0; row --) {
        for (int col = 0; col < 8; col ++) {
            short piece = get_board_piece(board, row * 8 + col);
            if (piece >= 0) {
                putchar(pieceChars[piece]);
            } else {
                putchar('.');
            }
        }
        putchar('\n');
    }
}

typedef enum enumSquare enumSquare;

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

typedef enum enumDirection enumDirection;

enum enumDirection {
    Nort, NoEa, East, SoEa, Sout, SoWe, West, NoWe
};

// attacks from each square in each direction on empty board
uint64_t rayAttacks[64][8];

// attacks for sliding pieces from each square on empty board
uint64_t emptyRookAttacks[64];
uint64_t emptyBishopAttacks[64];
uint64_t emptyQueenAttacks[64];

// attacks for non-sliding pieces from each square
uint64_t knightAttacks[64];
uint64_t pawnAttacks[64][2];
uint64_t kingAttacks[64];

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

// setup piece attacks
void setup_piece_attacks() {
    for (int i = 0; i < 64; i ++) {
        emptyRookAttacks[i] = rayAttacks[i][Nort] | rayAttacks[i][East] | 
        rayAttacks[i][Sout] | rayAttacks[i][West];
    }

    for (int i = 0; i < 64; i ++) {
        emptyBishopAttacks[i] = rayAttacks[i][NoEa] | rayAttacks[i][SoEa] | 
        rayAttacks[i][SoWe] | rayAttacks[i][NoWe];
    }

    for (int i = 0; i < 64; i ++) {
        emptyQueenAttacks[i] = emptyBishopAttacks[i] | emptyRookAttacks[i];
    }

    for (int i = 0; i < 64; i ++) {
        uint64_t east = east_one(1LL << i);
        uint64_t west = west_one(1LL << i);
        knightAttacks[i] = (east | west) << 16;
        knightAttacks[i] |= (east | west) >> 16;
        east = east_one(east);
        west = west_one(west);
        knightAttacks[i] |= (east | west) << 8;
        knightAttacks[i] |= (east | west) >> 8;
    }

    for (int i = 8; i < 56; i ++) {
        uint64_t east = east_one(1LL << i);
        uint64_t west = west_one(1LL << i);
        pawnAttacks[i][White] = (east | west) << 8;
        pawnAttacks[i][Black] = (east | west) >> 8;
    }

    for (int i = 0; i < 64; i ++) {
        uint64_t startSquare = 1LL << i;
        uint64_t east = east_one(startSquare);
        uint64_t west = west_one(startSquare);
        kingAttacks[i] = east | west;
        kingAttacks[i] |= (startSquare << 8) | (startSquare >> 8);
        kingAttacks[i] |= (east | west) << 8;
        kingAttacks[i] |= (east | west) >> 8;
    }
}

// get ray attacks in positive direction on occupied board
uint64_t get_positive_ray_attacks(uint64_t occupied, int square, enumDirection dir) {
    uint64_t attacks = rayAttacks[square][dir];
    uint64_t blockers = attacks & occupied;
    if (blockers) {
        attacks ^= rayAttacks[bitscan_forward(blockers)][dir];
    }
    return attacks;
}

// get ray attacks in negative direction on occupied board
uint64_t get_negative_ray_attacks(uint64_t occupied, int square, enumDirection dir) {
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

// hash table for fancy magic bitboards
// table contains all attacks for bishops and rooks on an occupied board
uint64_t attackTable[107648];

typedef struct SMagic SMagic;

// magic info for square
struct SMagic {
    uint64_t* ptr; // pointer to index of square's attack table
    uint64_t mask; // possible blockers
    uint64_t magic;
    int shift; // 64 - number of set bits in mask
};

uint64_t bishopMagics[64] = {
    0x440049104032280LL, 0x1021023c82008040LL, 0x404040082000048LL, 
    0x48c4440084048090LL, 0x2801104026490000LL, 0x4100880442040800LL, 
    0x181011002e06040LL, 0x9101004104200e00LL, 0x1240848848310401LL, 
    0x2000142828050024LL, 0x1004024d5000LL, 0x102044400800200LL, 
    0x8108108820112000LL, 0xa880818210c00046LL, 0x4008008801082000LL, 
    0x60882404049400LL, 0x104402004240810LL, 0xa002084250200LL, 
    0x100b0880801100LL, 0x4080201220101LL, 0x44008080a00000LL, 
    0x202200842000LL, 0x5006004882d00808LL, 0x200045080802LL, 
    0x86100020200601LL, 0xa802080a20112c02LL, 0x80411218080900LL, 
    0x200a0880080a0LL, 0x9a01010000104000LL, 0x28008003100080LL, 
    0x211021004480417LL, 0x401004188220806LL, 0x825051400c2006LL,
    0x140c0210943000LL, 0x242800300080LL, 0xc2208120080200LL, 
    0x2430008200002200LL, 0x1010100112008040LL, 0x8141050100020842LL, 
    0x822081014405LL, 0x800c049e40400804LL, 0x4a0404028a000820LL, 
    0x22060201041200LL, 0x360904200840801LL, 0x881a08208800400LL, 
    0x60202c00400420LL, 0x1204440086061400LL, 0x8184042804040LL, 
    0x64040315300400LL, 0xc01008801090a00LL, 0x808010401140c00LL, 
    0x4004830c2020040LL, 0x80005002020054LL, 0x40000c14481a0490LL, 
    0x10500101042048LL, 0x1010100200424000LL, 0x640901901040LL, 
    0xa0201014840LL, 0x840082aa011002LL, 0x10010840084240aLL, 
    0x420400810420608LL, 0x8d40230408102100LL, 0x4a00200612222409LL, 
    0xa08520292120600LL
};

uint64_t bishopBits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6
};

uint64_t rookMagics[64] = {
    0xa080041040028020LL, 0xa040200010004000LL, 0x8080200010011880LL, 
    0x380180080141000LL, 0x1a00060008211044LL, 0x410001000a0c0008LL, 
    0x9500060004008100LL, 0x100024284a20700LL, 0x802140008000LL, 
    0x80c01002a00840LL, 0x402004282011020LL, 0x9862000820420050LL, 
    0x1001448011100LL, 0x6432800200800400LL, 0x40100010002000cLL, 
    0x2800d0010c080LL, 0x90c0008000803042LL, 0x4010004000200041LL, 
    0x3010010200040LL, 0xa40828028001000LL, 0x123010008000430LL, 
    0x24008004020080LL, 0x60040001104802LL, 0x582200028400d1LL, 
    0x4000802080044000LL, 0x408208200420308LL, 0x610038080102000LL, 
    0x3601000900100020LL, 0x80080040180LL, 0xc2020080040080LL, 
    0x80084400100102LL, 0x4022408200014401LL, 0x40052040800082LL, 
    0xb08200280804000LL, 0x8a80a008801000LL, 0x4000480080801000LL, 
    0x911808800801401LL, 0x822a003002001894LL, 0x401068091400108aLL, 
    0x4a10a00004cLL, 0x2000800640008024LL, 0x1486408102020020LL, 
    0x100a000d50041LL, 0x810050020b0020LL, 0x204000800808004LL, 
    0x20048100a000cLL, 0x112000831020004LL, 0x9000040810002LL, 
    0x440490200208200LL, 0x8910401000200040LL, 0x6404200050008480LL, 
    0x4b824a2010010100LL, 0x4080801810c0080LL, 0x400802a0080LL, 
    0x8224080110026400LL, 0x40002c4104088200LL, 0x1002100104a0282LL, 
    0x1208400811048021LL, 0x3201014a40d02001LL, 0x5100019200501LL, 
    0x101000208001005LL, 0x2008450080702LL, 0x1002080301d00cLL, 
    0x410201ce5c030092LL
};

uint64_t rookBits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12
};

SMagic bishopMagicTable[64];
SMagic rookMagicTable[64];

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

void setup_magics() {
    // setup bishop magics
    uint64_t* nextPtr = attackTable;
    for (int i = 0; i < 64; i ++) {
        bishopMagicTable[i].ptr = nextPtr;
        bishopMagicTable[i].magic = bishopMagics[i];
        bishopMagicTable[i].mask = emptyBishopAttacks[i] & get_not_edge(i);
        bishopMagicTable[i].shift = 64 - bishopBits[i];
        nextPtr += 1 << bishopBits[i];
    }

    // setup rook magics
    for (int i = 0; i < 64; i ++) {
        rookMagicTable[i].ptr = nextPtr;
        rookMagicTable[i].magic = rookMagics[i];
        rookMagicTable[i].mask = emptyRookAttacks[i] & get_not_edge(i);
        rookMagicTable[i].shift = 64 - rookBits[i];
        nextPtr += 1 << rookBits[i];
    }
}

// get index of attack bitboard in table
uint64_t bishop_magic_hash(uint64_t occupied, int square) {
    occupied &= bishopMagicTable[square].mask;
    occupied *= bishopMagicTable[square].magic;
    occupied >>= bishopMagicTable[square].shift;
    return occupied;
}

// get bishop attack bitboard given blockers
uint64_t get_bishop_attacks_magic(uint64_t occupied, int square) {
    uint64_t* all_attacks = bishopMagicTable[square].ptr;
    return all_attacks[bishop_magic_hash(occupied, square)];
}

// set entry in attack table to appropiate attack bitboard
void set_bishop_attacks_magic(uint64_t occupied, int square) {
    uint64_t* all_attacks = bishopMagicTable[square].ptr;
    all_attacks[bishop_magic_hash(occupied, square)] = get_bishop_attacks_classical(occupied, square);
}

// get index of attack bitboard in table
uint64_t rook_magic_hash(uint64_t occupied, int square) {
    occupied &= rookMagicTable[square].mask;
    occupied *= rookMagicTable[square].magic;
    occupied >>= rookMagicTable[square].shift;
    return occupied;
}

// get rook attack bitboard given blockers
uint64_t get_rook_attacks_magic(uint64_t occupied, int square) {
    uint64_t* all_attacks = rookMagicTable[square].ptr;
    return all_attacks[rook_magic_hash(occupied, square)];
}

// set entry in attack table to appropiate attack bitboard
void set_rook_attacks_magic(uint64_t occupied, int square) {
    uint64_t* all_attacks = rookMagicTable[square].ptr;
    all_attacks[rook_magic_hash(occupied, square)] = 
    get_rook_attacks_classical(occupied, square);
}

void setup_attack_table() {
    // setup bishop attacks
    for (int i = 0; i < 64; i ++) {
        uint64_t blockers = 0;
        uint64_t totalNumBlockers = 1 << bishopBits[i];
        for (int j = 0; j < totalNumBlockers; j ++) {
            set_bishop_attacks_magic(blockers, i);
            blockers = (blockers - bishopMagicTable[i].mask) & 
            bishopMagicTable[i].mask; // carry rippler trick to traverse all subsets of mask
        }
    }

    // setup rook attacks
    for (int i = 0; i < 64; i ++) {
        uint64_t blockers = 0;
        uint64_t totalNumBlockers = 1 << rookBits[i];
        for (int j = 0; j < totalNumBlockers; j ++) {
            set_rook_attacks_magic(blockers, i);
            blockers = (blockers - rookMagicTable[i].mask) & 
            rookMagicTable[i].mask; // carry rippler trick to traverse all subsets of mask
        }
    }
}

uint64_t* get_directional_attacks(uint64_t bishops, uint64_t rooks, uint64_t queens, uint64_t occupied) {
    uint64_t* attacks = (uint64_t*) malloc(sizeof(uint64_t) * 8);

    for (int i = 0; i < 8; i ++) {
        attacks[i] = 0LL;
    }
    
    // bishop attacks
    int i = bitscan_forward(bishops);
    while (i != 0) {
        uint64_t bishopAttacks = get_bishop_attacks_magic(occupied, i);
        attacks[NoEa] |= bishopAttacks & rayAttacks[i][NoEa];
        attacks[SoEa] |= bishopAttacks & rayAttacks[i][SoEa];
        attacks[SoWe] |= bishopAttacks & rayAttacks[i][SoWe];
        attacks[NoWe] |= bishopAttacks & rayAttacks[i][NoWe];
        bishops &= bishops - 1;
        i = bitscan_forward(bishops);
    }

    // rook attacks
    i = bitscan_forward(rooks);
    while (i != 0) {
        uint64_t rookAttacks = get_rook_attacks_magic(occupied, i);
        attacks[Nort] |= rookAttacks & rayAttacks[i][Nort];
        attacks[East] |= rookAttacks & rayAttacks[i][East];
        attacks[Sout] |= rookAttacks & rayAttacks[i][Sout];
        attacks[West] |= rookAttacks & rayAttacks[i][West];
        rooks &= rooks - 1;
        i = bitscan_forward(rooks);
    }

    // queen attacks
    i = bitscan_forward(queens);
    while (i != 0) {
        uint64_t queenAttacks = get_bishop_attacks_magic(occupied, i) | get_rook_attacks_magic(occupied, i);
        attacks[Nort] |= queenAttacks & rayAttacks[i][Nort];
        attacks[NoEa] |= queenAttacks & rayAttacks[i][NoEa];
        attacks[East] |= queenAttacks & rayAttacks[i][East];
        attacks[SoEa] |= queenAttacks & rayAttacks[i][SoEa];
        attacks[Sout] |= queenAttacks & rayAttacks[i][Sout];
        attacks[SoWe] |= queenAttacks & rayAttacks[i][SoWe];
        attacks[West] |= queenAttacks & rayAttacks[i][West];
        attacks[NoWe] |= queenAttacks & rayAttacks[i][NoWe];
        queens &= queens - 1;
        i = bitscan_forward(queens);
    }

    return attacks;
}

uint64_t get_all_knight_attacks(uint64_t knights) {
    uint64_t attacks = 0;
    int i = bitscan_forward(knights);
    while (i != 0) {
        attacks |= knightAttacks[i];
        knights &= knights - 1;
        i = bitscan_forward(knights);
    }
    return attacks;
}

uint64_t get_all_pawn_attacks(uint64_t pawns, pieceColor side) {
    uint64_t attacks = 0;
    int i = bitscan_forward(pawns);
    while (i != 0) {
        attacks |= pawnAttacks[i][side];
        pawns &= pawns - 1;
        i = bitscan_forward(pawns);
    }
    return attacks;
}

uint64_t get_king_attacks(uint64_t king) {
    return kingAttacks[bitscan_forward(king)];
}

// get list of moves from bitboard of attacks
shortlist* get_moves_from_uint64(int start, uint64_t end, uint64_t friendlyPieces, uint64_t opponentPieces) {
    shortlist* moves = NULL;
    end &= ~friendlyPieces;
    int i = bitscan_forward(end); // square of set bit
    shortlist* tail;

    while (i != 0) {
        int isCapture = (opponentPieces >> i) & 1;
        shortlist* item = cons(start | (i << 6) | (isCapture << 14), NULL);
        if (moves) {
            tail->next = item; // rest of items
        } else {
            moves = item; // first item
        }
        tail = item;
        end &= end - 1;
        i = bitscan_forward(end);
    }

    return moves;
}

// convert move to string
char* move_to_string(int move) {
    char* result = (char*) malloc(sizeof(char) * 9);
    char start = move & 0x3F;
    move >>= 6;
    char end = move & 0x3F;
    move >>= 6;
    char flag = move;
    result[0] = 'a' + (start % 8);
    result[1] = '1' + (start / 8);
    result[2] = '-';
    result[3] = 'a' + (end % 8);
    result[4] = '1' + (end / 8);
    result[5] = ' ';
    if (flag > 9) {
        result[6] = '1';
        result[7] = '0' + (flag - 10);
    } else {
        result[6] = '0' + flag;
        result[7] = ' ';
    }
    result[8] = '\0';
    return result;
}

// get the squares attacked by opponent and get directional attacks
uint64_t get_attacked_squares(chessboard* board, uint64_t occupied, uint64_t** directionalAttacksPtr) {
    uint64_t attacked = 0LL; // all squares attacked by opponent

    if (board->turn == White) {
        *directionalAttacksPtr = get_directional_attacks(board->pieces[BlackBishop], board->pieces[BlackRook], board->pieces[BlackQueen], occupied);
        attacked |= get_all_knight_attacks(board->pieces[BlackKnight]);
        attacked |= get_all_pawn_attacks(board->pieces[BlackPawn], Black);
        attacked |= get_king_attacks(board->pieces[BlackKing]);
    } else {
        *directionalAttacksPtr = get_directional_attacks(board->pieces[WhiteBishop], board->pieces[WhiteRook], board->pieces[WhiteQueen], occupied);
        attacked |= get_all_knight_attacks(board->pieces[WhiteKnight]);
        attacked |= get_all_pawn_attacks(board->pieces[WhitePawn], Black);
        attacked |= get_king_attacks(board->pieces[WhiteKing]);
    }

    for (int i = 0; i < 8; i ++) {
        attacked |= (*directionalAttacksPtr)[i];
    }

    return attacked;
}

// get moves for pinned piece along diagonal and update 'allPinnedPieces'
shortlist* get_pinned_diagonal_moves(chessboard* board, enumDirection dir, uint64_t* directionalAttacks, uint64_t kingBishopMoves, int kingSquare, uint64_t friendlyPieces, uint64_t opponentPieces, uint64_t* allPinnedPieces) {
    uint64_t pinnedPiece = kingBishopMoves & rayAttacks[kingSquare][dir] & directionalAttacks[(dir + 4) % 8];
    if (popCount(pinnedPiece) == 1) {
        *allPinnedPieces |= pinnedPiece;
        if (pinnedPiece & (board->turn == White ? board->pieces[BlackBishop] | board->pieces[BlackQueen] : board->pieces[WhiteBishop] | board->pieces[WhiteQueen])) {
            return get_moves_from_uint64(bitscan_forward(pinnedPiece), kingBishopMoves & rayAttacks[kingSquare][dir], friendlyPieces, opponentPieces);
        }
    }
    return NULL;
}

// get moves for pinned piece along file/rank and update 'allPinnedPieces'
shortlist* get_pinned_straight_moves(chessboard* board, enumDirection dir, uint64_t* directionalAttacks, uint64_t kingRookMoves, int kingSquare, uint64_t friendlyPieces, uint64_t opponentPieces, uint64_t* allPinnedPieces) {
    uint64_t pinnedPiece = kingRookMoves & rayAttacks[kingSquare][dir] & directionalAttacks[(dir + 4) % 8];
    if (popCount(pinnedPiece) == 1) {
        *allPinnedPieces |= pinnedPiece;
        if (pinnedPiece & (board->turn == White ? board->pieces[BlackRook] | board->pieces[BlackQueen] : board->pieces[WhiteRook] | board->pieces[WhiteQueen])) {
            return get_moves_from_uint64(bitscan_forward(pinnedPiece), kingRookMoves & rayAttacks[kingSquare][dir], friendlyPieces, opponentPieces);
        }
    }
    return NULL;
}

shortlist* get_all_bishop_moves(uint64_t bishops, uint64_t occupied, uint64_t friendlyPieces, uint64_t opponentPieces, uint64_t mask) {
    shortlist* result = NULL;
    shortlist* tail = NULL;
    int i = bitscan_forward(bishops);
    while (i != 0) {
        shortlist* bishopMoves = get_moves_from_uint64(i, get_bishop_attacks_magic(occupied, i) & mask, friendlyPieces, opponentPieces);
        append_list(bishopMoves, &result, &tail);
        bishops &= bishops - 1;
        i = bitscan_forward(bishops);
    }
    return result;
}

shortlist* get_all_rook_moves(uint64_t rooks, uint64_t occupied, uint64_t friendlyPieces, uint64_t opponentPieces, uint64_t mask) {
    shortlist* result = NULL;
    shortlist* tail = NULL;
    int i = bitscan_forward(rooks);
    while (i != 0) {
        shortlist* rookMoves = get_moves_from_uint64(i, get_rook_attacks_magic(occupied, i) & mask, friendlyPieces, opponentPieces);
        append_list(rookMoves, &result, &tail);
        rooks &= rooks - 1;
        i = bitscan_forward(rooks);
    }
    return result;
}

shortlist* get_all_queen_moves(uint64_t queens, uint64_t occupied, uint64_t friendlyPieces, uint64_t opponentPieces, uint64_t mask) {
    shortlist* result = NULL;
    shortlist* tail = NULL;
    int i = bitscan_forward(queens);
    while (i != 0) {
        shortlist* queenMoves = get_moves_from_uint64(i, (get_bishop_attacks_magic(occupied, i) | get_rook_attacks_magic(occupied, i)) & mask, friendlyPieces, opponentPieces);
        append_list(queenMoves, &result, &tail);
        queens &= queens - 1;
        i = bitscan_forward(queens);
    }
    return result;
}

shortlist* get_all_knight_moves(uint64_t knights, uint64_t occupied, uint64_t friendlyPieces, uint64_t opponentPieces, uint64_t mask) {
    shortlist* result = NULL;
    shortlist* tail = NULL;
    int i = bitscan_forward(knights);
    while (i != 0) {
        shortlist* knightMoves = get_moves_from_uint64(i, knightAttacks[i] & mask, friendlyPieces, opponentPieces);
        append_list(knightMoves, &result, &tail);
        knights &= knights - 1;
        i = bitscan_forward(knights);
    }
    return result;
}

shortlist* get_pawn_pushes(uint64_t empty, int square, pieceColor turn) {
    shortlist* pawnMoves = NULL;
    int f = turn == White ? 1 : -1;
    int endSquare = square + 8 * f;
    if ((empty >> (endSquare)) & 1) {
        // single push
        pawnMoves = cons(square | (endSquare << 6), NULL);
        endSquare = square + 16 * f;
        if (square >= 28 - 20 * f && square < 36 - 20 * f && (empty >> endSquare) & 1) {
            // double push
            pawnMoves->next = cons(square | (endSquare << 6) | 0x1000, NULL);
        } else if (square >= 28 + 20 * f && square < 36 + 20 * f) {
            // promotion
            short flag = 0x8000;
            short move = pawnMoves->val;
            pawnMoves->val |= flag;
            shortlist* temp = pawnMoves;
            for (int i = 1; i < 4; i ++) {
                temp->next = cons(move | (flag + i), NULL);
                temp = temp->next;
            }
        }
    }
    return pawnMoves;
}

shortlist* get_pawn_captures(uint64_t opponentPieces, int square, pieceColor turn) {
    shortlist* pawnMoves = NULL;
    shortlist* tail = NULL;
    int endSquare = square + 7 - 16 * turn;
    for (int i = 0; i < 2; i ++) {
        if ((opponentPieces >> endSquare) & 1) {
            append_list(cons(square | (endSquare << 6), NULL), &pawnMoves, &tail);
            if (square >= 48 - 40 * turn && square < 56 - 40 * turn) {
                short flag = 0xc000;
                short move = tail->val;
                tail->val |= flag;
                for (int i = 1; i < 4; i ++) {
                    tail->next = cons(move | (flag + i), NULL);
                    tail = tail->next;
                }
            } else {
                tail->val |= 0x4000;
            }
        }
        endSquare += 2;
    }
    return pawnMoves;
}

shortlist* get_all_pawn_pushes(uint64_t pawns, uint64_t occupied, uint64_t mask, pieceColor turn) {
    shortlist* result = NULL;
    shortlist* tail = NULL;
    int i = bitscan_forward(pawns);
    while (i != 0) {
        shortlist* pawnMoves = get_pawn_pushes(~occupied & mask, i, turn);
        append_list(pawnMoves, &result, &tail);
        pawns &= pawns - 1;
        i = bitscan_forward(pawns);
    }
    return result;
}

shortlist* get_all_pawn_captures(uint64_t pawns, uint64_t occupied, uint64_t opponentPieces, uint64_t mask, pieceColor turn) {
    shortlist* result = NULL;
    shortlist* tail = NULL;
    int i = bitscan_forward(pawns);
    while (i != 0) {
        shortlist* pawnMoves = get_pawn_captures(opponentPieces, i, turn);
        append_list(pawnMoves, &result, &tail);
        pawns &= pawns - 1;
        i = bitscan_forward(pawns);
    }
    return result;
}

shortlist* get_all_pawn_en_passant(uint64_t pawns, uint64_t occupied, uint64_t opponentSliders, uint64_t pushMask, uint64_t captureMask, int kingSquare, int epSquare, pieceColor turn) {
    shortlist* result = NULL;
    shortlist* tail = NULL;

    if (epSquare > 0) {
        uint64_t leftPawn = pawns & (1LL << (epSquare - 1));
        uint64_t rightPawn = pawns & (1LL << (epSquare + 1));
        uint64_t capturePawn = 1LL << epSquare;
        int pushSquare = epSquare + 8 - 16 * turn;
        uint64_t push = ~occupied & (1LL < pushSquare);
        if (leftPawn && push && ((leftPawn & captureMask) || (push & pushMask))) {
            // check for check
            if (!(get_rook_attacks_magic(occupied & ~(leftPawn | capturePawn), kingSquare) & opponentSliders)) {
                append_list(cons((epSquare - 1) | (pushSquare << 6) | 0x5000, NULL), &result, &tail);
            }
        }
        if (rightPawn && push && ((rightPawn & captureMask) || (push & pushMask))) {
            // check for check
            if (!(get_rook_attacks_magic(occupied & ~(rightPawn | capturePawn), kingSquare) & opponentSliders)) {
                append_list(cons((epSquare + 1) | (pushSquare << 6) | 0x5000, NULL), &result, &tail);
            }
        }
    }

    return tail;
}

shortlist* get_all_moves(chessboard* board) {
    shortlist* moves = NULL;
    shortlist* tail = NULL;
    
    uint64_t whitePieces = board->pieces[WhiteKing] | board->pieces[WhiteBishop] | board->pieces[WhiteRook] | board->pieces[WhiteQueen] | board->pieces[WhiteKnight] | board->pieces[WhitePawn];
    uint64_t blackPieces = board->pieces[BlackKing] | board->pieces[BlackBishop] |
    board->pieces[BlackRook] | board->pieces[BlackQueen] | board->pieces[BlackKnight] | board->pieces[BlackPawn];
    uint64_t friendlyPieces = board->turn == White ? whitePieces : blackPieces;
    uint64_t opponentPieces = board->turn == White ? blackPieces : whitePieces;
    uint64_t occupied = friendlyPieces | opponentPieces; // all occupied squares

    occupied &= board->turn == White ? ~board->pieces[WhiteKing] : ~board->pieces[BlackKing]; // remove king

    uint64_t* directionalAttacks; // attacks from each direction
    uint64_t attacked = get_attacked_squares(board, occupied, &directionalAttacks); // all attacked squares

    int kingSquare = bitscan_forward(board->turn == White ? board->pieces[WhiteKing] : board->pieces[BlackKing]); // square of king
   
    // 1. get king moves

    append_list(get_moves_from_uint64(kingSquare, kingAttacks[kingSquare] & ~attacked, friendlyPieces, opponentPieces), &moves, &tail);

    // king-side castle
    if (board->castleKing[board->turn] && !((attacked | occupied) & (0x70LL << (56 * board->turn)))) {
        append_list(cons(kingSquare | (kingSquare + 2) << 6 | 0x200, NULL), &moves, &tail);
    } 

    // queen-side castle
    if (board->castleQueen[board->turn] && !(occupied & (0x1eLL << (56 * board->turn))) && !(attacked & (0x1cLL << (56 * board->turn)))) {
        append_list(cons(kingSquare | (kingSquare - 2) << 6 | 0x200, NULL), &moves, &tail);
    }

    occupied |= board->turn == White ? board->pieces[WhiteKing] : board->pieces[BlackKing]; // add king back in
    
    // 2. handle check

    uint64_t checkers; // squares of all checking pieces

    // moves from king
    uint64_t kingBishopMoves = get_bishop_attacks_magic(occupied, kingSquare);
    uint64_t kingRookMoves = get_rook_attacks_magic(occupied, kingSquare);
    uint64_t kingQueenMoves = kingBishopMoves | kingRookMoves;
    uint64_t kingKnightMoves = knightAttacks[kingSquare];
    uint64_t kingPawnMoves = board->turn == White ? pawnAttacks[kingSquare][White] : pawnAttacks[kingSquare][Black];

    uint64_t bishopCheckers, rookCheckers, queenCheckers, knightCheckers, pawnCheckers;

    if (board->turn == White) {
        bishopCheckers = kingBishopMoves & board->pieces[BlackBishop];
        rookCheckers = kingRookMoves & board->pieces[BlackRook];
        queenCheckers = kingQueenMoves & board->pieces[BlackQueen];
        knightCheckers = kingKnightMoves & board->pieces[BlackKnight];
        pawnCheckers = kingPawnMoves & board->pieces[BlackPawn];
    } else {
        bishopCheckers = kingBishopMoves & board->pieces[WhiteBishop];
        rookCheckers = kingRookMoves & board->pieces[WhiteRook];
        queenCheckers = kingQueenMoves & board->pieces[WhiteQueen];
        knightCheckers = kingKnightMoves & board->pieces[WhiteKnight];
        pawnCheckers = kingPawnMoves & board->pieces[WhitePawn];
    }

    checkers = bishopCheckers | rookCheckers | queenCheckers | knightCheckers | pawnCheckers;
    int numCheckers = popCount(checkers); // number of checking pieces

    if (numCheckers >= 2) {
        return moves; // double check; only king moves
    }

    uint64_t pushMask = 0xFFFFFFFFFFFFFFFFLL; // mask of allowable moves
    uint64_t captureMask = 0xFFFFFFFFFFFFFFFFLL; // mask of allowable captures

    // single check
    if (numCheckers == 1) {
        captureMask = checkers;
        if (bishopCheckers > 0) {
            pushMask = emptyBishopAttacks[bitscan_forward(bishopCheckers)] & kingBishopMoves;
        } else if (rookCheckers > 0) {
            pushMask = emptyRookAttacks[bitscan_forward(rookCheckers)] & kingRookMoves;
        } else if (queenCheckers > 0) {
            if (emptyBishopAttacks[kingSquare] & queenCheckers) {
                pushMask = emptyBishopAttacks[bitscan_forward(queenCheckers)] & kingBishopMoves;
            } else {
                pushMask = emptyRookAttacks[bitscan_forward(queenCheckers)] & kingRookMoves;
            }
        } else {
            pushMask = 0LL;
        }
    }

    // 3. get moves for pinned pieces
    
    uint64_t allPinnedPieces = 0;

    for (int i = 1; i < 8; i += 2) {
        shortlist* pinnedMoves = get_pinned_diagonal_moves(board, i, directionalAttacks, kingBishopMoves, kingSquare, friendlyPieces, opponentPieces, &allPinnedPieces);
        append_list(pinnedMoves, &moves, &tail);
    }
    
    for (int i = 0; i < 8; i += 2) {
        shortlist* pinnedMoves = get_pinned_straight_moves(board, i, directionalAttacks, kingRookMoves, kingSquare, friendlyPieces, opponentPieces, &allPinnedPieces);
        append_list(pinnedMoves, &moves, &tail);
    }

    // 4. get moves for all other pieces
    
    uint64_t friendlyBishops, friendlyRooks, friendlyQueens, friendlyKnights, friendlyPawns;
    uint64_t notPinned = ~allPinnedPieces;
    if (board->turn == White) {
        friendlyBishops = board->pieces[WhiteBishop] & notPinned;
        friendlyRooks = board->pieces[WhiteRook] & notPinned;
        friendlyQueens = board->pieces[WhiteQueen] & notPinned;
        friendlyKnights = board->pieces[WhiteKnight] & notPinned;
        friendlyPawns = board->pieces[WhitePawn] & notPinned;
    } else {
        friendlyBishops = board->pieces[BlackBishop] & notPinned;
        friendlyRooks = board->pieces[BlackRook] & notPinned;
        friendlyQueens = board->pieces[BlackQueen] & notPinned;
        friendlyKnights = board->pieces[BlackKnight] & notPinned;
        friendlyPawns = board->pieces[BlackPawn] & notPinned;
    }

    append_list(get_all_bishop_moves(friendlyBishops, occupied, friendlyPieces, opponentPieces, pushMask | captureMask), &moves, &tail);
    append_list(get_all_rook_moves(friendlyRooks, occupied, friendlyPieces, opponentPieces, pushMask | captureMask), &moves, &tail);
    append_list(get_all_queen_moves(friendlyQueens, occupied, friendlyPieces, opponentPieces, pushMask | captureMask), &moves, &tail);
    append_list(get_all_knight_moves(friendlyKnights, occupied, friendlyPieces, opponentPieces, pushMask | captureMask), &moves, &tail);
    append_list(get_all_pawn_pushes(friendlyPawns, occupied, pushMask, board->turn), &moves, &tail);
    append_list(get_all_pawn_captures(friendlyPawns, occupied, opponentPieces, captureMask, board->turn), &moves, &tail);
    uint64_t opponentSliders = board->turn == White ? (board->pieces[BlackRook] | board->pieces[BlackQueen]) : (board->pieces[WhiteRook] | board->pieces[WhiteQueen]);
    append_list(get_all_pawn_en_passant(friendlyPawns, occupied, opponentSliders, pushMask, captureMask, kingSquare, board->epSquare, board->turn), &moves, &tail);

    return moves;
}

void make_capture(chessboard* board, short square) {
    short capturedPiece = get_board_piece(board, square);
    board->pieces[capturedPiece] &= ~(1LL << square);
    board->captureLogTail->val = capturedPiece;
}

void make_move(chessboard* board, short move) {
    short startSquare = move & 0x3F;
    short piece = get_board_piece(board, startSquare);
    board->pieces[piece] &= ~(1LL << startSquare); // remove piece
    move >>= 6;
    short endSquare = move & 0x3F;
    move >>= 6;

    board->epSquare = -1; // reset en passant target

    // set log entries to default value
    append_list(cons(-1, NULL), &board->captureLog, &board->captureLogTail); 
    append_list(cons(board->castleKing[board->turn], NULL), &board->castleKingLog, &board->castleKingLogTail);
    append_list(cons(board->castleQueen[board->turn], NULL), &board->castleQueenLog, &board->castleQueenLogTail);
    append_list(cons(board->epSquare, NULL), &board->epLog, &board->epLogTail);

    // remove castling rights
    if (piece == White) {
        if (piece == WhiteRook) {
            if (startSquare == 0) {
                board->castleQueen[White] = 0;
            } else if (startSquare == 7) {
                board->castleKing[White] = 0;
            }
        } else if (piece == WhiteKing) {
            board->castleQueen[White] = 0;
            board->castleKing[White] = 0;
        }
    } else {
        if (piece == BlackRook) {
            if (startSquare == 56) {
                board->castleQueen[Black] = 0;
            } else if (startSquare == 63) {
                board->castleKing[Black] = 0;
            }
        } else if (piece == BlackKing) {
            board->castleQueen[Black] = 0;
            board->castleKing[Black] = 0;
        }
    }

    // handle flag
    switch (move) {
        case 0:
            break;
        case 1:
            board->epSquare = endSquare;
            break;
        case 2:
            if (board->turn == White) {
                board->pieces[WhiteRook] &= ~1LL;
                board->pieces[WhiteRook] |= 8LL;
            } else {
                board->pieces[BlackRook] &= ~0x100000000000000LL;
                board->pieces[BlackRook] |= 0x800000000000000LL;
            }
            break;
        case 3:
            if (board->turn == White) {
                board->pieces[WhiteRook] &= ~0x80LL;
                board->pieces[WhiteRook] |= 0x10LL;
            } else {
                board->pieces[BlackRook] &= ~0x8000000000000000LL;
                board->pieces[BlackRook] |= 0x1000000000000000LL;
            }
            break;
        case 4: 
            make_capture(board, endSquare);
            break;
        case 5: 
            make_capture(board, board->turn == White ? endSquare - 8 : endSquare + 8);
            break;
        case 8: 
            board->pieces[piece] &= ~(1 << endSquare);
            board->pieces[board->turn == White ? WhiteKnight : BlackKnight] |= 1 << endSquare;
            break;
        case 9:
            board->pieces[piece] &= ~(1 << endSquare);
            board->pieces[board->turn == White ? WhiteBishop : BlackBishop] |= 1 << endSquare;
            break;
        case 10:
            board->pieces[piece] &= ~(1 << endSquare);
            board->pieces[board->turn == White ? WhiteRook : BlackRook] |= 1 << endSquare;
            break;
        case 11:
            board->pieces[piece] &= ~(1 << endSquare);
            board->pieces[board->turn == White ? WhiteQueen : BlackQueen] |= 1 << endSquare;
            break;
        case 12: 
            board->pieces[piece] &= ~(1 << endSquare);
            make_capture(board, endSquare);
            board->pieces[board->turn == White ? WhiteKnight : BlackKnight] |= 1 << endSquare;
            break;
        case 13:
            board->pieces[piece] &= ~(1 << endSquare);
            make_capture(board, endSquare);
            board->pieces[board->turn == White ? WhiteBishop : BlackBishop] |= 1 << endSquare;
            break;
        case 14:
            board->pieces[piece] &= ~(1 << endSquare);
            make_capture(board, endSquare);
            board->pieces[board->turn == White ? WhiteRook : BlackRook] |= 1 << endSquare;
            break;
        case 15:
            board->pieces[piece] &= ~(1 << endSquare);
            make_capture(board, endSquare);
            board->pieces[board->turn == White ? WhiteQueen : BlackQueen] |= 1 << endSquare;
    }

    board->pieces[piece] |= 1LL << endSquare; // place piece

    board->turn = 1 - board->turn; // change turn
}

int setup() {
    setup_ms1b_table();
    setup_ray_attacks();
    setup_piece_attacks();
    setup_magics();
    setup_attack_table();
    srand(3);
    return 0;
}

int main(int argc, char* argv[]) {
    setup();
    chessboard board = new_board("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    shortlist* moves = get_all_moves(&board);
    for (int i = 0; i < 10; i ++) {
        shortlist* temp = moves;
        moves = get_all_moves(&board);
        int moveNum = get_list_length(moves);
        free(temp);
        print_board(&board);
        printf("\n");
        make_move(&board, get_item(moves, rand() % moveNum)->val);
    }
}
