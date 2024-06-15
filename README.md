# Summer Challenge 2024

MCTS with DUCT

Rollouts are played to the end of each game

Each medal counts are capped to 0.7 in order to make the score nonzero
Score for player is: (my_score - enemy1_score - enemy2_score) / (my_score + enemy1_score + enemy2_score)

Maybe good idea would be to make some bigger coeficient for enemies that have bigger priority to beat
(eg. if I'm 3rd and 1st place is unbeatable then I want to beat 2nd place instead of focusing on 1st)

Those coeficients needs to be chosen prior to MCTS

# TODO
- play with constant $C$
- further constant factor
- gather statistics on roller_skating and cut depth of playing to some constant depth