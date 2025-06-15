#ifndef UTIL_H
#define UTIL_H

#include <stdint.h>

#define SIZE(x) (sizeof(x) / sizeof (*(x)))

#define UNUSED(x) (void)(x)

#define XSTR(x) STR(x)
#define STR(x) #x

#define SEED 1274012836ull

uint64_t gxorshift64(void);
uint64_t xorshift64(uint64_t *seed);

static inline int max(int a, int b) { return a < b ? b : a; }
static inline int min(int a, int b) { return a < b ? a : b; }
static inline int clamp(int a, int b, int c) { return max(b, min(a, c)); }

int find_char(const char *s, char c);

#endif
