#include "engine.h"

void applyMove(int podId, Move const& move) {

}

void Engine::nextTurn() {
  f64 left = 1.0;

  std::array<Pod, POD_NB> prev_pods = pods;
  while (left > 0) {
    f64 t = left;
    int pod_a = 0, pod_b = 0;

    for (int i = POD_NB - 1; i >= 0; i--) {
      for (int j = i - 1; j >= 0; j--) {
        f64 c = pods[i].getCollision(pods[j], POD_RADIUS);
        if (c <= t) {
          t = c;
          pod_a = i;
          pod_b = j;
        }
      }
    }
    left -= t;

    for (int i = 0; i < POD_NB; i++) {
      pods[i].move(t);
    }

    if (pod_a != pod_b) {
      bounce(&pods[pod_a], &pods[pod_b]);
    }
    
    if (left > 0) {
      for (int i = 0; i < POD_NB; i++) {
        Pod& pod = pods[i];
        if (checkpointCollide(prev_pods[i], pod, cps[pod.next])) {
          checkpointCompleted(i);
        }

        prev_pods[i] = pod;
      }
    }
  }

  for (int i = 0; i < POD_NB; i++) {
    Pod& pod = pods[i];
    pod.adjust();

    if (checkpointCollide(prev_pods[i], pod, cps[pod.next])) {
      checkpointCompleted(i);
    }
  }

  for (int p = 0; p < PLAYER_NB; p++) {
    timeouts[p]--;
  }
}