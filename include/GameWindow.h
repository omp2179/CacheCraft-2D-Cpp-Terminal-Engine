#pragma once
#include "BlockType.h"
#include "CheatState.h"
#include "Coord.h"
#include "FastRand.h"
#include "Mob.h"
#include "MobStorage.h"
#include "Pathfinding.h"
#include "Pixel.h"
#include "Terrain.h"
#include "Window.h"
#include "World.h"
#include <string>

class GameWindow : public Window {
private:
  MobStorage mobs;
  World &world;
  int &player_x;
  int &player_y;
  int &facing;
  int *inventory;
  int &selected_block;
  int fall_timer = 0;
  const int GRAVITY_INTERVAL = 5;
  CheatState &cheats;
  int hp = 100;
  int max_hp = 100;
  int fall_distance = 0;
  int spawn_x = 0;
  int spawn_y = 0;
  int damage_cooldown = 0;
  bool is_dead = false;

  int spawn_timer = 0;
  const int SPAWN_INTERVAL = 120;
  const int MOB_MOVE_INTERVAL = 10;
  int mob_move_timer = 0;

public:
  bool wants_inventory = false;
  bool wants_quit = false;
  bool wants_pause = false;

  GameWindow(World &w, int &px, int &py, int &f, int *inv, int &sel,
             CheatState &cs)
      : world(w), player_x(px), player_y(py), facing(f), inventory(inv),
        selected_block(sel), cheats(cs), spawn_x(px), spawn_y(py) {}

  bool handle_input(const InputState &input) override {
    if (input.quit) {
      wants_quit = true;
      return true;
    }

    if (input.open_inventory) {
      wants_inventory = true;
      return false;
    }

    if (input.open_pause) {
      wants_pause = true;
      return false;
    }

    int nw_x = player_x;
    if (input.move_left) {
      nw_x--;
      facing = -1;
    }
    if (input.move_right) {
      nw_x++;
      facing = 1;
    }

    if (input.jump) {
      if (cheats.spectator_mode) {
        --player_y;
      } else {
        bool on_ground =
            world.get_block(player_x, player_y + 1) != BlockType::AIR;
        bool above_clear =
            world.get_block(player_x, player_y - 1) == BlockType::AIR;
        if (on_ground && above_clear) {
          player_y--;
          fall_timer = 0;
          fall_distance = 0;
        }
      }
    }

    if (input.mine_left) {
      BlockType target = world.get_block(player_x - 1, player_y);
      if (target != BlockType::AIR && target != BlockType::BEDROCK) {
        world.set_block(player_x - 1, player_y, BlockType::AIR);
        inventory[static_cast<int>(target)]++;
      }
    }
    if (input.mine_right) {
      BlockType target = world.get_block(player_x + 1, player_y);
      if (target != BlockType::AIR && target != BlockType::BEDROCK) {
        world.set_block(player_x + 1, player_y, BlockType::AIR);
        inventory[static_cast<int>(target)]++;
      }
    }
    if (input.mine_up) {
      BlockType target = world.get_block(player_x, player_y - 1);
      if (target != BlockType::AIR && target != BlockType::BEDROCK) {
        world.set_block(player_x, player_y - 1, BlockType::AIR);
        inventory[static_cast<int>(target)]++;
        player_y--;
        // fall_timer = 0;
      }
    }
    if (input.mine_down) {
      BlockType target = world.get_block(player_x, player_y + 1);
      if (target != BlockType::AIR && target != BlockType::BEDROCK) {
        world.set_block(player_x, player_y + 1, BlockType::AIR);
        inventory[static_cast<int>(target)]++;
      }
    }

    if (input.place_block) {
      int place_x, place_y;
      bool on_ground =
          world.get_block(player_x, player_y + 1) != BlockType::AIR;
      if (on_ground) {
        place_x = player_x + facing;
        place_y = player_y;
      } else {
        place_x = player_x;
        place_y = player_y + 1;
      }
      if (world.get_block(place_x, place_y) == BlockType::AIR) {
        BlockType block_toplace = static_cast<BlockType>(selected_block);
        if (inventory[selected_block] > 0) {
          world.set_block(place_x, place_y, block_toplace);
          inventory[selected_block]--;
        }
      }
    }

    if (input.select_block != 0) {
      selected_block = input.select_block;
    }

    if (cheats.spectator_mode or
        world.get_block(nw_x, player_y) == BlockType::AIR) {
      player_x = nw_x;
    }

    if (cheats.speed_boost) {
      int nw_x2 = player_x;
      if (input.move_left)
        nw_x2--;
      if (input.move_right)
        nw_x2++;
      if (cheats.spectator_mode or
          world.get_block(nw_x2, player_y) == BlockType::AIR) {
        player_x = nw_x2;
      }
    }

    if (!cheats.spectator_mode) {
      fall_timer++;
      if (fall_timer >= GRAVITY_INTERVAL) {
        fall_timer = 0;
        if (world.get_block(player_x, player_y + 1) == BlockType::AIR) {
          player_y++;
          fall_distance++;
        } else {
          if (fall_distance > 3 && !cheats.god_mode) {
            int damage = (fall_distance - 3) * 10;
            hp -= damage;
            if (hp < 0)
              hp = 0;
          }
          fall_distance = 0;
        }
      }
    }

    if (input.move_down) {
      if (cheats.spectator_mode) {
        ++player_y;
      }
    }

    ++spawn_timer;
    if (spawn_timer >= SPAWN_INTERVAL) {
      spawn_timer = 0;

      uint32_t r = fast_rand();
      int offset = (r & 31) + 15;
      if (r & 32) {
        offset = -offset;
      }

      int spawn_x = player_x + offset;
      int spawn_y = player_y;

      while (spawn_y < CHUNK_SIZE - 1 and
             world.get_block(spawn_x, spawn_y) == BlockType::AIR) {
        ++spawn_y;
      }
      --spawn_y;

      if (spawn_y > 0) {
        mobs.add(spawn_x, spawn_y, 20, MobType::ZOMBIE, AIState::CHASING);
      }
    }

    ++mob_move_timer;
    if (mob_move_timer >= MOB_MOVE_INTERVAL) {
      mob_move_timer = 0;

      Coord player_pos = {player_x, player_y};

      for (size_t i = 0; i < mobs.count(); ++i) {
        Coord mob_pos = mobs.get_pos(i);

        int dx = mob_pos.x - player_x;
        int dy = mob_pos.y - player_y;

        if ((dx * dx + dy * dy) > 3600) {
          continue;
        }

        // 1. STRICT GRAVITY: If in mid-air, you MUST fall down. No AI allowed.
        if (world.get_block(mob_pos.x, mob_pos.y + 1) == BlockType::AIR) {
          mobs.set_pos(i, {mob_pos.x, mob_pos.y + 1});
        } 
        // 2. ON GROUND: Use AI to pathfind to player
        else {
          std::vector<Coord> path = bfs_findpath(mob_pos, player_pos, world, 150);
          if (path.size() >= 2) {
            mobs.set_pos(i, path[1]);
          }
        }
      }
    }

    if (damage_cooldown > 0) {
      damage_cooldown--;
    }

    if (!cheats.god_mode && damage_cooldown == 0) {
      for (size_t i = 0; i < mobs.count(); ++i) {
        int dx = mobs.x[i] - player_x;
        int dy = mobs.y[i] - player_y;
        if (dx * dx + dy * dy <= 4) {
          hp -= 10;
          if (hp < 0)
            hp = 0;
          damage_cooldown = 40;

          int knockback_x = (dx <= 0) ? 1 : -1;
          for (int k = 0; k < 2; k++) {
            int nx = player_x + knockback_x;
            if (cheats.spectator_mode ||
                world.get_block(nx, player_y) == BlockType::AIR) {
              player_x = nx;
            }
          }

          break;
        }
      }
    }

    if (hp <= 0 && !is_dead) {
      is_dead = true;
    }

    if (is_dead) {
      if (input.confirm_inventory) {
        hp = max_hp;
        player_x = spawn_x;
        player_y = spawn_y;
        fall_distance = 0;
        damage_cooldown = 60;
        is_dead = false;
      }
      return false;
    }

    return false;
  }

  void render(ScreenBuffer &screen) override {
    screen.clear();

    if (is_dead) {
      screen.draw_text(30, 10, "YOU DIED!", Color::BRIGHT_RED);
      screen.draw_text(25, 13, "[Press Enter to Respawn]", Color::GRAY);
      return;
    }

    int cam_x = player_x - SCREEN_WIDTH / 2;
    int cam_y = player_y - SCREEN_HEIGHT / 2;

    for (int sy = 0; sy < SCREEN_HEIGHT; ++sy) {
      for (int sx = 0; sx < SCREEN_WIDTH; ++sx) {
        int wx = cam_x + sx;
        int wy = cam_y + sy;

        BlockType block;
        if (wy < 0) {
          block = BlockType::AIR;
        } else if (wy >= CHUNK_SIZE) {
          block = BlockType::BEDROCK;
        } else {
          block = world.get_block(wx, wy);
        }
        bool is_ore = (block == BlockType::DIAMOND or
                       block == BlockType::GOLD or block == BlockType::IRON);

        if (is_ore) {
          bool exposed = world.get_block(wx, wy + 1) == BlockType::AIR or
                         world.get_block(wx + 1, wy) == BlockType::AIR or
                         world.get_block(wx - 1, wy) == BlockType::AIR or
                         world.get_block(wx, wy - 1) == BlockType::AIR;

          if (exposed) {
            screen.set_pixel(sx, sy, block_to_pixel(block));
          } else {
            screen.set_pixel(sx, sy, block_to_pixel(BlockType::STONE));
          }
        } else {
          screen.set_pixel(sx, sy, block_to_pixel(block));
        }
      }
    }

    screen.set_pixel(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2,
                     {'$', Color::BRIGHT_CYAN});

    for (size_t i = 0; i < mobs.count(); ++i) {
      int sx = mobs.x[i] - cam_x;
      int sy = mobs.y[i] - cam_y;
      if (sx >= 0 && sx < SCREEN_WIDTH && sy >= 0 && sy < SCREEN_HEIGHT) {
        screen.set_pixel(sx, sy, mob_to_pixel(mobs.type[i]));
      }
    }

    std::string hud = "Pos: (" + std::to_string(player_x) + "," +
                      std::to_string(player_y) +
                      ")  [WASD+W]Move  [Arrows]Mine [1-6]Select [E]Inventory "
                      "[Space]Place  [Q]Quit";

    std::string inv_hud = "Inv:";
    inv_hud += (selected_block == 1 ? " >" : "  ");
    inv_hud += "Grass:" + std::to_string(inventory[1]);
    inv_hud += (selected_block == 2 ? " >" : "  ");
    inv_hud += "Dirt:" + std::to_string(inventory[2]);
    inv_hud += (selected_block == 3 ? " >" : "  ");
    inv_hud += "Stone:" + std::to_string(inventory[3]);
    inv_hud += (selected_block == 4 ? " >" : "  ");
    inv_hud += "Iron:" + std::to_string(inventory[4]);
    inv_hud += (selected_block == 5 ? " >" : "  ");
    inv_hud += "Gold:" + std::to_string(inventory[5]);
    inv_hud += (selected_block == 6 ? " >" : "  ");
    inv_hud += "Dia:" + std::to_string(inventory[6]);

    screen.draw_text(0, 0, hud, Color::MAGENTA);
    screen.draw_text(0, 1, inv_hud, Color::YELLOW);

    std::string hp_bar = "HP: [";
    int filled = (hp * 20) / max_hp;
    for (int i = 0; i < 20; i++) {
      hp_bar += (i < filled) ? '#' : '.';
    }
    hp_bar += "] " + std::to_string(hp) + "/" + std::to_string(max_hp);
    if (cheats.god_mode)
      hp_bar += " [GOD]";
    Color hp_color = (hp > 50)   ? Color::BRIGHT_GREEN
                     : (hp > 20) ? Color::YELLOW
                                 : Color::BRIGHT_RED;
    screen.draw_text(0, 2, hp_bar, hp_color);
  }

  bool is_opaque() const override { return true; }
};
