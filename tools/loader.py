# Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
# SPDX-License-Identifier: BSD-3-Clause
import json

def load_json(file_path):
    """
    Load a JSON file and return its contents as a dictionary.

    Args:
        file_path (str): The path to the JSON file.

    Returns:
        Dict: The contents of the JSON file.
    """
    with open(file_path, 'r') as file:
        return json.load(file)
