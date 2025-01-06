fcache is a data cache, using Google's FlatBuffers over WebSockets.

There is support for key-values, with containers such as arrays and lists, coming soon.

FlatBuffers offer zero-copy deserialising: when the server receives data, it can deserialise without an intermediate step which requires allocating memory (as with ProtoBuf), and is considerably more compact than JSON.