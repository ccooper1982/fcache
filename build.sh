#!/bin/sh

source "./build_externals.sh"

build_externals

if [ $? -eq 0 ]; then
  cmake --build /home/callum/projects/fcache/build --config Release --target fcache
fi

