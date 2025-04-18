from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class CommandId(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    CMD_NULL: _ClassVar[CommandId]
    CMD_SYETEM_RESET: _ClassVar[CommandId]
    CMD_SYSTEM_EMERGENCY_STOP: _ClassVar[CommandId]
    CMD_SET_DESIRED_STATE: _ClassVar[CommandId]

class SystemState(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = ()
    SS_OFF: _ClassVar[SystemState]
    SS_ON: _ClassVar[SystemState]
    SS_RECOVERY: _ClassVar[SystemState]
    SS_EMERGENCY_STOP: _ClassVar[SystemState]
CMD_NULL: CommandId
CMD_SYETEM_RESET: CommandId
CMD_SYSTEM_EMERGENCY_STOP: CommandId
CMD_SET_DESIRED_STATE: CommandId
SS_OFF: SystemState
SS_ON: SystemState
SS_RECOVERY: SystemState
SS_EMERGENCY_STOP: SystemState

class EmptyRequest(_message.Message):
    __slots__ = ()
    def __init__(self) -> None: ...

class EmptyReply(_message.Message):
    __slots__ = ()
    def __init__(self) -> None: ...

class DefaultReply(_message.Message):
    __slots__ = ("command_id",)
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    command_id: CommandId
    def __init__(self, command_id: _Optional[_Union[CommandId, str]] = ...) -> None: ...
