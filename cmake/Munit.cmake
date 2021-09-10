# SPDX-License-Identifier: CC0-1.0

#[=======================================================================[.rst:
Munit
-----

This module defines a function to help use the Munit test framework.

The :command:`munit_discover_tests` discovers tests by asking the compiled test
executable to enumerate its tests.  This does not require CMake to be re-run
when tests change.  However, it may not work in a cross-compiling environment,
and setting test properties is less convenient.

This command is intended to replace use of :command:`add_test` to register
tests, and will create a separate CTest test for each Munit test case.  Note
that this is in some cases less efficient, as common set-up and tear-down logic
cannot be shared by multiple test cases executing in the same instance.
However, it provides more fine-grained pass/fail information to CTest, which is
usually considered as more beneficial.  By default, the CTest test name is the
same as the Munit name; see also ``TEST_PREFIX`` and ``TEST_SUFFIX``.

.. command:: munit_discover_tests

  Automatically add tests with CTest by querying the compiled test executable
  for available tests::

    munit_discover_tests(target
                         [NO_FORK]
                         [SHOW_STDERR]
                         [FATAL_FAILURES]
                         [SINGLE]
                         [ITERATIONS n]
                         [WORKING_DIRECTORY dir]
                         [SEED s]
                         [LOG_VISIBLE level]
                         [LOG_FATAL level]
                         [TEST_PREFIX prefix]
                         [TEST_SUFFIX suffix]
                         [EXTRA_ARGS arg1...]
                         [PARAMS arg1...]
    )

  ``munit_discover_tests`` sets up a post-build command on the test executable
  that generates the list of tests by parsing the output from running the test
  with the ``--list`` argument.  This ensures that the full list of tests is
  obtained. Since test discovery occurs at build time, it is not necessary to
  re-run CMake when the list of tests changes.

  The options are:

  ``target``
    Specifies the Munit executable, which must be a known CMake executable
    target.  CMake will substitute the location of the built executable when
    running the test.

  ``NO_FORK``
    Disable forking for Munit test runner. Passes ``--no-fork`` to the
    executable.

  ``SHOW_STDERR``
    Show stderr even for successful tests for Munit test runner. Passes
    ``--no-fork`` to the executable.

  ``FATAL_FAILURES``
    Exit the test suite immediately if any tests fail instead of trying to
    also check the remaining suites. (Corresponds to ``--fatal-failures``).

  ``SINGLE``
    Corresponds to ``--single`` command line flag to Munit executable.

  ``ITERATIONS n``
    The iterations option allows you to run each test a N times (unless the
    test includes the ``MUNIT_TEST_OPTION_SINGLE_ITERATION`` flag).
    (Corresponds to ``--iterations n``).

  ``WORKING_DIRECTORY dir``
    Specifies the directory in which to run the discovered test cases. If this
    option is not provided, the current binary directory is used.

  ``SEED s``
    Specify the seed to pass to Munit executable. (Corresponds to ``--seed s``
    ).

  ``LOG_VISIBLE level``
    Corresponds to ``--log-visible level``.

  ``LOG_FATAL level``
    Corresponds to ``--log-fatal level``.

  ``TEST_PREFIX prefix``
    Specifies a ``prefix`` to be prepended to the name of each discovered test
    case.  This can be useful when the same test executable is being used in
    multiple calls to ``munit_discover_tests()``.

  ``TEST_SUFFIX suffix``
    Similar to ``TEST_PREFIX`` except the ``suffix`` is appended to the name
    of every discovered test case.  Both ``TEST_PREFIX`` and ``TEST_SUFFIX``
    may be specified.

  ``EXTRA_ARGS arg1...``
    Any extra arguments to pass on the command line to each test case.

  ``PARAMS arg1...``
    Pair of Key-Value pairs placed one after another in this list.
    Example: when PARAMS="a;b;c;d", the function passes
    ``--param a b --param c d`` to Munit executable.

#]=======================================================================]

function (munit_discover_tests TARGET)
  cmake_parse_arguments(
    ""
    "NO_FORK;SHOW_STDERR;FATAL_FAILURES;SINGLE"
    "ITERATIONS;WORKING_DIRECTORY;SEED;LOG_VISIBLE;LOG_FATAL;TEST_PREFIX;TEST_SUFFIX"
    "EXTRA_ARGS;PARAMS"
    ${ARGN}
  )

  if (NOT _WORKING_DIRECTORY)
    set(_WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
  endif ()

  list(LENGTH _PARAMS PARAMS_SIZE)
  math(EXPR PARAMS_SIZE_MODULO_2 "${PARAMS_SIZE} % 2")
  if (NOT PARAMS_SIZE_MODULO_2 EQUAL 0)
    message(FATAL_ERROR
      "Size of PARAMS variable in munit_discover_tests() is not multiple of 2.\n"
      "Value of PARAMS variable: ${_PARAMS}"
    )
  endif ()

  set(ctest_include_file "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_include.cmake")
  set(ctest_tests_file "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}_tests.cmake")

  get_property(emulator
    TARGET ${TARGET}
    PROPERTY CROSSCOMPILING_EMULATOR
  )

  add_custom_command(
    TARGET ${TARGET} POST_BUILD
    BYPRODUCTS "${ctest_tests_file}"
    COMMAND "${CMAKE_COMMAND}"
            -D "TEST_TARGET=${TARGET}"
            -D "TEST_EXECUTABLE=$<TARGET_FILE:${TARGET}>"
            -D "TEST_EXECUTOR=${emulator}"
            -D "TEST_WORKING_DIR=${_WORKING_DIRECTORY}"
            -D "TEST_ITERATIONS=${_ITERATIONS}"
            -D "TEST_SEED=${_SEED}"
            -D "TEST_SINGLE:BOOL=${_SINGLE}"
            -D "TEST_FATAL_FAILURES:BOOL=${_FATAL_FAILURES}"
            -D "TEST_SHOW_STDERR:BOOL=${_SHOW_STDERR}"
            -D "TEST_NO_FORK:BOOL=${_NO_FORK}"
            -D "TEST_LOG_VISIBLE=${_LOG_VISIBLE}"
            -D "TEST_LOG_FATAL=${_LOG_FATAL}"
            -D "TEST_PREFIX=${_TEST_PREFIX}"
            -D "TEST_SUFFIX=${_TEST_SUFFIX}"
            -D "TEST_PARAMS=${_PARAMS}"
            -D "TEST_EXTRA_ARGS=${_EXTRA_ARGS}"
            -D "MUNIT_CMAKE_PATH=${_MUNIT_CMAKE_SCRIPT}"
            -D "CTEST_FILE=${ctest_tests_file}"
            -P "${_MUNIT_DISCOVER_TESTS_SCRIPT}"
    VERBATIM
  )

  file(WRITE "${ctest_include_file}"
    "if(EXISTS \"${ctest_tests_file}\")\n"
    "  include(\"${ctest_tests_file}\")\n"
    "else()\n"
    "  add_test(${TARGET}_NOT_BUILT-${args_hash} ${TARGET}_NOT_BUILT-${args_hash})\n"
    "endif()\n"
  )

  # Add discovered tests to directory TEST_INCLUDE_FILES
  set_property(DIRECTORY
    APPEND PROPERTY TEST_INCLUDE_FILES "${ctest_include_file}"
  )
endfunction ()

function (munit_get_test_list EXECUTABLE LIST_OF_TESTS)
  cmake_parse_arguments(
    ""
    ""
    "CROSSCOMPILING_EMULATOR;OUTPUT_VARIABLE;ERRNO_VARIABLE;WORKING_DIRECTORY"
    ""
    ${ARGN}
  )

  execute_process(
    COMMAND ${CROSSCOMPILING_EMULATOR} "${EXECUTABLE}" --list
    OUTPUT_VARIABLE output
    RESULT_VARIABLE result
    WORKING_DIRECTORY "${_WORKING_DIRECTORY}"
  )

  string(REPLACE "\n" ";" output "${output}")
  set(${LIST_OF_TESTS} "${output}" PARENT_SCOPE)

  if (_ERRNO_VARIABLE)
    set(${_ERRNO_VARIABLE} "${result}" PARENT_SCOPE)
  endif ()

  if (_OUTPUT_VARIABLE)
    set(${_OUTPUT_VARIABLE} "${output}" PARENT_SCOPE)
  endif ()
endfunction ()

set(_MUNIT_CMAKE_SCRIPT
  ${CMAKE_CURRENT_LIST_DIR}/Munit.cmake
  CACHE INTERNAL "Munit full path to Munit.cmake helper file"
)

set(_MUNIT_DISCOVER_TESTS_SCRIPT
  ${CMAKE_CURRENT_LIST_DIR}/MunitAddTests.cmake
  CACHE INTERNAL "Munit full path to MunitAddTests.cmake helper file"
)
