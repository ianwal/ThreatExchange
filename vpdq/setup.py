# Copyright (c) Meta Platforms, Inc. and affiliates.

import setuptools
from setuptools.extension import Extension
from setuptools.command.build_ext import build_ext
import sys
import subprocess
from pathlib import Path
import logging
import shutil

logger = logging.getLogger("setup.py")
logger.setLevel(logging.INFO)
logging.basicConfig()

DIR = Path.absolute(Path(__file__).parent)

cpp_dir = DIR / "cpp"
cpp_build_dir = cpp_dir / "build"
cython_path = DIR / "python/vpdq.cpp"
libraries_dirs_path = DIR / "libraries-dirs.txt"

# Get the library directories and include directories from the environment variables
# These variables should be set in the CMakeLists.txt file
lib_dirs = []
include_dirs = [str(DIR), str(DIR.parent), str(DIR / "cpp")]


def make_clean():
    logger.info("Removing CPP build directory...")
    if Path.exists(cpp_build_dir):
        shutil.rmtree(cpp_build_dir)
    #logger.info("Removing compiled pyx .cpp file...")
    #Path.unlink(cython_path, missing_ok=True)

    logger.info("Removing libraries-dir.txt file...")
    Path.unlink(libraries_dirs_path, missing_ok=True)


class build_ext(build_ext):
    def run(self):
        global include_dirs
        global lib_dirs
        try:
            # make_clean()
            logger.info("Creating CPP build directory...")
            Path.mkdir(cpp_build_dir, exist_ok=True)

            logger.info("Running CMake...")
            cmake_proc = subprocess.run(
                ["cmake", f"{cpp_dir}"], cwd=cpp_build_dir, check=True, capture_output=True
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
                    include_dirs.append(line.strip())
        except subprocess.CalledProcessError as e:
            logger.critical(str(e.stderr, "utf-8"))
            logger.critical("Failed to compile vpdq library.")
            sys.exit(1)
        super().run()


EXTENSIONS = [
    Extension(
        name="vpdq",
        sources=[str(DIR) + "/python/vpdq.pyx"],
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
        extra_objects=[str(cpp_build_dir) + "/libvpdq.a"],
        library_dirs=lib_dirs,
        include_dirs=include_dirs,
    )
]


def get_version():
    version = (DIR / "version.txt").read_text(encoding="utf-8").strip()
    return version


setuptools.setup(
    version=get_version(),
    cmdclass={"build_ext": build_ext},
    ext_modules=EXTENSIONS,
    entry_points={"console_scripts": ["vpdq = vpdq:_cli"]},
)
