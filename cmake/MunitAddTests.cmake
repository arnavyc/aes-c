# SPDX-License-Identifier: CC0-1.0

set(tests)

include("${MUNIT_CMAKE_PATH}")

function(add_command NAME)
  set(args "")
  # use ARGV* instead of ARGN, because ARGN splits arrays into multiple arguments
  math(EXPR number_of_args ${ARGC}-1)
  foreach(i RANGE 1 ${number_of_args})
    set(arg "${ARGV${i}}")
    if(arg MATCHES "[^-./:a-zA-Z0-9_]")
      # if argument contains any character other than alphanumeric characters
      # along with '.', '/', ':' & '_'; use a bracket argument so that no
      # unwanted expansion takes place.
      set(args "${args} [==[${arg}]==]") # form a bracket_argument
    else()
      set(args "${args} ${arg}")
    endif()
  endforeach()
  set(script "${script}${NAME}(${args})\n" PARENT_SCOPE)
endfunction()

function(add_command_unescaped NAME)
  set(args "")
  foreach(arg ${ARGN})
    set(args "${args} ${arg}")
  endforeach()
  set(script "${script}${NAME}(${args})\n" PARENT_SCOPE)
endfunction()

if (NOT EXISTS "${TEST_EXECUTABLE}")
  message(FATAL_ERROR
    "Specified test executable '${TEST_EXECUTABLE}' not found."
  )
endif ()

munit_get_test_list(
  "${TEST_EXECUTABLE}"
  LIST_OF_TESTS
  WORKING_DIRECTORY "${_WORKING_DIRECTORY}"
  ERRNO_VARIABLE LISTCMD_ERRNO
  OUTPUT_VARIABLE LISTCMD_OUTPUT
  CROSSCOMPILING_EMULATOR "${TEST_EXECUTOR}"
)

if (NOT ${LISTCMD_ERRNO} EQUAL 0)
  message(FATAL_ERROR
    "Error running test executable '${TEST_EXECUTABLE}':\n"
    "  Result: ${LISTCMD_ERRNO}\n"
    "  Output: ${LISTCMD_OUTPUT}\n"
  )
endif ()

set(common_args)
if (TEST_ITERATIONS)
  list(APPEND common_args "--iterations" "${TEST_ITERATIONS}")
endif ()
if (TEST_NO_FORK)
  list(APPEND common_args "--no-fork")
endif ()
if (TEST_SHOW_STDERR)
  list(APPEND common_args "--show-stderr")
endif ()
if (TEST_FATAL_FAILURES)
  list(APPEND common_args "--fatal-failures")
endif ()
if (TEST_SINGLE)
  list(APPEND common_args "--single")
endif ()
if (TEST_SEED)
  list(APPEND common_args "--seed" "${TEST_SEED}")
endif ()

if (TEST_PARAMS)
  list(LENGTH TEST_PARAMS PARAMS_SIZE)
  math(EXPR PARAMS_SIZE_MODULO_2 "${PARAMS_SIZE} % 2")

  foreach (i RANGE 1 "${PARAMS_SIZE}" 2)
    list(POP_FRONT TEST_PARAMS KEY VAL)
    list(APPEND common_args "--param" "${KEY}" "${VAL}")
  endforeach ()
endif ()

if (TEST_LOG_VISIBLE)
  list(APPEND common_args "--log-visible" "${TEST_LOG_VISIBLE}")
endif ()

if (TEST_LOG_FATAL)
  list(APPEND common_args "--log-fatal" "${TEST_LOG_FATAL}")
endif ()

list(APPEND common_args ${TEST_EXTRA_ARGS})

foreach (line ${LISTCMD_OUTPUT})
  set(test "${line}")
  set(ctest_test "${TEST_PREFIX}${test}${TEST_SUFFIX}")

  add_command(add_test "${ctest_test}"
    ${TEST_EXECUTOR}
    "${TEST_EXECUTABLE}"
    "${test}"
    ${common_args}
  )
  add_command(set_tests_properties
    "${ctest_test}"
    PROPERTIES
    WORKING_DIRECTORY "${TEST_WORKING_DIR}"
  )

  list(APPEND tests "${ctest_test}")
endforeach ()

add_command(set tests "${tests}")

file(WRITE "${CTEST_FILE}" "${script}")

