#include "Test_Wheel.h"
int fd;

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


static void SoemEcat(void *arg)
{
    rt_task_set_periodic(NULL, TM_NOW, 1e6);    // 1ms loop

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

    // EtherCAT 2 - PDO mapping
    ec.ConfigurePDO();
   
    // EtherCAT 3- Enable motor
    if(ec.EnableMotor(0))
    {
        rt_printf("\n\033[1;32m[RT-SoemEcat] motors are enabled.\033[0m\n");
    }
    else
    {
        rt_printf("\033[1;31m\n[RT-SoemEcat] failed to process command.\033[0m\n");
    }

    int sw, cw;
    int thread_tick_1 = 0;
    start_time = rt_timer_read();
    working_soemecat = true;
    rt_printf("\033[1;34m[RT-SoemCat] Enter rt thread.\033[0m\n");
    while(working_soemecat)
    {
        thread_tick_1++;
        float Deg_ZM = 0.0f;
        float z_velm = 0.0f;
        read_and_parse_serial_data(fd, Deg_ZM, z_velm);

        // ec.SendCommand(0, CYCLIC_SYNC_VELOCITY_MODE, 300);

        float tau = 200 * Deg_ZM;
        ec.SendCommand(0, CYCLIC_SYNC_TORQUE_MODE, (int)tau);
        sw = ec.GetStatusWord(0);
        cw = ec.GetControlWord(0);   


        // Read and parse the data
  

        rt_printf("[RT-SoemCat] Cycle: %d, StatusWord: %x, CommandWord(shutdown): %x, Actual Velocity: %d, Deg: %.2f, Vel: %.2f\n"
                    , thread_tick_1, sw, cw
                    , ec.txPDO[0]->velocityActualValue, Deg_ZM, z_velm);

        // tick++;
        // if(tick>=1000)
        // {
        //     tick = 0;
        //     loop_time = (rt_timer_read() - start_time) / 1e9;
        //     rt_printf("[%.3f s] %s \n", loop_time, arg);
        // }

        rt_task_wait_period(NULL);
    }

    ec.DisableMotor(0);
    ec_close();
    rt_printf("[RT-SoemCat] quit. \n");
}



void error_callback()
{
    PRINT_RED
    printf("[error_callback] error \n");
    PRINT_NRM

    if(working_tictoc)
    {
        working_tictoc = false;
        sleep(1);
        rt_task_delete(&TicToc_task);
    }
    if(working_soemecat)
    {
        working_soemecat = false;
        sleep(1);
        rt_task_delete(&SoemEcat_task);
    }
    

    exit(1);
}

void signal_handler(int signum = 0)
{
    PRINT_RED
    printf("[signal_handler] signal is detected \n");
    PRINT_NRM

    if(working_tictoc)
    {
        working_tictoc = false;
        sleep(1);
        rt_task_delete(&TicToc_task);
    }
    if(working_soemecat)
    {
        working_soemecat = false;
        sleep(1);
        rt_task_delete(&SoemEcat_task);
    }

    exit(1);
}

bool read_and_parse_serial_data(int fd, float &deg, float &vel) {
    char read_buffer[256];  // Buffer to hold the received line
    int idx = 0;
    bool line_received = false;

    // Read one line from the serial port
    while (!line_received) {
        char ch;
        int n = read(fd, &ch, 1);  // Read one byte at a time

        if (n > 0) {
            // If we encounter a newline or carriage return, it's the end of the line
            if (ch == '\n') {
                // Null-terminate the string at the current index
                read_buffer[idx] = '\0';

                // Check if line starts with "$" and ends with "\r\n"
                if (read_buffer[0] == '$') {
                    // Parse the string with sscanf: expected format is "$deg, vel"
                    int num_parsed = sscanf(read_buffer, "$%f, %f", &deg, &vel);
                    
                    // If two numbers are successfully parsed
                    if (num_parsed == 2) {
                        line_received = true;
                    } else {
                        std::cerr << "Error: Invalid data format!" << std::endl;
                    }
                }

                // Reset the index for the next line
                idx = 0;
            } else {
                // Store the character in the buffer
                if (idx < sizeof(read_buffer) - 1) {
                    read_buffer[idx++] = ch;
                }
            }
        }
    }

    return line_received;
}

int main(int argc, char* argv[])
{
    rt_print_auto_init(1);

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    mlockall(MCL_CURRENT|MCL_FUTURE);

    // rt_printf("[main] RT TicToc Task is created \n");
    // rt_task_create(&TicToc_task, "TicToc_task", 0, 90, 0);
    // rt_task_start(&TicToc_task, &TicToc, NULL);

    const char* portname = "/dev/ttyACM0";  // 사용하려는 포트 (예: /dev/ttyUSB0, /dev/ttyACM0 등)
    fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);  // 포트 열기

    if (fd == -1) {
        std::cerr << "Unable to open port!" << std::endl;
        return 1;
    }
    printf("fd: %d\n", fd);
    // 시리얼 포트 설정
    struct termios options;
    tcgetattr(fd, &options);  // 기존 설정 읽기

    // Baudrate 설정
    cfsetispeed(&options, B115200);  // 수신 속도 9600bps
    cfsetospeed(&options, B115200);  // 송신 속도 9600bps

    // 기타 설정 (패리티, 비트수 등)
    options.c_cflag &= ~PARENB;    // 패리티 비트 사용 안 함
    options.c_cflag &= ~CSTOPB;    // 1개의 스톱 비트
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;        // 8비트 데이터
    options.c_cflag &= ~CRTSCTS;   // 하드웨어 흐름 제어 사용 안 함
    options.c_cflag |= CREAD | CLOCAL;  // 수신 활성화, 로컬 연결

    // 설정을 포트에 반영
    tcsetattr(fd, TCSANOW, &options);

    // Variables to hold parsed data
    float Deg_ZM = 0.0f;
    float z_velm = 0.0f;

    // Read and parse the data
    for(int i=0;i<100;i++){
            if (read_and_parse_serial_data(fd, Deg_ZM, z_velm)) {
        // Print the parsed values
        std::cout << "Parsed Deg: " << Deg_ZM << ", Vel: " << z_velm << std::endl;
    } else {
        std::cerr << "Failed to read valid data!" << std::endl;
    }
    }

    

    char* port_name = argv[1];
    PRINT_GRN
    rt_printf("\n[main] RT SoemEcat Task is created (port: %s) \n", port_name);
    PRINT_NRM
    rt_task_create(&SoemEcat_task, "SoemEcat_task", 0, 90, 0);
    rt_task_start(&SoemEcat_task, &SoemEcat, port_name);

    pause();
}