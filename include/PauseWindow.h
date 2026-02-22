#pragma once
#include "Input.h"
#include "Pixel.h"
#include "ScreenBuffer.h"
#include "Window.h"
#include <string>

class PauseWindow : public Window {
private:
  int cursor = 0;
  static const int NUM_OPTIONS = 3;

public:
  bool wants_cheat = false;
  bool wants_quit = false;

  bool handle_input(const InputState &input) override {
    if (input.open_pause) {
      return true;
    }

    if (input.mine_up) {
      --cursor;
      if (cursor < 0) {
        cursor = NUM_OPTIONS - 1;
      }
    }

    if (input.mine_down) {
      ++cursor;
      if (cursor >= NUM_OPTIONS) {
        cursor = 0;
      }
    }

    if (input.confirm_inventory) {
      switch (cursor) {
      case 0:
        return true;
      case 1:
        wants_cheat = true;
        return false;
      case 2:
        wants_quit = true;
        return true;
      }
      // return false;
    }
    return false;
  }

  void render(ScreenBuffer &screen) override {
    screen.clear();
    screen.draw_text(28, 4, "=== PAUSED ===", Color::BRIGHT_BLUE);

    std::string options[] = {"Resume", "Cheats", "Quit"};

    for (int i = 0; i < NUM_OPTIONS; ++i) {
      std::string prefix = (cursor == i) ? " >> " : "    ";
      Color c = (cursor == i) ? Color::BRIGHT_GREEN : Color::BRIGHT_WHITE;
      screen.draw_text(28, 8 + i * 2, prefix + options[i], c);
    }

    screen.draw_text(22, 16, "[Up/Down] Navigate  [Enter] Select  [P] Resume",
                     Color::GRAY);
  }
};