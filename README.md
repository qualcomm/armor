ARMOR
=========================

ARMOR is a tool for checking backward source-level compatibility of C/C++ software libraries using Clang AST.

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

### Basic Usage

Clone the repository and run the build_binary.sh script to build the armor tool:

```bash
bash build_binary.sh --build
```

This will build armor.

**Important:** All header files and include paths must exist in both `project_root1` and `project_root2`.

### ARMOR Command-Line Interface

The tool is invoked directly using the `armor` binary:

```bash
./build/src/armor/armor [OPTIONS] projectroot1 projectroot2 [headers...]
```

#### Positional Arguments

* **projectroot1** (REQUIRED)  
  Path to the project root directory of the older version

* **projectroot2** (REQUIRED)  
  Path to the project root directory of the newer version

* **headers** (optional)  
  List of header files to compare between the two versions.  
  **Note:** Headers must exist in both project roots.
  
  Header path interpretation depends on whether `--header-dir` is provided:
  
  - **With `--header-dir`:**  
    Headers are treated as basenames (e.g., `foo.h`) under the specified subdirectory.  
    Example:
    ```bash
    --header-dir include/api foo.h bar.hpp
    ```
    This will compare:  
    - `projectroot1/include/api/foo.h` with `projectroot2/include/api/foo.h`  
    - `projectroot1/include/api/bar.hpp` with `projectroot2/include/api/bar.hpp`
  
  - **Without `--header-dir`:**  
    Headers must be relative paths from the project root.  
    Example:
    ```bash
    include/api/foo.h include/api/bar.hpp
    ```
    This will compare:  
    - `projectroot1/include/api/foo.h` with `projectroot2/include/api/foo.h`  
    - `projectroot1/include/api/bar.hpp` with `projectroot2/include/api/bar.hpp`

#### Options

* **-h, --help**  
  Print help message and exit

* **--header-dir TEXT**  
  Subdirectory under each project root containing headers

* **-r, --report-format TEXT:{html,json}**  
  Report format: `html` (default).  
  If `json` is provided, both HTML and JSON reports will be generated.

* **--dump-ast-diff**  
  Dump AST diff JSON files for debugging

* **-v, --version**  
  Display program version information and exit

* **--log-level TEXT:{ERROR,LOG,INFO,DEBUG}**  
  Set debug log level: ERROR, LOG, INFO (default), DEBUG

* **-I, --include-paths TEXT ...**  
  Include paths for header dependencies (relative to project roots).  
  **Note:** Include paths must exist in both project roots.  
  Example:
  ```bash
  -I path/to/include1 -I path/to/include2
  ```
  This will add:  
  - `projectroot1/path/to/include1` and `projectroot2/path/to/include1`  
  - `projectroot1/path/to/include2` and `projectroot2/path/to/include2`

* **-m, --macro-flags TEXT**  
  Macro flags to be passed for headers

#### Usage Examples

1. **Basic comparison with header directory:**
   ```bash
   ./build/src/armor/armor /path/to/old/project /path/to/new/project --header-dir include foo.h bar.h
   ```

2. **Comparison with relative header paths:**
   ```bash
   ./build/src/armor/armor /path/to/old/project /path/to/new/project include/api/foo.h include/api/bar.h
   ```

3. **Generate JSON report with include paths:**
   ```bash
   ./build/src/armor/armor /path/to/old/project /path/to/new/project \
     --header-dir include \
     --report-format json \
     -I dependencies/include \
     -I third_party/headers \
     foo.h
   ```
   This assumes both projects have `dependencies/include` and `third_party/headers` directories.

4. **Debug mode with AST diff dump:**
   ```bash
   ./build/src/armor/armor /path/to/old/project /path/to/new/project \
     --header-dir include \
     --dump-ast-diff \
     --log-level DEBUG \
     foo.h
   ```

Test suite
----------

All tests are located in the `src/tests` folder. The test suite uses pytest for test execution and validation.

### Running Tests

To run all tests, use the following command from the project root:

```bash
pytest
```

### Running Specific Tests

You can also run specific test files:

```bash
pytest src/tests/test_example.py
```

### Test Requirements

Ensure pytest and deepdiff packages are installed before running tests:

```bash
pip install pytest==8.4.1 deepdiff==8.5.0
```

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

Armor is licensed under the [BSD-3-Clause-Clear License](https://spdx.org/licenses/BSD-3-Clause-Clear.html). See [LICENSE.txt](LICENSE.txt) for the full license text.