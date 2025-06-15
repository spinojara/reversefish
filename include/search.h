#ifndef SEARCH_H
#define SEARCH_H

#include "move.h"
#include "position.h"
#include "bitboard.h"

struct node {
	int w;
	int n;

	char leaf;
	char nmoves;
	move_t *moves;
	struct node *nodes;
};

void set_c(double a);

void set_cornervalue(int a);

void set_sidevalue(int a);

move_t mcts(struct position *pos, int maxtime, struct node **node, uint64_t *seed);

struct node *free_node(struct node *node, int except);

int rollout(const struct position *pos, uint64_t *seed);

int gameover(const struct position *pos);

#endif
