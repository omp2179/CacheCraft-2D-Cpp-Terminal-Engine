#pragma once
#include <cstdint>

inline uint32_t xorshift_state = 123456789;

inline void seed_fast_rand(uint32_t seed) { xorshift_state = seed ? seed : 1; }

inline uint32_t fast_rand() {
  xorshift_state ^= xorshift_state << 13;
  xorshift_state ^= xorshift_state >> 17;
  xorshift_state ^= xorshift_state << 5;
  return xorshift_state;
}
