# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clauseimport os
import os
import json
import subprocess
from deepdiff import DeepDiff


def test_ast_diff(binary_path, binary_args, request):

    test_dir = os.path.dirname(request.fspath)

    subprocess.run(
        [binary_path] + binary_args,
        check=True,
        cwd=os.path.dirname(request.fspath)
    )

    print(test_dir)

    with open(f'{test_dir}/expected_output.json', 'r') as f:
        expected_json = json.load(f)

    with open(f'{test_dir}/debug_output/ast_diffs/ast_diff_output_mylib.h.json', 'r') as f:
        actual_json = json.load(f)
    
    diff = DeepDiff(expected_json, actual_json, ignore_order=True)

    assert diff == {}

    # Compare text output files line by line
    with open(f'{test_dir}/expected_output.txt', 'r') as f:
        expected_lines = f.readlines()

    with open(f'{test_dir}/output.txt', 'r') as f:
        actual_lines = f.readlines()

    for i, (expected_line, actual_line) in enumerate(zip(expected_lines, actual_lines), 1):
        assert expected_line == actual_line, f"Line {i} differs:\nExpected: {expected_line}\nActual: {actual_line}"

    assert len(expected_lines) == len(actual_lines), f"File length differs: expected {len(expected_lines)} lines, got {len(actual_lines)} lines"
