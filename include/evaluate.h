#ifndef EVALUATE_H
#define EVALUATE_H

#define VALUE_INFINITE 0x7FFE
#define VALUE_WIN 0x2000
#define VALUE_MAX (2 * VALUE_WIN)

#include "position.h"

int evaluate(const struct position *pos);

#endif
