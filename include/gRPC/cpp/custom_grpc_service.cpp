//
// Created by surrogate on 24. 8. 20.
//

#include "custom_grpc_service.h"

grpc::Status ControlServiceImpl::MoveJ(grpc::ServerContext* context, const MoveJRequest* request, ControlDefaultReply* reply) {
    std::string prefix("Hello ");
    reply->set_message(prefix);
    reply->set_message(prefix + std::to_string(request->q_des()[0]) + std::to_string(request->q_des()[1]));

    // std::cout << "Received (MoveJ)" << std::endl;
    // std::cout << "q_desired = ["<< request->q_desired()[0] << " " << request->q_desired()[1] << "]" << std::endl;

    return grpc::Status::OK;
}


grpc::Status ControlServiceImpl::SetMobileState(grpc::ServerContext* context, const SetMobileStateRequest* request, MobileMotionData* reply) {
    // Write reply message
    reply->set_system_state(SystemState::SS_ON);
    reply->set_wheel_vel(request->wheel_vel());
    reply->set_steering_pos(request->steering_pos());

    // Update request_message
    resetRequestMessage();
    request_message->SetMobileStateRequest.command_id = request->command_id();
    request_message->SetMobileStateRequest.wheel_vel = request->wheel_vel();
    request_message->SetMobileStateRequest.steering_pos = request->steering_pos();

    // std::cout << "Received (SetMobileState)" S<< std::endl;

    return grpc::Status::OK;
}


grpc::Status MotionServiceImpl::GetMotionData(grpc::ServerContext* context, const EmptyRequest* request, MotionData* reply) {
    std::string prefix("Hello ");
    reply->set_message(prefix);

    return grpc::Status::OK;
}


grpc::Status SystemServiceImpl::SystemEmergencyStop(grpc::ServerContext* context, const ChangeSystemStateRequest* request, SystemStateReply* reply) {
    // Write reply message
    reply->set_command_id(request->command_id());
    reply->set_system_state(request->system_state());

    // Update request_message
    resetRequestMessage();
    request_message->ChangeSystemStateRequest.command_id = request->command_id();
    request_message->ChangeSystemStateRequest.system_state = request->system_state();

    std::cout << "Pressed emergency stop..." << std::endl;

    return grpc::Status::OK;
}


grpc::Status SystemServiceImpl::SystemReset(grpc::ServerContext* context, const ChangeSystemStateRequest* request, SystemStateReply* reply) {
    // Write reply message
    reply->set_command_id(request->command_id());
    reply->set_system_state(request->system_state());

    // Update request_message
    resetRequestMessage();
    request_message->ChangeSystemStateRequest.command_id = request->command_id();
    request_message->ChangeSystemStateRequest.system_state = request->system_state();

    std::cout << "Pressed reset..." << std::endl;

    return grpc::Status::OK;
}
