#pragma once
#include <conio.h>

enum class Action { NONE, MOVE_LEFT, MOVE_RIGHT, MOVE_UP, MOVE_DOWN, EXIT };

inline Action get_input() {
  Action last_action = Action::NONE;

  // Drain the ENTIRE buffer — use only the last key pressed
  // This prevents lag when holding a key (buffer fills up)
  while (_kbhit()) {
    char key = _getch();

    switch (key) {
    case 'a':
    case 'A':
      last_action = Action::MOVE_LEFT;
      break;
    case 'd':
    case 'D':
      last_action = Action::MOVE_RIGHT;
      break;
    case 'w':
    case 'W':
      last_action = Action::MOVE_UP;
      break;
    case 's':
    case 'S':
      last_action = Action::MOVE_DOWN;
      break;
    case 'q':
    case 'Q':
      last_action = Action::EXIT;
      break;
    default:
      break;
    }
  }

  return last_action;
}