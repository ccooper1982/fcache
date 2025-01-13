# Run

If building from source, the binary is in `server/release`.

## Command Line Arguments

|Name|Description|Default|
|---|---|:--:|
|ip|The IP address the server will listen on|127.0.0.1|
|port|The port|1987|
|maxPayload|Max size, in bytes, of the WebSocket payload. Messages exceeding this are ignored|2048|


Custom IP:
```
./fcache --ip=192.168.1.120
```

Custom port and max payload:
```
./fcache --port=2000 --maxPayload=8192
```