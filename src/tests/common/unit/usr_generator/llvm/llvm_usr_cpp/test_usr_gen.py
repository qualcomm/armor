# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
import os
import subprocess
from deepdiff import DeepDiff

def test_ast_diff(binary_path, binary_args, get_cpp_files, request):
    test_dir = os.path.dirname(request.fspath)

    cpp_files = get_cpp_files
    for cpp_file in cpp_files:
        # Run the binary
        subprocess.run(
            [binary_path] + [cpp_file] + binary_args,
            check=True,
            cwd=os.path.dirname(request.fspath)
        )

        print(test_dir)

        # Read the expected output file as lines
        with open(f'{test_dir}/expected.txt', 'r') as f:
            expected_lines = [line.strip() for line in f.readlines()]

        # Read the actual output file as lines
        with open(f'{test_dir}/output.txt', 'r') as f:
            actual_lines = [line.strip() for line in f.readlines()]

        # Compare the lines
        diff = DeepDiff(expected_lines, actual_lines, ignore_order=True)

        # Assert that there are no differences
        assert diff == {}, f"Files differ: {diff}"