#!/usr/bin/env python3

from subprocess import Popen, PIPE
import sys

stats = [ 0, 0, 0 ]

for i in range(1000):
    process = None
    if i % 2 == 0:
        process = Popen('./reverseplay 100 %d ./reversefish -- ../reversefish2/reversefish' % (i), shell=True, stdout=PIPE)
    else:
        process = Popen('./reverseplay 100 %d ../reversefish2/reversefish -- ./reversefish' % (i), shell=True, stdout=PIPE)
    output = process.communicate()[0]

    for line in output.decode("utf-8").splitlines():
        if i % 2 == 0:
            if line == 'result win black':
                stats[0] += 1
            if line == 'result draw':
                stats[1] += 1
            if line == 'result win white':
                stats[2] += 1
        else:
            if line == 'result win black':
                stats[2] += 1
            if line == 'result draw':
                stats[1] += 1
            if line == 'result win white':
                stats[0] += 1

    print(f'{stats[0]} - {stats[1]} - {stats[2]}')
