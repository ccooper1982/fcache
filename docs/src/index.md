fcache is a FlatBuffers cache over WebSockets.

There is support for key-values and lists, with more coming soon.

FlatBuffers offer zero-copy deserialising: when the server receives data, it can deserialise without an intermediate step which requires allocating memory (as with ProtoBuf), and is considerably more compact than JSON.