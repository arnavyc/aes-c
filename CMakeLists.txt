cmake_policy(SET CMP0091 NEW)

cmake_minimum_required(VERSION 3.16...3.20 FATAL_ERROR)

project(aes-c VERSION 0.1.0 LANGUAGES C)

include(cmake/DefaultConfig.cmake)

####################
# Section: Options #
####################

include(CMakeDependentOption)
include(FetchContent)
include(CTest)

cmake_dependent_option(${PROJECT_NAME}_BUILD_TESTING
  "Force building tests for ${PROJECT_NAME}."
  ON
  "BUILD_TESTING;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME"
  OFF
)

set(INTEL_SDE_PATH "" CACHE STRING "Path to Intel Software Development Emulator")

FetchContent_Declare(explicit-memset
  GIT_REPOSITORY https://github.com/arnavyc/explicit-memset
  GIT_TAG origin/main
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(explicit-memset)

####################
# Section: Targets #
####################

add_library(public-incdir-default INTERFACE)
target_include_directories(public-incdir-default INTERFACE include)

#add_library(private-incdir-default INTERFACE)
#target_include_directories(private-incdir-default PUBLIC src)

add_library(aes-ni src/aes-ni.c)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)" AND CMAKE_C_COMPILER_ID MATCHES "(GNU)|(.*Clang)|(IntelLLVM)")
  target_compile_options(aes-ni PRIVATE -maes -msse -msse2 -mssse3)
endif ()
target_include_directories(aes-ni PUBLIC src)
target_link_libraries(aes-ni PRIVATE internal-hexdump)
target_link_libraries(aes-ni PUBLIC public-incdir-default)

add_library(aes-lib src/aes.c)
target_include_directories(aes-lib PUBLIC src)
set_target_properties(aes-lib PROPERTIES PUBLIC_HEADER include/ay/aes.h)
target_link_libraries(aes-lib PRIVATE internal-hexdump aes-ni aes-bs cpu-capability)
target_link_libraries(aes-lib PUBLIC public-incdir-default)

add_library(aes-bs src/aes-bs.c)
target_include_directories(aes-bs PUBLIC src)
target_link_libraries(aes-bs PRIVATE internal-hexdump)
target_link_libraries(aes-bs PUBLIC public-incdir-default)

add_library(internal-hexdump hexdump.c)
target_include_directories(internal-hexdump PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

#add_library(aes-c::aes-c ALIAS aes-c)
#
#include(CMakePackageConfigHelpers)
#install(
#  TARGETS aes-c
#  EXPORT aes-c_targets
#  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
#  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
#  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
#  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
#)
#
#write_basic_package_version_file(
#  aes-cConfigVersion.cmake
#  VERSION ${${PROJECT_NAME}_VERSION}
#  COMPATIBILITY SameMinorVersion
#)
#
#install(
#  EXPORT aes-c_targets
#  FILE aes-c_targets.cmake
#  NAMESPACE aes-c::
#  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cmake/aes-c"
#)
#
#configure_file(aes-cConfig.cmake.in aes-cConfig.cmake @ONLY)
#install(
#  FILES
#    "${CMAKE_CURRENT_BINARY_DIR}/aes-cConfig.cmake"
#    "${CMAKE_CURRENT_BINARY_DIR}/aes-cConfigVersion.cmake"
#  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/cmake/aes-c"
#)

add_subdirectory(src/cpu-capability)

####################
# Section: Testing #
####################

if (${PROJECT_NAME}_BUILD_TESTING)
  add_subdirectory(tests)
endif ()

