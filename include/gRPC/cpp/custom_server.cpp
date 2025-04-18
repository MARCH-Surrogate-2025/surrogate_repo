//
// Created by surrogate on 24. 8. 1.
//

#include "custom_server.h"


void CustomServer::InitServer() {
    server_address = absl::StrFormat("0.0.0.0:%d", port);
    // server_address = absl::StrFormat("192.168.0.0:%d", port);

    grpc::EnableDefaultHealthCheckService(true);
    grpc::reflection::InitProtoReflectionServerBuilderPlugin();

    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&control_service);
    builder.RegisterService(&motion_service);
    builder.RegisterService(&system_service);
}


void CustomServer::RunServer() {

    InitServer();

    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());

    // Finally assemble the server.
    server = std::unique_ptr<grpc::Server>(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;

    // Wait for the server to shutdown. Note that some other thread must be
    // server->Wait();
}

