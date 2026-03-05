This project is all about creating the best possible bot for the "Mad Pod Racing" competition.


Here are the approaches taken by other contestants:

#### Genetic Algorithms

#### Beam Search

#### Neural Networks

#### DQN

#### DQN+Nash Equilibrium

## Plan

The key part is implementing the exact simulation. It must reproduce the same results as on the competition size.

That is the first part: Implement the exact simulation engine. It does not neet to be the fastest but it needs to be exact.

Next thing that comes to my mind is creating a functioning Neural Network. The data would be scraped out of the leaderboard. There is plenty of replays. This part is totally independed of engine simulation.

Third part would be creating the DQN framework. It would utilize the exact simulation engine and create the working arena for two bots. That would generate the appropriate replay buffers and create the learning loop. All *needs* to be highly parallelizable and run on GPU as the results tell the hardware lottery is real.

Forth point. Include the search. Given that some meaning full bot was created in the previous step we could include the computed neural network into search. That would require incorporate the aforementioned nash equilibrium