# automatically generated by the FlatBuffers compiler, do not modify

# namespace: response

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class ListDelete(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = ListDelete()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsListDelete(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # ListDelete
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

def ListDeleteStart(builder):
    builder.StartObject(0)

def Start(builder):
    ListDeleteStart(builder)

def ListDeleteEnd(builder):
    return builder.EndObject()

def End(builder):
    return ListDeleteEnd(builder)
