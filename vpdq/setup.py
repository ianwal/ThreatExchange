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

DIR = Path(__file__).parent

cpp_dir = DIR / "cpp"
cpp_build_dir = cpp_dir / "build"
cython_cpp_path = DIR / "python/vpdq.cpp"
cython_pyx_path = DIR / "python/vpdq.pyx"
libraries_dirs_path = cpp_dir / "libraries-dirs.txt"

lib_dirs: List[str] = []
include_dirs: List[str] = [str(DIR), str(DIR.parent), str(DIR / "cpp")]


def make_clean():
    """Remove CMake and Cython build files from previous runs."""
    logger.info(f"Removing CPP build directory {cpp_build_dir}...")
    if Path.exists(cpp_build_dir):
        shutil.rmtree(cpp_build_dir)

    logger.info(f"Removing compiled Cython files {cython_cpp_path}...")
    Path.unlink(cython_cpp_path, missing_ok=True)

    logger.info(f"Removing libraries-dir.txt file {libraries_dirs_path}...")
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

            logger.info("Compiling libvpdq with Make...")
            make_proc = subprocess.run(
                ["make"], cwd=cpp_build_dir, check=True, capture_output=True
            )
            logger.info(str(make_proc.stdout, "utf-8"))

            # Add the directories of required libraries that are found from CMake to lib_dirs
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
        sources=[str(cython_pyx_path)],
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
        extra_objects=[str(cpp_build_dir / "libvpdq.a")],
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
