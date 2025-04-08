def print_mask(mask):
	for i in range(3):
		for j in range(3):
			if mask >> (i * 3 + j) & 1:
				print(1, end='')
			else:
				print(0, end='')
		print()
	# print()

S = set()
for i in range(3):
	for j in range(3):
		for mask in range(1 << 9):
			cnt = 0
			ok = True
			for bit in range(9):
				if (mask >> bit) & 1:
					x = bit // 3
					y = bit % 3
				
					if abs(i - x) + abs(j - y) != 1:
						ok = False
						break

					cnt += 1

			if cnt >= 2 and ok:
				#print(i, j, mask)
				#print_mask(mask)
				S.add(mask)

for x in S:
	xx = 0
	for i in range(9):
		if x >> i & 1:
			xx |= 0b111 << (3 * i)

	print("/*")
	print_mask(x)
	print("*/");
	print(xx, ",");	
print()

print(len(S))
print(S)
