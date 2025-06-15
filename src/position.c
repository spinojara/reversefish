#include "position.h"

#include <string.h>
#include <stdio.h>

#include "bitboard.h"
#include "util.h"

void startpos(struct position *pos) {
	pos->turn = BLACK;
	pos->nomove = 0;
	pos->piece[WHITE] = bitboard(d5) | bitboard(e4);
	pos->piece[BLACK] = bitboard(d4) | bitboard(e5);
}

void print_position(const struct position *pos) {
	const int flip = 0;
	int i, j, t;
	char letters[] = "abcdefgh";

	printf("\n   ");
	for (i = 0; i < 8; i++)
		printf("   %c", letters[flip ? 7 - i : i]);
	printf("\n");
	for (i = 0; i < 8; i++) {
		printf("    +---+---+---+---+---+---+---+---+\n  %i |", flip ? 1 + i : 8 - i);
		for (j = 0; j < 8; j++) {
			t = flip ? 8 * i + (7 - j) : 8 * (7 - i) + j;
			if (get_bit(pos->piece[WHITE], t))
				printf(" W |");
			else if (get_bit(pos->piece[BLACK], t))
				printf(" B |");
			else
				printf("   |");
		}
		printf(" %i\n", flip ? 1 + i : 8 - i);
	}
	printf("    +---+---+---+---+---+---+---+---+\n   ");
	for (i = 0; i < 8; i++)
		printf("   %c", letters[flip ? 7 - i : i]);
	printf("\n\n");
}

int square(const char *algebraic) {
	if (strlen(algebraic) != 2) {
		return -1;
	}
	if (find_char("abcdefgh", algebraic[0]) == -1 || find_char("12345678", algebraic[1]) == -1)
		return -1;
	return find_char("abcdefgh", algebraic[0]) + 8 * find_char("12345678", algebraic[1]);
}

char *algebraic(char *str, int square) {
	if (square < 0 || 63 < square) {
		str[0] = '-';
		str[1] = '\0';
	}
	else {
		str[0] = "abcdefgh"[file_of(square)];
		str[1] = "12345678"[rank_of(square)];
	}
	str[2] = '\0';
	return str;
}
