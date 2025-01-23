# automatically generated by the FlatBuffers compiler, do not modify

# namespace: request

import flatbuffers
from flatbuffers.compat import import_numpy
np = import_numpy()

class Range(object):
    __slots__ = ['_tab']

    @classmethod
    def GetRootAs(cls, buf, offset=0):
        n = flatbuffers.encode.Get(flatbuffers.packer.uoffset, buf, offset)
        x = Range()
        x.Init(buf, n + offset)
        return x

    @classmethod
    def GetRootAsRange(cls, buf, offset=0):
        """This method is deprecated. Please switch to GetRootAs."""
        return cls.GetRootAs(buf, offset)
    # Range
    def Init(self, buf, pos):
        self._tab = flatbuffers.table.Table(buf, pos)

    # Range
    def Start(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(4))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

    # Range
    def Stop(self):
        o = flatbuffers.number_types.UOffsetTFlags.py_type(self._tab.Offset(6))
        if o != 0:
            return self._tab.Get(flatbuffers.number_types.Int32Flags, o + self._tab.Pos)
        return 0

def RangeStart(builder):
    builder.StartObject(2)

def Start(builder):
    RangeStart(builder)

def RangeAddStart(builder, start):
    builder.PrependInt32Slot(0, start, 0)

def AddStart(builder, start):
    RangeAddStart(builder, start)

def RangeAddStop(builder, stop):
    builder.PrependInt32Slot(1, stop, 0)

def AddStop(builder, stop):
    RangeAddStop(builder, stop)

def RangeEnd(builder):
    return builder.EndObject()

def End(builder):
    return RangeEnd(builder)
