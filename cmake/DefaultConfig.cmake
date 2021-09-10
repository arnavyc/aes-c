# SPDX-FileCopyrightText: 2021 arnavyc
#
# SPDX-License-Identifier: 0BSD

# Sets default, "sane" compiler and CMake options in the project

# Converts relative paths in target_sources() to absolute paths.
cmake_policy(SET CMP0076 NEW)

# Only interpret if() arguments as variables or keywords when unquoted.
cmake_policy(SET CMP0054 NEW)

# Add cmake/ directory to $CMAKE_MODULE_PATH
list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

# Include GNUInstallDirs module.
include(GNUInstallDirs)

# Set UTF-8 as source character set in MSVC
add_compile_options("$<$<C_COMPILER_ID:MSVC>:/source-charset:utf-8>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/source-charset:utf-8>")

# MSVC: Enable __cplusplus to report updated value for recent C++ standards.
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>")

