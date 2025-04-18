//
// Created by surrogate on 24. 8. 2.
//
#include "test_integration.h"

static void rt_task(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e6);

    RTIME current_time, previous_time;
    double loop_time, loop_frequency;

    working_tictoc = true;

    double wheel_vel;
    double steering_pos;

    while(working_tictoc)
    {   
        wheel_vel = custom_server.request_message.SetMobileStateRequest.wheel_vel;
        steering_pos = custom_server.request_message.SetMobileStateRequest.steering_pos;
        
        rt_printf("[TicToc-RT] 1ms tick, [%.3f, %.3f]\r", wheel_vel, steering_pos);
        rt_task_wait_period(NULL);
    }
}


static void ecat_func(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e6);

    RTIME current_time, previous_time;
    double loop_time, loop_frequency;

    // EtherCAT 1 - Configure & Initialize
    int slave_count = ec.InitSlaves((char*)arg);

    // EtherCAT 2 - Mapping
    ec.ConfigurePDO(WHEEL_IDX);
    // ec.ConfigureSDO(STEERING_IDX, EL6021_BAUDRATE_115200, TRUE, TRUE, EL6021_DATAFRAME_8N1);
    // ec.ConfigureSDO(BALANCING_IDX, EL6021_BAUDRATE_115200, FALSE, FALSE, EL6021_DATAFRAME_8N1);

    // EtherCAT 3 - Prepare opreation (state op)
    ec.PrepareOperation();
    
    // // EtherCAT 4 - Prepare communication (EL6021)
    // ec.PrepareCommunication(STEERING_IDX);
    // ec.PrepareCommunication(BALANCING_IDX);

    // EtherCAT 5 - Enable steering motor
    // DXL - Enable Motor
    if(ec.DXL_EnableMotor() == 0)
    {
        printf("[ecat_func] Steering motor is enabled.\n");
    }
    else
    {
        printf("\033[1;31m[ecat_func] Failed to enable Steering motor.\033[0m\n");
    }
    //DXL - Read Current Position
    if(ec.DXL_ReadCurrentPosition(&dxl_zero_position) == 0)
    {
        printf("[ecat_func] DXL - Zero position: %d\n",  dxl_zero_position);
    }
    else
    {
        printf("\033[1;31m[ecat_func] Failed to set zero position of Steering motor.\033[0m\n");
    }


    // EtherCAT 6 - Enable wheel motor
    if(ec.Wheel_EnableMotor() == 0)
    {
        printf("\033[1;32m[ecat_func] Wheel motor is enabled.\033[0m\n");
    }
    else
    {
        printf("\033[1;31m\n[ecat_func] Failed to enable wheel motor.\033[0m\n");
    }



    int tick_count = 0;
    start_time = rt_timer_read();
    working_ecat = true;
    printf("\n\033[1;32m Start Thread : ecat_func.\033[0m\n");
    while(working_ecat)
    {
        tick_count++;

        //wheel_velocity = custom_server.request_message.SetMobileStateRequest.wheel_vel;
        //steering_angle = custom_server.request_message.SetMobileStateRequest.steering_pos;
        //dxl_target_position = (int)(steering_angle * (4096/360)) + 2048;
        //target_velocity = (int)wheel_velocity;

        // update command (steering angle)
        double freq_s = 1, amp_s = 200;
        dxl_target_position = (int)(amp_s*sin(2*M_PI*freq_s*tick_count*0.001)) + dxl_zero_position;
        //dxl_target_position = (int)(rc_steering*(2048/M_PI)) + dxl_zero_position;
        //ec.DXL_WriteTargetPosition(dxl_target_position);
        //ec.DXL_ReadCurrentPosition(&dxl_current_position);

        // update command (wheel velocity)
        double freq_w = 0.5, amp_w = 200;
        target_velocity = (int)(amp_w*sin(2*M_PI*freq_w*tick_count*0.001));
        //target_velocity = rc_velocity;
        ec.Wheel_SendCommand(CYCLIC_SYNC_VELOCITY_MODE, target_velocity);

        // update command (balancing)
        uint8_t _arr[11];
        double freq_b = 6, amp_b = 3.0;
        blc_torque_command = amp_b*sin(2*M_PI*freq_b*tick_count*0.0001);
        // int read_data_size = ec.BLC_UpdateCommand_INTG(blc_control_command, blc_torque_command
        //                     , &blc_state, &blc_imu_x, &blc_imu_y, &blc_imu_z, &blc_imu_gy, _arr);


        ////////////////////////////////////////////////////////////////////////////////
        ec.DXL_ReadCurrentPosition_PrepareCommand();
        ec.DXL_WriteTargetPosition_PrepareCommand(dxl_target_position);
        ec.BLC_PrepareCommand(blc_control_command, blc_torque_command);

        ec.ProcessOneCycleCommand();

        ec.DXL_ReadCurrentPosition_ParseCommand(&dxl_current_position);
        ec.DXL_WriteTargetPosition_ParseCommand();
        ec.BLC_ParseCommand(&blc_state, &blc_imu_x, &blc_imu_y, &blc_imu_z, &blc_imu_gy, &rc_steering, &rc_velocity, _arr);
        ////////////////////////////////////////////////////////////////////////////////


        duration_time = rt_timer_read() - start_time;
        loop_frequency = 1 / (duration_time / 1e9);
        start_time = rt_timer_read();

        rt_printf("[ecat_func] freq: %5.2f Hz, v: %d, th: %d, tau: %3.3f, IMU-X: %3.3f, IMU-Y: %3.3f, IMU-Z: %3.3f, IMU-GY: %3.3f, RC-th: % 3.3f, RC-V: %d.\r"
        , loop_frequency, target_velocity, dxl_target_position, blc_torque_command
        , blc_imu_x, blc_imu_y, blc_imu_z, blc_imu_gy, rc_steering, rc_velocity);

        // rt_printf("[ecat_func] freq: %4.3f Hz, tick: %d, IMU-X: %3.2f, IMU-Y: %3.2f, IMU-Z: %3.2f, IMU-G: %3.2f, size: %d, [%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d].\n"
        //         , loop_frequency, tick_count, blc_imu_x, blc_imu_y, blc_imu_z, blc_imu_gy, read_data_size
        //         , _arr[0], _arr[1], _arr[2], _arr[3], _arr[4], _arr[5], _arr[6], _arr[7], _arr[8], _arr[9], _arr[10]);

        rt_task_wait_period(NULL);
    }

    // do
    // {
    //     ec.DXL_ReadCurrentPosition(&dxl_current_position);
    //     rt_printf("zz.\n");
    //     dxl_target_position = (dxl_current_position - 2048)*0.9 + 2048;
    //     ec.DXL_WriteTargetPosition(dxl_target_position);
    //     rt_printf("[RT-SoemCat] Homing mode - %d / 2048.\r", dxl_current_position);
    // } while (abs(dxl_current_position-2048) > 20);
    // ec.DXL_WriteTargetPosition(2048);
    // ec.DXL_ReadCurrentPosition(&dxl_current_position);
    // printf("\n[RT-SoemCat] Homing mode done - %d / 2048.\n", dxl_current_position);
    
    // sleep(1);
    if(ec.DXL_DisableMotor() == 0)
    {
        printf("\n[ecat_func] Steering motor is disabled.\n");
    }
    if(ec.Wheel_DisableMotor() == 0)
    {
        printf("\n[ecat_func] Wheel motor is disabled.\n");
    }

    ec_close();
}

void signal_handler(int signum = 0)
{
    printf("\033[1;31m\n[signal_handler] signal is detected.\033[0m\n");

    if(working_tictoc)
    {
        working_tictoc = false;
        rt_task_delete(&demo_task);
    }

    if(working_ecat)
    {
        working_ecat = false;
        sleep(3);
        rt_task_delete(&ecat_task);
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
    
    // rt_task_create(&demo_task, "demo", 0, 50, 0);
    // rt_task_start(&demo_task, &rt_task, 0);

    char* port_name = argv[1];
    rt_task_create(&ecat_task, " ecat", 0, 90, 0);
    rt_task_start(&ecat_task, &ecat_func, port_name);
    
    pause();
}
