#pragma once
#include "Window.h"

class InventoryWindow : public Window {
private:
  int cursor;
  int *inventory;
  int &selected_block;

public:
  InventoryWindow(int *inv, int &sel) : inventory(inv), selected_block(sel) {};

  bool handle_input(const InputState &input) override {
    if (input.open_inventory) {
      return true;
    }

    if (input.mine_up) {
      --cursor;
    }
    if (input.mine_down) {
      ++cursor;
    }
    if (cursor < 0) {
      cursor = 0;
    }
    if (cursor > 5) {
      cursor = 5;
    }

    if (input.confirm_inventory) {
      selected_block = cursor + 1;
      return true;
    }
    return false;
  }

  void render(ScreenBuffer &screen) override {
    screen.clear();
    screen.draw_text(25, 3, "===INVENTORY===", Color::BRIGHT_BLUE);

    std::string names[] = {"Grass", "Dirt", "Stone", "Iron", "Gold", "Diamond"};

    for (int i = 0; i < 6; ++i) {
      std::string prefix = (cursor == i) ? " >> " : "  ";
      std::string line = prefix + "[" + std::to_string(i + 1) + "]" + names[i] +
                         "..... " + std::to_string(inventory[i + 1]);
      Color c = (cursor == i) ? Color::BRIGHT_GREEN : Color::BRIGHT_WHITE;
      screen.draw_text(20, 6 + i, line, c);
    }

    screen.draw_text(20, 14, "[Up/Down] Navigate [Enter] Select [E] Close",
                     Color::GRAY);
  }
};