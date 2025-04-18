//
// Created by surrogate on 24. 7. 26.
//

#ifndef HELLO_RT_TASK_H
#define HELLO_RT_TASK_H

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

#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <trank/rtdk.h>

RT_TASK demo_task;

bool working_tictoc = false;

#endif //HELLO_RT_TASK_H
