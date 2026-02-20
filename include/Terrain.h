#pragma once
#include "BlockType.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>

constexpr int CHUNK_SIZE = 32;

inline float hash_noise(int x, int seed) {
  int n = x * 374761393 + seed * 668265263;
  n = (n << 13) ^ n;
  n = n * (n * n * 15731 + 789221) + 668265263;
  unsigned int m = (n & 0x007FFFFF) | 0x3F800000;

  float f;
  std::memcpy(&f, &m, sizeof(f));

  return f - 1.0f;
}

inline float hash_noise_2d(int x, int y, int seed) {
  int n = x * 374761393 + y * 668265263 + seed * 1274126177;
  n = (n << 13) ^ n;
  n = n * (n * n * 15731 + 789221) + 1376312589;
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

inline float smooth_noise_2d(float x, float y, int seed) {
  int ix = static_cast<int>(x) - (static_cast<int>(x) > x);
  int iy = static_cast<int>(y) - (static_cast<int>(y) > y);
  float fx = x - static_cast<float>(ix);
  float fy = y - static_cast<float>(iy);
  float tx = fx * fx * (3.0f - 2.0f * fx);
  float ty = fy * fy * (3.0f - 2.0f * fy);

  float c00 = hash_noise_2d(ix, iy, seed);
  float c10 = hash_noise_2d(ix + 1, iy, seed);
  float c01 = hash_noise_2d(ix, iy + 1, seed);
  float c11 = hash_noise_2d(ix + 1, iy + 1, seed);

  float a = c00 + tx * (c10 - c00);
  float b = c01 + tx * (c11 - c01);
  return a + ty * (b - a);
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

inline float fbm_2d(float x, float y, int seed, int octaves = 4) {
  float value = 0.0f;
  float amplitude = 1.0f;
  float max_amplitude = 0.0f;
  float frequency = 0.15f;
  for (int i = 0; i < octaves; i++) {
    value +=
        smooth_noise_2d(x * frequency, y * frequency, seed ^ (i * 0x2f2f2f2f)) *
        amplitude;
    max_amplitude += amplitude;
    amplitude *= 0.5f;
    frequency *= 2.0f;
  }
  return value / max_amplitude;
}

inline void generate_chunk_terrain(
    std::array<std::array<BlockType, CHUNK_SIZE>, CHUNK_SIZE> &blocks, int cx,
    int seed = 42) {
  for (int x = 0; x < CHUNK_SIZE; ++x) {
    int wx = cx * CHUNK_SIZE + x;

    float noise = fbm(static_cast<float>(wx), seed);

    int surface_y = 8 + static_cast<int>(noise * 8);

    if (surface_y < 2)
      surface_y = 2;
    if (surface_y > CHUNK_SIZE - 6)
      surface_y = CHUNK_SIZE - 6;

    for (int y = 0; y < CHUNK_SIZE; ++y) {
      if (y < surface_y) {
        blocks[y][x] = BlockType::AIR;
      } else if (y == surface_y) {
        blocks[y][x] = BlockType::GRASS;
      } else if (y < surface_y + 4) {
        blocks[y][x] = BlockType::DIRT;
      } else if (y < CHUNK_SIZE - 1) {

        float cave =
            fbm_2d(static_cast<float>(wx), static_cast<float>(y), seed + 777);

        if (cave > 0.55f) {
          blocks[y][x] = BlockType::AIR;
        } else {
          float ore_noise = hash_noise(wx * 100 + y, seed + 99);

          if (ore_noise > 0.95f and y > 20) {
            blocks[y][x] = BlockType::DIAMOND;
          } else if (ore_noise > 0.88f and y > 15) {
            blocks[y][x] = BlockType::GOLD;
          } else if (ore_noise > 0.80f) {
            blocks[y][x] = BlockType::IRON;
          } else {
            blocks[y][x] = BlockType::STONE;
          }
        }
      } else {
        blocks[y][x] = BlockType::BEDROCK;
      }
    }
  }
}