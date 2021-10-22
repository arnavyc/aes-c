cmake_minimum_required(VERSION 3.16...3.20 FATAL_ERROR)

project(aes-eg)

find_package(aes-c REQUIRED)

add_executable(aes-eg main.c)
if (aes-c_FOUND)
  message(STATUS "Found aes-c")
  target_link_libraries(aes-eg aes-c::aes-c)
endif ()
