# Copyright 2021 arnavyc <arnavyc@outlook.com>
#
# SPDX-License-Identifier: 0BSD

include(FetchContent)
FetchContent_Declare(munit
  GIT_REPOSITORY https://github.com/nemequ/munit.git
  GIT_TAG master
	CONFIGURE_COMMAND ""
	BUILD_COMMAND ""
)
FetchContent_Populate(munit)

add_library(munit ${munit_SOURCE_DIR}/munit.c)
target_include_directories(munit SYSTEM PUBLIC ${munit_SOURCE_DIR})
