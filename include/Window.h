#pragma once
#include "Input.h"
#include "ScreenBuffer.h"

class Window {
public:
  virtual ~Window() = default;

  virtual bool handle_input(const InputState &input) = 0;
  virtual void render(ScreenBuffer &screen) = 0;

  virtual bool is_opaque() const { return true; }
};