#!/bin/bash


function run_server()
{
  #CONFIG=$(pwd)/server.jsonc
  # ./fcache "--config=${CONFIG}" > /dev/null &

  cd ../../../server/release > /dev/null
  ./fcache  > /dev/null &
  cd - > /dev/null

  while ! pgrep -f "fcache" > /dev/null; do
    sleep 0.5
  done
}


function kill_server()
{
  pkill fcache > /dev/null
  wait $!
}