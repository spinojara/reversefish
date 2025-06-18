#!/usr/bin/env python3

import math

w = 20 + 19 + 20
d = 1 + 0 + 3
l = 13 + 12 + 9

n = w + d + l

w = w / n
d = d / n
l = l / n

s = 0.5 * d + w

def S(x):
    return 1 / (1 + 10 ** (-x / 400))

def Sinv(s):
    return -400 / math.log(10) * math.log(1 / s - 1)

def Sprime(x):
    return -math.log(10) / 400 * 10 ** (-x / 400) / S(x) ** 2

DE = Sinv(s)

lam = 1.96

print(f'{DE}')
