//
// Created by surrogate on 24. 8. 19.
//

#ifndef CUSTOM_REQUEST_MESSAGE_H
#define CUSTOM_REQUEST_MESSAGE_H

#include "control.grpc.pb.h"
#include "motion.grpc.pb.h"

// Service (namespace)
using namespace surrogate::grpc;

#define MAX_MESSAGE_SIZE 1024

struct DefaultRequestStruct {
        CommandId command_id;
    }__attribute__((packed));

struct ChangeSystemStateRequestStruct {
        CommandId command_id;
        SystemState system_state;
    }__attribute__((packed));

struct SetMobileStateRequestStruct {
        CommandId command_id;
        float wheel_vel;
        float steering_pos;
    }__attribute__((packed));

union RequestMessage{
    unsigned char byte[MAX_MESSAGE_SIZE];
    DefaultRequestStruct DefaultRequest;
    ChangeSystemStateRequestStruct ChangeSystemStateRequest;
    SetMobileStateRequestStruct SetMobileStateRequest;
};

#endif //CUSTOM_REQUEST_MESSAGE_H
