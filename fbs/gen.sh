#!/bin/sh

../externals/flatbuffers/flatc --cpp -o ../server/include/fc/fbs *.fbs
../externals/flatbuffers/flatc --python -o ../apis/python/fc/fbs *.fbs