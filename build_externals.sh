#!/bin/sh

get()
{
  git submodule add git@github.com:uNetworking/uWebSockets.git
  git submodule update --init --recursive

  git submodule add git@github.com:SergiusTheBest/plog.git
  git submodule add git@github.com:google/flatbuffers.git

  git submodule add git@github.com:martinus/unordered_dense.git

  cd ..
}

build()
{
  echo -- Building uWebSockets --
  cd uWebSockets
  make default
  make install prefix= DESTDIR=./
  
  cd -
  echo -- FlatBuffers --
  cd flatbuffers

  cmake . -G "Unix Makefiles" -D FLATBUFFERS_STATIC_FLATC=ON -D FLATBUFFERS_BUILD_TESTS=OFF
  make -j

  cd ../..
}

##

cd externals
get

cd externals
build

## fbs gen
cd fbs
./gen.sh