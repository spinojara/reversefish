#ifndef POSITION_H
#define POSITION_H

#include <assert.h>
#include <stdint.h>

enum square {
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8,
};

enum color { BLACK, WHITE };

struct position {
	uint64_t piece[2];
	int nomove;
	int turn;
};

static inline int other_color(int color) {
	return color ^ WHITE ^ BLACK;
}

static inline int rank_of(int square) {
	assert(0 <= square && square < 64);
	return square >> 3;
}

static inline int file_of(int square) {
	assert(0 <= square && square < 64);
	return square & 0x7;
}

static inline int make_square(int file, int rank) {
	return file + 8 * rank;
}

int square(const char *algebraic);

char *algebraic(char *str, int square);

void print_position(const struct position *pos);

void startpos(struct position *pos);

#endif
