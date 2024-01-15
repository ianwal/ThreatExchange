# Copyright (c) Meta Platforms, Inc. and affiliates.

from setuptools import setup
from setuptools.extension import Extension
from setuptools.command.build_ext import build_ext
import sys
import subprocess
from pathlib import Path
import logging
import shutil
from typing import List

logger = logging.getLogger("setup.py")
logger.setLevel(logging.INFO)
logging.basicConfig()

DIR = Path.absolute(Path(__file__).parent)

cpp_dir = DIR / "cpp"
cpp_build_dir = cpp_dir / "build"
cython_path = DIR / "python/vpdq.cpp"
libraries_dirs_path = cpp_dir / "libraries-dirs.txt"

lib_dirs: List[str] = []
include_dirs: List[str] = [str(DIR), str(DIR.parent), str(DIR / "cpp")]


def make_clean():
    """Remove CMake generated files."""
    logger.info(f"Removing CPP build directory {cpp_build_dir}...")
    if Path.exists(cpp_build_dir):
        shutil.rmtree(cpp_build_dir)
    logger.info(f"Removing compiled resulting Cython vpdq.cpp file {cython_path}...")
    Path.unlink(cython_path, missing_ok=True)

    logger.info(f"Clean: Removing libraries-dir.txt file {libraries_dirs_path}...")
    Path.unlink(libraries_dirs_path, missing_ok=True)


class build_ext(build_ext):
    def run(self):
        global lib_dirs
        try:
            make_clean()
            logger.info("Creating CPP build directory...")
            Path.mkdir(cpp_build_dir, exist_ok=True)

            logger.info("Running CMake...")
            cmake_proc = subprocess.run(
                ["cmake", f"{cpp_dir}"],
                cwd=cpp_build_dir,
                check=True,
                capture_output=True,
            )
            logger.info(str(cmake_proc.stdout, "utf-8"))

            logger.info("Compiling with Make...")
            make_proc = subprocess.run(
                ["make"], cwd=cpp_build_dir, check=True, capture_output=True
            )

            # Add the directories of required libraries that are found from CMake to lib_dirs
            logger.info(str(make_proc.stdout, "utf-8"))
            with open(libraries_dirs_path, "r") as file:
                lib_dirs = [line.strip() for line in file]

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


setup(
    version=get_version(),
    cmdclass={"build_ext": build_ext},
    ext_modules=EXTENSIONS,
    entry_points={"console_scripts": ["vpdq = vpdq:_cli"]},
)
