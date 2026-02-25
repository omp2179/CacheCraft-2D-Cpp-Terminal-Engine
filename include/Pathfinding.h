#pragma once
#include "BlockType.h"
#include "Coord.h"
#include "RobinHoodMap.h"
#include "World.h"
#include <queue>
#include <vector>

inline std::vector<Coord> bfs_findpath(Coord s, Coord tar, World &world,
                                       int max_depth = 80) {

  if (s == tar) {
    return {s};
  }

  std::queue<Coord> qq;
  qq.push(s);

  RobinHoodMap<Coord, Coord, CoordHash> parent;
  parent[s] = s;

  int depth = 0;
  int current_level_rem = 1;
  int next_level_cnt = 0;

  const Coord dirs[] = {{-1, 0},  {1, 0},  {0, 1},  {0, -1},
                        {-1, -1}, {1, -1}, {-1, 1}, {1, 1}};

  while (!qq.empty() and depth < max_depth) {
    Coord cur = qq.front();
    qq.pop();

    if (cur == tar) {
      break;
    }

    for (const Coord &dir : dirs) {
      Coord nei = cur + dir;

      if (parent.count(nei))
        continue;

      if (world.get_block(nei.x, nei.y) != BlockType::AIR)
        continue;

      if (dir.y == -1 and dir.x != 0) {
        if (world.get_block(cur.x + dir.x, cur.y) == BlockType::AIR) {
          continue;
        }
      }

      if (dir.y == -1 and dir.x == 0) {
        if (world.get_block(cur.x, cur.y + 1) == BlockType::AIR) {
          continue;
        }
      }

      if (dir.y == 0) {
        if (world.get_block(nei.x, nei.y + 1) == BlockType::AIR) {
          continue;
        }
      }

      if (dir.y == 1 and dir.x != 0) {
        if (world.get_block(nei.x, nei.y + 1) == BlockType::AIR) {
          continue;
        }
      }

      parent[nei] = cur;
      qq.push(nei);
      ++next_level_cnt;
    }

    --current_level_rem;
    if (current_level_rem == 0) {
      depth++;
      current_level_rem = next_level_cnt;
      next_level_cnt = 0;
    }
  }

  if (!parent.count(tar)) {
    return {};
  }

  std::vector<Coord> path;
  Coord cur = tar;
  while (cur != s) {
    path.push_back(cur);
    cur = parent[cur];
  }
  path.push_back(s);
  std::reverse(path.begin(), path.end());

  return path;
}
