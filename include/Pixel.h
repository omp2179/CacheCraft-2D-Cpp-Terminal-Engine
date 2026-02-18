#pragma once
#include "BlockType.h"
#include <cstdint>
#include <iostream>

enum class Color : uint8_t {
  BLACK = 30,
  RED = 31,
  GREEN = 32,
  YELLOW = 33,
  BLUE = 34,
  MAGENTA = 35,
  CYAN = 36,
  WHITE = 37,
  GRAY = 90,
  BRIGHT_RED = 91,
  BRIGHT_GREEN = 92,
  BRIGHT_YELLOW = 93,
  BRIGHT_BLUE = 94,
  BRIGHT_MAGENTA = 95,
  BRIGHT_CYAN = 96,
  BRIGHT_WHITE = 97,
  COUNT
};

struct Pixel {
  char ch = ' ';
  Color color = Color::WHITE;
};

inline std::ostream &operator<<(std::ostream &os, const Pixel &p) {
  os << "\033[" << static_cast<int>(p.color) << "m" << p.ch << "\033[0m";
  return os;
}

inline Pixel block_to_pixel(BlockType b) {
  switch (b) {
  case BlockType::AIR:
    return {' ', Color::BLACK};
  case BlockType::GRASS:
    return {'"', Color::BRIGHT_GREEN};
  case BlockType::DIRT:
    return {'.', Color::RED};
  case BlockType::STONE:
    return {'#', Color::GRAY};
  case BlockType::IRON:
    return {'I', Color::BRIGHT_WHITE};
  case BlockType::GOLD:
    return {'G', Color::BRIGHT_YELLOW};
  case BlockType::DIAMOND:
    return {'D', Color::BRIGHT_CYAN};
  case BlockType::BEDROCK:
    return {'B', Color::MAGENTA};
  default:
    return {'?', Color::BRIGHT_RED};
  }
}