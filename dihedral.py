from math import *
from random import *

def apply(p, v):
	return [v[i] for i in p]

def inv(v):
	w = [0 for i in range(len(v))]
	for i in range(len(v)):
		w[v[i]] = i
	return w

def rot(v):
	return apply([1, 2, 3, 0], v)

def sym(v):
	return apply([3, 2, 1, 0], v)

v = [0, 1, 2, 3]

#print(v)
#print(rot(v))
#print(sym(v))

z = []

z.append(v)
z.append(rot(z[-1]))
z.append(rot(z[-1]))
z.append(rot(z[-1]))
z.append(sym(z[-1]))
z.append(rot(z[-1]))
z.append(rot(z[-1]))
z.append(rot(z[-1]))

for i in range(len(z)):
	for j in range(i + 1, len(z)):
		assert(z[i] != z[j])

for i in range(len(z)):
	print("{", end='')
	for j in range(len(z)):
		pp = apply(z[j], inv(z[i]))
		#pp = apply(z[j], z[i])
		k = -1
		for kk in range(len(z)):
			if pp == z[kk]:
				k = kk
				break

		print(k, end=",")
	print("},")
