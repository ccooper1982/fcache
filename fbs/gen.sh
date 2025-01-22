#!/bin/sh

../externals/flatbuffers/flatc --cpp -o ../server/include/fc/fbs *.fbs

# Bug with flatc: if we do *.fbs the kv_request.fbs and kv_response.fbs are ignored, 
# with KVSet.py files being empty. 
# see https://github.com/google/flatbuffers/issues/5692
../externals/flatbuffers/flatc --python -o ../apis/python/fc/fbs request.fbs kv_request.fbs list_request.fbs
../externals/flatbuffers/flatc --python -o ../apis/python/fc/fbs response.fbs kv_response.fbs list_response.fbs
../externals/flatbuffers/flatc --python -o ../apis/python/fc/fbs common.fbs