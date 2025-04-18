//
// Created by surrogate on 24. 8. 2.
//

#ifndef HELLO_GRPC_H
#define HELLO_GRPC_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/mman.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <time.h>
#include <sys/ioctl.h>
#include <math.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include <inttypes.h>

#include <sys/types.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include "../../../include/SharedMemory/shared_memory.h"

#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <trank/rtdk.h>

#include "../../../include/gRPC/cpp/custom_server.h"
//#include "../../../src/Test_Dynamixel/src/EL6021.h"
#include "MarchSurrogate.h"

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");
CustomServer custom_server(absl::GetFlag(FLAGS_port));

RT_TASK demo_task;

bool working_tictoc = false;


#pragma region jaehyun 240820

RT_TASK ecat_task;
bool working_ecat = false;


MarchSurrogate ec;  // Beckhoff EL6021 + Dynamixel class

double wheel_velocity=0, steering_angle=0;
int dxl_zero_position, dxl_target_position, dxl_current_position;
int target_velocity=0;

int blc_control_command, blc_state;
double blc_torque_command, blc_torque_current;
double blc_imu_x, blc_imu_y, blc_imu_z, blc_imu_gy;
double blc_steering;
int16_t blc_velocity;

double rc_steering;
int rc_velocity;

RTIME start_time;
double duration_time, loop_frequency=0;

static void ecat_func(void *arg);

#pragma endregion



#endif //HELLO_GRPC_H
