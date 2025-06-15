#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>

#include "bitboard.h"
#include "magicbitboard.h"
#include "position.h"
#include "move.h"
#include "search.h"

int main(void) {
	bitboard_init();
	magicbitboard_init();
	search_init();

	struct position pos;

#if 0
	startpos(&pos);

	move_t moves[64];

	movegen(moves, &pos);

	print_position(&pos);

	char str[3];
	for (int i = 0; moves[i] != MOVE_NULL; i++)
		printf("move: %s\n", algebraic(str, moves[i]));

	search(&pos, 64, NULL, 1000);

#else
	char buf[BUFSIZ];

	int player = BLACK;

	while (1) {
		printf("Do you want to play as white or black? ");
		if (!fgets(buf, sizeof(buf), stdin))
			exit(1);
		if (!strcasecmp("white\n", buf)) {
			player = WHITE;
			break;
		}
		if (!strcasecmp("black\n", buf)) {
			player = BLACK;
			break;
		}
		printf("Your input was not recognized.\n");
		printf("Your options are: white black\n");
	}

	int maxtime = -1;
	while (1) {
		printf("Thinking time (ms): ");
		if (!fgets(buf, sizeof(buf), stdin))
			exit(1);
		errno = 0;
		char *endptr;
		maxtime = strtol(buf, &endptr, 10);
		if (errno || *endptr != '\n' || maxtime < 1) {
			printf("Your input was not recognized.\n");
		}
		else {
			break;
		}
	}

	startpos(&pos);

	print_position(&pos);

	int first = 1;

	while (1) {
		if (!(first && player == pos.turn)) {
			move_t bestmove;
			search(&pos, 64, &bestmove, maxtime);
			if (bestmove != MOVE_NULL)
				do_move(&pos, bestmove);
			else
				pos.turn = other_color(pos.turn);
			print_position(&pos);
		}

		first = 0;
		move_t move;
		while (1) {
			printf("Make a move: ");
			if (!fgets(buf, sizeof(buf), stdin))
				exit(1);
			buf[strcspn(buf, "\n")] = '\0';

			if (string_to_move(&move, &pos, buf)) {
				printf("Your input was not recognized\n");
				printf("Your options are:");
				move_t moves[64];
				movegen(moves, &pos);
				if (moves[0] == MOVE_NULL) {
					printf(" stand");
				}
				for (int i = 0; moves[i] != MOVE_NULL; i++) {
					char tmp[3];
					printf(" %s", algebraic(tmp, moves[i]));
				}
				printf("\n");
			}
			else {
				break;
			}
		}

		do_move(&pos, move);
		print_position(&pos);
	}
#endif
	return 0;
}
