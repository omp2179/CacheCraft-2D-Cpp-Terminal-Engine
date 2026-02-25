#pragma once
#include "Coord.h"
#include "RobinHoodMap.h"
#include <chrono>
#include <iostream>
#include <unordered_map>
#include <vector>

// ============================================================================
//  Hash Map Benchmark: std::unordered_map vs RobinHoodMap
// ============================================================================
//
//  Tests: Sequential Insert, Random Lookup (Hit), Random Lookup (Miss),
//         Full Iteration. All using Coord keys (real game data type).
//
// ============================================================================

inline void run_hash_benchmark() {
  const int NUM_ENTRIES = 100000;
  const int NUM_LOOKUPS = 100000;

  std::cout << "\n========================================\n";
  std::cout << "   HASH MAP BENCHMARK\n";
  std::cout << "   std::unordered_map vs RobinHoodMap\n";
  std::cout << "   " << NUM_ENTRIES << " entries, " << NUM_LOOKUPS
            << " lookups\n";
  std::cout << "========================================\n\n";

  // Pre-generate keys
  std::vector<Coord> keys(NUM_ENTRIES);
  for (int i = 0; i < NUM_ENTRIES; ++i) {
    keys[i] = {i * 7 + 13, i * 3 - 500}; // scattered coords
  }

  // Pre-generate miss keys (shifted to avoid hits)
  std::vector<Coord> miss_keys(NUM_LOOKUPS);
  for (int i = 0; i < NUM_LOOKUPS; ++i) {
    miss_keys[i] = {i * 7 + 13 + 1000000, i * 3 - 500 + 1000000};
  }

  volatile int sink = 0;

  // ---- BENCHMARK 1: SEQUENTIAL INSERT ----
  std::cout << "--- Sequential Insert ---\n";

  // std::unordered_map insert
  auto t1 = std::chrono::high_resolution_clock::now();
  {
    std::unordered_map<Coord, int, CoordHash> umap;
    for (int i = 0; i < NUM_ENTRIES; ++i) {
      umap[keys[i]] = i;
    }
    sink = static_cast<int>(umap.size());
  }
  auto t2 = std::chrono::high_resolution_clock::now();
  auto umap_insert =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  // RobinHoodMap insert
  t1 = std::chrono::high_resolution_clock::now();
  {
    RobinHoodMap<Coord, int, CoordHash> rmap;
    for (int i = 0; i < NUM_ENTRIES; ++i) {
      rmap[keys[i]] = i;
    }
    sink = static_cast<int>(rmap.size());
  }
  t2 = std::chrono::high_resolution_clock::now();
  auto rmap_insert =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  double insert_speedup =
      static_cast<double>(umap_insert) / static_cast<double>(rmap_insert);
  std::cout << "  unordered_map: " << umap_insert << " us\n";
  std::cout << "  RobinHoodMap:  " << rmap_insert << " us\n";
  std::cout << "  Speedup:       " << insert_speedup << "x\n\n";

  // ---- SETUP: Populate both maps for lookup/iterate tests ----
  std::unordered_map<Coord, int, CoordHash> umap_filled;
  RobinHoodMap<Coord, int, CoordHash> rmap_filled;
  for (int i = 0; i < NUM_ENTRIES; ++i) {
    umap_filled[keys[i]] = i;
    rmap_filled[keys[i]] = i;
  }

  // ---- BENCHMARK 2: RANDOM LOOKUP (HIT) ----
  std::cout << "--- Random Lookup (Hit) ---\n";

  t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_LOOKUPS; ++i) {
    auto it = umap_filled.find(keys[i % NUM_ENTRIES]);
    if (it != umap_filled.end())
      sink = it->second;
  }
  t2 = std::chrono::high_resolution_clock::now();
  auto umap_hit =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_LOOKUPS; ++i) {
    auto it = rmap_filled.find(keys[i % NUM_ENTRIES]);
    if (it != rmap_filled.end()) {
      auto [k, v] = *it;
      sink = v;
    }
  }
  t2 = std::chrono::high_resolution_clock::now();
  auto rmap_hit =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  double hit_speedup =
      static_cast<double>(umap_hit) / static_cast<double>(rmap_hit);
  std::cout << "  unordered_map: " << umap_hit << " us\n";
  std::cout << "  RobinHoodMap:  " << rmap_hit << " us\n";
  std::cout << "  Speedup:       " << hit_speedup << "x\n\n";

  // ---- BENCHMARK 3: RANDOM LOOKUP (MISS) ----
  std::cout << "--- Random Lookup (Miss) ---\n";

  t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_LOOKUPS; ++i) {
    auto it = umap_filled.find(miss_keys[i]);
    if (it != umap_filled.end())
      sink = it->second;
  }
  t2 = std::chrono::high_resolution_clock::now();
  auto umap_miss =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  t1 = std::chrono::high_resolution_clock::now();
  for (int i = 0; i < NUM_LOOKUPS; ++i) {
    auto it = rmap_filled.find(miss_keys[i]);
    if (it != rmap_filled.end()) {
      auto [k, v] = *it;
      sink = v;
    }
  }
  t2 = std::chrono::high_resolution_clock::now();
  auto rmap_miss =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  double miss_speedup =
      static_cast<double>(umap_miss) / static_cast<double>(rmap_miss);
  std::cout << "  unordered_map: " << umap_miss << " us\n";
  std::cout << "  RobinHoodMap:  " << rmap_miss << " us\n";
  std::cout << "  Speedup:       " << miss_speedup << "x\n\n";

  // ---- BENCHMARK 4: FULL ITERATION ----
  std::cout << "--- Full Iteration ---\n";

  t1 = std::chrono::high_resolution_clock::now();
  for (auto &[k, v] : umap_filled) {
    sink = v;
  }
  t2 = std::chrono::high_resolution_clock::now();
  auto umap_iter =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  t1 = std::chrono::high_resolution_clock::now();
  for (auto [k, v] : rmap_filled) {
    sink = v;
  }
  t2 = std::chrono::high_resolution_clock::now();
  auto rmap_iter =
      std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();

  double iter_speedup =
      static_cast<double>(umap_iter) / static_cast<double>(rmap_iter);
  std::cout << "  unordered_map: " << umap_iter << " us\n";
  std::cout << "  RobinHoodMap:  " << rmap_iter << " us\n";
  std::cout << "  Speedup:       " << iter_speedup << "x\n\n";

  // ---- SUMMARY ----
  std::cout << "========================================\n";
  std::cout << "   SUMMARY\n";
  std::cout << "   Insert speedup:      " << insert_speedup << "x\n";
  std::cout << "   Lookup (hit) speedup:" << hit_speedup << "x\n";
  std::cout << "   Lookup (miss) speedup:" << miss_speedup << "x\n";
  std::cout << "   Iteration speedup:   " << iter_speedup << "x\n";
  std::cout << "========================================\n\n";

  (void)sink;
}
