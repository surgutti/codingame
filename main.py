from math import *
from random import *

def score(A, B):
    result = 0
    for i in range(4):
        for j in range(3):

            if A[i][j] == -1:
                continue
            
            if A[i][j] < B[i][j] or B[i][j] == -1:
                result += (j + 1) * 2
            else:
                result += (j + 1) * 1
    
    for i in range(4):
        mnA, mxA, mnB, mxB = 0, 0, 0, 0

        for j in range(3):
            mnA = min(mnA, A[i][j])
            mxA = max(mxA, A[i][j])
            mnB = min(mnB, B[i][j])
            mxB = max(mxB, B[i][j])
        
        if mnA == -1:
            continue
        
        if mnB == -1 or mxA < mxB:
            result += 3 * 2
        else:
            result += 3 * 1
    
    for j in range(3):
        mnA, mxA, mnB, mxB = 0, 0, 0, 0

        for i in range(4):
            mnA = min(mnA, A[i][j])
            mxA = max(mxA, A[i][j])
            mnB = min(mnB, B[i][j])
            mxB = max(mxB, B[i][j])
        
        if mnA == -1:
            continue
        
        if mnB == -1 or mxA < mxB:
            result += 4 * 2
        else:
            result += 4 * 1

    return result

A = [[1, 1, 1],
     [1, 1, 1],
     [-1, 1, 1],
     [1, 1, -1]]

B = [[2, 2, 2],
     [2, 2, 2],
     [2, 2, 2],
     [2, 2, 2]]

print("A = ", score(A, B))
print("B = ", score(B, A))

def f(x, r):
    return (x / 4) * x * 12 * (ceil(2 * r / x) + 1)

for x in range(50, 1001, 10):
    if f(x, 300) < 5*10**6:
        print(x, f(x, 300))
