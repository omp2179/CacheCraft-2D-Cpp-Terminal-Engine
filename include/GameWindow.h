#pragma once
#include "Pixel.h"
#include "Terrain.h"
#include "Window.h"
#include "World.h"
#include <string>

class GameWindow : public Window {
private:
  World &world;
  int &player_x;
  int &player_y;
  int &facing;
  int *inventory;
  int &selected_block;
  int fall_timer = 0;
  const int GRAVITY_INTERVAL = 5;

public:
  bool wants_inventory = false;
  bool wants_quit = false;

  GameWindow(World &w, int &px, int &py, int &f, int *inv, int &sel)
      : world(w), player_x(px), player_y(py), facing(f), inventory(inv),
        selected_block(sel) {}

  bool handle_input(const InputState &input) override {
    if (input.quit) {
      wants_quit = true;
      return true;
    }

    if (input.open_inventory) {
      wants_inventory = true;
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
      bool on_ground =
          world.get_block(player_x, player_y + 1) != BlockType::AIR;
      bool above_clear =
          world.get_block(player_x, player_y - 1) == BlockType::AIR;
      if (on_ground && above_clear) {
        player_y--;
        fall_timer = 0;
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

    if (world.get_block(nw_x, player_y) == BlockType::AIR) {
      player_x = nw_x;
    }

    fall_timer++;
    if (fall_timer >= GRAVITY_INTERVAL) {
      fall_timer = 0;
      if (world.get_block(player_x, player_y + 1) == BlockType::AIR) {
        player_y++;
      }
    }

    return false;
  }

  void render(ScreenBuffer &screen) override {
    screen.clear();

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
  }

  bool is_opaque() const override { return true; }
};
