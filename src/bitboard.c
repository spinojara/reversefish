#include "bitboard.h"

#include <stdio.h>

uint64_t between_lookup[64 * 64];

uint64_t between_calc(int source, int target) {
	if (source == target)
		return 0;
	int sourcef = file_of(source);
	int sourcer = rank_of(source);
	int targetf = file_of(target);
	int targetr = rank_of(target);
	uint64_t ret = 0;
	if (sourcef == targetf) {
		for (int r = sourcer; r <= targetr; r++)
			ret |= bitboard(make_square(sourcef, r));
		for (int r = sourcer; r >= targetr; r--)
			ret |= bitboard(make_square(sourcef, r));
	}
	else if (sourcer == targetr) {
		for (int f = sourcef; f <= targetf; f++)
			ret |= bitboard(make_square(f, sourcer));
		for (int f = sourcef; f >= targetf; f--)
			ret |= bitboard(make_square(f, sourcer));
	}
	else if (sourcef - targetf == sourcer - targetr) {
		for (int i = 0; sourcef + i <= targetf && sourcef + i < 8 && sourcer + i < 8; i++)
			ret |= bitboard(make_square(sourcef + i, sourcer + i));
		for (int i = 0; sourcef + i >= targetf && sourcef + i >= 0 && sourcer + i >= 0; i--)
			ret |= bitboard(make_square(sourcef + i, sourcer + i));
	}
	else if (sourcef - targetf == targetr - sourcer) {
		for (int i = 0; sourcef + i <= targetf && sourcef + i < 8 && sourcer - i < 8; i++)
			ret |= bitboard(make_square(sourcef + i, sourcer - i));
		for (int i = 0; sourcef + i >= targetf && sourcef + i >= 0 && sourcer - i >= 0; i--)
			ret |= bitboard(make_square(sourcef + i, sourcer - i));
	}
	return ret;
}

void print_bitboard(uint64_t b) {
	printf("\n       a   b   c   d   e   f   g   h\n");
	for (int i = 0; i < 8; i++) {
		printf("     +---+---+---+---+---+---+---+---+\n   %i |", 8 - i);
		for (int j = 0; j < 8; j++) {
			if (get_bit(b, 8 * (7 - i) + j)) {
				printf(" 1 |");
			}
			else {
				printf(" 0 |");
			}
		}
		printf(" %i\n", 8 - i);
	}
	printf("     +---+---+---+---+---+---+---+---+\n");
	printf("       a   b   c   d   e   f   g   h\n\n");
}

void bitboard_init(void) {
	for (int i = 0; i < 64; i++)
		for (int j = 0; j < 64; j++)
			between_lookup[i + 64 * j] = between_calc(i, j);
}
