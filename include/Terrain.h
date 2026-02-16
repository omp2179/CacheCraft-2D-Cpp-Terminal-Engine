#pragma once
#include "BlockType.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

inline float hash_noise(int x, int seed) {
  int n = x * 374761393 + seed * 668265263;
  n = (n << 13) ^ n;
  n = n * (n * n * 15731 + 789221) + 668265263;
  unsigned int m = (n & 0x007FFFFF) | 0x3F800000;

  float f;
  std::memcpy(&f, &m, sizeof(f));

  return f - 1.0f;
}

inline float smooth_noise(float x, int seed) {
  int i = static_cast<int>(x);
  int xi = i - (i > x);

  float frac = x - static_cast<float>(xi);

  float t = frac * frac * (3.0f - 2.0f * frac);

  float a = hash_noise(xi, seed);
  float b = hash_noise(xi + 1, seed);

  return a + t * (b - a);
}

inline float fbm(float x, int seed, int octaves = 4) {
  float value = 0.0f;
  float amplitude = 1.0f;
  float max_amplitude = 0.0f;

  float frequency = 0.1f;

  const float lacunarity = 2.0f;
  const float gain = 0.5f;

  for (int i = 0; i < octaves; i++) {
    value += smooth_noise(x * frequency, seed ^ (i * 0x1f1f1f1f)) * amplitude;
    max_amplitude += amplitude;
    amplitude *= gain;
    frequency *= lacunarity;
  }

  return value / max_amplitude;
}

inline void
generate_chunk_terrain(std::array<std::array<BlockType, 10>, 10> &blocks,
                       int cx, int seed = 42) {
  for (int x = 0; x < 10; ++x) {
    int wx = cx * 10 + x;

    float noise = fbm(static_cast<float>(wx), seed);

    int surface_y = 2 + static_cast<int>(noise * 4);

    for (int y = 0; y < 10; ++y) {
      if (y < surface_y) {
        blocks[y][x] = BlockType::AIR;
      } else if (y == surface_y) {
        blocks[y][x] = BlockType::GRASS;
      } else if (y < surface_y + 3) {
        blocks[y][x] = BlockType::DIRT;
      } else if (y < 9) {

        float ore_noise = hash_noise(wx * 100 + y, seed + 99);

        if (ore_noise > 0.95f) {
          blocks[y][x] = BlockType::DIAMOND;
        } else if (ore_noise > 0.88f) {
          blocks[y][x] = BlockType::GOLD;
        } else if (ore_noise > 0.80f) {
          blocks[y][x] = BlockType::IRON;
        } else {
          blocks[y][x] = BlockType::STONE;
        }
      } else {
        blocks[y][x] = BlockType::BEDROCK;
      }
    }
  }
}