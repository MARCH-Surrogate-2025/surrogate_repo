#ifndef Test_Wheel_Test_Wheel_H
#define Test_Wheel_Test_Wheel_H

#ifndef __XENO__
#define __XENO__
#endif

// define - text color
#define KNRM    "\033[0m"
#define KRED    "\033[1;31m"
#define KGRN    "\033[1;32m"
#define KYEL    "\033[1;33m"
#define KBLU    "\033[1;34m"
#define KMAG    "\033[1;35m"
#define KCYN    "\033[1;36m"
#define KWHT    "\033[1;37m"
#define PRINT_NRM   rt_printf("%s", KNRM);
#define PRINT_RED   rt_printf("%s", KRED);
#define PRINT_GRN   rt_printf("%s", KGRN);
#define PRINT_YEL   rt_printf("%s", KYEL);
#define PRINT_BLU   rt_printf("%s", KBLU);
#define PRINT_MAG   rt_printf("%s", KMAG);
#define PRINT_CYN   rt_printf("%s", KCYN);
#define PRINT_WHT   rt_printf("%s", KWHT);

// define - SOEM
#define EC_TIMEOUTMON 500

// ------------------------------- //

// include
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
#include <stdint.h>

// xenomai //
#include <xeno_config.h>
#include <alchemy/task.h>
#include <alchemy/timer.h>
#include <trank/rtdk.h>

// SOEM //
#include "ethercat.h"

// Custom //
#include "../utils/CustomECAT/PDO.h"
#include "../utils/CustomECAT/CustomEtherCAT.h"

// Serial //
#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <cstring>


// ------------------------------- //

// -- xenomai -- //
// RT Task
RT_TASK TicToc_task;
RT_TASK SoemEcat_task;


// class
CustomEtherCAT ec;


// loop flag
bool working_tictoc = false;
bool working_soemecat = false;


// ------------------------------- //

// func.
static void TicToc(void *arg);
static void SoemEcat(void *arg);

void signal_handler();
void error_callback();

bool read_and_parse_serial_data(int fd, float &deg, float &vel);

#endif