# automatically generated by the FlatBuffers compiler, do not modify

# namespace: response

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class KVClear(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = KVClear()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsKVClear(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # KVClear
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

def KVClearStart(builder):
    builder.StartObject(0)

def Start(builder):
    KVClearStart(builder)

def KVClearEnd(builder):
    return builder.EndObject()

def End(builder):
    return KVClearEnd(builder)
