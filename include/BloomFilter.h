#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

class BloomFilter {
  std::vector<uint8_t> bits;
  size_t num_bits;
  int num_hashes;

  size_t hash(int x, int y, int seed) const {
    uint32_t h = static_cast<uint32_t>(seed);
    uint32_t k1 = static_cast<uint32_t>(x);
    uint32_t k2 = static_cast<uint32_t>(y);

    k1 *= 0xcc9e2d51;
    k1 = (k1 << 15) | (k1 >> 17);
    k1 *= 0x1b873593;
    h ^= k1;
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;

    k2 *= 0xcc9e2d51;
    k2 = (k2 << 15) | (k2 >> 17);
    k2 *= 0x1b873593;
    h ^= k2;
    h = (h << 13) | (h >> 19);
    h = h * 5 + 0xe6546b64;

    h ^= 12;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return static_cast<size_t>(h) % num_bits;
  }

public:
  BloomFilter(size_t bits_count = 8192, int hashes = 3)
      : bits((bits_count + 7) / 8, 0), num_bits(bits_count),
        num_hashes(hashes) {}

  void insert(int x, int y) {
    for (int i = 0; i < num_hashes; ++i) {
      size_t idx = hash(x, y, i);
      bits[idx / 8] |= (1 << (idx % 8));
    }
  }

  bool maybe_contains(int x, int y) const {
    for (int i = 0; i < num_hashes; ++i) {
      size_t idx = hash(x, y, i);
      if (!(bits[idx / 8] & (1 << (idx % 8))))
        return false;
    }
    return true;
  }

  void clear() {
    for (auto &b : bits)
      b = 0;
  }

  size_t memory_bytes() const { return bits.size(); }
};
