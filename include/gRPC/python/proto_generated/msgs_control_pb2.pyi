import msgs_common_pb2 as _msgs_common_pb2
from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class MoveJRequest(_message.Message):
    __slots__ = ("command_id", "q_des")
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    Q_DES_FIELD_NUMBER: _ClassVar[int]
    command_id: _msgs_common_pb2.CommandId
    q_des: _containers.RepeatedScalarFieldContainer[float]
    def __init__(self, command_id: _Optional[_Union[_msgs_common_pb2.CommandId, str]] = ..., q_des: _Optional[_Iterable[float]] = ...) -> None: ...

class MoveTRequest(_message.Message):
    __slots__ = ("command_id", "p_des")
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    P_DES_FIELD_NUMBER: _ClassVar[int]
    command_id: _msgs_common_pb2.CommandId
    p_des: _containers.RepeatedScalarFieldContainer[float]
    def __init__(self, command_id: _Optional[_Union[_msgs_common_pb2.CommandId, str]] = ..., p_des: _Optional[_Iterable[float]] = ...) -> None: ...

class SetMobileStateRequest(_message.Message):
    __slots__ = ("command_id", "wheel_vel", "steering_pos")
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    WHEEL_VEL_FIELD_NUMBER: _ClassVar[int]
    STEERING_POS_FIELD_NUMBER: _ClassVar[int]
    command_id: _msgs_common_pb2.CommandId
    wheel_vel: float
    steering_pos: float
    def __init__(self, command_id: _Optional[_Union[_msgs_common_pb2.CommandId, str]] = ..., wheel_vel: _Optional[float] = ..., steering_pos: _Optional[float] = ...) -> None: ...

class ControlDefaultReply(_message.Message):
    __slots__ = ("command_id", "message")
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    MESSAGE_FIELD_NUMBER: _ClassVar[int]
    command_id: _msgs_common_pb2.CommandId
    message: str
    def __init__(self, command_id: _Optional[_Union[_msgs_common_pb2.CommandId, str]] = ..., message: _Optional[str] = ...) -> None: ...
