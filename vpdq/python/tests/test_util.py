# Copyright (c) Meta Platforms, Inc. and affiliates.
import os
from pathlib import Path

# FIXME: Must fix this. This is needed on Windows currently because of "ImportError: DLL load failed while importing vpdq: The specified module could not be found."
os.add_dll_directory((Path(__file__).parent.parent.parent / "cpp/ffmpeg/bin").absolute())

import vpdq  # type: ignore
import typing as t


def read_file_to_hash(input_hash_filename: str) -> t.List[vpdq.VpdqFeature]:
    """Read hash file and return vpdq hash

    Args:
        input_hash_filename (str): Input hash file path

    Returns:
        list of VpdqFeature: vpdq hash from the hash file"""

    hash = []
    with open(input_hash_filename, "r") as file:
        lines = file.readlines()
    for line in lines:
        line = line.strip()
        content = line.split(",")
        pdq_hash = vpdq.str_to_hash(content[2])
        feature = vpdq.VpdqFeature(
            int(content[1]), int(content[0]), pdq_hash, float(content[3])
        )
        hash.append(feature)

    return hash
