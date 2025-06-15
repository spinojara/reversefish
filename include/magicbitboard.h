#ifndef MAGICBITBOARD_H
#define MAGICBITBOARD_H

#include <stdint.h>

struct magic {
	uint64_t *attacks;
	uint64_t mask;
	uint64_t magic;
	unsigned shift;
};

extern struct magic bishop_magic[64];
extern struct magic rook_magic[64];

static inline uint64_t magic_index(const struct magic *magic, uint64_t b) {
#ifdef PEXT
	return _pext_u64(b, magic->mask);
#else
	return ((b & magic->mask) * magic->magic) >> magic->shift;
#endif
}

static inline uint64_t bishop_attacks_pre(int square, uint64_t b) {
	const struct magic *magic = &bishop_magic[square];
	return magic->attacks[magic_index(magic, b)];
}

static inline uint64_t rook_attacks_pre(int square, uint64_t b) {
	const struct magic *magic = &rook_magic[square];
	return magic->attacks[magic_index(magic, b)];
}

void magicbitboard_init(void);

#endif
