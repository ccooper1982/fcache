# automatically generated by the FlatBuffers compiler, do not modify

# namespace: request

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class StringValue(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = StringValue()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsStringValue(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # StringValue
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # StringValue
    def V(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.String(o + self._tab.Pos)
        return None

def StringValueStart(builder):
    builder.StartObject(1)

def Start(builder):
    StringValueStart(builder)

def StringValueAddV(builder, v):
    builder.PrependUOffsetTRelativeSlot(0, flatbuffers.number_types.UOffsetTFlags.py_type(v), 0)

def AddV(builder, v):
    StringValueAddV(builder, v)

def StringValueEnd(builder):
    return builder.EndObject()

def End(builder):
    return StringValueEnd(builder)
