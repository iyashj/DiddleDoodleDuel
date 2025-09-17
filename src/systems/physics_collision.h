#ifndef DIDDLEDOODLEDUEL_PHYSICS_COLLISION_H
#define DIDDLEDOODLEDUEL_PHYSICS_COLLISION_H
#include "components/collision_state.h"
#include "components/position.h"
#include "components/renderable.h"
#include "components/velocity.h"
#include <cmath>
#include <entt/entity/registry.hpp>
#include <raymath.h>

struct PhysicsCollisionSystem {
    explicit PhysicsCollisionSystem(entt::registry& registry, GameConfig& gameConfig) 
        : registry(registry), gameConfig(gameConfig) {
    }

    void update(const float& deltaTime) const {
        // Check all entity pairs for collisions
        for (const auto view = registry.view<Position, CollisionState, Renderable, Velocity>();
             const auto entityA : view) {
            for (const auto entityB : view) {
                if (entityA == entityB) { continue; }

                auto& positionA = registry.get<Position>(entityA);
                auto& positionB = registry.get<Position>(entityB);

                const auto& renderableA = registry.get<Renderable>(entityA);
                const auto& renderableB = registry.get<Renderable>(entityB);

                auto& velocityA = registry.get<Velocity>(entityA);
                auto& velocityB = registry.get<Velocity>(entityB);

                auto& stateA = registry.get<CollisionState>(entityA);
                auto& stateB = registry.get<CollisionState>(entityB);

                // Don't process collision if either object is already in collision cooldown
                if ((stateA.isInCollision && stateA.bounceTimer > 0.0f) || 
                    (stateB.isInCollision && stateB.bounceTimer > 0.0f)) {
                    continue;
                }

                if (auto [collision, collisionData] = checkCollision(positionA.position, positionB.position, renderableA, renderableB); 
                    collision) {
                    
                    // Separate overlapping objects
                    separateObjects(positionA, positionB, collisionData, renderableA, renderableB);
                    
                    // Apply realistic collision response
                    applyPhysicsCollision(velocityA, velocityB, collisionData, stateA, stateB);
                }
            }
        }

        // Update collision timers
        updateCollisionStates(deltaTime);
    }

private:
    entt::registry& registry;
    const GameConfig& gameConfig;

    struct CollisionData {
        Vector2 normal;      // Collision normal (from A to B)
        float penetration;   // How much objects overlap
        Vector2 contactPoint; // Point of contact
    };

    std::tuple<bool, CollisionData> checkCollision(const Vector2& posA, const Vector2& posB,
                                                   const Renderable& renderableA, const Renderable& renderableB) const {
        Vector2 delta = {posA.x - posB.x, posA.y - posB.y};
        float distance = Vector2Length(delta);
        float radiusSum = renderableA.radius + renderableB.radius;

        if (distance < radiusSum && distance > 0.0f) {
            CollisionData data;
            data.normal = Vector2Scale(delta, 1.0f / distance); // Normalize
            data.penetration = radiusSum - distance;
            data.contactPoint = Vector2Add(posB, Vector2Scale(data.normal, renderableB.radius));
            return {true, data};
        }

        return {false, {}};
    }

    void separateObjects(Position& posA, Position& posB, const CollisionData& data,
                        const Renderable& renderableA, const Renderable& renderableB) const {
        // Calculate separation based on mass ratio (heavier objects move less)
        float totalRadius = renderableA.radius + renderableB.radius;
        float separationA = data.penetration * (renderableB.radius / totalRadius) * 0.5f;
        float separationB = data.penetration * (renderableA.radius / totalRadius) * 0.5f;

        Vector2 separationVecA = Vector2Scale(data.normal, separationA);
        Vector2 separationVecB = Vector2Scale(data.normal, -separationB);

        posA.position.x += separationVecA.x;
        posA.position.y += separationVecA.y;
        posB.position.x += separationVecB.x;
        posB.position.y += separationVecB.y;
    }

    void applyPhysicsCollision(Velocity& velA, Velocity& velB, const CollisionData& data,
        CollisionState& stateA, CollisionState& stateB) const {
        
        // Calculate collision direction (from A to B)
        Vector2 collisionNormal = data.normal;
        
        // Calculate much stronger bounce force for dramatic impact
        float bounceForce = gameConfig.collisionForceMultiplier * gameConfig.brushMovementSpeed;
        
        // Apply opposite forces to bounce objects apart
        Vector2 bounceVelocityA = Vector2Scale(collisionNormal, bounceForce);
        Vector2 bounceVelocityB = Vector2Scale(collisionNormal, -bounceForce);

        // Set collision states - during this time objects can't collide or paint
        stateA.isInCollision = true;
        stateA.bounceTimer = gameConfig.bounceDuration;
        stateA.bounceVelocity = bounceVelocityA;

        stateB.isInCollision = true;
        stateB.bounceTimer = gameConfig.bounceDuration;
        stateB.bounceVelocity = bounceVelocityB;

        // Update rotation to show impact direction for visual feedback
        velA.rotation = atan2f(bounceVelocityA.y, bounceVelocityA.x) * RAD2DEG;
        velB.rotation = atan2f(bounceVelocityB.y, bounceVelocityB.x) * RAD2DEG;
    }

    void updateCollisionStates(const float deltaTime) const {
        for (const auto view = registry.view<CollisionState>(); const auto& entity : view) {
            if (auto& [isInCollision, bounceTimer, bounceVelocity] =
                    registry.get<CollisionState>(entity);
                bounceTimer > 0) {
                bounceTimer -= deltaTime;
                if (bounceTimer <= 0) {
                    isInCollision = false;
                    bounceVelocity = {.x=0.0F, .y=0.0F};
                }
            }
        }
    }
};

#endif // DIDDLEDOODLEDUEL_PHYSICS_COLLISION_H