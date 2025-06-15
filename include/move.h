#ifndef MOVE_H
#define MOVE_H

#define MOVE_NULL 64

#include "position.h"

typedef char move_t;

move_t *movegen(move_t *moves, const struct position *pos);

void do_move(struct position *pos, move_t move);

int flip_count(const struct position *pos, move_t move);

int string_to_move(move_t *move, const struct position *pos, const char *str);

#endif
