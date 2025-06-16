#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <strings.h>
#include <errno.h>
#include <time.h>
#include <getopt.h>

#include "bitboard.h"
#include "magicbitboard.h"
#include "position.h"
#include "move.h"
#include "search.h"

int main(int argc, char **argv) {
	bitboard_init();
	magicbitboard_init();

	setbuf(stdout, NULL);

	struct position pos;
	startpos(&pos);

	static struct option opts[] = {
		{ "moves",  required_argument, NULL, 'm', },
	};

	char *endptr;
	int c, option_index = 0;
	int error = 0;
	while ((c = getopt_long(argc, argv, "qm:", opts, &option_index)) != -1) {
		switch (c) {
		case 'm':;
			char str[3] = { 0 };
			for (size_t i = 0; 2 * i + 1 < strlen(optarg); i++) {
				str[0] = optarg[2 * i];
				str[1] = optarg[2 * i + 1];

				printf("trying to make move\n");

				move_t move;
				if (string_to_move(&move, &pos, str)) {
					error = 1;
					fprintf(stderr, "error: bad move '%s'\n", str);
					break;
				}
				else {
					if (move == MOVE_NULL)
						pos.turn = other_color(pos.turn);
					else
						do_move(&pos, move);
				}
			}
			break;
		default:
			error = 1;
			break;
		}
	}

	if (error)
		return 1;

	for (int i = optind; i < argc; i++) {
		if (!strchr(argv[i], '=')) {
			error = 1;
			fprintf(stderr, "error: invalid option '%s'\n", argv[i]);
			break;
		}
		char *option = argv[i];
		char *value = strchr(argv[i], '=');
		*value = '\0';
		value++;

		if (!strcmp(option, "c")) {
			errno = 0;
			double a = strtod(value, &endptr);
			if (errno || *endptr != '\0') {
				error = 1;
				break;
			}
			set_c(a);
		}
		else if (!strcmp(option, "cornervalue")) {
			errno = 0;
			double a = strtod(value, &endptr);
			if (errno || *endptr != '\0') {
				error = 1;
				break;
			}
			set_cornervalue(a);
		}
		else if (!strcmp(option, "sidevalue")) {
			errno = 0;
			double a = strtod(value, &endptr);
			if (errno || *endptr != '\0') {
				error = 1;
				break;
			}
			set_sidevalue(a);
		}
		else if (!strcmp(option, "basevalue")) {
			errno = 0;
			double a = strtod(value, &endptr);
			if (errno || *endptr != '\0') {
				error = 1;
				break;
			}
			set_basevalue(a);
		}
		else if (!strcmp(option, "flipvalue")) {
			errno = 0;
			double a = strtod(value, &endptr);
			if (errno || *endptr != '\0') {
				error = 1;
				break;
			}
			set_flipvalue(a);
		}
		else {
			error = 1;
			fprintf(stderr, "error: unknown option '%s'\n", option);
			break;
		}
	}

	if (error)
		return 1;

	uint64_t seed = time(NULL);

	struct node *root = calloc(1, sizeof(*root));
	if (!root) {
		fprintf(stderr, "error: calloc\n");
		exit(1);
	}

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
		maxtime = strtol(buf, &endptr, 10);
		if (errno || *endptr != '\n' || maxtime < 1)
			printf("Your input was not recognized.\n");
		else
			break;
	}

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

	return 0;
}
