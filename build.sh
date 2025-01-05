#!/bin/bash

source ./build_externals.sh

build_externals

if [ $? -eq 0 ]; then
  mkdir -p build
  cmake --build build --config Release --target fcache
fi

