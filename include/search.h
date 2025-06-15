#ifndef SEARCH_H
#define SEARCH_H

#include <stdint.h>

#include "position.h"
#include "move.h"

struct searchinfo {
	uint64_t nodes;

	move_t pv[64][64];

	int root_depth;

	int64_t maxtime;
	int64_t starttime;

	int interrupt;
};

void search(struct position *pos, int depth, move_t *bestmove, int maxtime);

void search_init(void);

#endif
