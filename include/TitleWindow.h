#pragma once
#include "Input.h"
#include "Pixel.h"
#include "ScreenBuffer.h"
#include "Window.h"
#include <string>

class TitleWindow : public Window {
private:
  int cursor = 0;
  static const int NUM_OPTIONS = 3;

public:
  bool wants_new = false;
  bool wants_load = false;
  bool wants_quit = false;

  bool handle_input(const InputState &input) override {
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
        wants_new = true;
        return true;
      case 1:
        wants_load = true;
        return true;
      case 2:
        wants_quit = true;
        return true;
      }
    }

    if (input.quit) {
      wants_quit = true;
      return true;
    }

    return false;
  }

  void render(ScreenBuffer &screen) override {
    screen.clear();

    int cx = SCREEN_WIDTH / 2;
    int ty = 3;

    screen.draw_text(cx - 25, ty + 0,
                     " __  __ ___ _   _ ___  ___ ___    _   ___ _____",
                     Color::BRIGHT_GREEN);
    screen.draw_text(cx - 25, ty + 1,
                     "|  \\/  |_ _| \\ | / _ \\/ __| _ \\  /_\\ | __|_   _|",
                     Color::BRIGHT_GREEN);
    screen.draw_text(cx - 25, ty + 2,
                     "| |\\/| || ||  \\| \\___/ (__)|   / / _ \\| _|  | |",
                     Color::BRIGHT_GREEN);
    screen.draw_text(cx - 25, ty + 3,
                     "|_|  |_|___|_|\\_|___/\\___|_|_\\/_/ \\_\\_|   |_|",
                     Color::BRIGHT_GREEN);

    screen.draw_text(cx - 10, ty + 5, "2 D   E D I T I O N",
                     Color::BRIGHT_CYAN);

    screen.draw_text(cx - 15, ty + 7,
                     "################################",
                     Color::RED);

    std::string options[] = {"New Game", "Load Game", "Quit"};

    for (int i = 0; i < NUM_OPTIONS; ++i) {
      std::string prefix = (cursor == i) ? "  >> " : "     ";
      Color c = (cursor == i) ? Color::BRIGHT_YELLOW : Color::BRIGHT_WHITE;
      screen.draw_text(cx - 8, ty + 10 + i * 2, prefix + options[i], c);
    }

    screen.draw_text(cx - 18, ty + 18,
                     "[Up/Down] Navigate    [Enter] Select    [Q] Quit",
                     Color::GRAY);

    screen.draw_text(cx - 12, ty + 20, "Built with C++17 | No Dependencies",
                     Color::MAGENTA);
  }
};
