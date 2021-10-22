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

# Include common module.
include(CMakeDependentOption)
include(GNUInstallDirs)

# MSVC: Set UTF-8 as source character set
# MSVC: Enable __cplusplus to report updated value for recent C++ standards.
# MSVC: Enable support for the Standard C++ exception handling model that safely
# unwinds stack objects
# MSVC: Make the compiler assume that operator new shall throw an exception on
# allocation failure
# MSVC: Enable a token-based preprocessor that conforms to C99 and C++11 and
# later standards
# MSVC: Specify standards conformance mode to the compiler (C++)
# MSVC: Selects strict volatile semantics as defined by the ISO C & C++
set(msvc_c_opts)
set(msvc_cxx_opts)
list(APPEND msvc_c_opts /source-charset:utf-8 /EHsc /Zc:preprocessor /volatile:iso)
list(APPEND msvc_cxx_opts /source-charset:utf-8 /Zc:__cplusplus /EHsc /Zc:throwingNew /Zc:preprocessor /permissive- /volatile:iso)
add_compile_options("$<$<C_COMPILER_ID:MSVC>:${msvc_c_opts}>")
add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:${msvc_cxx_opts}>")

# MSVC: Enable support for the Standard C++ exception handling model that safely
# unwinds stack objects
list(APPEND msvc_c_opts /EHsc)
list(APPEND msvc_cxx_opts /EHsc)

# MSVC: Make the compiler assume that operator new shall throw an exception on
# allocation failure
list(APPEND msvc_cxx_opts /Zc:throwingNew>)

# MSVC: Enable a token-based preprocessor that conforms to C99 and C++11 and
# later standards
list(APPEND msvc_c_opts /Zc:preprocessor>)
list(APPEND msvc_cxx_opts /Zc:preprocessor>)

# MSVC: Specify standards conformance mode to the compiler (C++)
list(APPEND "$<$<CXX_COMPILER_ID:MSVC>:/permissive->")

# MSVC: Selects strict volatile semantics as defined by the ISO C & C++
#add_compile_options("$<$<C_COMPILER_ID:MSVC>:/volatile:iso>")
#add_compile_options("$<$<CXX_COMPILER_ID:MSVC>:/volatile:iso>")
