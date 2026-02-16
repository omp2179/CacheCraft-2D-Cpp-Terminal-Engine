#pragma once
#include "BlockType.h"
#include "Coord.h"
#include "Pixel.h"
#include "Terrain.h"
#include <array>
#include <iostream>


constexpr int CHUNK_SIZE = 10;

class Chunk {
  std::array<std::array<BlockType, CHUNK_SIZE>, CHUNK_SIZE> blocks;
  Coord position;

public:
  Chunk(Coord pos) : position(pos) { generate_terrain(); }

  BlockType get_block(int xx, int yy) const {
    if (xx < 0 or xx >= CHUNK_SIZE or yy < 0 or yy >= CHUNK_SIZE) {
      return BlockType::AIR;
    }
    return blocks[yy][xx];
  }

  void set_block(int xx, int yy, BlockType type) {
    if (xx < 0 or xx >= CHUNK_SIZE or yy < 0 or yy >= CHUNK_SIZE) {
      return;
    }
    blocks[yy][xx] = type;
  }

  Coord get_position() const { return position; }

private:
  void generate_terrain() { generate_chunk_terrain(blocks, position.x); }
};

inline void print_chunk(const Chunk &chunk) {
  std::cout << "Chunk at " << chunk.get_position() << "\n";
  for (int i = 0; i < CHUNK_SIZE; ++i) {
    for (int j = 0; j < CHUNK_SIZE; ++j) {
      std::cout << block_to_pixel(chunk.get_block(j, i));
    }
    std::cout << "\n";
  }
}