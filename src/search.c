#define _POSIX_C_SOURCE 199309L
#include "search.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#include "position.h"
#include "move.h"
#include "evaluate.h"
#include "move.h"
#include "bitboard.h"

#define TPPERSEC 1000000000l
#define TPPERMS     1000000l

struct tt {
	char turn;
	char move;
	char depth;
	int32_t eval;
	uint64_t key;
};
struct tt *tt;
uint64_t size;

static inline uint64_t tt_key(const struct position *pos) {
	return (pos->piece[WHITE] | (pos->piece[BLACK] * 12803411)) ^ ((pos->piece[WHITE] | (pos->piece[BLACK] * 123873)) >> 32);
}

static inline uint64_t tt_index(uint64_t key) {
	return (((key * 12498124013) & 0xFFFFFFFF) * size) >> 32;
}

int64_t time_now(void) {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	return (int64_t)tp.tv_sec * TPPERSEC + (int64_t)tp.tv_nsec;
}

void print_pv(move_t pv[64][64], int depth) {
	char str[3];
	for (int i = 0; i < depth && pv[0][i]; i++)
		printf(" %s", algebraic(str, pv[0][i]));
}

static inline void store_pv_move(const move_t move, int ply, move_t pv[64][64]) {
	assert(0 <= move && move < MOVE_NULL);
	pv[ply][ply] = move;
	memcpy(pv[ply] + ply + 1, pv[ply + 1] + ply + 1, sizeof(**pv) * (64 - (ply + 1)));
}

int32_t negamax(struct position *pos, int depth, int ply, int32_t alpha, int32_t beta, struct searchinfo *si) {
	if (si->interrupt)
		return 0;
	if (!(si->nodes & (0x100 - 1)) && time_now() >= si->maxtime)
		si->interrupt = 1;
	int eval, best_eval = -VALUE_INFINITE;

	const int pv_node = (beta != alpha + 1);

	/* End of game. */
	if (popcount(pos->piece[WHITE] | pos->piece[BLACK]) == 64) {
		eval = evaluate(pos);
		if (eval > 0)
			eval += VALUE_WIN;
		if (eval < 0)
			eval -= VALUE_WIN;
	}

	uint64_t key = tt_key(pos);
	struct tt *entry = &tt[tt_index(key)];
	int tthit = entry->key == key && entry->turn == pos->turn;
	move_t tt_move = entry->move;

	if (tthit && depth <= entry->depth && 0)
		return entry->eval;

	if (depth <= 0)
		return evaluate(pos);

	move_t moves[64];
	movegen(moves, pos);

	if (moves[0] == MOVE_NULL) {
		pos->turn = other_color(pos->turn);
		return -negamax(pos, depth - 1, ply + 1, -beta, -alpha, si);
	}

	if (tthit) {
		for (int i = 0; moves[i] != MOVE_NULL; i++) {
			if (moves[i] == tt_move) {
				moves[i] = moves[0];
				moves[0] = tt_move;
				break;
			}
		}
	}

	move_t best_move = MOVE_NULL;
	for (int i = 0; moves[i] != MOVE_NULL; i++) {
		move_t move = moves[i];
		struct position copy = *pos;
		do_move(&copy, move);
		si->nodes++;
		eval = -negamax(&copy, depth - 1, ply + 1, - beta, -alpha, si);

		if (eval > best_eval) {
			best_eval = eval;

			if (eval > alpha) {
				best_move = move;

				if (!si->interrupt)
					store_pv_move(move, ply, si->pv);

				if (pv_node && eval < beta)
					alpha = eval;
				else
					break;
			}
		}
	}

	if (best_move != MOVE_NULL) {
		entry->turn = pos->turn;
		entry->move = best_move;
		entry->depth = depth;
		entry->eval = best_eval;
		entry->key = key;
	}

	return best_eval;
}

void search(struct position *pos, int depth, move_t *bestmove, int maxtime) {
	struct searchinfo si = { 0 };
	si.starttime = time_now();
	si.maxtime = si.starttime + maxtime * TPPERMS;

	move_t moves[64];
	movegen(moves, pos);
	if (moves[0] == MOVE_NULL) {
		if (bestmove)
			*bestmove = MOVE_NULL;
		printf("bestmove stand\n");
		return;
	}

	for (int d = 1; d <= depth; d++) {
		si.root_depth = d;
		int eval = negamax(pos, d, 0, -VALUE_MAX, VALUE_MAX, &si);
		if (si.interrupt)
			break;
		printf("info depth %d score cp %d nodes %ld time %ld nps %ld pv", si.root_depth, eval, si.nodes, (time_now() - si.starttime) / TPPERMS + 1, TPPERSEC * si.nodes / (time_now() - si.starttime + 1));
		print_pv(si.pv, si.root_depth);
		printf("\n");
	}

	if (bestmove)
		*bestmove = si.pv[0][0];
	char str[3];
	printf("bestmove %s\n", algebraic(str, si.pv[0][0]));
}

void search_init(void) {
	size = ((uint64_t)TT * 1024 * 1024) / sizeof(struct tt);
	tt = calloc(size, sizeof(struct tt));
	if (!tt) {
		fprintf(stderr, "error: malloc\n");
		exit(4);
	}

	for (uint64_t i = 0; i < size; i++) {
		tt[i].move = MOVE_NULL;
	}
}
