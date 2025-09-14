#ifndef DIDDLEDOODLEDUEL_COLLISION_STATE_H
#define DIDDLEDOODLEDUEL_COLLISION_STATE_H
#include <raylib.h>

struct  CollisionState{
    bool isInCollision {false};
    float bounceTimer {0.0F};
    Vector2 bounceVelocity {0.0F, 0.0F};
};

#endif // DIDDLEDOODLEDUEL_COLLISION_STATE_H
