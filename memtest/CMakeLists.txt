cmake_minimum_required(VERSION 3.20)

include_directories(
  "../externals/plog/include"
  "../server/include")


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/release)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_STANDARD 20)

add_executable(mem
  "mem.cpp")


target_compile_options(mem PRIVATE -Wall)

