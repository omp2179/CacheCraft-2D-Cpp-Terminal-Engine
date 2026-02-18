#pragma once
#include <conio.h>

// All keys pressed in a single frame — allows simultaneous actions
struct InputState {
  bool move_left = false;
  bool move_right = false;
  bool jump = false;
  bool mine_left = false;
  bool mine_right = false;
  bool mine_up = false;
  bool mine_down = false;
  bool place_block = false;
  bool quit = false;
};

inline InputState get_input() {
  InputState state;

  while (_kbhit()) {
    int key = _getch();

    if (key == 0 || key == 224) {
      int arrow = _getch();

      switch (arrow) {
      case 75:
        state.mine_left = true;
        break;
      case 77:
        state.mine_right = true;
        break;
      case 72:
        state.mine_up = true;
        break;
      case 80:
        state.mine_down = true;
        break;
      }
    } else {
      switch (key) {
      case 'a':
      case 'A':
        state.move_left = true;
        break;
      case 'd':
      case 'D':
        state.move_right = true;
        break;
      case 'w':
      case 'W':
        state.jump = true;
        break;
      case ' ':
        state.place_block = true;
        break;
      case 'q':
      case 'Q':
        state.quit = true;
        break;
      }
    }
  }

  return state;
}