#ifndef DIDDLEDOODLEDUEL_MOVEMENT_STRUCTS_H
#define DIDDLEDOODLEDUEL_MOVEMENT_STRUCTS_H
#include <raylib.h>

struct Position {
    Vector2 pos{0.0F, 0.0F};
};

struct Velocity {
    Vector2 vel{0.0F, 0.0F};
    float rotationSpeed {180.0F};
    float speed {5000.0F};
    float rotation {0.0F};
};

#endif // DIDDLEDOODLEDUEL_MOVEMENT_STRUCTS_H
