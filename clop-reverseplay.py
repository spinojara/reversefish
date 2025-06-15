#!/usr/bin/env python3

from subprocess import Popen, PIPE
import sys

argv = sys.argv[1:]

if len(argv) < 3 or len(argv) % 2 == 0:
    sys.stderr.write('Too few arguments\n')
    sys.exit(2)

clop_seed = int(argv[2])

engine1 = './reversefish'
engine2 = './reversefish'

for i in range(3, len(argv) - 1, 2):
    engine2 += ' %s=%s' % (argv[i], argv[i + 1])

if clop_seed % 2 != 0:
    engine1, engine2 = engine2, engine1

command = './reverseplay %s %s %s -- %s' % (argv[0], argv[2], engine1, engine2)
process = Popen(command, shell=True, stdout=PIPE)
output = process.communicate()[0]

for line in output.decode("utf-8").splitlines():
    if clop_seed % 2 == 0:
        if line == 'result win black':
            sys.stdout.write('L\n')
        if line == 'result draw':
            sys.stdout.write('D\n')
        if line == 'result win white':
            sys.stdout.write('W\n')
    else:
        if line == 'result win black':
            sys.stdout.write('W\n')
        if line == 'result draw':
            sys.stdout.write('D\n')
        if line == 'result win white':
            sys.stdout.write('L\n')
