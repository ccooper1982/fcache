#!/bin/bash

if pgrep -x "fcache" #> /dev/null
then
  echo "FAIL: server already running"
else
  
  # to find base.py
  BASE=$(pwd)
  # to find Py API
  PY_API=$(pwd)/..
  
  export PYTHONPATH="$BASE:$PY_API"

  source ./useful.sh  

  run_server
    
  python3 -m unittest -v test_kv

  kill_server
  
fi
