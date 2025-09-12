cmake_minimum_required(VERSION 3.20)

# --- Project metadata ---
project(DiddleDoodleDuel
        VERSION 1.0.0
        DESCRIPTION "Diddle Doodle Duel - Sample game using HumbleEngine"
        HOMEPAGE_URL "https://github.com/iyashj/humble-engine"
        LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(DDD_ENABLE_LTO "Enable Link Time Optimization" ON)
option(DDD_ENABLE_SANITIZERS "Enable sanitizers (Debug only)" OFF)
set(DDD_SANITIZERS "address;undefined" CACHE STRING "List of sanitizers to enable in Debug builds")

# --- Dependencies via submodule ---
# Expect the engine to be available as a git submodule at humble-engine/
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/humble-engine/CMakeLists.txt")
    add_subdirectory(humble-engine)
else()
    message(FATAL_ERROR "humble-engine submodule not found. Please run 'git submodule update --init --recursive'.")
endif()

# --- Game executable ---
add_executable(${PROJECT_NAME}
        main.cpp
        src/diddle_doodle_duel.cpp
        src/diddle_doodle_duel.h
        src/components/movement_structs.h
        src/components/drawing_structs.h
        src/components/player.h
        src/components/input_structs.h
        src/systems/paint.h
        src/systems/movement.h
        src/systems/input.h
        src/systems/ui.h
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PRIVATE HumbleEngine::HumbleEngine)
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/humble-engine/include)

# Link entt if available
find_package(entt REQUIRED)
if(entt_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)
endif()

# --- Tests ---
if(DOODLEDUEL_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# Warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wpedantic -Wshadow -Wconversion
            $<$<CONFIG:Debug>:-g3 -O0 -fno-omit-frame-pointer>
            $<$<CONFIG:RelWithDebInfo>:-O2 -g -fno-omit-frame-pointer>
            $<$<CONFIG:Release>:-O3 -DNDEBUG>
    )
elseif(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
            /W4 /permissive- /utf-8
            $<$<CONFIG:Debug>:/Od /Zi>
            $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
            $<$<CONFIG:Release>:/O2 /DNDEBUG>
            /MP
    )
    # Ensure PDBs are generated for debug builds
    target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:/DEBUG>
            $<$<CONFIG:RelWithDebInfo>:/DEBUG>
    )
endif()

# Sanitizers (Debug only)
if(DDD_ENABLE_SANITIZERS AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    foreach(sanitizer IN LISTS DDD_SANITIZERS)
        if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
            target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=${sanitizer})
            target_link_options(${PROJECT_NAME} PRIVATE -fsanitize=${sanitizer})
        endif()
    endforeach()
endif()

# LTO only for non-Debug configs
if(DDD_ENABLE_LTO)
    include(CheckIPOSupported)
    check_ipo_supported(RESULT ipo_supported OUTPUT error)
    if(ipo_supported)
        # Enable IPO per-config to avoid impacting Debug symbols
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        set_property(TARGET ${PROJECT_NAME} PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELWITHDEBINFO TRUE)
    endif()
endif()