import sys
import math
from random import randint

# Score points by scanning valuable fish faster than your opponent.

creature_count = int(input())
creature_types = []
for i in range(creature_count):
    creature_id, color, _type = [int(j) for j in input().split()]

    while creature_id >= len(creature_types):
        creature_types.append(-2)

    creature_types[creature_id] = _type

# game loop

WIDTH, HEIGHT = 10000, 10000

turn = 0

last_save = [10, 10]

B = 3
BLOCK_SIZE = WIDTH / B

vis = [[0 for i in range(B)] for j in range(B)]

rush = True

def normalize(x, y):
    l = math.sqrt(x * x + y * y)
    if l == 0:
        return (0, 0)
    return (x / l, y / l)

def distance(x1, y1, x2, y2):
    return math.sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2))

def dot(x1, y1, x2, y2):
    return x1 * x2 + y1 * y2

def distance_from_line(x1, y1, x2, y2, x3, y3):
    if dot(x2 - x1, y2 - y1, x3 - x1, y3 - y1) <= 0:
        return distance(x1, y1, x3, y3)
    if dot(x1 - x2, y1 - y2, x3 - x2, y3 - y2) <= 0:
        return distance(x2, y2, x3, y3)
    
    return abs((x2 - x1) * (y1 - y3) - (x1 - x3) * (y2 - y1)) / math.sqrt((x2 - x1) ** 2 + (y2 - y1) ** 2)

def collision(dx1, dy1, dx2, dy2, ux3, uy3, ugly_vx, ugly_vy):
    if distance(dx1, dy1, ux3, uy3) <= 502:
        return True

    # ugly_vx = dx1 - ux3
    # ugly_vy = dy1 - uy3

    if ugly_vx == 0 and ugly_vy == 0:
        return distance_from_line(dx1, dy1, dx2, dy2, ux3, uy3) <= 502

    # ugly_vv = math.sqrt(ugly_vx * ugly_vx + ugly_vy * ugly_vy)
    # ugly_vx = ugly_vx / ugly_vv * 540
    # ugly_vy = ugly_vy / ugly_vv * 540

    x = ux3
    y = uy3
    ux = dx1
    uy = dy1

    x2 = x - ux
    y2 = y - uy
    r2 = 500 + 2

    vx2 = ugly_vx - (dx2 - dx1)
    vy2 = ugly_vy - (dy2 - dy1)

    a = vx2 * vx2 + vy2 * vy2

    if a <= 0:
        return False
    
    b = 2.0 * (x2 * vx2 + y2 * vy2)
    c = x2 * x2 + y2 * y2 - r2 * r2
    delta = b * b - 4.0 * a * c

    if delta < 0.0:
        return False
    
    t = (-b - math.sqrt(delta)) / (2.0 * a)

    if t <= 0:
        return False
    
    if t > 1.0:
        return False
    
    return True

while True:
    turn += 1

    my_score = int(input())
    foe_score = int(input())
    
    my_scan_count = int(input())
    for i in range(my_scan_count):
        creature_id = int(input())

    foe_scan_count = int(input())
    for i in range(foe_scan_count):
        creature_id = int(input())

    my_drone_count = int(input())
    my_drone = []

    for i in range(my_drone_count):
        drone_id, drone_x, drone_y, emergency, battery = [int(j) for j in input().split()]

        my_drone.append((drone_x, drone_y))

        xx = int(drone_x // BLOCK_SIZE)
        yy = int(drone_y // BLOCK_SIZE)

        vis[xx][yy] = turn

        print("> ", drone_id, drone_x, drone_y, xx, yy, file=sys.stderr)

        if drone_y <= 500:
            last_save[i] = turn
        
        if drone_y > 10000 - 700:
            rush = False

    foe_drone_count = int(input())
    for i in range(foe_drone_count):
        drone_id, drone_x, drone_y, emergency, battery = [int(j) for j in input().split()]

    drone_scan_count = int(input())
    for i in range(drone_scan_count):
        drone_id, creature_id = [int(j) for j in input().split()]
    
    visible_creature_count = int(input())
    ugly = []

    for i in range(visible_creature_count):
        creature_id, creature_x, creature_y, creature_vx, creature_vy = [int(j) for j in input().split()]
        
        if creature_types[creature_id] == -1:
            ugly.append((creature_x, creature_y, creature_vx, creature_vy))
            print("UGLY: ", creature_x, creature_y, creature_vx, creature_vy, file=sys.stderr)

    radar_blip_count = int(input())
    for i in range(radar_blip_count):
        inputs = input().split()
        drone_id = int(inputs[0])
        creature_id = int(inputs[1])
        radar = inputs[2]

    order = []

    for i in range(my_drone_count):

        x = randint(1200, 10000 - 1200)
        y = randint(1200, 10000 - 1200)
        xd = 100000

        for k in range(turn > 20, B):
            for j in range(B):
                print(vis[j][k], end=' ', file=sys.stderr)
                
                if i == 0 and j > B // 2:
                    continue
                if i == 1 and j < B // 2:
                    continue

                if vis[j][k] < xd:
                    x = j * BLOCK_SIZE + BLOCK_SIZE * 3 / 4 + randint(1, 100)
                    y = k * BLOCK_SIZE + BLOCK_SIZE * 3 / 4 + randint(1, 100)
                    xd = vis[j][k]
                    
            print(file=sys.stderr)

        if last_save[i] + 20 < turn:
            y = 499

        if rush:
            y = 10000 - 600
        
        drone_x, drone_y = my_drone[i]

        msg=""

        possible_moves = []

        for alpha in range(1000):
            angle = alpha * 2 * math.pi / 1000
            xx = int(drone_x + math.cos(angle) * 600)
            yy = int(drone_y + math.sin(angle) * 600)

            ok = True

            if xx < 0 or xx > WIDTH or yy < 0 or yy > HEIGHT:
                ok = False

            for ugly_x, ugly_y, vx, vy in ugly:
                if collision(drone_x, drone_y, xx, yy, ugly_x, ugly_y, vx, vy):
                    ok = False

            if ok:
                possible_moves.append((xx, yy))

        xxx, yyy = -1e9, -1e9

        for xx, yy in possible_moves:
            if distance(xx, yy, x, y) < distance(xxx, yyy, x, y):
                xxx = xx
                yyy = yy
        
        print("xxx: ", xxx, "yyy: ", yyy, file=sys.stderr)

        x = xxx
        y = yyy

        x = int(x)
        y = int(y)

        if x < 0 or x > 9999:
            x = 0
            y = 0
        
        print("x: ", x, "y:", y, file=sys.stderr)

        if turn < 8:
            print("MOVE", x, y, 0, msg)
        else:
            print("MOVE", x, y, end=' ')

            if turn % 4 == 0:
                print(1, msg)
            else:
                print(0, msg)