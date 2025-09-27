cmake_minimum_required(VERSION 3.20)

# --- Project metadata ---
project(DiddleDoodleDuel
        VERSION 1.0.0
        DESCRIPTION "Diddle Doodle Duel - Sample game using HumbleEngine"
        HOMEPAGE_URL "https://github.com/iyashj/humble-engine"
        LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure gameconfig.json as a dependency to trigger rebuilds when it changes
configure_file(${CMAKE_SOURCE_DIR}/gameconfig.json ${CMAKE_BINARY_DIR}/gameconfig.json COPYONLY)

# Options
option(DDD_ENABLE_LTO "Enable Link Time Optimization" ON)
option(DDD_ENABLE_SANITIZERS "Enable sanitizers (Debug only)" OFF)
set(DDD_SANITIZERS "address;undefined" CACHE STRING "List of sanitizers to enable in Debug builds")

# --- Conan Dependencies ---
# Find all packages required by the project and sub-projects before configuring them.
find_package(raylib REQUIRED)
find_package(fmt REQUIRED)
find_package(entt REQUIRED)
find_package(imgui REQUIRED)
find_package(enet REQUIRED)
find_package(nlohmann_json REQUIRED)

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
        src/components/renderable.h
        src/systems/paint.h
        src/systems/movement.h
        src/systems/input.h
        src/systems/ui.h
        src/systems/collision.h
        src/systems/imgui_system.h
        src/systems/imgui_system.cpp
        src/components/collision_state.h
        src/components/velocity.h
        src/components/position.h
        src/components/input_mapping.h
        src/components/input_action.h
        src/network/network_interface.h
        src/network/enet_network.h
        src/network/enet_network.cpp
        src/network/game_protocol.h
        src/network/multiplayer_manager.h
        src/network/multiplayer_manager.cpp
        src/network/message_definitions.h
        src/core/game_config_manager.h
        src/core/game_config_manager.cpp
)

add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/resources
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/gameconfig.json
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/gameconfig.json
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_23)

target_link_libraries(${PROJECT_NAME} PRIVATE HumbleEngine::HumbleEngine)
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/humble-engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

find_package(entt REQUIRED)
if(entt_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE EnTT::EnTT)
endif()

# Link ENet for networking
if(enet_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE enet::enet)
endif()

# Link nlohmann_json for serialization
if(nlohmann_json_FOUND)
    target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
endif()

# --- Additional executables for client/server ---
# Client executable
add_executable(${PROJECT_NAME}Client
        client_main.cpp
        src/diddle_doodle_duel.cpp
        src/diddle_doodle_duel.h
        src/components/renderable.h
        src/systems/paint.h
        src/systems/movement.h
        src/systems/input.h
        src/systems/ui.h
        src/systems/collision.h
        src/systems/imgui_system.h
        src/systems/imgui_system.cpp
        src/components/collision_state.h
        src/components/velocity.h
        src/components/position.h
        src/components/input_mapping.h
        src/components/input_action.h
        src/network/network_interface.h
        src/network/enet_network.h
        src/network/enet_network.cpp
        src/network/game_protocol.h
        src/network/multiplayer_manager.h
        src/network/multiplayer_manager.cpp
        src/core/game_config_manager.h
        src/core/game_config_manager.cpp
)

# Server executable
add_executable(${PROJECT_NAME}Server
        server_main.cpp
        src/diddle_doodle_duel.cpp
        src/diddle_doodle_duel.h
        src/components/renderable.h
        src/systems/paint.h
        src/systems/movement.h
        src/systems/input.h
        src/systems/ui.h
        src/systems/collision.h
        src/systems/imgui_system.h
        src/systems/imgui_system.cpp
        src/components/collision_state.h
        src/components/velocity.h
        src/components/position.h
        src/components/input_mapping.h
        src/components/input_action.h
        src/network/network_interface.h
        src/network/enet_network.h
        src/network/enet_network.cpp
        src/network/game_protocol.h
        src/network/multiplayer_manager.h
        src/network/multiplayer_manager.cpp
        src/core/game_config_manager.h
        src/core/game_config_manager.cpp
)

# Configure client executable
target_compile_features(${PROJECT_NAME}Client PUBLIC cxx_std_23)
target_link_libraries(${PROJECT_NAME}Client PRIVATE HumbleEngine::HumbleEngine)
target_include_directories(${PROJECT_NAME}Client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/humble-engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Configure server executable  
target_compile_features(${PROJECT_NAME}Server PUBLIC cxx_std_23)
target_link_libraries(${PROJECT_NAME}Server PRIVATE HumbleEngine::HumbleEngine)
target_include_directories(${PROJECT_NAME}Server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/humble-engine/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link libraries to client and server
foreach(target IN ITEMS ${PROJECT_NAME}Client ${PROJECT_NAME}Server)
    if(entt_FOUND)
        target_link_libraries(${target} PRIVATE EnTT::EnTT)
    endif()
    
    if(enet_FOUND)
        target_link_libraries(${target} PRIVATE enet::enet)
    endif()
    
    if(nlohmann_json_FOUND)
        target_link_libraries(${target} PRIVATE nlohmann_json::nlohmann_json)
    endif()
    
    # Copy resources for each executable
    add_custom_command(
        TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_SOURCE_DIR}/resources
            $<TARGET_FILE_DIR:${target}>/resources
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${CMAKE_SOURCE_DIR}/gameconfig.json
            $<TARGET_FILE_DIR:${target}>/gameconfig.json
    )
endforeach()

# Apply ImGui configuration to all executables
foreach(target IN ITEMS ${PROJECT_NAME} ${PROJECT_NAME}Client ${PROJECT_NAME}Server)
    # Ensure third_party/imgui is in the include path for backend headers
    target_include_directories(${target} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui)

    # Always link glfw (regardless of imgui or backends)
    set(GLFW_TARGET_FOUND OFF)
    foreach(glfw_candidate IN ITEMS glfw glfw3 glfw::glfw)
        if(TARGET ${glfw_candidate})
            target_link_libraries(${target} PRIVATE ${glfw_candidate})
            get_target_property(GLFW_INCLUDE_DIRS ${glfw_candidate} INTERFACE_INCLUDE_DIRECTORIES)
            if(GLFW_INCLUDE_DIRS)
                target_include_directories(${target} PRIVATE ${GLFW_INCLUDE_DIRS})
            endif()
            set(GLFW_TARGET_FOUND ON)
            break()
        endif()
    endforeach()
    if(NOT GLFW_TARGET_FOUND)
        message(FATAL_ERROR "No suitable GLFW target found; make sure Conan has provided glfw (glfw, glfw3, or glfw::glfw)")
    endif()

    # Ensure GLFW headers are visible
    foreach(glfw_target IN ITEMS glfw::glfw glfw3::glfw glfw)
        if(TARGET ${glfw_target})
            get_target_property(_glfw_inc ${glfw_target} INTERFACE_INCLUDE_DIRECTORIES)
            if(_glfw_inc)
                target_include_directories(${target} PRIVATE ${_glfw_inc})
            endif()
        endif()
    endforeach()

    if(imgui_FOUND)
        # Link core ImGui
        target_link_libraries(${target} PRIVATE imgui::imgui)
        
        # Add our local backend include directories and sources
        target_include_directories(${target} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui_backends")
        
        # Add backend source files
        target_sources(${target} PRIVATE
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui_backends/imgui_impl_glfw.cpp"
            "${CMAKE_CURRENT_SOURCE_DIR}/third_party/imgui_backends/imgui_impl_opengl3.cpp"
        )

        # Link GLFW if available under any known target name
        if(TARGET glfw::glfw)
            target_link_libraries(${target} PRIVATE glfw::glfw)
        elseif(TARGET glfw3::glfw)
            target_link_libraries(${target} PRIVATE glfw3::glfw)
        elseif(TARGET glfw)
            target_link_libraries(${target} PRIVATE glfw)
        endif()

        # Link OpenGL from Conan (opengl::opengl) or CMake if available
        if(TARGET opengl::opengl)
            target_link_libraries(${target} PRIVATE opengl::opengl)
        else()
            find_package(OpenGL REQUIRED)
            target_link_libraries(${target} PRIVATE OpenGL::GL)
        endif()

        # Do not add vendored imgui_impl_glfw.cpp or imgui_impl_opengl3.cpp; use Conan-provided ImGui only
    endif()

    # Fallback: ensure GLFW include directories are visible even if generator expressions fail
    if(TARGET glfw::glfw)
        get_target_property(_glfw_inc glfw::glfw INTERFACE_INCLUDE_DIRECTORIES)
        if(_glfw_inc)
            target_include_directories(${target} PRIVATE ${_glfw_inc})
        endif()
    endif()
    if(TARGET glfw)
        get_target_property(_glfw_inc2 glfw INTERFACE_INCLUDE_DIRECTORIES)
        if(_glfw_inc2)
            target_include_directories(${target} PRIVATE ${_glfw_inc2})
        endif()
    endif()
endforeach()

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

