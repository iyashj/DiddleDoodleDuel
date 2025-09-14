#ifndef DIDDLEDOODLEDUEL_VELOCITY_H
#define DIDDLEDOODLEDUEL_VELOCITY_H
#include <raylib.h>

struct Velocity {
    Vector2 velocity{0.0F, 0.0F};
    float rotation {0.0F};

    float speed {5000.0F};
    float rotationSpeed {180.0F};
};

#endif // DIDDLEDOODLEDUEL_VELOCITY_H
