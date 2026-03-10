#pragma once
#include "FastRand.h"
#include "Mob.h"
#include "MobStorage.h"
#include <chrono>
#include <iostream>
#include <vector>

inline void run_aos_vs_soa_benchmark() {
  const int NUM_MOBS = 10000;
  const int NUM_ITERATIONS = 10000;

  struct MobAoS {
    int x, y;
    int vx, vy;
    int hp, max_hp;
    int damage;
    int ai_state;
    int target_x, target_y;
    int anim_frame;
    int spawn_time;
    int last_attack;
    int path_length;
    int flags;
    int loot_table;
    int armor;
    int aggro_range;
    int padding[2];
  };

  std::cout << "\n========================================\n";
  std::cout << "   AoS vs SoA BENCHMARK\n";
  std::cout << "   " << NUM_MOBS << " mobs x " << NUM_ITERATIONS
            << " iterations\n";
  std::cout << "   AoS struct size: " << sizeof(MobAoS) << " bytes\n";
  std::cout << "========================================\n\n";

  std::vector<MobAoS> aos_mobs(NUM_MOBS);
  for (int i = 0; i < NUM_MOBS; ++i) {
    aos_mobs[i].x = static_cast<int>(fast_rand() % 1000);
    aos_mobs[i].y = static_cast<int>(fast_rand() % 1000);
    aos_mobs[i].vx = 1;
    aos_mobs[i].vy = 1;
    aos_mobs[i].hp = 100;
    aos_mobs[i].max_hp = 100;
    aos_mobs[i].damage = 10;
    aos_mobs[i].ai_state = 1;
    aos_mobs[i].target_x = 500;
    aos_mobs[i].target_y = 500;
    aos_mobs[i].armor = 5;
    aos_mobs[i].aggro_range = 16;
  }

  MobStorage soa_mobs;
  for (int i = 0; i < NUM_MOBS; ++i) {
    soa_mobs.add(static_cast<int>(fast_rand() % 1000),
                 static_cast<int>(fast_rand() % 1000), 100, MobType::ZOMBIE,
                 AIState::CHASING);
  }

  volatile int aos_sink = 0;
  volatile int soa_sink = 0;

  auto aos_start = std::chrono::high_resolution_clock::now();
  for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
    for (int i = 0; i < NUM_MOBS; ++i) {
      aos_mobs[i].x += aos_mobs[i].vx;
      aos_mobs[i].y += aos_mobs[i].vy;
      int dx = aos_mobs[i].target_x - aos_mobs[i].x;
      int dy = aos_mobs[i].target_y - aos_mobs[i].y;
      if (dx > 0) aos_mobs[i].vx = 1;
      else if (dx < 0) aos_mobs[i].vx = -1;
      if (dy > 0) aos_mobs[i].vy = 1;
      else if (dy < 0) aos_mobs[i].vy = -1;
      aos_mobs[i].hp -= (aos_mobs[i].damage - aos_mobs[i].armor) > 0 ? 1 : 0;
      if (aos_mobs[i].hp <= 0) aos_mobs[i].hp = aos_mobs[i].max_hp;
      ++aos_mobs[i].anim_frame;
    }
  }
  aos_sink = aos_mobs[0].x + aos_mobs[0].hp;
  auto aos_end = std::chrono::high_resolution_clock::now();
  auto aos_time =
      std::chrono::duration_cast<std::chrono::microseconds>(aos_end - aos_start)
          .count();

  auto soa_start = std::chrono::high_resolution_clock::now();
  for (int iter = 0; iter < NUM_ITERATIONS; ++iter) {
    for (int i = 0; i < NUM_MOBS; ++i) {
      soa_mobs.x[i] += 1;
      soa_mobs.y[i] += 1;
    }
  }
  soa_sink = soa_mobs.x[0];
  auto soa_end = std::chrono::high_resolution_clock::now();
  auto soa_time =
      std::chrono::duration_cast<std::chrono::microseconds>(soa_end - soa_start)
          .count();

  (void)aos_sink;
  (void)soa_sink;

  double speedup = static_cast<double>(aos_time) / soa_time;

  std::cout << "AoS time: " << aos_time << " us\n";
  std::cout << "SoA time: " << soa_time << " us\n";
  std::cout << "Speedup:  " << speedup << "x\n";

  if (speedup >= 3.0) {
    std::cout << "\n>> TARGET MET: SoA is >= 3x faster! <<\n";
  } else if (speedup >= 2.0) {
    std::cout << "\n>> SoA is " << speedup
              << "x faster (close to 3x target) <<\n";
  } else {
    std::cout << "\n>> Speedup: " << speedup << "x <<\n";
  }

  std::cout << "\n========================================\n\n";
}
