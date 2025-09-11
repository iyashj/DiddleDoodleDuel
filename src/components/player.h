#ifndef DIDDLEDOODLEDUEL_PLAYER_H
#define DIDDLEDOODLEDUEL_PLAYER_H
#include "drawing_structs.h"

struct Player : Renderable {
    float speed{30000.0F};
    float rotation{18000.0F};
};

#endif // DIDDLEDOODLEDUEL_PLAYER_H
