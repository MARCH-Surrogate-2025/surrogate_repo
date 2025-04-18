#ifndef TEST_BALANCER_H
#define TEST_BALANCER_H

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

#include "CustomBalancer.h"


RT_TASK ecat_task, ecat_task2;
bool working_ecat = false;
bool working_ecat2 = false;

CustomBalancer ec;

RTIME start_time;
double duration_time, loop_frequency=0;

// UART command variables
int blc_control_command;
double blc_torque_command;
int blc_state;
double blc_torque_current;
double blc_imu_x;
double blc_imu_y;
double blc_imu_z;
double blc_imu_gy;
double blc_steering;
int16_t blc_velocity;
uint8_t sw4=0, sw5=0, sw6=6;

static void ecat_func(void *arg);
static void ecat_func2(void *arg);

#endif