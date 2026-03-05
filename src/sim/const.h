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

constexpr f64 PI = 3.141592653589793;
constexpr f64 EPSILON = 1E-5;
constexpr f64 NULL_COLLISION = 2.0;

constexpr f64 CHECKPOINT_RADIUS = 600;
constexpr f64 MAX_ROTATION = PI / 20;
constexpr f64 POD_RADIUS = 400;

constexpr u32 SHIELD_START = 4;
constexpr i32 TIMEOUT = 100;

constexpr i32 POD_NB = 4;

#endif // CONST_H