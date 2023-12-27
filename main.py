from math import *
from random import *

rep = 1000000
s = 0
for i in range(rep):
    angle = randint(0, 10000) / 1001 * 2 * pi
    dx = round(cos(angle) * 270)

    s += abs(dx)

print(s / rep / 2)