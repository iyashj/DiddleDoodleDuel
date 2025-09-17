#ifndef DIDDLEDOODLEDUEL_COLLISION_H
#define DIDDLEDOODLEDUEL_COLLISION_H
#include "components/renderable.h"
#include "components/collision_state.h"
#include "components/position.h"
#include "components/velocity.h"
#include <entt/entity/registry.hpp>
#include <imgui.h>
#include <raymath.h>

struct CollisionSystem {
    explicit CollisionSystem(entt::registry& registry, GameConfig& gameConfig) : registry(registry), gameConfig(gameConfig) {
    }

    void update(const float& deltaTime) const {

        for (const auto view = registry.view<Position, CollisionState, Renderable, Velocity>();
             const auto entityA : view) {
            for (const auto entityB : view) {
                if (entityA == entityB) { continue; }

                auto& [positionA] = registry.get<Position>(entityA);
                auto& [positionB] = registry.get<Position>(entityB);

                const auto& renderableA = registry.get<Renderable>(entityA);
                const auto& renderableB = registry.get<Renderable>(entityB);

                auto& velocityA = registry.get<Velocity>(entityA);
                auto& velocityB = registry.get<Velocity>(entityB);

                if (auto [positionAAndBCanCollide, resultantVector] =
                    canCollide(positionA, positionB,renderableA, renderableB); positionAAndBCanCollide) {

                    applyCollision(registry.get<CollisionState>(entityA), resultantVector, velocityA, gameConfig.collisionForceMultiplier);
                    applyCollision(registry.get<CollisionState>(entityB), {-resultantVector.x, -resultantVector.y}, velocityB, gameConfig.collisionForceMultiplier);

                    if (const float overlapDistance =
                            (renderableA.radius + renderableB.radius) -
                            sqrt(powf(positionA.x - positionB.x, 2) + powf(positionA.y - positionB.y, 2));
                        overlapDistance > 0) {
                        const Vector2 separation = {resultantVector.x * overlapDistance * 0.5F, resultantVector.y * overlapDistance *0.5F};
                        positionA = {.x=positionA.x + separation.x, .y=positionA.y + separation.y};
                        positionB = {.x=positionB.x - separation.x, .y=positionB.y - separation.y};
                    }
                }
            }
        }

        for (const auto view = registry.view<CollisionState>(); const auto& entity : view) {
            if (auto& [isInCollision, bounceTimer, bounceVelocity] =
                    registry.get<CollisionState>(entity);
                bounceTimer > 0) {
                bounceTimer -= deltaTime;
                if (bounceTimer <= 0) {
                    isInCollision = false;
                    bounceVelocity = {0.0F, 0.0F};
                }
            }
        }
    }

    void applyCollision(CollisionState& collisionState, const Vector2& resultantVector, Velocity& velocity, const float& forceMultiplier) const {
        auto& [isInCollisionA, bounceTimerA, bounceVelocityA] = collisionState;
        isInCollisionA = true;

        bounceTimerA = gameConfig.bounceDuration;
        const float impulse = gameConfig.brushMovementSpeed * forceMultiplier;
        bounceVelocityA = Vector2Scale(Vector2Normalize(resultantVector), impulse);
        const auto bounceAngle = atan2f(resultantVector.y, resultantVector.x) * RAD2DEG;
        velocity.rotation = bounceAngle;
    }

    [[nodiscard]] std::tuple<bool, Vector2> canCollide(const Vector2& position1,
                                                       const Vector2& position2,
                                         const Renderable& renderable1,
                                         const Renderable& renderable2) const {
        float dx = position1.x - position2.x;
        float dy = position1.y - position2.y;
        float distance = sqrt(dx*dx + dy*dy);

        return std::make_tuple(
            distance < renderable1.radius + renderable2.radius,
            Vector2 {dx/distance, dy/distance});
    }

private:
    entt::registry& registry;
    const GameConfig& gameConfig;
};

#endif // DIDDLEDOODLEDUEL_COLLISION_H
