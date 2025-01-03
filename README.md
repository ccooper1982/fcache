# fcache

fcache is a data cache, using Google's FlatBuffers over WebSockets.

There is support for:

- Key Values
- Arrays
- Lists

More will be added in future releases.

<br/>

# Python Client
TODO

<br/>

# Why FlatBuffers
Flat Buffers offer zero-copy deserialising: when the server receives data, it can be deserialised without an intermediate step which requires allocating memory (such as ProtoBuf), and is considerably more compact than JSON.

