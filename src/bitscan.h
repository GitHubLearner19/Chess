#ifndef BITSCAN
#define BITSCAN

#include <stdint.h>

int popCount(uint64_t x);

int bitscan_forward(uint64_t x);

void setup_ms1b_table();

int bitscan_reverse(uint64_t x);

#endif
