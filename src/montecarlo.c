#define _POSIX_C_SOURCE 199309L
#include "montecarlo.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "search.h"
#include "util.h"

#define TPPERSEC 1000000000l
#define TPPERMS     1000000l

const double c = 1.414;

int64_t time_now(void) {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC_RAW, &tp);
	return (int64_t)tp.tv_sec * TPPERSEC + (int64_t)tp.tv_nsec;
}

int gameover(const struct position *pos) {
	return popcount(pos->piece[WHITE] | pos->piece[BLACK]) == 64 || pos->nomove >= 2;
}

static int evaluate(const struct position *pos) {
	return (int)popcount(pos->piece[pos->turn]) - (int)popcount(pos->piece[other_color(pos->turn)]);
}

int best_node(const struct node *node) {
	int j = 0;
	double value;
	for (int i = 0; i < node->nmoves; i++) {
		int n = node->nodes[i].n;
		int w = node->nodes[i].w;
		int N = node->n;
		double value1 = n == 0 ? INFINITY : (1.0 - (double)w / (2 * n)) + c * sqrt(log(N) / n);
		if (i == 0 || value1 > value) {
			j = i;
			value = value1;
		}
	}

	return j;
}

void print_pv(const struct node *node) {
	if (!node || !node->n || node->n < node->nmoves)
		return;
	int bn = best_node(node);
	char str[3];
	if (node->nmoves == 0)
		printf(" stand");
	else
		printf(" %s", algebraic(str, node->moves[bn]));
	print_pv(&node->nodes[bn]);
}

int rollout(const struct position *pos, uint64_t *seed) {
	if (gameover(pos))
		return evaluate(pos);

	move_t moves[64];
	movegen(moves, pos);
	struct position copy = *pos;
	if (moves[0] == MOVE_NULL) {
		copy.nomove++;
		copy.turn = other_color(copy.turn);
	}
	else {
		copy.nomove = 0;
		int nmoves;
		for (nmoves = 0; moves[nmoves] != MOVE_NULL; nmoves++);

		move_t move = uniformint(seed, 0, nmoves);
		do_move(&copy, moves[(int)move]);
	}

	return -rollout(&copy, seed);
}

int mcts_internal(const struct position *pos, uint64_t *seed, struct node *node, int ply) {
	int result;
	if (node->n == 0) {
		move_t moves[64];
		movegen(moves, pos);

		for (node->nmoves = 0; moves[(int)node->nmoves] != MOVE_NULL; node->nmoves++);

		if (node->nmoves == 0) {
			node->moves = NULL;
			node->nodes = calloc(1, sizeof(*node->nodes));
		}
		else {
			node->moves = malloc(node->nmoves * sizeof(*node->moves));
			memcpy(node->moves, moves, node->nmoves * sizeof(*node->moves));
			node->nodes = calloc(node->nmoves, sizeof(*node->nodes));
		}

		result = rollout(pos, seed);
	}
	else if (gameover(pos)) {
		result = evaluate(pos);
	}
	else {
		struct position copy = *pos;

		int j = 0;
		if (node->nmoves > 0) {
			double value;
			for (int i = 0; i < node->nmoves; i++) {
				int n = node->nodes[i].n;
				int w = node->nodes[i].w;
				int N = node->n;
				double value1 = n == 0 ? INFINITY : (1.0 - (double)w / (2 * n)) + c * sqrt(log(N) / n);
				if (i == 0 || value1 > value) {
					j = i;
					value = value1;
				}
			}
			do_move(&copy, node->moves[j]);
			copy.nomove = 0;
		}
		else {
			copy.turn = other_color(copy.turn);
			copy.nomove++;
		}

		result = -mcts_internal(&copy, seed, &node->nodes[j], ply + 1);
	}

	node->n++;

	if (result > 0)
		node->w += 2;
	if (result == 0)
		node->w++;

	return result;
}

struct node *free_node(struct node *node, int except) {
	if (!node || !node->n)
		return NULL;

	if (node->nmoves) {
		for (int i = 0; i < node->nmoves; i++) {
			if (i != except)
				free_node(&node->nodes[i], -1);
		}

		free(node->moves);
	}
	else if (except != 0) {
		free_node(node->nodes, -1);
	}
	struct node *ret = NULL;
	if (except >= 0) {
		ret = malloc(sizeof(*ret));
		memcpy(ret, &node->nodes[except], sizeof(*ret));
	}
	free(node->nodes);

	return ret;
}

move_t mcts(struct position *pos, int maxtime, struct node **root, uint64_t *seed) {
	int64_t starttime = time_now();
	int64_t endtime = starttime + maxtime * TPPERMS;
	int64_t lastinfotime = starttime;

	move_t moves[64];
	movegen(moves, pos);
	if (moves[0] == MOVE_NULL) {
		printf("bestmove stand\n");
		if ((*root)->n > 0) {
			struct node *ret = &(*root)->nodes[0];
			free(*root);
			*root = ret;
		}
		return MOVE_NULL;
	}

	int64_t now;
	while ((now = time_now()) < endtime) {
		mcts_internal(pos, seed, *root, 0);

		if (now - lastinfotime > 500 * TPPERMS) {
			lastinfotime = now;
			int bn = best_node(*root);
			printf("info score %lf nodes %d pv", 1.0 - (double)(*root)->nodes[bn].w / (2 * (*root)->nodes[bn].n), (*root)->n);
			print_pv(*root);
			printf("\n");
		}
	}

	int bn = best_node(*root);

	printf("info score %lf nodes %d pv", 1.0 - (double)(*root)->nodes[bn].w / (2 * (*root)->nodes[bn].n), (*root)->n);
	print_pv(*root);
	printf("\n");

	char str[3] = { 0 };
	printf("bestmove %s\n", algebraic(str, (*root)->moves[bn]));

	move_t bestmove = (*root)->moves[bn];

	struct node *ret = free_node(*root, bn);
	free(*root);
	*root = ret;

	return bestmove;
}
