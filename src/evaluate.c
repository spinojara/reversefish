#include "evaluate.h"

#include "bitboard.h"

int evaluate(const struct position *pos) {
	return popcount(pos->piece[pos->turn]) - popcount(pos->piece[other_color(pos->turn)]);
}
