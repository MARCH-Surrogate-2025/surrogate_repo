//
// Created by surrogate on 24. 8. 20.
//

#ifndef CUSTOM_GRPC_SERVICE_H
#define CUSTOM_GRPC_SERVICE_H

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>

#include "control.grpc.pb.h"
#include "motion.grpc.pb.h"
#include "system.grpc.pb.h"

#include "custom_request_message.h"

// Service (namespace)
using namespace surrogate::grpc;


class BaseServiceImpl {
public:
    RequestMessage* request_message;
    void setRequestMessage(RequestMessage &in) {request_message = &in; resetRequestMessage();};
    void resetRequestMessage() {memset(request_message->byte, 0x00, MAX_MESSAGE_SIZE);};
};


// Logic and data behind the server's behavior.
class ControlServiceImpl final : public BaseServiceImpl, public ControlService::Service {
public:
    grpc::Status MoveJ(grpc::ServerContext* context, const MoveJRequest* request, ControlDefaultReply* reply) override;
    grpc::Status SetMobileState(grpc::ServerContext* context, const SetMobileStateRequest* request, MobileMotionData* reply) override;
};


class MotionServiceImpl final : public BaseServiceImpl, public MotionService::Service {
public:
    grpc::Status GetMotionData(grpc::ServerContext* context, const EmptyRequest* request, MotionData* reply) override;
};


class SystemServiceImpl final : public BaseServiceImpl, public SystemService::Service {
public:
    grpc::Status SystemEmergencyStop(grpc::ServerContext* context, const ChangeSystemStateRequest* request, SystemStateReply* reply) override;
    grpc::Status SystemReset(grpc::ServerContext* context, const ChangeSystemStateRequest* request, SystemStateReply* reply) override;
};

#endif //CUSTOM_GRPC_SERVICE_H
