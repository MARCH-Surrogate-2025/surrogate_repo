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

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");
CustomServer custom_server(absl::GetFlag(FLAGS_port));

RT_TASK demo_task;

bool working_tictoc = false;

#endif //HELLO_GRPC_H
