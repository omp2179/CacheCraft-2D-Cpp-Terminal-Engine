#pragma once
#include "Chunk.h"
#include "MobStorage.h"
#include "World.h"
#include <cstdint>
#include <cstring>
#include <fstream>
#include <memory>
#include <string>

inline bool save_game(const std::string &path, World &world, int px, int py,
                      int hp, int facing, int sel, int *inv,
                      MobStorage &mobs) {
  std::ofstream f(path, std::ios::binary);
  if (!f)
    return false;

  f.write("MC2D", 4);

  int nc = static_cast<int>(world.chunk_count());
  f.write(reinterpret_cast<char *>(&nc), 4);
  f.write(reinterpret_cast<char *>(&px), 4);
  f.write(reinterpret_cast<char *>(&py), 4);
  f.write(reinterpret_cast<char *>(&hp), 4);
  f.write(reinterpret_cast<char *>(&facing), 4);
  f.write(reinterpret_cast<char *>(&sel), 4);
  f.write(reinterpret_cast<char *>(inv), 9 * 4);

  for (auto [pos, chunk_ptr] : world) {
    Coord cp = chunk_ptr->get_position();
    f.write(reinterpret_cast<char *>(&cp.x), 4);
    f.write(reinterpret_cast<char *>(&cp.y), 4);

    auto &blk = chunk_ptr->get_blocks();
    for (int r = 0; r < CHUNK_SIZE; ++r) {
      for (int c = 0; c < CHUNK_SIZE; ++c) {
        uint8_t b = static_cast<uint8_t>(blk[r][c]);
        f.write(reinterpret_cast<char *>(&b), 1);
      }
    }
  }

  int nm = static_cast<int>(mobs.count());
  f.write(reinterpret_cast<char *>(&nm), 4);
  for (int i = 0; i < nm; ++i) {
    f.write(reinterpret_cast<char *>(&mobs.x[i]), 4);
    f.write(reinterpret_cast<char *>(&mobs.y[i]), 4);
    f.write(reinterpret_cast<char *>(&mobs.hp[i]), 4);
    uint8_t t = static_cast<uint8_t>(mobs.type[i]);
    uint8_t s = static_cast<uint8_t>(mobs.state[i]);
    f.write(reinterpret_cast<char *>(&t), 1);
    f.write(reinterpret_cast<char *>(&s), 1);
  }

  return f.good();
}

inline bool load_game(const std::string &path, World &world, int &px, int &py,
                      int &hp, int &facing, int &sel, int *inv,
                      MobStorage &mobs) {
  std::ifstream f(path, std::ios::binary);
  if (!f)
    return false;

  char magic[4];
  f.read(magic, 4);
  if (std::memcmp(magic, "MC2D", 4) != 0)
    return false;

  int nc;
  f.read(reinterpret_cast<char *>(&nc), 4);
  f.read(reinterpret_cast<char *>(&px), 4);
  f.read(reinterpret_cast<char *>(&py), 4);
  f.read(reinterpret_cast<char *>(&hp), 4);
  f.read(reinterpret_cast<char *>(&facing), 4);
  f.read(reinterpret_cast<char *>(&sel), 4);
  f.read(reinterpret_cast<char *>(inv), 9 * 4);

  world.clear();
  for (int i = 0; i < nc; ++i) {
    int cx, cy;
    f.read(reinterpret_cast<char *>(&cx), 4);
    f.read(reinterpret_cast<char *>(&cy), 4);

    std::array<std::array<BlockType, CHUNK_SIZE>, CHUNK_SIZE> blk;
    for (int r = 0; r < CHUNK_SIZE; ++r) {
      for (int c = 0; c < CHUNK_SIZE; ++c) {
        uint8_t b;
        f.read(reinterpret_cast<char *>(&b), 1);
        blk[r][c] = static_cast<BlockType>(b);
      }
    }

    Coord pos = {cx, cy};
    world.load_chunk(pos, std::make_unique<Chunk>(pos, std::move(blk)));
  }

  mobs.x.clear();
  mobs.y.clear();
  mobs.hp.clear();
  mobs.type.clear();
  mobs.state.clear();

  int nm;
  f.read(reinterpret_cast<char *>(&nm), 4);
  for (int i = 0; i < nm; ++i) {
    int mx, my, mhp;
    uint8_t t, s;
    f.read(reinterpret_cast<char *>(&mx), 4);
    f.read(reinterpret_cast<char *>(&my), 4);
    f.read(reinterpret_cast<char *>(&mhp), 4);
    f.read(reinterpret_cast<char *>(&t), 1);
    f.read(reinterpret_cast<char *>(&s), 1);
    mobs.add(mx, my, mhp, static_cast<MobType>(t), static_cast<AIState>(s));
  }

  return f.good();
}
