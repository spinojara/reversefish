#include "move.h"

#include <stdint.h>
#include <string.h>

#include "bitboard.h"
#include "magicbitboard.h"

move_t *movegen(move_t *moves, const struct position *pos) {
	uint64_t pieces = pos->piece[pos->turn];
	uint64_t attacks;
	int sq;
	uint64_t allmoves = 0;
	while (pieces) {
		sq = ctz(pieces);

		attacks = bishop_attacks_pre(sq, pos->piece[other_color(pos->turn)]) | rook_attacks_pre(sq, pos->piece[other_color(pos->turn)]);
		while (attacks) {
			sq = ctz(attacks);

			if (!get_bit(pos->piece[WHITE] | pos->piece[BLACK] | allmoves, sq)) {
				allmoves |= bitboard(sq);
				*moves++ = sq;
			}

			attacks = clear_ls1b(attacks);
		}

		while (attacks) {
			sq = ctz(attacks);

			if (!get_bit(pos->piece[WHITE] | pos->piece[BLACK] | allmoves, sq)) {
				allmoves |= bitboard(sq);
				*moves++ = sq;
			}

			attacks = clear_ls1b(attacks);
		}

		pieces = clear_ls1b(pieces);
	}
	*moves++ = MOVE_NULL;
	return moves;
}

void do_move(struct position *pos, move_t move) {
	uint64_t attacks = rook_attacks_pre(move, pos->piece[other_color(pos->turn)]) | bishop_attacks_pre(move, pos->piece[other_color(pos->turn)]);
	int sq;

	while (attacks) {
		sq = ctz(attacks);

		if (get_bit(pos->piece[pos->turn], sq)) {
			pos->piece[pos->turn] |= between(move, sq);
			pos->piece[other_color(pos->turn)] &= ~pos->piece[pos->turn];
		}

		attacks = clear_ls1b(attacks);
	}

	pos->turn = other_color(pos->turn);
}

int string_to_move(move_t *move, const struct position *pos, const char *str) {
	move_t moves[64];
	movegen(moves, pos);

	if (moves[0] == MOVE_NULL && !strcmp("stand", str)) {
		*move = MOVE_NULL;
		return 0;
	}
	char tmp[3];
	for (int i = 0; moves[i] != MOVE_NULL; i++) {
		if (!strcmp(algebraic(tmp, moves[i]), str)) {
			*move = moves[i];
			return 0;
		}
	}

	return 1;
}
