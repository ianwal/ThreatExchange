from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain, CMake
from conan.tools.files import load
from conan.errors import ConanInvalidConfiguration
from conan.tools.system.package_manager import Apt
from pathlib import Path


class VpdqRecipe(ConanFile):
    name = "vpdq"
    settings = "os", "compiler", "build_type", "arch"
    generators = ("CMakeDeps", "CMakeToolchain")

    def set_version(self):
        self.version = load(self, Path(__file__).parent / "version.txt").rstrip()

    def system_requirements(self):
        Apt(self).install([
            "libavdevice-dev",
            "libavfilter-dev",
            "libavformat-dev",
            "libavcodec-dev",
            "libswresample-dev",
            "libswscale-dev",
            "libswscale-dev"
        ])

    def requirements(self):
        self.tool_requires("cmake/[>3.13]")

    def layout(self):
        cmake_layout(self, src_folder="cpp")

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def validate(self):
        if self.settings.os == "Windows":
            raise ConanInvalidConfiguration("Windows is not supported yet by the conanfile.")
