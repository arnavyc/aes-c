cmake_policy(SET CMP0091 NEW)

cmake_minimum_required(VERSION 3.16...3.20 FATAL_ERROR)

project(
  aes-c
  LANGUAGES C
  VERSION 0.0.1
)

include(cmake/DefaultConfig.cmake)

# --- Section: Options ---

include(FetchContent)
include(CTest)

cmake_dependent_option(
  ${PROJECT_NAME}_BUILD_TESTING "Force building tests for ${PROJECT_NAME}." ON
  "BUILD_TESTING;CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

cmake_dependent_option(
  ${PROJECT_NAME}_INSTALL "Install all targets of ${PROJECT_NAME}." ON
  "CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME" OFF
)

set(INTEL_SDE_PATH
    ""
    CACHE STRING "Path to Intel Software Development Emulator"
)

include(CPM)
if (CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  find_program(HAS_CMAKE_FORMAT cmake-format)
  if (HAS_CMAKE_FORMAT)
    set(FORMAT_SKIP_CMAKE NO)
  else ()
    set(FORMAT_SKIP_CMAKE YES)
  endif ()

  CPMAddPackage(
    NAME Format.cmake
    VERSION 1.7.3
    GITHUB_REPOSITORY TheLartians/Format.cmake
    OPTIONS "FORMAT_SKIP_CMAKE ${FORMAT_SKIP_CMAKE}"
            "CMAKE_FORMAT_EXCLUDE cmake/*"
  )
endif ()

CPMAddPackage("gh:arnavyc/explicit-memset#origin/main")

# --- Section: Targets ---

set(PUBLIC_HEADER_DIR $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>)

add_library(public-incdir-default INTERFACE)
target_include_directories(public-incdir-default INTERFACE include)

# add_library(private-incdir-default INTERFACE)
# target_include_directories(private-incdir-default PUBLIC src)

add_library(aes-ni OBJECT src/aes-ni.c)
if (CMAKE_SYSTEM_PROCESSOR MATCHES "(x86)|(X86)|(amd64)|(AMD64)"
    AND CMAKE_C_COMPILER_ID MATCHES "(GNU)|(.*Clang)|(IntelLLVM)"
)
  target_compile_options(aes-ni PRIVATE -maes -msse -msse2 -mssse3)
endif ()
target_include_directories(aes-ni PUBLIC src)
# target_link_libraries(aes-ni PRIVATE internal-hexdump)
target_link_libraries(aes-ni PUBLIC public-incdir-default)

add_library(aes-c src/aes.c $<TARGET_OBJECTS:aes-ni> $<TARGET_OBJECTS:aes-bs>)
target_include_directories(
  aes-c
  PRIVATE src ${PUBLIC_HEADER_DIR} SYSTEM
  INTERFACE ${PUBLIC_HEADER_DIR}
            # $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
set_target_properties(
  aes-c PROPERTIES PUBLIC_HEADER include/ay/aes.h PRIVATE_HEADER
                                                  include/ay/aes/hedley.h
)
target_link_libraries(aes-c PRIVATE cpu-capability)

add_library(aes-bs OBJECT src/aes-bs.c)
target_include_directories(aes-bs PUBLIC src)
# target_link_libraries(aes-bs PRIVATE internal-hexdump)
target_link_libraries(aes-bs PUBLIC public-incdir-default)

add_library(internal-hexdump hexdump.c)
target_include_directories(internal-hexdump PUBLIC "${CMAKE_CURRENT_LIST_DIR}")

add_subdirectory(src/cpu-capability)

add_library(${PROJECT_NAME}::aes-c ALIAS aes-c)

include(Warnings)
foreach (i aes-c aes-ni aes-bs cpu-capability)
  target_add_compiler_options(${i} PREFIX ${PROJECT_NAME} TARGET_TYPE PRIVATE)
endforeach ()

# --- Section: Installation ---

if (${PROJECT_NAME}_INSTALL)
  include(CMakePackageConfigHelpers)
  install(
    TARGETS aes-c cpu-capability
    EXPORT ${PROJECT_NAME}-targets
    # Lines commented are not required as they are all set to CMAKE_INSTALL_*
    # variables by default if they are set.
    # LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    # RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ay
    PRIVATE_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ay/aes
  )

  write_basic_package_version_file(
    ${PROJECT_NAME}-config-version.cmake
    VERSION ${${PROJECT_NAME}_VERSION}
    COMPATIBILITY SameMinorVersion
  )

  install(
    EXPORT ${PROJECT_NAME}-targets
    FILE ${PROJECT_NAME}-targets.cmake
    NAMESPACE ${PROJECT_NAME}::
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )

  configure_file(
    ${PROJECT_NAME}-config.cmake.in ${PROJECT_NAME}-config.cmake @ONLY
  )
  install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake"
          "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake"
    DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
  )
endif ()

# --- Section: Testing ---

if (${PROJECT_NAME}_BUILD_TESTING)
  add_subdirectory(tests)
endif ()
