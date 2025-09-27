#ifndef DIDDLEDOODLEDUEL_GAME_CONFIG_MANAGER_H
#define DIDDLEDOODLEDUEL_GAME_CONFIG_MANAGER_H

#include "game_config.h"
#include <filesystem>
#include <nlohmann/json_fwd.hpp>

class GameConfigManager {
public:
    static bool loadFromJson(const std::filesystem::path& configPath, GameConfig& config);
    static bool saveToJson(const std::filesystem::path& configPath, const GameConfig& config);
    static std::filesystem::path getConfigPath();
    
private:
    static constexpr const char* CONFIG_FILENAME = "gameconfig.json";
    static constexpr const char* CONFIG_VERSION = "1.0";
    static constexpr size_t MAX_CONFIG_FILE_SIZE = 1024 * 1024; // 1MB max
    
    template<typename T>
    static void loadIfPresent(const nlohmann::json& json, const std::string& key, T& value);
    
    static bool validateConfigFileSize(const std::filesystem::path& configPath);
    static bool isVersionSupported(const std::string& version);
};

#endif // DIDDLEDOODLEDUEL_GAME_CONFIG_MANAGER_H