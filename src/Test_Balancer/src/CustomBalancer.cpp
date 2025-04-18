#include "CustomBalancer.h"

int CustomBalancer::InitSlaves(char *ifname)
{
    // initialize SOEM, bind socket to ifname
    if (ec_init(ifname) <= 0)
    {
        return -1;
    }

    // find and auto-config slaves
    if (ec_config_init(FALSE) < 0)
    {
        ec_close();
        return -1;
    }
    else
    {
        if (ec_slavecount > 0)
        {
            for (int i = 1; i <= ec_slavecount; i++)
            {
                printf("[InitSlaves] slave %d is found. (name: %s)\n", i, ec_slave[i].name);
            }
        }
        else
        {
            printf("\033[1;31m[ecat_func] no slaves are found.\033[0m\n");
        }

        return ec_slavecount;
    }
}

int CustomBalancer::ConfigureSDO(int _slave)
{
    // EL6021 - 0. Info.
    // std::string name;
    // int name_size = 2;
    // ec_SDOread(_slave, 0x1009, 0, FALSE, &name_size, (void*)name, EC_TIMEOUTSAFE);
    // printf("[ConfigureSDO] HW version: %s.\n", name);

    // EL6021 - 1. COM Settings
    // set baudrate (115200)
    // uint8_t baud_rate = EL6021_BAUDRATE_19200;
    uint8_t baud_rate = 0x0A;
    // ec_SDOwrite(_slave, EL6021_COMSETTINGS_INDEX, EL6021_BAUDRATE_SUBINDEX, FALSE, EL6021_BAUDRATE_SIZE, &baud_rate, EC_TIMEOUTSAFE);
    ec_SDOwrite(_slave, 0x4073, 0, FALSE, 2, &baud_rate, EC_TIMEOUTSAFE);

    uint8_t real_baud = 0x01;
    int baud_size = 2;
    ec_SDOread(_slave, 0x4073, 0, FALSE, &baud_size, &real_baud, EC_TIMEOUTSAFE);
    printf("[ConfigureSDO-4073] Current baud-rate: %x.\n", real_baud);

    real_baud = 0x01;
    ec_SDOread(_slave, 0x8000, 0x11, FALSE, &baud_size, &real_baud, EC_TIMEOUTSAFE);
    printf("[ConfigureSDO-8000] Current baud-rate: %x.\n", real_baud);

    // enable full duplex (RS422)
    // bool is_RS485 = false;
    // ec_SDOwrite(_slave, EL6021_COMSETTINGS_INDEX, EL6021_ENABLE_HALF_DUPLEX_SUBINDEX, FALSE, EL6021_ENABLE_HALF_DUPLEX_SIZE, &is_RS485, EC_TIMEOUTSAFE);
    uint8_t is_RS422 = 0x0;
    ec_SDOwrite(_slave, 0x4075, 0x01, FALSE, EL6021_ENABLE_HALF_DUPLEX_SIZE, &is_RS422, EC_TIMEOUTSAFE);

    // enable point-to-point connection (RS422))
    // bool is_p2p = true;
    // ec_SDOwrite(_slave, EL6021_COMSETTINGS_INDEX, EL6021_ENABLE_POINT_TO_POINT_SUBINDEX, FALSE, EL6021_ENABLE_POINT_TO_POINT_SIZE, &is_p2p, EC_TIMEOUTSAFE);

    // disable transfer optimization
    uint8_t optimize_activate = 0x0;
    ec_SDOwrite(_slave, 0x4075, 0x06, FALSE, 1, &optimize_activate, EC_TIMEOUTSAFE);

    // data frame (8N1)
    uint8_t data_frame = EL6021_DATAFRAME_8N1;
    ec_SDOwrite(_slave, EL6021_COMSETTINGS_INDEX, EL6021_DATAFRAME_SUBINDEX, FALSE, EL6021_DATAFRAME_SIZE, &data_frame, EC_TIMEOUTSAFE);

    printf("[ConfigureSDO] Finished to configure SDO for balancing motor.\n");
    return 0;
}

int CustomBalancer::PrepareOperation()
{
    ec_config_map(&IOmap);
    ec_configdc();

    // steering
    rxPDO = (struct balancer_rx *)(ec_slave[BALANCER_IDX].outputs);
    txPDO = (struct balancer_tx *)(ec_slave[BALANCER_IDX].inputs);

    // wait for all slaves to reach SAFE_OP state
    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

    printf("\033[1;32m[PrepareOperation] request operational state for all slaves.\033[0m\n");
    ec_slave[0].state = EC_STATE_OPERATIONAL;

    // send one valid process data to make outputs in slaves happy
    ec_send_processdata();
    ec_receive_processdata(EC_TIMEOUTRET);

    // request OP state for all slaves
    ec_writestate(0);
    int chk = 40;

    // wait for all slaves to reach OP state
    do
    {
        ec_send_processdata();
        ec_receive_processdata(EC_TIMEOUTRET);
        ec_statecheck(0, EC_STATE_OPERATIONAL, 50000);
    } while (chk-- && (ec_slave[0].state != EC_STATE_OPERATIONAL));

    if (ec_slave[0].state == EC_STATE_OPERATIONAL)
    {
        printf("\033[1;32m[PrepareOperation] slaves reached operational state.\033[0m\n");
        system_ready = true;
    }

    // uint8_t baud_rate = 0x10;
    // ec_SDOwrite(2, EL6021_COMSETTINGS_INDEX, EL6021_BAUDRATE_SUBINDEX, FALSE, EL6021_BAUDRATE_SIZE, &baud_rate, EC_TIMEOUTSAFE);

    // uint8_t real_baud = 0x01;
    // int baud_size = 2;
    // ec_SDOread(2, 0x8000, 0x11, FALSE, &baud_size, &real_baud, EC_TIMEOUTSAFE);
    // printf("[PrepareOperation] Current baud-rate: %x.\n", real_baud);

    return 0;
}

int CustomBalancer::PrepareCommunication(int _slave)
{
    do
    {
        rxPDO->controlWord = EL6021_CONTROLWORD_INITIALIZE;
        ProcessOneCycleCommand();
        usleep(1000);
    } while (GetStatusWord(_slave) != EL6021_STATUSWORD_INITIALIZED);
    printf("[PrepareCommunication] Terminal initialization is completed (%d).\n", GetStatusWord(_slave));

    do
    {
        rxPDO->controlWord = EL6021_CONTROLWORD_PREPARE;
        ProcessOneCycleCommand();
        usleep(1000);
    } while (GetStatusWord(_slave) != EL6021_STATUSWORD_READYTOEXCHANGE);
    printf("[PrepareCommunication] Terminal is ready for serial data exchange (%d).\n", GetStatusWord(_slave));

    usleep(1000);
    return 0;
}

bool CustomBalancer::IsSystemReady()
{
    return system_ready;
}

bool CustomBalancer::ProcessOneCycleCommand()
{
    ec_send_processdata();
    wkc = ec_receive_processdata(EC_TIMEOUTRET);
    if (wkc >= expected_WKC)
        return true;
    else
        return false;
}

int CustomBalancer::GetStatusWord(int _slave)
{
    if (_slave == BALANCER_IDX)
    {
        return (txPDO->statusWord) & 0xffff;
    }
    else
    {
        return -1;
    }
}

int CustomBalancer::GetControlWord(int _slave)
{
    if (_slave == BALANCER_IDX)
    {
        return (rxPDO->controlWord) & 0x000f;
    }
    else
    {
        return -1;
    }
}

int CustomBalancer::BLC_UpdateCommand_2(uint _command, double _torque, int *_state, double *_imu_x, double *_imu_y, double *_imu_z, double *_imu_gy, double *_steering, int16_t *_velocity, uint8_t *err1, uint8_t *err2, uint8_t *err3, uint8_t *_read_byte_arr)
{
    if (!system_ready)
    {
        return -1;
    }
    int test_step = -10;

    // 0. basic settings
    int write_size = 5;
    int read_size = 11;
    int actual_size = 0;

    // 1. write - prepare command
    uint8_t command_packet[write_size];
    command_packet[0] = 0x02;     // sof
    command_packet[1] = _command; // command
    int16_t _torque_int = (int16_t)(_torque * 10.0);
    command_packet[2] = (_torque_int & 0x00FF);      // torque high-bit
    command_packet[3] = (_torque_int >> 8) & 0x00FF; // torque low-bit
    command_packet[4] = 0x03;                        // eof

    for (int i = 0; i < write_size; i++)
    {
        rxPDO->data[i] = command_packet[i];
    }

    // 2. write - update controlword
    if (write_flag == READY)
    {
        (rxPDO->controlWord) &= 0x00FF;            // make bit 8-15 to zero containing data size info.
        (rxPDO->controlWord) |= (write_size << 8); // write data size
        (rxPDO->controlWord) ^= (0x01);            // toggle bit0
        sw0 = (txPDO->statusWord) & 0x01;
        write_flag = ACTIVATED;
    }
    else if (write_flag == ACTIVATED)
    {
        if (((txPDO->statusWord) & 0x01) != sw0)
        {
            write_flag = READY; // statusword acknowledged transmission of data
        }
    }

    // if reception FIFO is full, request initialization
    if ((((txPDO->statusWord) & (0x01 << 3)) == 0x08) && (write_flag != INIT_REQUEST))
    {
        write_flag = INIT_REQUEST;
        read_flag = INIT_REQUEST;
        rxPDO->controlWord = (0x01 << 2);

        ProcessOneCycleCommand();
        *err1 = ((rxPDO->controlWord) & 0x07);
        *err2 = ((txPDO->statusWord) & 0x07);
        *err3 = ((txPDO->statusWord) & (0x01 << 6));
        return error_count;
    }
    if (write_flag == INIT_REQUEST)
    {
        if ((txPDO->statusWord) == 0x04)
        {
            // init accepted
            rxPDO->controlWord = 0x00;
            write_flag = INIT_DONE;

            ProcessOneCycleCommand();
            *err1 = ((rxPDO->controlWord) & 0x07);
            *err2 = ((txPDO->statusWord) & 0x07);
            *err3 = ((txPDO->statusWord) & (0x01 << 6));
            return error_count;
        }
    }
    if (write_flag == INIT_DONE)
    {
        if ((txPDO->statusWord) == 0x00)
        {
            error_count++;

            // ready
            write_flag = READY;
            read_flag = READY;

            sw0 = 0;
            sw1 = 0;

            ProcessOneCycleCommand();
            *err1 = ((rxPDO->controlWord) & 0x07);
            *err2 = ((txPDO->statusWord) & 0x07);
            *err3 = ((txPDO->statusWord) & (0x01 << 6));
            return error_count;
        }
        else
        {
            rxPDO->controlWord = 0x00;
            ProcessOneCycleCommand();
            *err1 = ((rxPDO->controlWord) & 0x07);
            *err2 = ((txPDO->statusWord) & 0x07);
            *err3 = ((txPDO->statusWord) & (0x01 << 6));
            return error_count;
        }
    }

    // 3. process command
    ProcessOneCycleCommand();

    uint8_t tmp_buf[read_size];
    if (read_flag == READY)
    {
        actual_size = ((txPDO->statusWord) >> 8) + ((txPDO->statusWord) >> 12) * 16;
        // check whether sw2 bit is changed
        if ((((txPDO->statusWord) & 0x02) != sw1))
        {
            (rxPDO->controlWord) ^= (0x01 << 1); // toggle cw1 bit
            // actual_size = ((txPDO->statusWord) >> 8) + ((txPDO->statusWord) >> 12) * 16;
        }
        else
        {
            sw1 = (txPDO->statusWord) & (0x01 << 1);
            // actual_size = 0;
        }

        // 5. read - parse command
        /*if ((((txPDO->data[0]) == 0x02) && (txPDO->data[read_size - 1] == 0x03)) && (actual_size == read_size))
        {
            *_state = txPDO->data[1];
            int16_t _imu_x_int = (txPDO->data[2]) + (txPDO->data[3] << 8);
            int16_t _imu_y_int = (txPDO->data[4]) + (txPDO->data[5] << 8);
            int16_t _imu_z_int = (txPDO->data[6]) + (txPDO->data[7] << 8);
            int16_t _imu_gy_int = (txPDO->data[8]) + (txPDO->data[9] << 8);
            // int16_t _steering_int = (txPDO->data[10]) + (txPDO->data[11] << 8);
            // int16_t _velocity_int = (txPDO->data[12]) + (txPDO->data[13] << 8);

            *_imu_x = (double)(_imu_x_int) / 100.0;
            *_imu_y = (double)(_imu_y_int) / 100.0;
            *_imu_z = (double)(_imu_z_int) / 100.0;
            *_imu_gy = (double)(_imu_gy_int) / 131.0;
            // *_steering = (double)(_steering_int) / 100.0;
            // *_velocity = _velocity_int;
        }*/

        // 5. read - parse command (ring buffer)
        if ((((txPDO->data[0]) == 0x02) && (txPDO->data[read_size - 1] == 0x03)) && (actual_size >= read_size))
        {
            for(int i=0; i<read_size; i++)
            {
                tmp_buf[i] = (txPDO->data[i]);
            }
            *_state = txPDO->data[1];
            int16_t _imu_x_int = (txPDO->data[2]) + (txPDO->data[3] << 8);
            int16_t _imu_y_int = (txPDO->data[4]) + (txPDO->data[5] << 8);
            int16_t _imu_z_int = (txPDO->data[6]) + (txPDO->data[7] << 8);
            int16_t _imu_gy_int = (txPDO->data[8]) + (txPDO->data[9] << 8);
            // int16_t _steering_int = (txPDO->data[10]) + (txPDO->data[11] << 8);
            // int16_t _velocity_int = (txPDO->data[12]) + (txPDO->data[13] << 8);

            *_imu_x = (double)(_imu_x_int) / 100.0;
            *_imu_y = (double)(_imu_y_int) / 100.0;
            *_imu_z = (double)(_imu_z_int) / 100.0;
            *_imu_gy = (double)(_imu_gy_int) / 131.0;
        }
        else if((actual_size >= read_size))
        {
            for (int i = 0; i < read_size; i++)
            {
                if ((txPDO->data[i + 1] == 0x02) && (txPDO->data[i] == 0x03))
                {
                    int head = i+1;
                    for (int j = 0; j < read_size; j++)
                    {
                        if((j+1+head) <= read_size)
                        {
                            tmp_buf[j] = (txPDO->data[j+head]);
                        }
                        else
                        {
                            tmp_buf[j] = (txPDO->data[j+head-read_size]);
                        }
                        *_state = tmp_buf[1];
                        int16_t _imu_x_int = (tmp_buf[2]) + (tmp_buf[3] << 8);
                        int16_t _imu_y_int = (tmp_buf[4]) + (tmp_buf[5] << 8);
                        int16_t _imu_z_int = (tmp_buf[6]) + (tmp_buf[7] << 8);
                        int16_t _imu_gy_int = (tmp_buf[8]) + (tmp_buf[9] << 8);

                        *_imu_x = (double)(_imu_x_int) / 100.0;
                        *_imu_y = (double)(_imu_y_int) / 100.0;
                        *_imu_z = (double)(_imu_z_int) / 100.0;
                        *_imu_gy = (double)(_imu_gy_int) / 131.0;
                    }
                    //break;
                }
            }
        }
    }

    for (int i = 0; i < read_size; i++)
    {
        _read_byte_arr[i] = 0;
    }
    for (int i = 0; i < actual_size; i++)
    {
        _read_byte_arr[i] = tmp_buf[i];
        //_read_byte_arr[i] = txPDO->data[i];
    }

    *err1 = ((rxPDO->controlWord) & 0x07);
    *err2 = ((txPDO->statusWord) & 0x07);
    *err3 = ((txPDO->statusWord) & (0x01 << 6));
    //*err3 = ((0x04)&&(0x02));

    return actual_size;
}

int CustomBalancer::BLC_WriteCommand(uint _command, double _torque)
{
    int packet_length = 5;

    uint8_t command_packet[packet_length];
    command_packet[0] = 0x02;     // sof
    command_packet[1] = _command; // command
    int16_t _torque_int = (int16_t)(_torque * 10.0);
    command_packet[2] = (_torque_int & 0x00FF);      // torque high-bit
    command_packet[3] = (_torque_int >> 8) & 0x00FF; // torque low-bit
    command_packet[4] = 0x03;                        // eof

    for (int i = 0; i < packet_length; i++)
    {
        rxPDO->data[i] = command_packet[i];
    }

    //////////////////////////////////////////////
    ////////// EL6021 Communication  /////////////
    //////////////////////////////////////////////

    if (system_ready)
    {
        int parse_data_size = packet_length;
        // Beckohff - ControlWord (transfer 24 bytes)
        int required_packet_size = 11; // write instruction always receivce 88 bytes status

        int cw_bit = (rxPDO->controlWord) & 0x0001;
        (rxPDO->controlWord) &= 0x00FE;
        (rxPDO->controlWord) |= (parse_data_size << 8);
        if (cw_bit == 1)
        {
            (rxPDO->controlWord) &= 0xFFFE;
        }
        else if (cw_bit == 0)
        {
            (rxPDO->controlWord) += 0x0001;
        }

        ProcessOneCycleCommand();

        return 0;
    }
    else
        return -1;
}

int CustomBalancer::BLC_ReadState(int *_state, double *_imu_x, double *_imu_y, double *_imu_z)
{
    if (system_ready)
    {
        int required_packet_size = 11; // write instruction always receivce 88 bytes status

        ProcessOneCycleCommand();
        int sw_bit = (txPDO->statusWord) & 0x0002;

        int first_tick = 0;
        (txPDO->statusWord) &= 0x00ff;

        if (sw_bit != ((txPDO->statusWord) & 0x0002)) // if previous statusword != current statusword
        {
            // controlword acknowledgement
            int cw_bit = (rxPDO->controlWord) & 0x0002;
            if (cw_bit != 0x0002) // Bit is already 0
            {
                (rxPDO->controlWord) |= 0x0002;
            }
            else if (cw_bit == 0x0002) // Bit is already 1
            {
                (rxPDO->controlWord) -= 0x0002;
            }

            // printf("[%d] check point (sw: %x, data size: %d)\n", test_tick, (txPDO->statusWord) & 0x0003, arrived_data_size);
        }

        *_state = txPDO->data[1];
        int16_t _imu_x_int = (txPDO->data[4]) + (txPDO->data[5] << 8);
        int16_t _imu_y_int = (txPDO->data[6]) + (txPDO->data[7] << 8);
        int16_t _imu_z_int = (txPDO->data[8]) + (txPDO->data[9] << 8);

        *_imu_x = (double)(_imu_x_int) / 10.0;
        *_imu_y = (double)(_imu_y_int) / 10.0;
        *_imu_z = (double)(_imu_z_int) / 10.0;

        return 0;
    }
    else
    {
        return -1;
    }
}
