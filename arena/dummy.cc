#include <bits/stdc++.h>
#include <poll.h>
#include <fcntl.h>
#include <unistd.h>

int main() {
  std::this_thread::sleep_for(std::chrono::milliseconds(900));
  ::fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
  while (true) {
    pollfd pfd{};
    pfd.fd = STDIN_FILENO;
    pfd.events = POLLIN;
    while (poll(&pfd, 1, 0) > 0 && (pfd.revents & POLLIN)) {
      char buffer[256];
      while (true) {
        ssize_t n = ::read(STDIN_FILENO, buffer, sizeof(buffer));
        if (n <= 0) {
          break;
        }
      }
      pfd.revents = 0;
    }

    printf("WAIT\n");
    std::this_thread::sleep_for(std::chrono::milliseconds(30));
    fflush(stdout);
  }
}
