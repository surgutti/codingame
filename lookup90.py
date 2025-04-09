

for i in range(1 << 9):
	res = 0
	for j in range(3):
		val = (i >> (3 * j)) & 0b111
		res |= val << (9 * j)
	
	print(hex(res), end=',')
