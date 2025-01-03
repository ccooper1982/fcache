# fcache

# Externals

## uWebSockets
`cd externals`

`git submodule add git@github.com:uNetworking/uWebSockets.git`

`git submodule update --init --recursive`

`cd uWebSockets`

`make default`

`make install prefix= DESTDIR=./`

`cd ../..`

## plog
`git submodule add git@github.com:SergiusTheBest/plog.git`


## flatbuffers
`git submodule add git@github.com:google/flatbuffers.git`

`cd flatbuffers`

`cmake -G "Unix Makefiles" -D FLATBUFFERS_STATIC_FLATC=ON -D FLATBUFFERS_BUILD_TESTS=OFF`
`make -j`


## ankerl
`git submodule add git@github.com:martinus/unordered_dense.git`

## Generate Flat Buffers Code

`cd ../../fbs`

`./gen.sh`