#pragma once
#include "BlockType.h"
#include "Chunk.h"
#include "Coord.h"
#include "Pixel.h"
#include "RobinHoodMap.h"
#include <iostream>
#include <memory>

class World {
private:
  RobinHoodMap<Coord, std::unique_ptr<Chunk>, CoordHash> chunks;

public:
  Chunk &get_chunk(Coord pos) {
    auto it = chunks.find(pos);
    if (it == chunks.end()) {
      chunks[pos] = std::make_unique<Chunk>(pos);
      return *chunks[pos];
    }
    auto [key, val] = *it;
    return *val;
  }

  BlockType get_block(int wx, int wy) {
    Coord chunk_pos = world_to_chunk(wx, wy);
    int cx = wx % CHUNK_SIZE;
    int cy = wy % CHUNK_SIZE;
    if (cx < 0)
      cx += CHUNK_SIZE;
    if (cy < 0)
      cy += CHUNK_SIZE;
    return get_chunk(chunk_pos).get_block(cx, cy);
  }

  void set_block(int wx, int wy, BlockType type) {
    Coord chunk_pos = world_to_chunk(wx, wy);
    int cx = wx % CHUNK_SIZE;
    int cy = wy % CHUNK_SIZE;
    if (cx < 0)
      cx += CHUNK_SIZE;
    if (cy < 0)
      cy += CHUNK_SIZE;
    get_chunk(chunk_pos).set_block(cx, cy, type);
  }

  size_t chunk_count() const { return chunks.size(); }

private:
  static Coord world_to_chunk(int wx, int wy) {
    int cx, cy;

    if (wx >= 0) {
      cx = wx / CHUNK_SIZE;
    } else {
      cx = (wx - CHUNK_SIZE + 1) / CHUNK_SIZE;
    }

    if (wy >= 0) {
      cy = wy / CHUNK_SIZE;
    } else {
      cy = (wy - CHUNK_SIZE + 1) / CHUNK_SIZE;
    }

    return {cx, cy};
  }
};

inline void print_world(World &world, int min_x, int max_x, int min_y,
                        int max_y) {
  for (int y = min_y; y <= max_y; ++y) {
    for (int x = min_x; x <= max_x; ++x) {
      std::cout << block_to_pixel(world.get_block(x, y));
    }
    std::cout << "\n";
  }
}
