#ifndef CONST_H
#define CONST_H

using f32 = float;
using f64 = double;

using i32 = int;
using u32 = unsigned int;

using i64 = long long;
using u64 = unsigned long long;

constexpr i32 WIDTH = 16000;
constexpr i32 HEIGHT = 9000;

constexpr i32 PLAYER_NB = 2;
constexpr i32 PODS_PER_PLAYER = 2;
constexpr i32 POD_NB = PLAYER_NB * PODS_PER_PLAYER;

constexpr i32 MAX_THRUST = 200;
constexpr i32 BOOST_THRUST = 650;
constexpr i32 TIMEOUT = 100;
constexpr i32 SHIELD_START = 4;

constexpr f64 PI = 3.14159265358979323846;
constexpr f64 DEG_TO_RAD = PI / 180.0;
constexpr f64 RAD_TO_DEG = 180.0 / PI;
constexpr f64 EPSILON = 1E-5;
constexpr f64 NO_COLLISION = 10.0;

constexpr f64 MAX_ROTATION = 18.0 * DEG_TO_RAD;
constexpr f64 FRICTION_FACTOR = 0.85;

constexpr f64 CHECKPOINT_RADIUS = 600.0;
constexpr f64 CHECKPOINT_RADIUS_SQ = CHECKPOINT_RADIUS * CHECKPOINT_RADIUS;

constexpr f64 POD_RADIUS = 400.0;
constexpr f64 POD_DIAMETER = POD_RADIUS * 2.0;
constexpr f64 POD_DIAMETER_SQ = POD_DIAMETER * POD_DIAMETER;

constexpr f64 START_ANGLE = -1.0 * DEG_TO_RAD;

#endif // CONST_H
