//
// Created by surrogate on 24. 8. 2.
//
#include "hello_grpc.h"

static void rt_task(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e9);

    RTIME current_time, previous_time;
    double loop_time, loop_frequency;

    working_tictoc = true;

    double wheel_vel;
    double steering_pos;

    while(working_tictoc)
    {   
        wheel_vel = custom_server.request_message.SetMobileStateRequest.wheel_vel;
        steering_pos = custom_server.request_message.SetMobileStateRequest.steering_pos;
        
        rt_printf("[TicToc-RT] 1s tick, [%.3f, %.3f]\n", wheel_vel, steering_pos);
        rt_task_wait_period(NULL);
    }
}

void signal_handler(int signum = 0)
{
    printf("[signal_handler] signal is detected \n");

    if(working_tictoc)
    {
        working_tictoc = false;
        rt_task_delete(&demo_task);
    }
    exit(1);
}

int main(int argc, char* argv[])
{   
    // Run gRPC server
    custom_server.RunServer();

    // Run Xenomai_RT_task
    rt_print_auto_init(1);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    mlockall(MCL_CURRENT|MCL_FUTURE); // 메모리 고정용?

    printf("start task\n");
    
    rt_task_create(&demo_task, "demo", 0, 50, 0);
    rt_task_start(&demo_task, &rt_task, 0);

    std::cout << "Hello world!" << std::endl;
    pause();
}