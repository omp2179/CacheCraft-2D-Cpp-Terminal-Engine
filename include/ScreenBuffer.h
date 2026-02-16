#pragma once
#include "Pixel.h"
#include <array>
#include <iostream>
#include <string>

constexpr int SCREEN_WIDTH = 80;
constexpr int SCREEN_HEIGHT = 24;

class ScreenBuffer {
private:
  std::array<std::array<Pixel, SCREEN_WIDTH>, SCREEN_HEIGHT> buffer;

public:
  void clear() {
    Pixel empty{' ', Color::WHITE};

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
      for (int x = 0; x < SCREEN_WIDTH; ++x) {
        buffer[y][x] = empty;
      }
    }
  }

  void set_pixel(int x, int y, Pixel p) {
    if (x < 0 or x >= SCREEN_WIDTH or y < 0 or y >= SCREEN_HEIGHT) {
      return;
    }

    buffer[y][x] = p;
  }

  Pixel get_pixel(int x, int y) const {
    if (x < 0 or x >= SCREEN_WIDTH or y < 0 or y >= SCREEN_HEIGHT) {
      return {' ', Color::WHITE};
    }
    return buffer[y][x];
  }

  void render() const {
    std::string frame;
    frame.reserve(SCREEN_WIDTH * SCREEN_HEIGHT * 12);

    frame += "\033[H";

    Color last_color = Color::WHITE;

    for (int y = 0; y < SCREEN_HEIGHT; ++y) {
      for (int x = 0; x < SCREEN_WIDTH; ++x) {
        const Pixel &p = buffer[y][x];

        if (p.color != last_color) {
          frame += "\033[";

          frame += std::to_string(static_cast<int>(p.color));

          frame += "m";

          last_color = p.color;
        }
        frame += p.ch;
      }
      frame += "\n";
    }
    frame += "\033[m";
    std::cout << frame;
  }

  void draw_text(int x, int y, const std::string &text,
                 Color color = Color::WHITE) {
    for (int i = 0; i < static_cast<int>(text.size()); ++i) {
      set_pixel(x + i, y, {text[i], color});
    }
  }
};