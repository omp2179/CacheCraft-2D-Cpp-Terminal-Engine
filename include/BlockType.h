#pragma once
#include <cstdint>
#include <string>

enum class BlockType : uint8_t {
  AIR = 0,
  GRASS,
  DIRT,
  STONE,
  IRON,
  GOLD,
  DIAMOND,
  WOOD,
  LEAF,
  BEDROCK,
  COUNT
};

inline char block_to_char(BlockType b) {
  switch (b) {
  case BlockType::AIR:
    return ' ';
  case BlockType::GRASS:
    return '"';
  case BlockType::DIRT:
    return '.';
  case BlockType::STONE:
    return '#';
  case BlockType::IRON:
    return 'I';
  case BlockType::GOLD:
    return 'G';
  case BlockType::DIAMOND:
    return 'D';
  case BlockType::WOOD:
    return '|';
  case BlockType::LEAF:
    return '*';
  case BlockType::BEDROCK:
    return 'B';
  default:
    return '?';
  }
}

inline std::string block_to_string(BlockType b) {
  switch (b) {
  case BlockType::AIR:
    return "Air";
  case BlockType::GRASS:
    return "Grass";
  case BlockType::DIRT:
    return "Dirt";
  case BlockType::STONE:
    return "Stone";
  case BlockType::IRON:
    return "Iron";
  case BlockType::GOLD:
    return "Gold";
  case BlockType::DIAMOND:
    return "Diamond";
  case BlockType::WOOD:
    return "Wood";
  case BlockType::LEAF:
    return "Leaf";
  case BlockType::BEDROCK:
    return "Bedrock";
  default:
    return "Unknown";
  }
}