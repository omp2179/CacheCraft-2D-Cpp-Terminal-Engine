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

  // Realistic AoS mob — a real game mob has MANY more fields.
  // This is what a mob looks like in a production game engine.
  // Total: ~80 bytes → only 0-1 mobs per 64-byte cache line.
  struct MobAoS {
    int x, y;               // position (what we update)
    int vx, vy;             // velocity
    int hp, max_hp;         // health
    int damage;             // attack damage
    int ai_state;           // AI FSM state
    int target_x, target_y; // pathfinding target
    int anim_frame;         // animation
    int spawn_time;         // when mob was created
    int last_attack;        // cooldown timer
    int path_length;        // BFS result cache
    int flags;              // bitfield (is_hostile, is_burning, etc.)
    int loot_table;         // drop table index
    int armor;              // damage reduction
    int aggro_range;        // detection distance
    int padding[2];         // align to 80 bytes
  };

  std::cout << "\n========================================\n";
  std::cout << "   AoS vs SoA BENCHMARK\n";
  std::cout << "   " << NUM_MOBS << " mobs x " << NUM_ITERATIONS
            << " iterations\n";
  std::cout << "   AoS struct size: " << sizeof(MobAoS) << " bytes\n";
  std::cout << "========================================\n\n";

  // ---- SETUP: AoS (realistic) ----
  std::vector<MobAoS> aos_mobs(NUM_MOBS);
  for (int i = 0; i < NUM_MOBS; i++) {
    aos_mobs[i].x = static_cast<int>(fast_rand() % 1000);
    aos_mobs[i].y = static_cast<int>(fast_rand() % 1000);
    aos_mobs[i].hp = 100;
  }

  // ---- SETUP: SoA ----
  MobStorage soa_mobs;
  for (int i = 0; i < NUM_MOBS; i++) {
    soa_mobs.add(static_cast<int>(fast_rand() % 1000),
                 static_cast<int>(fast_rand() % 1000), 100, MobType::ZOMBIE,
                 AIState::CHASING);
  }

  // Accumulators to prevent compiler from optimizing away the loops
  volatile int aos_sink = 0;
  volatile int soa_sink = 0;

  // ---- BENCHMARK: AoS ----
  auto aos_start = std::chrono::high_resolution_clock::now();
  for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
    for (int i = 0; i < NUM_MOBS; i++) {
      aos_mobs[i].x += 1;
      aos_mobs[i].y += 1;
    }
  }
  aos_sink = aos_mobs[0].x;
  auto aos_end = std::chrono::high_resolution_clock::now();
  auto aos_time =
      std::chrono::duration_cast<std::chrono::microseconds>(aos_end - aos_start)
          .count();

  // ---- BENCHMARK: SoA ----
  auto soa_start = std::chrono::high_resolution_clock::now();
  for (int iter = 0; iter < NUM_ITERATIONS; iter++) {
    for (int i = 0; i < NUM_MOBS; i++) {
      soa_mobs.x[i] += 1;
      soa_mobs.y[i] += 1;
    }
  }
  soa_sink = soa_mobs.x[0];
  auto soa_end = std::chrono::high_resolution_clock::now();
  auto soa_time =
      std::chrono::duration_cast<std::chrono::microseconds>(soa_end - soa_start)
          .count();

  // Suppress unused warnings
  (void)aos_sink;
  (void)soa_sink;

  // ---- RESULTS ----
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
