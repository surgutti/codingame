

Make sure the snakes does not easily suicide
- does not go out of the platform
- does not go into the dead end (closed area without exit)


If fighting for a power make sure you near the head of the enemy
only if its the power he will gain. Sometimes if optimal to make a
trap around the power so that enemy gets traps inside.

Traps:

AAAAAA>
BBBBBB>   .
...........



Lifts:


  <       < A
. A       . A
  A         
  ^         ^
  B         B
  B         B
...       ...  (not correct?)

General strategy:

MCTS for each snake?
Fast rollout

Compute distance for each snake.
For long ones do only the dp over the last solid.

             #shapes * W * H * 4 * #snakes
length:3: 12 259200
length:4: 36 777600
length:5: 100 2160000
length:6: 284 6134400
length:7: 780 16848000
length:8: 2172 46915200
length:9: 5916 127785600
length:10: 16268 351388800

-> probably up to 4/5 is possible
-> simple heuristics with strategies?
go[shape][dir] = new_shape;
dp[x][y][shape] -> 3 directions

-> find only few turn distances?
-> many of them are unattainable.