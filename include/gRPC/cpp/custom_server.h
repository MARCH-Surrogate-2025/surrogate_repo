//
// Created by surrogate on 24. 8. 1.
//

#ifndef CUSTOM_SERVER_H
#define CUSTOM_SERVER_H

#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include "control.grpc.pb.h"
#include "motion.grpc.pb.h"
#include "system.grpc.pb.h"

#include "custom_request_message.h"
#include "custom_grpc_service.h"

using namespace surrogate::grpc;

class CustomServer final {
public:
    CustomServer(uint16_t _port) {
        port = _port;
        control_service.setRequestMessage(request_message);
        motion_service.setRequestMessage(request_message);
        system_service.setRequestMessage(request_message);
    };
    ~CustomServer() {};

public:
    RequestMessage request_message;
private:
    uint16_t port;
    
    ControlServiceImpl control_service;
    MotionServiceImpl motion_service;
    SystemServiceImpl system_service;

    std::string server_address;
    grpc::ServerBuilder builder;

    std::unique_ptr<grpc::Server> server;
public:
    void InitServer();
    void RunServer();
};

#endif //CUSTOM_SERVER_H
