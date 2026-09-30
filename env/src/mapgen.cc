#include "mapgen.h"

#include <algorithm>
#include <array>
#include <cstdint>

namespace {

class JavaRandom {
public:
  explicit JavaRandom(i64 seed)
    : seed_((static_cast<u64>(seed) ^ kMultiplier) & kMask) {}

  i32 nextInt(i32 bound) {
    if (bound <= 0) {
      return 0;
    }

    if ((bound & -bound) == bound) {
      return static_cast<i32>((bound * static_cast<i64>(next(31))) >> 31);
    }

    i32 bits = 0;
    i32 value = 0;
    do {
      bits = next(31);
      value = bits % bound;
    } while (bits - value + (bound - 1) < 0);
    return value;
  }

private:
  i32 next(i32 bits) {
    seed_ = (seed_ * kMultiplier + kAddend) & kMask;
    return static_cast<i32>(seed_ >> (48 - bits));
  }

  static constexpr u64 kMultiplier = 0x5DEECE66DULL;
  static constexpr u64 kAddend = 0xBULL;
  static constexpr u64 kMask = (1ULL << 48) - 1;

  u64 seed_;
};

constexpr std::array<std::array<std::pair<i32, i32>, 6>, 13> kBaseMaps = {{
  {{{12460, 1350}, {10540, 5980}, {3580, 5180}, {13580, 7600}, {0, 0}, {0, 0}}},
  {{{3600, 5280}, {13840, 5080}, {10680, 2280}, {8700, 7460}, {7200, 2160}, {0, 0}}},
  {{{4560, 2180}, {7350, 4940}, {3320, 7230}, {14580, 7700}, {10560, 5060}, {13100, 2320}}},
  {{{5010, 5260}, {11480, 6080}, {9100, 1840}, {0, 0}, {0, 0}, {0, 0}}},
  {{{14660, 1410}, {3450, 7220}, {9420, 7240}, {5970, 4240}, {0, 0}, {0, 0}}},
  {{{3640, 4420}, {8000, 7900}, {13300, 5540}, {9560, 1400}, {0, 0}, {0, 0}}},
  {{{4100, 7420}, {13500, 2340}, {12940, 7220}, {5640, 2580}, {0, 0}, {0, 0}}},
  {{{14520, 7780}, {6320, 4290}, {7800, 860}, {7660, 5970}, {3140, 7540}, {9520, 4380}}},
  {{{10040, 5970}, {13920, 1940}, {8020, 3260}, {2670, 7020}, {0, 0}, {0, 0}}},
  {{{7500, 6940}, {6000, 5360}, {11300, 2820}, {0, 0}, {0, 0}, {0, 0}}},
  {{{4060, 4660}, {13040, 1900}, {6560, 7840}, {7480, 1360}, {12700, 7100}, {0, 0}}},
  {{{3020, 5190}, {6280, 7760}, {14100, 7760}, {13880, 1220}, {10240, 4920}, {6100, 2200}}},
  {{{10323, 3366}, {11203, 5425}, {7259, 6656}, {5425, 2838}, {0, 0}, {0, 0}}},
}};

constexpr std::array<i32, 13> kBaseMapSizes = {4, 5, 6, 3, 4, 4, 4, 6, 4, 3, 5, 6, 4};

} // namespace

std::vector<std::pair<i32, i32>> const& refereeBaseMap(i32 mapIndex) {
  static const std::vector<std::vector<std::pair<i32, i32>>> kMaps = [] {
    std::vector<std::vector<std::pair<i32, i32>>> maps;
    maps.reserve(kBaseMaps.size());
    for (size_t index = 0; index < kBaseMaps.size(); ++index) {
      maps.emplace_back(
        kBaseMaps[index].begin(),
        kBaseMaps[index].begin() + kBaseMapSizes[index]);
    }
    return maps;
  }();

  i32 clamped = std::max(0, std::min(static_cast<i32>(kMaps.size()) - 1, mapIndex));
  return kMaps[clamped];
}

std::vector<std::pair<i32, i32>> generateRefereeMap(i64 seed, i32 mapIndex) {
  JavaRandom random(seed);
  if (mapIndex < 0) {
    mapIndex = random.nextInt(static_cast<i32>(kBaseMaps.size()));
  }

  std::vector<std::pair<i32, i32>> points = refereeBaseMap(mapIndex);
  if (points.empty()) {
    return points;
  }

  i32 rotation = random.nextInt(static_cast<i32>(points.size()));
  std::rotate(points.begin(), points.end() - rotation, points.end());

  for (std::pair<i32, i32>& point : points) {
    point.first += random.nextInt(201) - 100;
    point.second += random.nextInt(199) - 100;
  }

  return points;
}
