#include "util.h"

#include <stdint.h>

uint64_t gseed = SEED;
uint64_t gxorshift64(void) {
	return xorshift64(&gseed);
}
uint64_t xorshift64(uint64_t *seed) {
	*seed ^= *seed >> 12;
	*seed ^= *seed << 25;
	*seed ^= *seed >> 27;
	return *seed * 2685821657736338717ull;
}

double uniform(uint64_t *seed) {
	const uint64_t max = (uint64_t)1 << 32;
	return (double)(xorshift64(seed) % max) / max;
}
int uniformint(uint64_t *seed, int a, int b) {
	return (int)((double)a + uniform(seed) * (b - a));
}

int find_char(const char *s, char c) {
	for (int i = 0; s[i]; i++)
		if (s[i] == c)
			return i;
	return -1;
}
