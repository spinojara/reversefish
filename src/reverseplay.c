#define _POSIX_C_SOURCE 1
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>

#include "position.h"
#include "move.h"
#include "bitboard.h"
#include "magicbitboard.h"
#include "util.h"

int gameover(const struct position *pos) {
	if (popcount(pos->piece[WHITE] | pos->piece[BLACK]) == 64)
		return 1;

	struct position copy = *pos;

	move_t moves[64];
	movegen(moves, &copy);
	if (moves[0] != MOVE_NULL)
		return 0;

	copy.turn = other_color(copy.turn);
	movegen(moves, &copy);
	if (moves[0] != MOVE_NULL)
		return 0;

	return 1;
}

int evaluate(const struct position *pos) {
	return popcount(pos->piece[WHITE]) - popcount(pos->piece[BLACK]);
}

int main(int argc, char **argv) {
	int separator = -1;
	for (int i = 4; i < argc; i++) {
		if (!strcmp(argv[i], "--")) {
			separator = i;
			break;
		}
	}

	if (argc < 6 || separator == -1) {
		fprintf(stderr, "usage: %s tc seed engine1 opt=val ... -- engine2 opt=val ...\n", argv[0]);
		return 1;
	}
	bitboard_init();
	magicbitboard_init();

	char *endptr;
	errno = 0;
	int tc = strtol(argv[1], &endptr, 10);
	if (errno || *endptr != '\0' || tc <= 0) {
		fprintf(stderr, "error: bad tc '%s'\n", argv[1]);
		return 2;
	}

	errno = 0;
	uint64_t seed = strtol(argv[2], &endptr, 10);
	if (errno || *endptr != '\0') {
		fprintf(stderr, "error: bad seed '%s'\n", argv[2]);
		return 2;
	}

	seed = seed / 2;

	int random_moves = uniformint(&seed, 6, 11);
	random_moves = (random_moves / 2) * 2;

	char **engine1 = calloc(argc, sizeof(*engine1));
	char **engine2 = calloc(argc, sizeof(*engine2));

	struct position pos;
	startpos(&pos);

	char movesstr[128] = { 0 };
	for (int i = 0; i < random_moves; i++) {
		move_t moves[64];
		movegen(moves, &pos);

		int nmoves;
		for (nmoves = 0; moves[nmoves] != MOVE_NULL; nmoves++);
		if (nmoves == 0)
			break;

		int n = uniformint(&seed, 0, nmoves);
		do_move(&pos, moves[n]);
		algebraic(movesstr + 2 * i, moves[n]);
	}

	printf("movesstr: %s\n", movesstr);

	int i;
	for (i = 3; i < separator; i++)
		engine1[i - 3] = argv[i];
	engine1[i++ - 3] = "--moves";
	engine1[i++ - 3] = movesstr;

	for (i = separator + 1; i < argc; i++)
		engine2[i - (separator + 1)] = argv[i];
	engine2[i++ - (separator + 1)] = "--moves";
	engine2[i++ - (separator + 1)] = movesstr;

	printf("command: ");
	for (i = 0; engine1[i]; i++) {
		printf(" %s", engine1[i]);
	}
	printf("\n");
	printf("command: ");
	for (i = 0; engine2[i]; i++) {
		printf(" %s", engine2[i]);
	}
	printf("\n");

	if (!engine2 || !engine1[0] || !engine2[0]) {
		fprintf(stderr, "usage: %s tc seed engine1 opt=val ... -- engine2 opt=val ...\n", argv[0]);
		return 1;
	}

	FILE *w[2], *r[2];

	int childparent[2];
	int parentchild[2];

	pipe(childparent);
	pipe(parentchild);

	pid_t pid1, pid2;
	pid1 = fork();
	if (pid1 == 0) {
		close(parentchild[1]);
		close(childparent[0]);
		dup2(parentchild[0], STDIN_FILENO);
		dup2(childparent[1], STDOUT_FILENO);
		execvp(engine2[0], engine2);
		fprintf(stderr, "error: execvp\n");
		exit(1);
	}

	close(parentchild[0]);
	close(childparent[1]);

	w[WHITE] = fdopen(parentchild[1], "w");
	r[WHITE] = fdopen(childparent[0], "r");

	pipe(childparent);
	pipe(parentchild);

	pid2 = fork();
	if (pid2 == 0) {
		close(parentchild[1]);
		close(childparent[0]);
		dup2(parentchild[0], STDIN_FILENO);
		dup2(childparent[1], STDOUT_FILENO);
		execvp(engine1[0], engine1);
		fprintf(stderr, "error: execvp\n");
		exit(1);
	}

	close(parentchild[0]);
	close(childparent[1]);

	w[BLACK] = fdopen(parentchild[1], "w");
	r[BLACK] = fdopen(childparent[0], "r");

	setbuf(w[WHITE], NULL);
	setbuf(w[BLACK], NULL);

	fprintf(w[WHITE], "black\n%d\n", tc);
	fprintf(w[BLACK], "white\n%d\n", tc);

	char buf[BUFSIZ];
	move_t move;
	while (1) {
		if (gameover(&pos))
			break;

		while (1) {
			fgets(buf, sizeof(buf), r[BLACK]);
			printf("engine1: %s", buf);
			if (strstr(buf, "bestmove "))
				break;
		}

		buf[strcspn(buf, "\n")] = '\0';
		printf("got move: %s\n", strstr(buf, "bestmove ") + 9);

		if (string_to_move(&move, &pos, strstr(buf, "bestmove ") + 9)) {
			fprintf(stderr, "error: what?\n");
			exit(1);
		}

		if (move != MOVE_NULL)
			do_move(&pos, move);
		else
			pos.turn = other_color(pos.turn);

		fprintf(w[WHITE], "%s\n", strstr(buf, "bestmove ") + 9);
		printf("wrote: .%s.\n", strstr(buf, "bestmove ") + 9);

		if (gameover(&pos))
			break;

		while (1) {
			fgets(buf, sizeof(buf), r[WHITE]);
			printf("engine2: %s", buf);
			if (strstr(buf, "bestmove "))
				break;
		}

		buf[strcspn(buf, "\n")] = '\0';
		printf("got move: %s\n", strstr(buf, "bestmove ") + 9);

		if (string_to_move(&move, &pos, strstr(buf, "bestmove ") + 9)) {
			fprintf(stderr, "error: what?\n");
			exit(1);
		}

		if (move != MOVE_NULL)
			do_move(&pos, move);
		else
			pos.turn = other_color(pos.turn);

		fprintf(w[BLACK], "%s\n", strstr(buf, "bestmove ") + 9);
		printf("wrote: .%s.\n", strstr(buf, "bestmove ") + 9);
	}

	kill(pid1, SIGKILL);
	kill(pid2, SIGKILL);

	printf("result ");
	int result = evaluate(&pos);
	if (result > 0)
		printf("win white\n");
	else if (result == 0)
		printf("draw\n");
	else
		printf("win black\n");
}
