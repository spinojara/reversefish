#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <time.h>

#include "bitboard.h"
#include "magicbitboard.h"
#include "position.h"
#include "move.h"
#include "search.h"

int main(void) {
	bitboard_init();
	magicbitboard_init();

	struct position pos;

#if 0
	uint64_t seed = 127849123;

	startpos(&pos);
	pos.piece[WHITE] = 0b1111111111111111111111111111111111111111111111111111111111110100;
	pos.piece[BLACK] = 0b0000000000000000000000000000000000000000000000000000000000001000;
	pos.turn = BLACK;

	int wins = 0, draws = 0, losses = 0;
	for (int i = 0; i < 10000; i++) {
		int result = rollout(&pos, &seed);
		if (result > 0)
			wins++;
		if (result == 0)
			draws++;
		if (result < 0)
			losses++;
	}
	printf("wins - draws - losses: %d - %d - %d\n", wins, draws, losses);

	print_position(&pos);

	struct node *root = calloc(1, sizeof(*root));
	mcts(&pos, 5000, &root, &seed);

#else
	uint64_t seed = time(NULL);

	struct node *root = calloc(1, sizeof(*root));

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
		if (gameover(&pos)) {
			printf("Game over, ");
			int result = popcount(pos.piece[WHITE]) - popcount(pos.piece[BLACK]);
			if (result > 0)
				printf("white wins!");
			else if (result == 0)
				printf("draw!");
			else
				printf("black wins!");
			printf(" (%lu - %lu)\n", popcount(pos.piece[WHITE]), popcount(pos.piece[BLACK]));
			break;
		}
		if (!(first && player == pos.turn)) {
			move_t bestmove = mcts(&pos, maxtime, &root, &seed);
			if (bestmove != MOVE_NULL) {
				pos.nomove = 0;
				do_move(&pos, bestmove);
			}
			else {
				pos.nomove++;
				pos.turn = other_color(pos.turn);
			}
			print_position(&pos);
		}

		if (gameover(&pos)) {
			printf("Game over, ");
			int result = popcount(pos.piece[WHITE]) - popcount(pos.piece[BLACK]);
			if (result > 0)
				printf("white wins!");
			else if (result == 0)
				printf("draw!");
			else
				printf("black wins!");
			printf(" (%lu - %lu)\n", popcount(pos.piece[WHITE]), popcount(pos.piece[BLACK]));
			break;
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

		if (move == MOVE_NULL) {
			pos.nomove++;
			pos.turn = other_color(pos.turn);
			if (root->n) {
				struct node *old = root;
				root = &root->nodes[0];
				free(old);
			}
		}
		else {
			do_move(&pos, move);
			pos.nomove = 0;
			if (root->n) {
				int i;
				for (i = 0; root->moves[i] != move && i < root->nmoves; i++);
				if (i == root->nmoves) {
					fprintf(stderr, "error: what?!\n");
					exit(5);
				}

				struct node *old = root;
				root = free_node(root, i);
				free(old);
			}
		}

		print_position(&pos);
	}
	free_node(root, -1);
	free(root);
#endif

	return 0;
}
