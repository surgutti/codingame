#include "java_math.h"

#include <array>
#include <bit>
#include <cmath>
#include <limits>

namespace java_math {
namespace {

constexpr i32 EXP_BITS = 0x7ff00000;
constexpr i32 EXP_SIGNIF_BITS = 0x7fffffff;
constexpr f64 HUGE_VALUE = 1.0e300;

constexpr std::array<f64, 4> ATAN_HI = {
  0x1.dac670561bb4fp-2,
  0x1.921fb54442d18p-1,
  0x1.f730bd281f69bp-1,
  0x1.921fb54442d18p0,
};

constexpr std::array<f64, 4> ATAN_LO = {
  0x1.a2b7f222f65e2p-56,
  0x1.1a62633145c07p-55,
  0x1.007887af0cbbdp-56,
  0x1.1a62633145c07p-54,
};

constexpr std::array<f64, 11> ATAN_T = {
  0x1.555555555550dp-2,
  -0x1.999999998ebc4p-3,
  0x1.24924920083ffp-3,
  -0x1.c71c6fe231671p-4,
  0x1.745cdc54c206ep-4,
  -0x1.3b0f2af749a6dp-4,
  0x1.10d66a0d03d51p-4,
  -0x1.dde2d52defd9ap-5,
  0x1.97b4b24760debp-5,
  -0x1.2b4442c6a6c2fp-5,
  0x1.0ad3ae322da11p-6,
};

constexpr f64 TINY = 1.0e-300;
constexpr f64 PI_OVER_4 = 0x1.921fb54442d18p-1;
constexpr f64 PI_OVER_2 = 0x1.921fb54442d18p0;
constexpr f64 PI_LOW = 0x1.1a62633145c07p-53;

inline i32 hi(f64 x) {
  return static_cast<i32>(std::bit_cast<u64>(x) >> 32);
}

inline i32 lo(f64 x) {
  return static_cast<i32>(std::bit_cast<u64>(x) & 0xffffffffu);
}

} // namespace

f64 atan(f64 x) {
  i32 hx = hi(x);
  i32 ix = hx & EXP_SIGNIF_BITS;
  i32 id = 0;

  if (ix >= 0x44100000) {
    if (ix > EXP_BITS || (ix == EXP_BITS && lo(x) != 0)) {
      return x + x;
    }
    return (hx > 0) ? (ATAN_HI[3] + ATAN_LO[3]) : (-ATAN_HI[3] - ATAN_LO[3]);
  }

  if (ix < 0x3fdc0000) {
    if (ix < 0x3e200000 && HUGE_VALUE + x > 1.0) {
      return x;
    }
    id = -1;
  } else {
    x = std::abs(x);
    if (ix < 0x3ff30000) {
      if (ix < 0x3fe60000) {
        id = 0;
        x = (2.0 * x - 1.0) / (2.0 + x);
      } else {
        id = 1;
        x = (x - 1.0) / (x + 1.0);
      }
    } else if (ix < 0x40038000) {
      id = 2;
      x = (x - 1.5) / (1.0 + 1.5 * x);
    } else {
      id = 3;
      x = -1.0 / x;
    }
  }

  f64 z = x * x;
  f64 w = z * z;
  f64 s1 = z * (ATAN_T[0] + w * (ATAN_T[2] + w * (ATAN_T[4] + w * (ATAN_T[6] + w * (ATAN_T[8] + w * ATAN_T[10])))));
  f64 s2 = w * (ATAN_T[1] + w * (ATAN_T[3] + w * (ATAN_T[5] + w * (ATAN_T[7] + w * ATAN_T[9]))));

  if (id < 0) {
    return x - x * (s1 + s2);
  }

  z = ATAN_HI[id] - ((x * (s1 + s2) - ATAN_LO[id]) - x);
  return (hx < 0) ? -z : z;
}

f64 atan2(f64 y, f64 x) {
  i32 hx = hi(x);
  i32 hy = hi(y);
  i32 ix = hx & EXP_SIGNIF_BITS;
  i32 iy = hy & EXP_SIGNIF_BITS;
  i32 lx = lo(x);
  i32 ly = lo(y);

  if (std::isnan(x) || std::isnan(y)) {
    return x + y;
  }
  if (((hx - 0x3ff00000) | lx) == 0) {
    return atan(y);
  }

  i32 m = ((hy >> 31) & 1) | ((hx >> 30) & 2);

  if ((iy | ly) == 0) {
    switch (m) {
      case 0:
      case 1:
        return y;
      case 2:
        return PI + TINY;
      default:
        return -PI - TINY;
    }
  }

  if ((ix | lx) == 0) {
    return (hy < 0) ? (-PI_OVER_2 - TINY) : (PI_OVER_2 + TINY);
  }

  if (ix == EXP_BITS) {
    if (iy == EXP_BITS) {
      switch (m) {
        case 0:
          return PI_OVER_4 + TINY;
        case 1:
          return -PI_OVER_4 - TINY;
        case 2:
          return 3.0 * PI_OVER_4 + TINY;
        default:
          return -3.0 * PI_OVER_4 - TINY;
      }
    }

    switch (m) {
      case 0:
        return 0.0;
      case 1:
        return -0.0;
      case 2:
        return PI + TINY;
      default:
        return -PI - TINY;
    }
  }

  if (iy == EXP_BITS) {
    return (hy < 0) ? (-PI_OVER_2 - TINY) : (PI_OVER_2 + TINY);
  }

  f64 z = 0.0;
  i32 k = (iy - ix) >> 20;
  if (k > 60) {
    z = PI_OVER_2 + 0.5 * PI_LOW;
  } else if (hx < 0 && k < -60) {
    z = 0.0;
  } else {
    z = atan(std::abs(y / x));
  }

  switch (m) {
    case 0:
      return z;
    case 1:
      return -z;
    case 2:
      return PI - (z - PI_LOW);
    default:
      return (z - PI_LOW) - PI;
  }
}

} // namespace java_math
