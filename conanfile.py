from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout
from conan.tools.files import copy
import os

class DoodleDuelConan(ConanFile):
    name = "doodle-duel"
    version = "1.0.0"
    package_type = "application"
    
    # Binary configuration
    settings = "os", "compiler", "build_type", "arch"
    options = { "build_tests": [True, False] }
    default_options = { "build_tests": False }

    def layout(self):
        # Use simple layout to avoid nested build directories
        self.folders.build = "."
        self.folders.generators = "."

    def requirements(self):
        # Engine submodule needs these dependencies available at parent level
        self.requires("raylib/5.5")
        self.requires("fmt/10.2.1")
        # Add entt for ECS functionality
        self.requires("entt/3.15.0")
        # Dear ImGui (vanilla core library only)
        self.requires("imgui/1.89.9")
        # GLFW needed for ImGui platform backend
        self.requires("glfw/3.4")

    def configure(self):
        # Configure ImGui options - some packages may not support these options
        try:
            self.options["imgui"].backend_glfw = True
            self.options["imgui"].backend_opengl3 = True
        except:
            # If backend options don't exist, the package may include them by default
            pass

    def build_requirements(self):
        if self.options.build_tests:
            self.test_requires("catch2/3.5.2")

    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        
        tc = CMakeToolchain(self)
        tc.variables["DOODLEDUEL_BUILD_TESTS"] = self.options.build_tests
        # Don't generate user presets to avoid conflicts
        tc.user_presets_path = False
        tc.generate()
