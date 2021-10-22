CPMAddPackage("gh:arnavyc/Munit.cmake#origin/main")
munit_cmake_init()

include(DownloadMunitCPM)

add_executable(aes-ni-tests tests.c)
target_link_libraries(aes-ni-tests aes-ni munit internal-hexdump)

if (NOT "${INTEL_SDE_PATH}" STREQUAL "")
  set_target_properties(
    aes-ni-tests PROPERTIES CROSSCOMPILING_EMULATOR "${INTEL_SDE_PATH}"
  )
endif ()
munit_discover_tests(aes-ni-tests)

add_executable(aes-bs-tests aes-bs-tests.c)
target_link_libraries(aes-bs-tests aes-bs munit internal-hexdump)
munit_discover_tests(aes-bs-tests)

add_executable(aes-tests aes-tests.c)
target_link_libraries(aes-tests aes-c munit internal-hexdump)
munit_discover_tests(aes-tests)
