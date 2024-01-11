# Copyright (c) Meta Platforms, Inc. and affiliates.

from setuptools import setup
from setuptools.extension import Extension
from setuptools.command.build_ext import build_ext
import sys
import subprocess
from pathlib import Path
import os
import logging

logger = logging.getLogger("setup.py")
logger.setLevel(logging.INFO)
logging.basicConfig()

# setup.py cannot be run directly, it has to be run through vpdq-release.py
# because the paths are relative to the parent to the vpdq directory.
# THIS HAS TO BE CHANGED!

DIR = Path(__file__).parent / "vpdq"
read_me = DIR / Path("python/README.md")
long_description = read_me.read_text()
version = (DIR / "version.txt").read_text(encoding="utf-8").strip()
cpp_dir = DIR / "cpp"
cpp_build_dir = cpp_dir / "build"

# Get the library directories and include directories from the environment variables
# These variables should be set in the CMakeLists.txt file
lib_dirs = []
include_dirs = ["./"] # WTF? Why does this only compile pdq if I include "./" ??


class build_ext(build_ext):
    def run(self):
        global include_dirs
        global lib_dirs
        try:
            # TODO: Clean the build directory before building
            logger.info("Removing compiled pyx .cpp file...")
            Path.unlink(DIR / "python/vpdq.cpp", missing_ok=True)

            logger.info("Creating build directory...")
            subprocess.run(["mkdir", "build"], cwd=cpp_dir, check=False)
            logger.info("Running CMake...")
            cmake_proc = subprocess.run(
                ["cmake", ".."], cwd=cpp_build_dir, check=True, capture_output=True
            )
            logger.info(str(cmake_proc.stdout, "utf-8"))
            logger.info("Compiling with Make...")
            make_proc = subprocess.run(
                ["make"], cwd=cpp_build_dir, check=True, capture_output=True
            )
            logger.info(str(make_proc.stdout, "utf-8"))
            with open(str(cpp_dir) + "/libraries-dirs.txt", "r") as file:
                for line in file:
                    lib_dirs.append(line.strip())
            include_dirs.extend(lib_dirs)
        except subprocess.CalledProcessError as e:
            logger.critical(str(e.stderr, "utf-8"))
            logger.critical("Failed to compile vpdq library.")
            sys.exit(1)
        super().run()


EXTENSIONS = [
    Extension(
        "vpdq",
        sources=["vpdq/python/vpdq.pyx"],
        language="c++",
        libraries=[
            "avdevice",
            "avfilter",
            "avformat",
            "avcodec",
            "swresample",
            "swscale",
            "avutil",
        ],
        extra_objects=["vpdq/cpp/build/libvpdqlib.a"],
        library_dirs=lib_dirs,
        include_dirs=include_dirs,
        extra_compile_args=["--std=c++14"],
    )
]

setup(
    name="vpdq",
    author="Facebook",
    description="Python bindings for Facebook VPDQ hash",
    author_email="threatexchange@fb.com",
    version=version,
    license_files="LICENSE.txt",
    license="BSD",
    long_description=long_description,
    long_description_content_type="text/markdown",
    install_requires=["cython"],
    include_package_data=True,
    cmdclass={"build_ext": build_ext},
    ext_modules=EXTENSIONS,
    entry_points={"console_scripts": ["vpdq = vpdq:_cli"]},
)
