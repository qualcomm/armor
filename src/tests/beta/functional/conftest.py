# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

import os
import pytest

def find_path_from_project_root(marker):
    while True:
        current = os.getcwd()
        if os.path.exists(os.path.join(current, marker)):
            return os.path.join(current, marker)
        parent = os.path.dirname(current)
        if parent == current:
            break  # Reached the filesystem root
        os.chdir(parent)
    raise RuntimeError(f"Project root with marker '{marker}' not found.")

def get_build_dir():
    return find_path_from_project_root("build")

@pytest.fixture
def binary_path():
    """Returns the absolute path to the binary."""
    return os.path.join(get_build_dir(), "src/tests/beta/src/beta")

@pytest.fixture
def binary_args(request):
    """Returns the arguments for the binary with debug and JSON output enabled."""
    curr_dir = os.path.dirname(request.fspath)
    prj_root1 = os.path.join(curr_dir, "v1")
    prj_root2 = os.path.join(curr_dir, "v2")
    return [prj_root1, prj_root2, "mylib.h", "--dump-ast-diff", "-r", "json"]
