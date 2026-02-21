#pragma once
#include "Coord.h"
#include "Pixel.h"

enum class MobType : uint8_t{
    ZOMBIE=0,
    COUNT
};

enum class AIState : uint8_t{
    CHASING=0,
    IDLE
};

struct Mob{
    int x,y;
    int hp;
    MobType type;
    AIState state;
};

inline Pixel mob_to_pixel(MobType type){
    switch (type) {
        case MobType::ZOMBIE:
            return {'Z',Color::GREEN};
        default:
            return {'?',Color::WHITE};
    }
}