import msgs_common_pb2 as _msgs_common_pb2
from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Optional as _Optional

DESCRIPTOR: _descriptor.FileDescriptor

class Frame(_message.Message):
    __slots__ = ("fpos",)
    FPOS_FIELD_NUMBER: _ClassVar[int]
    fpos: _containers.RepeatedScalarFieldContainer[float]
    def __init__(self, fpos: _Optional[_Iterable[float]] = ...) -> None: ...

class FTsensorFrame(_message.Message):
    __slots__ = ("translation", "rotation")
    TRANSLATION_FIELD_NUMBER: _ClassVar[int]
    ROTATION_FIELD_NUMBER: _ClassVar[int]
    translation: float
    rotation: float
    def __init__(self, translation: _Optional[float] = ..., rotation: _Optional[float] = ...) -> None: ...

class JointPos(_message.Message):
    __slots__ = ("jpos",)
    JPOS_FIELD_NUMBER: _ClassVar[int]
    jpos: _containers.RepeatedScalarFieldContainer[float]
    def __init__(self, jpos: _Optional[_Iterable[float]] = ...) -> None: ...

class TaskPos(_message.Message):
    __slots__ = ("tpos",)
    TPOS_FIELD_NUMBER: _ClassVar[int]
    tpos: _containers.RepeatedScalarFieldContainer[float]
    def __init__(self, tpos: _Optional[_Iterable[float]] = ...) -> None: ...
