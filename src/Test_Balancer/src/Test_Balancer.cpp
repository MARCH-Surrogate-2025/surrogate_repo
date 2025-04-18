#include "Test_Balancer.h"

static void ecat_func(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e6);

    RTIME current_time, previous_time;
    double loop_time, loop_frequency;

    
    blc_control_command = 5;
    blc_torque_command = 6.7;

    uint8_t _arr[11];

    int tick_count = 0;
    start_time = rt_timer_read();
    working_ecat = true;
    printf("\n\033[1;32m Start Thread : ecat_func.\033[0m\n");
    while(working_ecat)
    {
        tick_count++;

        double freq_s = 0.2, amp_s = 2.0;
        blc_torque_command = amp_s*sin(2*M_PI*freq_s*tick_count*0.01);

        int res = ec.BLC_UpdateCommand_2(blc_control_command, blc_torque_command
                            , &blc_state, &blc_imu_x, &blc_imu_y, &blc_imu_z, &blc_imu_gy
                            , &blc_steering, &blc_velocity
                            , &sw4, &sw5, &sw6, _arr);

        //int res = ec.BLC_WriteCommand(blc_control_command, blc_torque_command);


        duration_time = rt_timer_read() - start_time;
        loop_frequency = 1 / (duration_time / 1e9);
        start_time = rt_timer_read();

        // rt_printf("[ecat_func] freq: %4.3f Hz, tick: %d, state: %d, torqueTest: %2.3f, IMU-X: %3.2f, IMU-Y: %3.2f, IMU-Z: %3.2f, IMU-G: %3.2f, Steering: %2.3f, Velocity: %d, data_size: %d.\n"
        //         , loop_frequency, tick_count
        //         , blc_torque_command, blc_state, blc_imu_x, blc_imu_y, blc_imu_z, blc_imu_gy
        //         , blc_steering, blc_velocity, res);
        rt_printf("[ecat_func] freq: %4.3f Hz, tick: %d, IMU-X: %3.2f, IMU-Y: %3.2f, IMU-Z: %3.2f, IMU-G: %3.2f, size: %d, [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d].\n"
                , loop_frequency, tick_count, blc_imu_x, blc_imu_y, blc_imu_z, blc_imu_gy, res
                , _arr[0], _arr[1], _arr[2], _arr[3], _arr[4], _arr[5], _arr[6], _arr[7], _arr[8], _arr[9], _arr[10]);

        rt_task_wait_period(NULL);
    }

    ec_close();
}

static void ecat_func2(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e6);

    RTIME current_time, previous_time;
    double loop_time, loop_frequency;

    
    blc_control_command = 5;
    blc_torque_command = 6.7;

    int tick_count = 0;
    start_time = rt_timer_read();
    working_ecat = true;
    printf("\n\033[1;32m Start Thread : ecat_func.\033[0m\n");
    while(working_ecat)
    {
        tick_count++;

        if(tick_count==100)
        {
            tick_count = 0;
            blc_control_command++;
        }
        // int res = ec.BLC_UpdateCommand_2(blc_control_command, blc_torque_command
        //                     , &blc_state, &blc_imu_x, &blc_imu_y, &blc_imu_z);

        int res = ec.BLC_ReadState(&blc_state, &blc_imu_x, &blc_imu_y, &blc_imu_z);


        // duration_time = rt_timer_read() - start_time;
        // loop_frequency = 1 / (duration_time / 1e9);
        // start_time = rt_timer_read();

        // rt_printf("[ecat_func] freq: %5.2f Hz, state: %d, IMU-X: %3.2f, IMU-Y: %3.2f, IMU-Z: %3.2f, res: %d.\r"
        //         , loop_frequency, blc_state, blc_imu_x, blc_imu_y, blc_imu_z, res);

        rt_task_wait_period(NULL);
    }

    ec_close();
}


void signal_handler(int signum = 0)
{
    printf("\033[1;31m\n[signal_handler] signal is detected.\033[0m\n");

    if(working_ecat)
    {
        working_ecat = false;
        sleep(3);
        rt_task_delete(&ecat_task);
    }
    if(working_ecat2)
    {
        working_ecat2 = false;
        sleep(3);
        rt_task_delete(&ecat_task2);
    }

    exit(1);
}

int main(int argc, char* argv[])
{
    // Run Xenomai_RT_task
    rt_print_auto_init(1);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    mlockall(MCL_CURRENT|MCL_FUTURE); // 메모리 고정용?

#pragma region from rt_task
    // EtherCAT 1 - Configure & Initialize
    int slave_count = ec.InitSlaves(argv[1]);

    // EtherCAT 2 - Mapping
    ec.ConfigureSDO(BALANCER_IDX);

    // EtherCAT 3 - Prepare opreation (state op)
    ec.PrepareOperation();

    // EtherCAT 4 - Prepare communication (EL6021)
    ec.PrepareCommunication(BALANCER_IDX);

    // ec.BLC_GetMotorInfo();
#pragma endregion


    printf("start task\n");

    char* port_name = argv[1];
    rt_task_create(&ecat_task, "ecat", 0, 90, 0);
    rt_task_start(&ecat_task, &ecat_func, port_name);

    // rt_task_create(&ecat_task2, "ecat2", 0, 90, 0);
    // rt_task_start(&ecat_task2, &ecat_func2, port_name);
    
    pause();
}