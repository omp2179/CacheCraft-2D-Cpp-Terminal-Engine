#pragma once
#include <conio.h>

struct InputState {
  bool move_left = false;
  bool move_right = false;
  bool jump = false;
  bool move_down = false;
  bool mine_left = false;
  bool mine_right = false;
  bool mine_up = false;
  bool mine_down = false;
  bool place_block = false;
  bool quit = false;
  int select_block = 0;
  bool open_inventory = false;
  bool confirm_inventory = false;
  bool open_pause=false;
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
      case 13:
        state.confirm_inventory = true;
        break;
      case 'a':
      case 'A':
        state.move_left = true;
        break;
      case 'd':
      case 'D':
        state.move_right = true;
        break;
      case 's':
      case 'S':
        state.move_down = true;
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
      case 'e':
      case 'E':
        state.open_inventory = true;
        break;
      case 'p':
      case 'P':
        state.open_pause = true;
        break;
      case '1':
        state.select_block = 1;
        break;
      case '2':
        state.select_block = 2;
        break;
      case '3':
        state.select_block = 3;
        break;
      case '4':
        state.select_block = 4;
        break;
      case '5':
        state.select_block = 5;
        break;
      case '6':
        state.select_block = 6;
        break;
      }
    }
  }

  return state;
}