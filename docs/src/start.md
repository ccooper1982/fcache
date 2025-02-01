# Build and Run

## Build

!!! note
    - C++20 required
    - Tested on GCC 13.2 and GCC 14.2


1. From the repo root: `./build.sh` 
    - Clones then builds the submodules
    - Generates Flatbuffers code
2. Binary is in `server/release`


## Run

### Command Line Arguments

|Name|Description|Default|Notes|
|---|---|:--:|---|
|ip|The IP address the server will listen on|127.0.0.1||
|port|The port|1987||
|maxPayload|Max size, in bytes, of the WebSocket payload|16,384|Min: 64 bytes<br/>Max: 8 MB|


!!! warning
    - If a message exceeds `maxPayload` the client is disconnected
    - Increase this value on the command line to avoid disconnections
    - The absolute max is set in `fcache.cpp`. There is no particular reason for this value, other than it seems reasonable for most uses cases


Custom IP:
```
./fcache --ip=192.168.1.120
```

Default IP, custom port and max payload:
```
./fcache --port=2000 --maxPayload=1048576
```

Note, no unit suffixes permitted (i.e. "1MB", etc).