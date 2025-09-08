API Compatibility Checker
=========================

API Compatibility Checker is a tool for checking backward source-level compatibility of C/C++ software libraries using Clang AST.

Contents
--------
1. [ About      ](#about)
2. [ Install    ](#install)
3. [ Usage      ](#usage)
4. [ Test suite ](#test-suite)
5. [Troubleshooting & Environment Setup](#troubleshoot)

About
-----
This tool analyzes two versions of a C/C++ header file and detects source-level API incompatibilities. It leverages Clang's AST to perform deep structural comparisons, helping maintainers ensure backward compatibility in evolving codebases.


Install
-------

    sudo apt update
    sudo apt install clang-14 make cmake
    sudo apt install libclang-common-14-dev libclang-14-dev llvm-14-dev


###### Requires

* make
* cmake
* llvm-14
* clang-14

###### Platforms

* Linux

Usage
-----

Clone the repository and run the build_binary.sh script with two header files as input:


    bash build_binary.sh <header_file1> <heder_file2>

This will build and run the compatibility checker on the provided headers.

Test suite
----------

Troubleshooting & Environment Setup
-----------------------------------
If you encounter build errors, ensure the following environment setup:

1. You have C++ version 11 or higher (e.g., 23):

    ls /usr/include/c++


2. The same C++ version is available under the platform-specific path:

    ls /usr/include/x86_64-linux-gnu/c++


3. You have Clang version 14 and subversion 14.0.0:

    ls /usr/include/clang

4. You only have LLVM version 14 installed (Clang 14 is only compatible with LLVM 14):

    ls /usr/lib | grep llvm-
## License

api_compatibility_checker is licensed under the [BSD-3-clause License](https://spdx.org/licenses/BSD-3-Clause.html). See [LICENSE.txt](LICENSE.txt) for the full license text.
