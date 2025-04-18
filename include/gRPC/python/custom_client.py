from __future__ import print_function
import logging

import os
import sys
sys.path.append(os.path.dirname(os.path.abspath(__file__)) + '/proto_generated')

import grpc
from proto_generated import msgs_common_pb2, msgs_common_pb2_grpc
from proto_generated import msgs_control_pb2, msgs_control_pb2_grpc, control_pb2, control_pb2_grpc
from proto_generated import msgs_system_pb2, msgs_system_pb2_grpc, system_pb2, system_pb2_grpc
from proto_generated import motion_pb2, motion_pb2_grpc

def debug(func):
    def wrapper(*args, **kwargs):
        print('[' + func.__name__ + ']')
        func(*args, **kwargs)
    return wrapper

class CustomClient:
    def __init__(self, connect_ip='localhost', connect_port=50051):
        self._ip = connect_ip
        self._port = connect_port

    @debug
    def SetMobileState(self, wheel_vel, steering_pos):
        with grpc.insecure_channel("{0}:{1}".format(self._ip, self._port)) as channel:
            stub = control_pb2_grpc.ControlServiceStub(channel)
            res = stub.SetMobileState(msgs_control_pb2.SetMobileStateRequest(
                command_id=msgs_common_pb2.CommandId.CMD_SET_DESIRED_STATE,
                wheel_vel=wheel_vel, 
                steering_pos=steering_pos)
            )
        print("Client received ({0}): [{1:.4f} {2:.4f}]".format(res.system_state, res.wheel_vel, res.steering_pos))

    @debug
    def SystemEmergencyStop(self):
        with grpc.insecure_channel("{0}:{1}".format(self._ip, self._port)) as channel:
            stub = system_pb2_grpc.SystemServiceStub(channel)
            res = stub.SystemEmergencyStop(msgs_system_pb2.ChangeSystemStateRequest(
                command_id=msgs_common_pb2.CommandId.CMD_SET_DESIRED_STATE,
                system_state=msgs_common_pb2.SystemState.SS_EMERGENCY_STOP)
            )
        print("Client received ({0}, {1})".format(res.command_id, res.system_state))

    @debug
    def SystemReset(self):
        with grpc.insecure_channel("{0}:{1}".format(self._ip, self._port)) as channel:
            stub = system_pb2_grpc.SystemServiceStub(channel)
            res = stub.SystemReset(msgs_system_pb2.ChangeSystemStateRequest(
                command_id=msgs_common_pb2.CommandId.CMD_SET_DESIRED_STATE,
                system_state=msgs_common_pb2.SystemState.SS_RECOVERY)
            )
        print("Client received ({0}, {1})".format(res.command_id, res.system_state))

if __name__ == "__main__":
    # logging.basicConfig()
    custom_client = CustomClient()
    custom_client.SetMobileVel(0.123, 40123)