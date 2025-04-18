/*
 *
 * Copyright 2015 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#include "control.grpc.pb.h"
#include "motion.grpc.pb.h"

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

using namespace surrogate::grpc;

class CustomClient {
 public:
  CustomClient(std::shared_ptr<Channel> channel)
      : stub_control(ControlService::NewStub(channel))
      , stub_motion(MotionService::NewStub(channel)) {}

  // Assembles the client's payload, sends it and presents the response back
  // from the server.
  std::string MoveJ(const double* q_desired) {
    // Data we are sending to the server.
    MoveJRequest request;

    request.add_q_desired(q_desired[0]);
    request.add_q_desired(q_desired[1]);

    // Container for the data we expect from the server.
    ControlDefaultReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_control->MoveJ(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC failed";
    }
  }

  std::string SetMobileVel(const double wheel_vel, const double steering_pos) {
    // Data we are sending to the server.
    SetMobileVelRequest request;

    request.set_wheel_vel(wheel_vel);
    request.set_steering_pos(steering_pos);

    // Container for the data we expect from the server.
    ControlDefaultReply reply;

    // Context for the client. It could be used to convey extra information to
    // the server and/or tweak certain RPC behaviors.
    ClientContext context;

    // The actual RPC.
    Status status = stub_control->SetMobileVel(&context, request, &reply);

    // Act upon its status.
    if (status.ok()) {
      return reply.message();
    } else {
      std::cout << status.error_code() << ": " << status.error_message() << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<ControlService::Stub> stub_control;
  std::unique_ptr<MotionService::Stub> stub_motion;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  // Instantiate the client. It requires a channel, out of which the actual RPCs
  // are created. This channel models a connection to an endpoint specified by
  // the argument "--target=" which is the only expected argument.
  std::string target_str = absl::GetFlag(FLAGS_target);
  // We indicate that the channel isn't authenticated (use of
  // InsecureChannelCredentials()).
  CustomClient custom(grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  double q_desired[2] = {1.2934, -2.345123};
  std::string reply_1 = custom.MoveJ(q_desired);
  std::cout << "Greeter received: " << reply_1 << std::endl;

  double wheel_vel = 2.2934;
  double steering_pos = -3.345123;
  std::string reply_2 = custom.SetMobileVel(wheel_vel, steering_pos);
  std::cout << "Greeter received: " << reply_2 << std::endl;

  return 0;
}
