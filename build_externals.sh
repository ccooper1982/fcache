#!/bin/bash

function get()
{
  git submodule add git@github.com:uNetworking/uWebSockets.git
  git submodule update --init --recursive

  git submodule add git@github.com:SergiusTheBest/plog.git
  git submodule add git@github.com:google/flatbuffers.git

  git submodule add git@github.com:martinus/unordered_dense.git

  cd ..
}


function build()
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


function build_externals()
{
  cd externals
  get

  if [ $? -eq 0 ]; then
    cd externals
    build

    if [ $? -eq 0 ]; then
      ## fbs gen
      cd fbs
      ./gen.sh
      cd -
    fi
  fi
}

