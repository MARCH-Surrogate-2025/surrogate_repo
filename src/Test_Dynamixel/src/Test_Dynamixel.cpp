#include "Test_Dynamixel.h"

static void TicToc(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e6);
    // RT_TASK_INFO task_info;
    // rt_task_inquire(NULL, &task_info);
    // rt_printf("[TicToc] rt-task info. [name] %s\n", task_info.name);

    RTIME current_time, previous_time;
    double loop_time, loop_frequency;

    uint tick = 0;

    working_tictoc = true;
    while(working_tictoc)
    {
        tick++;
        if(tick >= 1000)
        {
            rt_printf("[TicToc-RT] 1s tick\n");
            tick = 0;
        }
        rt_task_wait_period(NULL);
    }
}

static void KeyInterrupt(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e7);    // 10 ms

    working_keyinterrupt = true;
    while(working_keyinterrupt)
    {

        rt_task_wait_period(NULL);
    }
}


static void SoemEcat(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e7);    // 1ms loop
    
    RTIME curent_time, start_time;
    uint tick = 0;
    double loop_time;
    
    // EtherCAT 1 - initialize
    int slave_count = ec.InitSlaves((char*)arg);
    if(slave_count>0)
    {
        rt_printf("\033[1;32m[RT-SoemEcat] %d slaves are found.\033[0m\n", slave_count);
    }
    else
    {
        rt_printf("\033[1;31m[RT-SoemEcat] no slaves are found.\033[0m\n");
    }


    // EL6021 - 2: COM settings + IO mapping
    ec.SetSDO();
   
    // EL6021 - 3: Initialize and ready to communicate
    ec.PrepareCommunication(1);


    // ec.SendCommand();
    // PRINT_BLU
    // rt_printf("[RT-SoemCat] send command 1\n\n");
    // PRINT_NRM
    // usleep(2000);
    
    /// DXL - Enable Motor
    if(ec.DXL_EnableMotor() == 0)
    {
        PRINT_BLU
        rt_printf("[RT-SoemCat] Dynamixel motors are enabled.\n");
        PRINT_NRM
    }
    else
    {
        PRINT_RED
        rt_printf("[RT-SoemCat] Failed to enable Dynamixel motors.\n");
        PRINT_NRM
    }

    // DXL - Read Current Position
    if(ec.DXL_ReadCurrentPosition(&dxl_zero_position) == 0)
    {
        PRINT_BLU
        rt_printf("[RT-SoemCat] DXL 1 - Zero position: %d\n",  dxl_zero_position);
        PRINT_NRM
    }
    else
    {
        PRINT_RED
        rt_printf("[RT-SoemCat] Failed to set zero position of Dynamixel motors.\n");
        PRINT_NRM
    }
    

    int sw, cw;
    int thread_tick_1 = 0;
    start_time = rt_timer_read();
    double duration_time;
    working_soemecat = true;
    rt_printf("\033[1;34m[RT-SoemCat] Enter rt thread.\033[0m\n");
    while(working_soemecat)
    {
        thread_tick_1++;

        // update command
        double freq = 3;
        double amp = 500;
        dxl_target_position = (int)(amp*sin(2*M_PI*freq*thread_tick_1*0.001)) + dxl_zero_position;
        ec.DXL_WriteTargetPosition(dxl_target_position);
        ec.DXL_ReadCurrentPosition(&dxl_current_position);
        duration_time = rt_timer_read() - start_time;
        start_time = rt_timer_read();

        rt_printf("[RT-SoemCat] Cycle: %d (dt: %2.3f ms), Target Position: %d, CurrentPosition: %d \r"
                    , thread_tick_1, duration_time / 1e6
                    , dxl_target_position, dxl_current_position);

         
        rt_task_wait_period(NULL);
    }

    do
    {
        ec.DXL_ReadCurrentPosition(&dxl_current_position);
        dxl_target_position = (dxl_current_position - 2048)*0.9 + 2048;
        ec.DXL_WriteTargetPosition(dxl_target_position);
        rt_printf("[RT-SoemCat] Homing mode - %d / 2048.\r", dxl_current_position);
    } while (abs(dxl_current_position-2048) > 20);
    ec.DXL_WriteTargetPosition(2048);
    ec.DXL_ReadCurrentPosition(&dxl_current_position);
    rt_printf("\n[RT-SoemCat] Homing mode done - %d / 2048.\n", dxl_current_position);
    
    sleep(1);
    if(ec.DXL_DisableMotor() == 0)
    {
        PRINT_BLU
        rt_printf("\n[RT-SoemCat] Dynamixel motors are disabled.\n");
        PRINT_NRM
    }
    ec_close();
    rt_printf("[RT-SoemCat] quit. \n");
}


void error_callback()
{
    PRINT_RED
    printf("\n[error_callback] error \n");
    PRINT_NRM

    if(working_tictoc)
    {
        working_tictoc = false;
        sleep(3);
        rt_task_delete(&TicToc_task);
    }
    if(working_soemecat)
    {
        working_soemecat = false;
        sleep(3);
        rt_task_delete(&SoemEcat_task);
    }
    

    exit(1);
}

void signal_handler(int signum = 0)
{
    PRINT_RED
    printf("\n[signal_handler] signal is detected \n");
    PRINT_NRM

    if(working_tictoc)
    {
        working_tictoc = false;
        sleep(3);
        rt_task_delete(&TicToc_task);
    }
    if(working_soemecat)
    {
        working_soemecat = false;
        sleep(3);
        rt_task_delete(&SoemEcat_task);
    }

    exit(1);
}

int main(int argc, char* argv[])
{
    rt_print_auto_init(1);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    mlockall(MCL_CURRENT|MCL_FUTURE);


    // PRINT_GRN
    // rt_printf("\n[main] RT Tictoc Task is created\n");
    // PRINT_NRM
    // rt_task_create(&TicToc_task, "TicToc_task", 0, 90, 0);
    // rt_task_start(&TicToc_task, &TicToc, NULL);

    char* port_name = argv[1];
    PRINT_GRN
    rt_printf("\n[main] RT SoemEcat Task is created (port: %s) \n", port_name);
    PRINT_NRM
    rt_task_create(&SoemEcat_task, "SoemEcat_task", 0, 90, 0);
    rt_task_start(&SoemEcat_task, &SoemEcat, port_name);

    pause();
}