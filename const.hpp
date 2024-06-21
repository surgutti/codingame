#ifndef CONST_HPP
#define CONST_HPP

const int INF = 1'000'000;

const int TRACK_LENGTH = 30;
const int ARCHERY_LENGTH = 12 + 3; // 12 + random.nextInt(4);
const int DIVING_LENGTH = 12 + 3; // 12 + random.nextInt(4);

const int MCTSNODE_POOL = 6'000'000;
const int BRAIN_POOL = 10'000'000;

// TODO: run psyleague with different C values
const float C = 0.45f; // 0.4f;

int PLAYER_IDX;

const float UP_P = 0.25;
const float LEFT_P = 0.25;
const float DOWN_P = 0.25;
const float RIGHT_P = 0.25;

const float MOVE_P[4] = {
    UP_P, LEFT_P, DOWN_P, RIGHT_P
};

#endif // CONST_HPP
