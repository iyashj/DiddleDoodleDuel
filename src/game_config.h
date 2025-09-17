#ifndef DIDDLEDOODLEDUEL_GAME_CONFIG_H
#define DIDDLEDOODLEDUEL_GAME_CONFIG_H

struct GameConfig {
    float brushSize {25.0F};
    float brushMovementSpeed {5000.0F};
    float collisionForceMultiplier {2.0F};
    float bounceDuration {0.6F};
    float controlDuringBounceFactor {0.3F};
    float debugCollisionRadius {25.0F};
    
    // Collision physics
    float restitution {0.8F};              // Bounce factor (0-1)
    float collisionDamping {0.7F};         // Velocity reduction on collision
    float separationForce {100.0F};        // Force to separate overlapping objects
};

#endif // DIDDLEDOODLEDUEL_GAME_CONFIG_H
