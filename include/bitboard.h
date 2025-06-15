#ifndef BITBOARD_H
#define BITBOARD_H

#include <assert.h>
#include <stdint.h>

#include "position.h"
#include "magicbitboard.h"

extern uint64_t between_lookup[64 * 64];

static inline uint64_t ctz(uint64_t b) {
	assert(b);
	return __builtin_ctzll(b);
}

static inline uint64_t popcount(uint64_t b) {
	return __builtin_popcountll(b);
}

static inline uint64_t bitboard(int i) {
	return (uint64_t)1 << i;
}

static inline uint64_t get_bit(uint64_t b, int i) {
	return b & bitboard(i);
}

static inline uint64_t set_bit(uint64_t b, int i) {
	return b | bitboard(i);
}

static inline uint64_t clear_bit(uint64_t b, int i) {
	return b & ~bitboard(i);
}

static inline uint64_t clear_ls1b(uint64_t b) {
	return b & (b - 1);
}

static inline uint64_t between(int source_square, int target_square) {
	return between_lookup[source_square + target_square * 64];
}

void print_bitboard(uint64_t b);

void bitboard_init(void);

#endif
