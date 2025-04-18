import msgs_common_pb2 as _msgs_common_pb2
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class ChangeSystemStateRequest(_message.Message):
    __slots__ = ("command_id", "system_state")
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    SYSTEM_STATE_FIELD_NUMBER: _ClassVar[int]
    command_id: _msgs_common_pb2.CommandId
    system_state: _msgs_common_pb2.SystemState
    def __init__(self, command_id: _Optional[_Union[_msgs_common_pb2.CommandId, str]] = ..., system_state: _Optional[_Union[_msgs_common_pb2.SystemState, str]] = ...) -> None: ...

class SystemStateReply(_message.Message):
    __slots__ = ("command_id", "system_state")
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    SYSTEM_STATE_FIELD_NUMBER: _ClassVar[int]
    command_id: _msgs_common_pb2.CommandId
    system_state: _msgs_common_pb2.SystemState
    def __init__(self, command_id: _Optional[_Union[_msgs_common_pb2.CommandId, str]] = ..., system_state: _Optional[_Union[_msgs_common_pb2.SystemState, str]] = ...) -> None: ...
