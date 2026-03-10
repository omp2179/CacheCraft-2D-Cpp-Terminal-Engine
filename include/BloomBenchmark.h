#pragma once
#include "BloomFilter.h"
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <unordered_set>

struct PairHash {
  size_t operator()(const std::pair<int, int> &p) const {
    return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 16);
  }
};

inline void run_bloom_benchmark() {
  using namespace std::chrono;
  using std::cout;

  const int N = 10000;
  const int RANGE = 2000;
  const size_t BLOOM_BITS = 65536;
  const int BLOOM_HASHES = 4;

  cout << "\n========================================\n";
  cout << "   BLOOM FILTER BENCHMARK\n";
  cout << "   " << N << " spawn location checks\n";
  cout << "   Range: " << RANGE << "x" << RANGE << " coords\n";
  cout << "   Bloom: " << BLOOM_BITS << " bits, " << BLOOM_HASHES
       << " hashes\n";
  cout << "========================================\n";

  srand(42);

  std::vector<std::pair<int, int>> coords;
  coords.reserve(N);
  for (int i = 0; i < N; ++i) {
    int x = rand() % RANGE - RANGE / 2;
    int y = rand() % RANGE - RANGE / 2;
    coords.push_back({x, y});
  }

  std::unordered_set<std::pair<int, int>, PairHash> hashset;
  int set_dupes = 0;

  auto t0 = high_resolution_clock::now();
  for (auto &[x, y] : coords) {
    if (hashset.count({x, y})) {
      ++set_dupes;
    } else {
      hashset.insert({x, y});
    }
  }
  auto t1 = high_resolution_clock::now();
  long long set_us = duration_cast<microseconds>(t1 - t0).count();

  BloomFilter bloom(BLOOM_BITS, BLOOM_HASHES);
  int bloom_dupes = 0;
  int false_pos = 0;

  std::unordered_set<std::pair<int, int>, PairHash> truth;

  auto t2 = high_resolution_clock::now();
  for (auto &[x, y] : coords) {
    if (bloom.maybe_contains(x, y)) {
      ++bloom_dupes;
      if (!truth.count({x, y}))
        ++false_pos;
    } else {
      bloom.insert(x, y);
    }
    truth.insert({x, y});
  }
  auto t3 = high_resolution_clock::now();
  long long bloom_us = duration_cast<microseconds>(t3 - t2).count();

  int unique_coords = static_cast<int>(hashset.size());
  double true_negatives = unique_coords; // every unique coord is a true negative upon its FIRST encounter
  double fp_rate =
      (true_negatives > 0)
          ? (static_cast<double>(false_pos) / true_negatives) * 100.0
          : 0.0;
  double speedup = (bloom_us > 0) ? static_cast<double>(set_us) /
                                        static_cast<double>(bloom_us)
                                  : 0.0;

  cout << "\n--- unordered_set (baseline) ---\n";
  cout << "  Time: " << set_us << " us\n";
  cout << "  Unique coords: " << unique_coords << "/" << N << "\n";
  cout << "  Duplicates found: " << set_dupes << "\n";

  cout << "\n--- BloomFilter (" << BLOOM_BITS << " bits, " << BLOOM_HASHES
       << " hashes) ---\n";
  cout << "  Time: " << bloom_us << " us\n";
  cout << "  Duplicates caught: " << bloom_dupes << "\n";
  cout << "  False positives: " << false_pos << " (" << fp_rate << "% FPR)\n";
  cout << "  Memory: " << bloom.memory_bytes() << " bytes vs ~"
       << unique_coords * 16 << " bytes (set)\n";

  cout << "\n--- RESULTS ---\n";
  cout << "  Speedup: " << speedup << "x\n";
  cout << "  Memory savings: "
       << (1.0 - static_cast<double>(bloom.memory_bytes()) /
                     (unique_coords * 16)) *
              100.0
       << "%\n";
  cout << "========================================\n";
}
