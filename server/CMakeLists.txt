cmake_minimum_required(VERSION 3.20)

include_directories(
  "./include"
  "../externals/uWebSockets/uSockets/src"
  "../externals/uWebSockets/include"
  "../externals/flatbuffers/include"
  "../externals/plog/include"
  "../externals/unordered_dense/include")

link_directories(
  "../externals/flatbuffers"
  "../externals/uWebSockets/uSockets")


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# build Flatbuffers
# set(FLATBUFFERS_SRC_DIR ${CMAKE_SOURCE_DIR}/externals/flatbuffers)

# add_custom_command(
#   OUTPUT Makefile
#   COMMAND cmake -G "Unix Makefiles" -D FLATBUFFERS_STATIC_FLATC=ON -D FLATBUFFERS_BUILD_TESTS=OFF
#   WORKING_DIRECTORY ${FLATBUFFERS_SRC_DIR}
#   COMMENT "Configuring flatbuffers"
#   )

# add_custom_target(libflatbuffers
#   COMMAND make -j
#   DEPENDS Makefile
#   BYPRODUCTS libflatbuffers.a  
#   WORKING_DIRECTORY ${FLATBUFFERS_SRC_DIR}
#   COMMENT "Building flatbuffers"
#   )


## fcache
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_SOURCE_DIR}/debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_SOURCE_DIR}/release)
set(CMAKE_CXX_STANDARD_REQUIRED)
set(CMAKE_CXX_STANDARD 20)

add_executable(fcache
  "fcache.cpp"
  "src/Server.cpp"
  "src/KvHandler.cpp"
  "src/ListHandler.cpp"
  "src/Map.cpp")

#add_dependencies(fcache libflatbuffers)

target_compile_options(fcache PRIVATE -Wall)
target_link_libraries(fcache PRIVATE "" -l:uSockets.a -lflatbuffers -lz)

