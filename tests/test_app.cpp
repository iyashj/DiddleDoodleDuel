#include "../diddle_doodle_duel.h"
#include "../src/components/movement_structs.h"
#include "../src/components/player.h"
#include "core/engine_core.h"
#include "logging/logger.h"
#include "rendering/renderer.h"
#include <catch2/catch_test_macros.hpp>
#include <entt/entt.hpp>
#include <iterator>

struct Renderable;
TEST_CASE("EngineCore initializes/shuts down", "[engine][core]") {
    auto& core = engine::EngineCore::getInstance();
    REQUIRE(core.initialize() == true);
    core.shutdown();
}

TEST_CASE("Logger basic usage does not throw", "[engine][logging]") {
    REQUIRE_NOTHROW(engine::Logger::initialize());
    REQUIRE_NOTHROW(LOG_INFO_MSG("Info"));
    REQUIRE_NOTHROW(LOG_WARN_MSG("Warn"));
    REQUIRE_NOTHROW(LOG_ERROR_MSG("Error"));
    REQUIRE_NOTHROW(engine::Logger::shutdown());
}

TEST_CASE("ECS registry basic operations", "[ecs][entt]") {
    entt::registry registry;
    
    SECTION("Entity creation and destruction") {
        auto entity = registry.create();
        REQUIRE(registry.valid(entity));
        
        registry.destroy(entity);
        REQUIRE_FALSE(registry.valid(entity));
    }
    
    SECTION("Component attachment and retrieval") {
        auto entity = registry.create();
        
        registry.emplace<Position>(entity, 10.0f, 20.0f);
        REQUIRE(registry.all_of<Position>(entity));
        
        auto& [pos] = registry.get<Position>(entity);
        REQUIRE(pos.x == 10.0f);
        REQUIRE(pos.y == 20.0f);
    }
    
    SECTION("Multiple components per entity") {
        auto entity = registry.create();
        
        registry.emplace<Position>(entity, 5.0F, 15.0F);
        registry.emplace<Velocity>(entity, 1.0F, -1.0F);
        
        REQUIRE(registry.all_of<Position, Velocity>(entity));
        
        auto& [position] = registry.get<Position>(entity);
        auto& [velocity, rotationSpeed] = registry.get<Velocity>(entity);
        
        REQUIRE(position.x == 5.0F);
        REQUIRE(velocity.x == 1.0F);
    }
    
    SECTION("View iteration") {
        // Create entities with different component combinations
        auto e1 = registry.create();
        auto e2 = registry.create();
        auto e3 = registry.create();
        
        registry.emplace<Position>(e1, 1.0f, 1.0f);
        registry.emplace<Position>(e2, 2.0f, 2.0f);
        registry.emplace<Velocity>(e2, 0.5f, 0.5f);
        registry.emplace<Position>(e3, 3.0f, 3.0f);
        registry.emplace<Velocity>(e3, -0.5f, -0.5f);
        
        // Test view with single component
        auto posView = registry.view<Position>();
        REQUIRE(std::distance(posView.begin(), posView.end()) == 3);
        
        // Test view with multiple components
        auto movementView = registry.view<Position, Velocity>();
        REQUIRE(std::distance(movementView.begin(), movementView.end()) == 2);
        
        // Test iteration
        int count = 0;
        for (auto entity : movementView) {
            REQUIRE(registry.all_of<Position, Velocity>(entity));
            count++;
        }
        REQUIRE(count == 2);
    }
}

TEST_CASE("DiddleDoodleDuel game initialization", "[game][ddd]") {
    engine::Renderer renderer;
    
    // Initialize renderer for headless testing
    auto init = renderer.initialize(800, 600, "Test Window");
    REQUIRE(init.has_value());
    
    diddle_doodle_duel game(renderer);
    
    SECTION("Game initializes without crashing") {
        REQUIRE_NOTHROW(game.onInitialize());
    }
    
    SECTION("Game update doesn't crash with zero delta") {
        game.onInitialize();
        REQUIRE_NOTHROW(game.onUpdate(0.0f));
    }
    
    SECTION("Game update handles normal delta time") {
        game.onInitialize();
        REQUIRE_NOTHROW(game.onUpdate(0.016f)); // ~60 FPS
    }
    
    SECTION("Game render doesn't crash") {
        game.onInitialize();
        REQUIRE_NOTHROW(game.onRender());
    }
    
    renderer.shutdown();
}

TEST_CASE("Game component systems", "[game][ecs][components]") {
    entt::registry registry;
    
    SECTION("Player entity creation") {
        auto player = registry.create();
        registry.emplace<Position>(player, Vector2{400.0f, 300.0f});
        registry.emplace<Velocity>(player, Vector2{0.0f, 0.0f});
        registry.emplace<Renderable>(player, 20.0f, Color{0, 255, 255, 255}); // CYAN
        registry.emplace<Player>(player, 200.0F);

        REQUIRE(registry.all_of<Position, Velocity, Renderable, Player>(player));
        
        auto& pos = registry.get<Position>(player);
        auto& vel = registry.get<Velocity>(player);
        auto& render = registry.get<Renderable>(player);
        auto& playerComp = registry.get<Player>(player);
        
        REQUIRE(pos.pos.x == 400.0f);
        REQUIRE(pos.pos.y == 300.0f);
        REQUIRE(vel.vel.x == 0.0f);
        REQUIRE(vel.vel.y == 0.0f);
        REQUIRE(render.radius == 20.0f);
        REQUIRE(playerComp.speed == 30000.0f);
    }
    
    SECTION("Movement system simulation") {
        auto entity = registry.create();
        registry.emplace<Position>(entity, Vector2{100.0f, 100.0f});
        registry.emplace<Velocity>(entity, Vector2{50.0f, -25.0f});
        
        // Simulate movement update
        float deltaTime = 0.1f;
        auto view = registry.view<Position, Velocity>();
        for (auto ent : view) {
            auto& pos = registry.get<Position>(ent);
            auto& vel = registry.get<Velocity>(ent);
            
            pos.pos.x += vel.vel.x * deltaTime;
            pos.pos.y += vel.vel.y * deltaTime;
        }
        
        auto& finalPos = registry.get<Position>(entity);
        REQUIRE(finalPos.pos.x == 105.0f); // 100 + 50*0.1
        REQUIRE(finalPos.pos.y == 97.5f);  // 100 + (-25)*0.1
    }
    
    SECTION("Boundary checking") {
        auto entity = registry.create();
        registry.emplace<Position>(entity, Vector2{-10.0f, 50.0f});
        registry.emplace<Velocity>(entity, Vector2{-100.0f, 0.0f});
        
        const float radius = 20.0f;
        
        auto& pos = registry.get<Position>(entity);
        auto& vel = registry.get<Velocity>(entity);
        
        // Simulate boundary check (left edge)
        if (pos.pos.x - radius < 0) {
            pos.pos.x = radius;
            vel.vel.x = -vel.vel.x;
        }
        
        REQUIRE(pos.pos.x == radius);
        REQUIRE(vel.vel.x == 100.0f); // Velocity should be reversed
    }
}

TEST_CASE("Renderer integration", "[renderer][integration]") {
    engine::Renderer renderer;
    
    auto init = renderer.initialize(640, 480, "Test Renderer");
    REQUIRE(init.has_value());
    
    SECTION("Basic rendering operations") {
        REQUIRE_NOTHROW(renderer.beginFrame());
        REQUIRE_NOTHROW(renderer.drawCircle(Vector2{100, 100}, 25.0f, Color{255, 0, 0, 255})); // RED
        REQUIRE_NOTHROW(renderer.drawText("Test", Vector2{10, 10}, 20, Color{255, 255, 255, 255})); // WHITE
        REQUIRE_NOTHROW(renderer.endFrame());
    }
    
    SECTION("Renderer properties") {
        REQUIRE(renderer.getWindowWidth() == 640);
        REQUIRE(renderer.getWindowHeight() == 480);
        
        auto center = renderer.getScreenCenter();
        REQUIRE(center.x == 320.0f);
        REQUIRE(center.y == 240.0f);
    }
    
    renderer.shutdown();
}
