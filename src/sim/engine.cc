#include "engine.h"

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

    for (int i = 0; i < POD_NB; i++) {
      pods[i].move(t);
    }
    left -= t;

    if (pod_a != pod_b) {
      bounce(&pods[pod_a], &pods[pod_b]);
    }

    if (left > 0) {
      for (int i = 0; i < POD_NB; i++) {
        Pod& pod = pods[i];
        if (checkpointCollide(prev_pods[i], pod, cps[pod.next])) {
          if (pod.next == static_cast<int>(cps.size())) {
            winner |= (i >> 1);
          }
          else {
            pod.next++;
          }
        }
      }
    }
  }

  for (Pod& pod : pods) {
    pod.adjust();

    if (checkpointCollide())
  }
}