cmake_minimum_required(VERSION 3.20)

# Catch2 v3 provides Catch2::Catch2WithMain and CTest integration via Catch.cmake
find_package(Catch2 3 CONFIG REQUIRED)

add_executable(ddd_tests
    test_game_config.cpp
    ../src/game_config.h
    ../src/core/game_config_manager.h
    ../src/core/game_config_manager.cpp
)

target_link_libraries(ddd_tests PRIVATE
    Catch2::Catch2WithMain
    HumbleEngine::HumbleEngine
)

# Link required dependencies
find_package(entt REQUIRED)
find_package(nlohmann_json REQUIRED)

if(entt_FOUND)
    target_link_libraries(ddd_tests PRIVATE EnTT::EnTT)
endif()

if(nlohmann_json_FOUND)
    target_link_libraries(ddd_tests PRIVATE nlohmann_json::nlohmann_json)
endif()

target_include_directories(ddd_tests PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/../humble-engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../src
)

# High warnings for tests as well
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(ddd_tests PRIVATE -Wall -Wextra -Wpedantic -Wshadow -Wconversion)
elseif(MSVC)
    target_compile_options(ddd_tests PRIVATE /W4 /permissive- /utf-8 /MP)
endif()

include(CTest)
include(Catch)
catch_discover_tests(ddd_tests)
