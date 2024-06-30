from conan import ConanFile
from conan.tools.cmake import cmake_layout, CMakeDeps, CMakeToolchain, CMake
from conan.tools.files import load
from conan.errors import ConanInvalidConfiguration
from conan.tools.gnu import PkgConfig
from pathlib import Path


class ExampleRecipe(ConanFile):
    name = "vpdq"
    version = "0.2.1"
    settings = "os", "compiler", "build_type", "arch"
    generators = ("CMakeDeps", "CMakeToolchain")

    def set_version(self):
        self.version = load(self, Path(__file__).parents[1] / "version.txt").rstrip()

    def configure(self):
        self.options["ffmpeg"].with_ssl = False
        if self.settings.os == "Linux":
            self.options["ffmpeg"].with_pulse = False
        self.options["ffmpeg"].disable_all_protocols = True

    def requirements(self):
        self.requires("ffmpeg/[>=6]")
        self.tool_requires("cmake/[>3.12]")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def validate(self):
        pass
        #if self.settings.os == "Windows":
        #    raise ConanInvalidConfiguration("Windows not supported")

def package_info(self):
    pkg_config = PkgConfig(self, "libavdevice")
    pkg_config.fill_cpp_info(self.cpp_info, is_system=False, system_libs=["m", "rt"])
