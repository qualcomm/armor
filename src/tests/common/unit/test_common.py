# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause

import pytest
import subprocess
from pathlib import Path

GTEST_EXECUTABLE = Path(__file__).parent.parent.parent.parent.parent / "build/src/tests/common/common_unit_tests"

def test_run_all_cpp_unit_tests():
    print(GTEST_EXECUTABLE)
    result = subprocess.run([str(GTEST_EXECUTABLE)], capture_output=True, text=True)
    print(result.stdout)
    assert result.returncode == 0
