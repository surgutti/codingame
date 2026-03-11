#include <iostream>
#include <utility>
#include <vector>

int main() {
  int laps = 0;
  int checkpointCount = 0;
  if (!(std::cin >> laps >> checkpointCount)) {
    return 0;
  }

  std::vector<std::pair<int, int>> checkpoints;
  checkpoints.reserve(checkpointCount);
  for (int i = 0; i < checkpointCount; ++i) {
    int x = 0;
    int y = 0;
    std::cin >> x >> y;
    checkpoints.emplace_back(x, y);
  }

  while (true) {
    int x[4];
    int y[4];
    int vx[4];
    int vy[4];
    int angle[4];
    int next[4];
    for (int i = 0; i < 4; ++i) {
      if (!(std::cin >> x[i] >> y[i] >> vx[i] >> vy[i] >> angle[i] >> next[i])) {
        return 0;
      }
    }

    for (int i = 0; i < 2; ++i) {
      std::pair<int, int> checkpoint = checkpoints[next[i] % checkpointCount];
      std::cout << checkpoint.first << ' ' << checkpoint.second << " 50" << std::endl;
    }
  }
}
