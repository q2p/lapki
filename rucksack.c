#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

enum {
  STEPS = 5,
  MAX_RES = 15,
  MAX_RES_N = MAX_RES + 1,
  MAX_PER_STEP_MAX = 5,
};

uint16_t COST[STEPS * MAX_PER_STEP_MAX] = {
  150, 300, 400, 500, 500,
  130, 250, 360, 500, 500,
  150, 300, 450, 600, 600,
  130, 250, 360, 500, 500,
  160, 330, 480, 650, 800
};

uint16_t MAX_PER_STEP[STEPS] = { 4, 4, 4, 3, 5 };

uint16_t results[STEPS*MAX_RES_N];
uint8_t best_x[STEPS*MAX_RES_N];
uint8_t rev[STEPS];

void back_track(int s, int r) {
  for (uint8_t t = 0; t <= MAX_PER_STEP_MAX; t++) {
    if (best_x[s*MAX_RES_N+r] & (1 << t)) {
      rev[s] = t;
      if (s == 0) {
        printf("( ");
        for (uint8_t i = 0; i != STEPS; i++) {
          printf("%d ", rev[i]);
        }
        printf(")\n");
      } else {
        back_track(s-1, r-t);
      }
    }
  }
}

void main() {
  for (uint8_t i = 0; i != STEPS*MAX_RES_N; i++) {
    results[i] = 0;
    best_x[i] = 0;
  }
  for (uint8_t s = 0; s != STEPS; s++) {
    int mps = MAX_PER_STEP[s];
    for (uint8_t r = 0; r != MAX_RES_N; r++) {
      int minr = mps;
      if (r < mps) {
        minr = r;
      }
      int max_cost = 0;
      uint8_t steps = 0;
      for (uint8_t t = 0; t <= minr; t++) {
        int cost = 0;
        if (t) {
          cost = COST[s*MAX_PER_STEP_MAX+(t-1)];
        }
        if (s) {
          cost += results[(s-1)*MAX_RES_N+(r-t)];
        }
        if (cost > max_cost) {
          max_cost = cost;
          steps = 0;
        }
        if (cost == max_cost) {
          steps |= (1 << t);
        }
      }

      results[s*MAX_RES_N+r] += max_cost;
      best_x[s*MAX_RES_N+r] = steps;
    }
  }
  for (uint8_t r = 0; r != MAX_RES_N; r++) {
    printf("y: %2d ", r);
    for (uint8_t s = 0; s != STEPS; s++) {
      printf("%5d ", results[s*MAX_RES_N+r]);
    }
    printf("\n");
  }
  back_track(STEPS-1, MAX_RES);
}
