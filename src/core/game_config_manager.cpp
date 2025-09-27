#include "game_config_manager.h"
#include "logging/logger.h"
#include "resources/resource_manager.h"
#include <nlohmann/json.hpp>
#include <fstream>


template <typename T>
void GameConfigManager::loadIfPresent( const nlohmann::json& json, const std::string& key, T& value) {
    if (json.contains(key)) {
        value = json[key];
    }
}

bool GameConfigManager::loadFromJson(const std::filesystem::path& configPath, GameConfig& config) {
    try {
        if (!std::filesystem::exists(configPath)) {
            LOG_INFO_MSG("Config file not found at: %s. Using default values.", configPath.string().c_str());
            return false;
        }
        
        // Validate file size before attempting to load
        if (!validateConfigFileSize(configPath)) {
            LOG_ERROR_MSG("Config file too large: %s", configPath.string().c_str());
            return false;
        }

        std::ifstream file(configPath);
        if (!file.is_open()) {
            LOG_ERROR_MSG("Failed to open config file: %s", configPath.string().c_str());
            return false;
        }

        nlohmann::json jsonObject;
        file >> jsonObject;

        // Check version compatibility
        if (jsonObject.contains("version")) {
            std::string fileVersion = jsonObject["version"];
            if (!isVersionSupported(fileVersion)) {
                LOG_WARN_MSG("Config file version %s may not be fully compatible with current version %s", 
                           fileVersion.c_str(), CONFIG_VERSION);
            }
        } else {
            LOG_INFO_MSG("Config file has no version field, assuming legacy format");
        }

        loadIfPresent(jsonObject, "brushSize", config.brushSize);
        loadIfPresent(jsonObject, "brushMovementSpeed", config.brushMovementSpeed);
        loadIfPresent(jsonObject, "collisionForceMultiplier", config.collisionForceMultiplier);
        loadIfPresent(jsonObject, "bounceDuration", config.bounceDuration);
        loadIfPresent(jsonObject, "controlDuringBounceFactor", config.controlDuringBounceFactor);
        loadIfPresent(jsonObject, "debugCollisionRadius", config.debugCollisionRadius);
        loadIfPresent(jsonObject, "restitution", config.restitution);
        loadIfPresent(jsonObject, "collisionDamping", config.collisionDamping);
        loadIfPresent(jsonObject, "separationForce", config.separationForce);
        loadIfPresent(jsonObject, "enableProfiler", config.enableProfiler);
        loadIfPresent(jsonObject, "enableFpsCounter", config.enableFpsCounter);

        LOG_INFO_MSG("Successfully loaded config from: %s", configPath.string().c_str());
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR_MSG("Error loading config file: %s", e.what());
        return false;
    }
}

bool GameConfigManager::saveToJson(const std::filesystem::path& configPath, const GameConfig& config) {
    try {
        nlohmann::json jsonObject;
        
        jsonObject["version"] = CONFIG_VERSION;
        
        jsonObject["brushSize"] = config.brushSize;
        jsonObject["brushMovementSpeed"] = config.brushMovementSpeed;
        jsonObject["collisionForceMultiplier"] = config.collisionForceMultiplier;
        jsonObject["bounceDuration"] = config.bounceDuration;
        jsonObject["controlDuringBounceFactor"] = config.controlDuringBounceFactor;
        jsonObject["debugCollisionRadius"] = config.debugCollisionRadius;
        jsonObject["restitution"] = config.restitution;
        jsonObject["collisionDamping"] = config.collisionDamping;
        jsonObject["separationForce"] = config.separationForce;
        jsonObject["enableProfiler"] = config.enableProfiler;
        jsonObject["enableFpsCounter"] = config.enableFpsCounter;

        std::ofstream file(configPath);
        if (!file.is_open()) {
            LOG_ERROR_MSG("Failed to open config file for writing: %s", configPath.string().c_str());
            return false;
        }

        file << jsonObject.dump(2); // Pretty print with 2-space indentation
        LOG_INFO_MSG("Successfully saved config to: %s", configPath.string().c_str());
        return true;

    } catch (const std::exception& e) {
        LOG_ERROR_MSG("Error saving config file: %s", e.what());
        return false;
    }
}

std::filesystem::path GameConfigManager::getConfigPath() {
    return engine::resources::getExecutableDir() / CONFIG_FILENAME;
}

bool GameConfigManager::validateConfigFileSize(const std::filesystem::path& configPath) {
    try {
        const auto fileSize = std::filesystem::file_size(configPath);
        return fileSize <= MAX_CONFIG_FILE_SIZE;
    } catch (const std::filesystem::filesystem_error& e) {
        LOG_ERROR_MSG("Failed to get file size for %s: %s", configPath.string().c_str(), e.what());
        return false;
    }
}

bool GameConfigManager::isVersionSupported(const std::string& version) {
    // For now, we support version 1.0 and treat missing versions as legacy (compatible)
    // Future versions could implement more sophisticated compatibility checking
    return version == "1.0" || version.empty();
}