# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
import os
import json
import subprocess
from deepdiff import DeepDiff


def test_ast_diff(binary_path, binary_args, request):

    test_dir = os.path.dirname(request.fspath)

    result = subprocess.run(
        [binary_path] + binary_args,
        cwd=os.path.dirname(request.fspath)
    )

    print(test_dir)

    assert result.returncode == 0
    