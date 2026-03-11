#include <bits/stdc++.h>

int main() {
  std::this_thread::sleep_for(std::chrono::milliseconds(900));
  while (1) {
    printf("WAIT\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    fflush(stdout);
  }
}