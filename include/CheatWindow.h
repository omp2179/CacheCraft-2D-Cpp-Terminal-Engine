#pragma once
#include "CheatState.h"
#include "Window.h"
#include <string>

class CheatWindow : public Window {
private:
  int cursor = 0;
  static const int NUM_OPTIONS = 4;
  CheatState &cheats;
  int *inventory;

public:
  CheatWindow(CheatState &cs, int *inv) : cheats(cs), inventory(inv) {}

  bool handle_input(const InputState &input) override {
    if (input.open_pause) {
      return true;
    }

    if (input.mine_up) {
      --cursor;
      if (cursor < 0)
        cursor = NUM_OPTIONS - 1;
    }

    if (input.mine_down) {
      ++cursor;
      if (cursor >= NUM_OPTIONS)
        cursor = 0;
    }

    if (input.confirm_inventory) {
      switch (cursor) {
      case 0:
        cheats.spectator_mode = !cheats.spectator_mode;
        break;
      case 1:
        cheats.speed_boost = !cheats.speed_boost;
        break;
      case 2:
        cheats.god_mode = !cheats.god_mode;
        break;
      case 3:
        inventory[6] += 64;
        break;
      }
    }
    return false;
  }

  void render(ScreenBuffer &screen) override {
    screen.clear();
    screen.draw_text(26, 3, "=== CHEAT MENU ===", Color::BRIGHT_RED);

    std::string labels[] = {"Spectator Mode", "Speed Boost", "God Mode",
                            "Give 64 Diamonds"};
    bool states[] = {cheats.spectator_mode, cheats.speed_boost, cheats.god_mode,
                     false};

    for (int i = 0; i < NUM_OPTIONS; ++i) {
      std::string prefix = (cursor == i) ? " >> " : "    ";
      std::string line = prefix + labels[i];

      if (i < 3) {
        line += states[i] ? "  [ON]" : "  [OFF]";
      }

      Color c;
      if (cursor != i) {
        c = Color::BRIGHT_WHITE;
      } else if (i < 3 && states[i]) {
        c = Color::BRIGHT_GREEN;
      } else {
        c = Color::BRIGHT_YELLOW;
      }

      screen.draw_text(24, 6 + i * 2, line, c);
    }

    screen.draw_text(20, 16, "[Up/Down] Navigate  [Enter] Toggle  [P] Back",
                     Color::GRAY);
  }
};
