#pragma once
#include "Coord.h"
#include "Mob.h"
#include <cstddef>
#include <vector>

struct MobStorage {
  std::vector<int> x;
  std::vector<int> y;
  std::vector<int> hp;
  std::vector<MobType> type;
  std::vector<AIState> state;

  void add(int mx, int my, int mhp, MobType mtype, AIState mstate) {
    x.push_back(mx);
    y.push_back(my);
    hp.push_back(mhp);
    type.push_back(mtype);
    state.push_back(mstate);
  }

  void remove(size_t index) {
    if (index >= x.size())
      return;
    size_t last = x.size() - 1;
    if (index != last) {
      x[index] = x[last];
      y[index] = y[last];
      hp[index] = hp[last];
      type[index] = type[last];
      state[index] = state[last];
    }
    x.pop_back();
    y.pop_back();
    hp.pop_back();
    type.pop_back();
    state.pop_back();
  }

  size_t count() const { return x.size(); }

  Coord get_pos(size_t idx) { return {x[idx], y[idx]}; }

  void set_pos(size_t i, Coord pos) {
    x[i] = pos.x;
    y[i] = pos.y;
  }

  void set_hp(size_t i, int new_hp){
    hp[i] = new_hp;
  }

  void set_state(size_t i, AIState new_state){
    state[i] = new_state;
  }
};