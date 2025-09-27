#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "game_config.h"
#include "core/game_config_manager.h"
#include <filesystem>
#include <fstream>

TEST_CASE("GameConfig default values", "[gameconfig]") {
    GameConfig config{};
    
    SECTION("Brush settings have expected defaults") {
        REQUIRE(config.brushSize == 25.0F);
        REQUIRE(config.brushMovementSpeed == 5000.0F);
        REQUIRE(config.collisionForceMultiplier == 2.0F);
        REQUIRE(config.debugCollisionRadius == 25.0F);
    }
    
    SECTION("Physics settings have expected defaults") {
        REQUIRE(config.restitution == 0.8F);
        REQUIRE(config.collisionDamping == 0.7F);
        REQUIRE(config.separationForce == 100.0F);
        REQUIRE(config.bounceDuration == 0.6F);
        REQUIRE(config.controlDuringBounceFactor == 0.3F);
    }
    
    SECTION("Debug flags have expected defaults") {
        REQUIRE(config.enableProfiler == false);
        REQUIRE(config.enableFpsCounter == false);
    }
}

TEST_CASE("GameConfig modification and validation", "[gameconfig]") {
    GameConfig config{};
    
    SECTION("Brush size can be modified within reasonable bounds") {
        config.brushSize = 50.0F;
        REQUIRE(config.brushSize == 50.0F);
        
        config.brushSize = 10.0F;
        REQUIRE(config.brushSize == 10.0F);
    }
    
    SECTION("Physics values can be modified") {
        config.restitution = 0.5F;
        config.collisionDamping = 0.9F;
        config.separationForce = 200.0F;
        
        REQUIRE(config.restitution == Catch::Approx(0.5F));
        REQUIRE(config.collisionDamping == Catch::Approx(0.9F));
        REQUIRE(config.separationForce == Catch::Approx(200.0F));
    }
    
    SECTION("Boolean flags can be toggled") {
        config.enableProfiler = true;
        config.enableFpsCounter = true;
        
        REQUIRE(config.enableProfiler == true);
        REQUIRE(config.enableFpsCounter == true);
        
        config.enableProfiler = false;
        config.enableFpsCounter = false;
        
        REQUIRE(config.enableProfiler == false);
        REQUIRE(config.enableFpsCounter == false);
    }
}

TEST_CASE("GameConfigManager JSON serialization", "[gameconfig]") {
    const std::filesystem::path testConfigPath = "test_game_config.json";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testConfigPath)) {
        std::filesystem::remove(testConfigPath);
    }
    
    SECTION("Save and load default config") {
        GameConfig originalConfig{};
        
        // Save the config
        REQUIRE(GameConfigManager::saveToJson(testConfigPath, originalConfig));
        REQUIRE(std::filesystem::exists(testConfigPath));
        
        // Load it back
        GameConfig loadedConfig{};
        REQUIRE(GameConfigManager::loadFromJson(testConfigPath, loadedConfig));
        
        // Verify all values match
        REQUIRE(loadedConfig.brushSize == originalConfig.brushSize);
        REQUIRE(loadedConfig.brushMovementSpeed == originalConfig.brushMovementSpeed);
        REQUIRE(loadedConfig.collisionForceMultiplier == originalConfig.collisionForceMultiplier);
        REQUIRE(loadedConfig.restitution == originalConfig.restitution);
        REQUIRE(loadedConfig.collisionDamping == originalConfig.collisionDamping);
        REQUIRE(loadedConfig.separationForce == originalConfig.separationForce);
        REQUIRE(loadedConfig.enableProfiler == originalConfig.enableProfiler);
        REQUIRE(loadedConfig.enableFpsCounter == originalConfig.enableFpsCounter);
    }
    
    SECTION("Save and load modified config") {
        GameConfig originalConfig{};
        originalConfig.brushSize = 42.0f;
        originalConfig.restitution = 0.95f;
        originalConfig.enableProfiler = true;
        originalConfig.enableFpsCounter = true;
        
        // Save the modified config
        REQUIRE(GameConfigManager::saveToJson(testConfigPath, originalConfig));
        
        // Load it back
        GameConfig loadedConfig{};
        REQUIRE(GameConfigManager::loadFromJson(testConfigPath, loadedConfig));
        
        // Verify modified values
        REQUIRE(loadedConfig.brushSize == Catch::Approx(42.0f));
        REQUIRE(loadedConfig.restitution == Catch::Approx(0.95f));
        REQUIRE(loadedConfig.enableProfiler == true);
        REQUIRE(loadedConfig.enableFpsCounter == true);
    }
    
    SECTION("Loading non-existent file returns false") {
        GameConfig config{};
        REQUIRE_FALSE(GameConfigManager::loadFromJson("non_existent_config.json", config));
    }
    
    SECTION("Loading malformed JSON returns false") {
        // Create a malformed JSON file
        std::ofstream badFile(testConfigPath);
        badFile << "{ \"brushSize\": 25.0, \"invalid\": }";
        badFile.close();
        
        GameConfig config{};
        REQUIRE_FALSE(GameConfigManager::loadFromJson(testConfigPath, config));
    }
    
    SECTION("Loading partial JSON file preserves existing values") {
        // Create a JSON file with only some values
        std::ofstream partialFile(testConfigPath);
        partialFile << R"({
            "version": "1.0",
            "brushSize": 99.0,
            "enableProfiler": true
        })";
        partialFile.close();
        
        GameConfig config{};
        // Set some initial values that shouldn't be overwritten
        config.restitution = 0.123f;
        config.enableFpsCounter = false;
        
        REQUIRE(GameConfigManager::loadFromJson(testConfigPath, config));
        
        // Verify that specified values were loaded
        REQUIRE(config.brushSize == Catch::Approx(99.0f));
        REQUIRE(config.enableProfiler == true);
        
        // Verify that unspecified values were preserved
        REQUIRE(config.restitution == Catch::Approx(0.123f));
        REQUIRE(config.enableFpsCounter == false);
    }
    
    SECTION("Loading config with missing fields uses defaults correctly") {
        // Create config with only a few fields
        std::ofstream minimalFile(testConfigPath);
        minimalFile << R"({
            "version": "1.0",
            "brushSize": 50.0
        })";
        minimalFile.close();
        
        GameConfig config{};
        REQUIRE(GameConfigManager::loadFromJson(testConfigPath, config));
        
        // Verify loaded field
        REQUIRE(config.brushSize == Catch::Approx(50.0f));
        
        // Verify all other fields remain at their default values
        REQUIRE(config.brushMovementSpeed == 5000.0f);
        REQUIRE(config.collisionForceMultiplier == 2.0f);
        REQUIRE(config.bounceDuration == 0.6f);
        REQUIRE(config.controlDuringBounceFactor == 0.3f);
        REQUIRE(config.debugCollisionRadius == 25.0f);
        REQUIRE(config.restitution == 0.8f);
        REQUIRE(config.collisionDamping == 0.7f);
        REQUIRE(config.separationForce == 100.0f);
        REQUIRE(config.enableProfiler == false);
        REQUIRE(config.enableFpsCounter == false);
    }
    
    // Clean up
    if (std::filesystem::exists(testConfigPath)) {
        std::filesystem::remove(testConfigPath);
    }
}

TEST_CASE("GameConfigManager path handling", "[gameconfig]") {
    SECTION("getConfigPath returns valid path") {
        auto path = GameConfigManager::getConfigPath();
        REQUIRE(!path.empty());
        REQUIRE(path.filename() == "gameconfig.json");
    }
}

TEST_CASE("GameConfigManager version support", "[gameconfig") {
    const std::filesystem::path testConfigPath = "test_version_config.json";
    
    // Clean up any existing test file
    if (std::filesystem::exists(testConfigPath)) {
        std::filesystem::remove(testConfigPath);
    }
    
    SECTION("Config with supported version loads successfully") {
        std::ofstream file(testConfigPath);
        file << R"({
            "version": "1.0",
            "brushSize": 42.0,
            "enableProfiler": true
        })";
        file.close();
        
        GameConfig config{};
        REQUIRE(GameConfigManager::loadFromJson(testConfigPath, config));
        REQUIRE(config.brushSize == Catch::Approx(42.0f));
        REQUIRE(config.enableProfiler == true);
    }
    
    SECTION("Config without version field loads with warning") {
        std::ofstream file(testConfigPath);
        file << R"({
            "brushSize": 35.0,
            "enableProfiler": false
        })";
        file.close();
        
        GameConfig config{};
        REQUIRE(GameConfigManager::loadFromJson(testConfigPath, config));
        REQUIRE(config.brushSize == Catch::Approx(35.0f));
        REQUIRE(config.enableProfiler == false);
    }
    
    SECTION("Config with unsupported version loads with warning") {
        std::ofstream file(testConfigPath);
        file << R"({
            "version": "2.0",
            "brushSize": 55.0
        })";
        file.close();
        
        GameConfig config{};
        REQUIRE(GameConfigManager::loadFromJson(testConfigPath, config));
        REQUIRE(config.brushSize == Catch::Approx(55.0f));
    }
    
    SECTION("Saved config includes version") {
        GameConfig config{};
        config.brushSize = 88.0f;
        
        REQUIRE(GameConfigManager::saveToJson(testConfigPath, config));
        
        // Verify version is present in saved file
        std::ifstream file(testConfigPath);
        std::string content((std::istreambuf_iterator<char>(file)),
                           std::istreambuf_iterator<char>());
        REQUIRE(content.find("\"version\"") != std::string::npos);
        REQUIRE(content.find("\"1.0\"") != std::string::npos);
    }
    
    // Clean up
    if (std::filesystem::exists(testConfigPath)) {
        std::filesystem::remove(testConfigPath);
    }
}

TEST_CASE("GameConfig edge cases", "[gameconfig]") {
    SECTION("Config handles extreme values gracefully") {
        GameConfig config{};
        
        // Test with very large values
        config.brushSize = 1000000.0f;
        config.brushMovementSpeed = 999999.0f;
        config.separationForce = 1e6f;
        
        REQUIRE(config.brushSize == 1000000.0f);
        REQUIRE(config.brushMovementSpeed == 999999.0f);
        REQUIRE(config.separationForce == Catch::Approx(1e6f));
        
        // Test with very small values
        config.brushSize = 0.001f;
        config.restitution = 0.0f;
        config.collisionDamping = 0.0f;
        
        REQUIRE(config.brushSize == Catch::Approx(0.001f));
        REQUIRE(config.restitution == Catch::Approx(0.0f));
        REQUIRE(config.collisionDamping == Catch::Approx(0.0f));
    }
    
    SECTION("Config serialization/deserialization with extreme values") {
        const std::filesystem::path testConfigPath = "test_extreme_config.json";
        
        // Clean up any existing test file
        if (std::filesystem::exists(testConfigPath)) {
            std::filesystem::remove(testConfigPath);
        }
        
        GameConfig originalConfig{};
        originalConfig.brushSize = 0.00001f;
        originalConfig.brushMovementSpeed = 1000000.0f;
        originalConfig.restitution = 1.0f;
        originalConfig.collisionDamping = 0.0f;
        
        REQUIRE(GameConfigManager::saveToJson(testConfigPath, originalConfig));
        
        GameConfig loadedConfig{};
        REQUIRE(GameConfigManager::loadFromJson(testConfigPath, loadedConfig));
        
        REQUIRE(loadedConfig.brushSize == Catch::Approx(0.00001f));
        REQUIRE(loadedConfig.brushMovementSpeed == Catch::Approx(1000000.0f));
        REQUIRE(loadedConfig.restitution == Catch::Approx(1.0f));
        REQUIRE(loadedConfig.collisionDamping == Catch::Approx(0.0f));
        
        // Clean up
        if (std::filesystem::exists(testConfigPath)) {
            std::filesystem::remove(testConfigPath);
        }
    }
}