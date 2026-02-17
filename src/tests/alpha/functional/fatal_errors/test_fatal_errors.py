# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
import os
import json
import subprocess

def test_ast_diff(binary_path, dependent_binary_args, request):

    test_dir = os.path.dirname(request.fspath)

    subprocess.run(
        [binary_path] + dependent_binary_args,
        check=True,
        cwd=os.path.dirname(request.fspath)
    )

    print(test_dir)

    with open(f'{test_dir}/expected_output.json', 'r') as f:
        expected_json = json.load(f)

    with open(f'{test_dir}/debug_output/ast_diffs/ast_diff_output_mylib.h.json', 'r') as f:
        actual_json = json.load(f)

    # Check all expected keys are present
    expected_keys = set(expected_json.keys())
    actual_keys = set(actual_json.keys())
    assert expected_keys == actual_keys, f"Key mismatch. Expected: {expected_keys}, Actual: {actual_keys}"

    # Check astDiff
    assert actual_json['astDiff'] == expected_json['astDiff'], "astDiff mismatch"

    # Check parsed_status and unparsed_status
    assert actual_json['parsed_status'] == expected_json['parsed_status'], "parsed_status mismatch"
    assert actual_json['unparsed_status'] == expected_json['unparsed_status'], "unparsed_status mismatch"

    # Check headerResolutionFailures by comparing last 7 parts of paths
    assert 'headerResolutionFailures' in actual_json, "headerResolutionFailures not found in output"
    
    header_failures_actual = actual_json['headerResolutionFailures']
    header_failures_expected = expected_json['headerResolutionFailures']
    
    # Normalize expected failures by extracting last 7 parts
    expected_normalized = []
    for failure in header_failures_expected:
        path_parts = failure['file'].split('/')
        last_8_parts = '/'.join(path_parts[-7:]) if len(path_parts) >= 7 else failure['file']
        expected_normalized.append({
            'file': last_8_parts,
            'header': failure['header']
        })
    
    # Normalize actual failures by extracting last 7 parts
    actual_normalized = []
    for failure in header_failures_actual:
        path_parts = failure['file'].split('/')
        last_8_parts = '/'.join(path_parts[-7:]) if len(path_parts) >= 7 else failure['file']
        actual_normalized.append({
            'file': last_8_parts,
            'header': failure['header']
        })
    
    # Sort both lists for comparison
    expected_normalized_sorted = sorted(expected_normalized, key=lambda x: (x['file'], x['header']))
    actual_normalized_sorted = sorted(actual_normalized, key=lambda x: (x['file'], x['header']))
    
    assert expected_normalized_sorted == actual_normalized_sorted, \
        f"headerResolutionFailures mismatch.\nExpected: {expected_normalized_sorted}\nActual: {actual_normalized_sorted}"
