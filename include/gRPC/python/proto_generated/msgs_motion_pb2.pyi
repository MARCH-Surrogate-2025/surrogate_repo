import msgs_common_pb2 as _msgs_common_pb2
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class MotionData(_message.Message):
    __slots__ = ("message",)
    MESSAGE_FIELD_NUMBER: _ClassVar[int]
    message: str
    def __init__(self, message: _Optional[str] = ...) -> None: ...

class MobileMotionData(_message.Message):
    __slots__ = ("system_state", "wheel_vel", "steering_pos")
    SYSTEM_STATE_FIELD_NUMBER: _ClassVar[int]
    WHEEL_VEL_FIELD_NUMBER: _ClassVar[int]
    STEERING_POS_FIELD_NUMBER: _ClassVar[int]
    system_state: _msgs_common_pb2.SystemState
    wheel_vel: float
    steering_pos: float
    def __init__(self, system_state: _Optional[_Union[_msgs_common_pb2.SystemState, str]] = ..., wheel_vel: _Optional[float] = ..., steering_pos: _Optional[float] = ...) -> None: ...
