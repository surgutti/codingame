#ifndef CONST_H
#define CONST_H

#include <cmath>

int gameTurn;
const int WIDTH = 10000;
const int HEIGHT = 10000;

const int DRONES_PER_PLAYER = 2;

const int UGLY_UPPER_Y_LIMIT = 2500;
const int DRONE_UPPER_Y_LIMIT = 0;
const int DRONE_START_Y = 500;

const int COLORS_PER_FISH = 4;
const int DRONE_MAX_BATTERY = 30;
const int LIGHT_BATTERY_COST = 5;
const int DRONE_BATTERY_REGEN = 1;
const int DRONE_MAX_SCANS = 1000; // Integer.MAX_VALUE;

const int DARK_SCAN_RANGE = 800;
const int LIGHT_SCAN_RANGE = 2000;
const int UGLY_EAT_RANGE = 300;
const int DRONE_HIT_RANGE = 200;
const int FISH_HEARING_RANGE = (DARK_SCAN_RANGE + LIGHT_SCAN_RANGE) / 2;

const int DRONE_MOVE_SPEED = 600;
const int DRONE_SINK_SPEED = 300;
const int DRONE_EMERGENCY_SPEED = 300;
double DRONE_MOVE_SPEED_LOSS_PER_SCAN = 0;

const int FISH_SWIM_SPEED = 200;
const int FISH_AVOID_RANGE = 600;
const int FISH_FLEE_SPEED = 400;
const int UGLY_ATTACK_SPEED = (int) (DRONE_MOVE_SPEED * 0.9);
const int UGLY_SEARCH_SPEED = (int) (UGLY_ATTACK_SPEED / 2);

const int FISH_X_SPAWN_LIMIT = 1000;
const int FISH_SPAWN_MIN_SEP = 1000;

const Vector CENTER((WIDTH - 1) / 2.0, (HEIGHT - 1) / 2.0);

const int MAX_TURNS = 201;

// BOT CONST

const int CREATURE_COUNT = 30;

const double PI = acos(-1);
const double TAU = 2 * PI;

#endif // CONST_H