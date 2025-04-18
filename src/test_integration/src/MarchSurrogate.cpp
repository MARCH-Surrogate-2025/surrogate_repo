#include "MarchSurrogate.h"

int MarchSurrogate::InitSlaves(char *ifname)
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

int MarchSurrogate::ConfigureSDO(int _slave, uint16_t _baud, bool _is_RS485, bool _tr_opt, uint16_t _data_frame)
{
    // EL6021 - 1. COM Settings
    // set baudrate
    ec_SDOwrite(_slave, EL6021_BAUDRATE_INDEX, EL6021_BAUDRATE_SUBINDEX, FALSE, EL6021_BAUDRATE_SIZE, &_baud, EC_TIMEOUTSAFE);

    // enable half duplex (RS485))
    ec_SDOwrite(_slave, EL6021_ENABLE_HALF_DUPLEX_INDEX, EL6021_ENABLE_HALF_DUPLEX_SUBINDEX, FALSE, EL6021_ENABLE_HALF_DUPLEX_SIZE, &_is_RS485, EC_TIMEOUTSAFE);

    // enable transfer optimization
    ec_SDOwrite(_slave, EL6021_ENABLE_OPTIMIZATIN_INDEX, EL6021_ENABLE_OPTIMIZATIN_SUBINDEX, FALSE, 1, &_tr_opt, EC_TIMEOUTSAFE);

    // data frame (8N1)
    ec_SDOwrite(_slave, EL6021_COMSETTINGS_INDEX, EL6021_DATAFRAME_SUBINDEX, FALSE, EL6021_DATAFRAME_SIZE, &_data_frame, EC_TIMEOUTSAFE);

    printf("[ConfigureSDO] Finished to configure SDO for %d Beckhoff bus.\n", _slave);
    return 0;
}

int MarchSurrogate::ConfigurePDO(int _slave)
{
    int retval = 0;
    int retval_PDO = 0;
    uint8_t setZero = 0x00;
    uint8_t rxPDO_size = 0x03;
    uint8_t txPDO_size = 0x03;

    // receive PDO
    uint32_t map_1600[3] = {0x60400010, 0x60600008, 0x60ff0020};
    uint16_t map_1c12[2] = {0x0001, 0x1600};

    // transmit PDO
    uint32_t map_1a00[3] = {0x60410010, 0x60610008, 0x606C0020};
    uint16_t map_1c13[2] = {0x0001, 0x1a00};

    if (Wheel_EnableMotor())
    {
        rt_printf("\n\033[1;32m[RT-SoemEcat] motors are enabled.\033[0m\n");
    }
    else
    {
        rt_printf("\033[1;31m\n[RT-SoemEcat] failed to process command.\033[0m\n");
    }

    retval += ec_SDOwrite(_slave, 0x1c12, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE * 4);
    retval_PDO += ec_SDOwrite(_slave, 0x1600, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
    retval_PDO += ec_SDOwrite(_slave, 0x1601, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
    retval_PDO += ec_SDOwrite(_slave, 0x1602, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
    retval_PDO += ec_SDOwrite(_slave, 0x1603, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);

    /* Define RxPdo */
    for (int i = 0; i < rxPDO_size; i++)
    {
        retval_PDO += ec_SDOwrite(_slave, 0x1600, 0x01 + i, FALSE, sizeof(map_1600[i]), &(map_1600[i]), EC_TIMEOUTSAFE);
    }
    retval_PDO += ec_SDOwrite(_slave, 0x1600, 0x00, FALSE, sizeof(rxPDO_size), &rxPDO_size, EC_TIMEOUTSAFE);

    retval += ec_SDOwrite(_slave, 0x1c12, 0x01, FALSE, sizeof(map_1c12[1]), &(map_1c12[1]), EC_TIMEOUTSAFE * 4);
    retval += ec_SDOwrite(_slave, 0x1c12, 0x00, FALSE, sizeof(map_1c12[0]), &(map_1c12[0]), EC_TIMEOUTSAFE * 4);

    /* Clear TxPdo */
    retval += ec_SDOwrite(_slave, 0x1c13, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE * 4);
    retval_PDO += ec_SDOwrite(_slave, 0x1a00, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
    retval_PDO += ec_SDOwrite(_slave, 0x1a01, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
    retval_PDO += ec_SDOwrite(_slave, 0x1a02, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
    retval_PDO += ec_SDOwrite(_slave, 0x1a03, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);

    /* Define TxPdo*/
    for (int i = 0; i < txPDO_size; i++)
    {
        retval_PDO += ec_SDOwrite(_slave, 0x1a00, 0x01 + i, FALSE, sizeof(map_1a00[i]), &(map_1a00[i]), EC_TIMEOUTSAFE);
    }
    retval_PDO += ec_SDOwrite(_slave, 0x1a00, 0x00, FALSE, sizeof(txPDO_size), &txPDO_size, EC_TIMEOUTSAFE);

    retval += ec_SDOwrite(_slave, 0x1c13, 0x01, FALSE, sizeof(map_1c13[1]), &(map_1c13[1]), EC_TIMEOUTSAFE * 4);
    retval += ec_SDOwrite(_slave, 0x1c13, 0x00, FALSE, sizeof(map_1c13[0]), &(map_1c13[0]), EC_TIMEOUTSAFE * 4);

    printf("[ConfigurePDO] Finished to configure PDO for wheel motor.\n");
    return 0;
}

int MarchSurrogate::PrepareOperation()
{
    ec_config_map(&IOmap);
    ec_configdc();

    // wheel
    rxPDO1 = (struct wheel_rx *)(ec_slave[WHEEL_IDX].outputs);
    txPDO1 = (struct wheel_tx *)(ec_slave[WHEEL_IDX].inputs);

    // // steering
    // rxPDO2 = (struct EL6021_rx *)(ec_slave[STEERING_IDX].outputs);
    // txPDO2 = (struct EL6021_tx *)(ec_slave[STEERING_IDX].inputs);

    // // balancing
    // rxPDO3 = (struct EL6021_rx *)(ec_slave[BALANCING_IDX].outputs);
    // txPDO3 = (struct EL6021_tx *)(ec_slave[BALANCING_IDX].inputs);

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

    return 0;
}

int MarchSurrogate::PrepareCommunication(int _slave)
{
    do
    {
        if (_slave == STEERING_IDX)
        {
            rxPDO2->controlWord = EL6021_CONTROLWORD_INITIALIZE;
        }
        else if (_slave == BALANCING_IDX)
        {
            rxPDO3->controlWord = EL6021_CONTROLWORD_INITIALIZE;
        }
        ProcessOneCycleCommand();
        usleep(1000);
    } while (GetStatusWord(_slave) != EL6021_STATUSWORD_INITIALIZED);
    printf("[PrepareCommunication] Terminal %d initialization is completed (%d).\n", _slave, GetStatusWord(_slave));

    do
    {
        if (_slave == STEERING_IDX)
        {
            rxPDO2->controlWord = EL6021_CONTROLWORD_PREPARE;
        }
        else if (_slave == BALANCING_IDX)
        {
            rxPDO3->controlWord = EL6021_CONTROLWORD_PREPARE;
        }
        ProcessOneCycleCommand();
        usleep(1000);
    } while (GetStatusWord(_slave) != EL6021_STATUSWORD_READYTOEXCHANGE);
    printf("[PrepareCommunication] Terminal %d is ready for serial data exchange (%d).\n", _slave, GetStatusWord(_slave));

    usleep(1000);
    return 0;
}

bool MarchSurrogate::IsSystemReady()
{
    return system_ready;
}

bool MarchSurrogate::ProcessOneCycleCommand()
{
    ec_send_processdata();
    wkc = ec_receive_processdata(EC_TIMEOUTRET);
    if (wkc >= expected_WKC)
        return true;
    else
        return false;
}

int MarchSurrogate::GetStatusWord(int _slave)
{
    if (_slave == WHEEL_IDX)
    {
        return (txPDO1->statusWord) & STATUSWORD_STATE_MASK;
    }
    else if (_slave == STEERING_IDX)
    {
        return (txPDO2->statusWord) & 0xffff;
    }
    else if (_slave == BALANCING_IDX)
    {
        return (txPDO3->statusWord) & STATUSWORD_STATE_MASK;
    }
    else
    {
        return -1;
    }
}

int MarchSurrogate::GetControlWord(int _slave)
{
    if (_slave == WHEEL_IDX)
    {
        return (rxPDO1->controlWord) & CONTROLWORD_COMMAND_ENABLEOPERATION_MASK;
    }
    else if (_slave == STEERING_IDX)
    {
        return (rxPDO2->controlWord) & 0x000f;
    }
    else if (_slave == BALANCING_IDX)
    {
        return (rxPDO3->controlWord) & 0x000f;
    }
    else
    {
        return -1;
    }
}

#pragma region Wheel
int MarchSurrogate::Wheel_EnableMotor()
{
    do
    {
        if (!ProcessOneCycleCommand())
        {
            return -1;
        }
        status_now = (txPDO1->statusWord) & STATUSWORD_STATE_MASK;
        if (status_now == STATUSWORD_STATE_SWITCHEDONDISABLED)
        {
            rxPDO1->controlWord = CONTROLWORD_COMMAND_SHUTDOWN;
        }
        else if (status_now == STATUSWORD_STATE_READYTOSWTICHON)
        {
            rxPDO1->controlWord = CONTROLWORD_COMMAND_SWITCHON_ENABLEOPERATION;
        }
        usleep(1000);
    } while (((txPDO1->statusWord) & STATUSWORD_STATE_MASK) != STATUSWORD_STATE_OPERATIONENABLED);

    rxPDO1->controlWord = CONTROLWORD_COMMAND_ENABLEOPERATION;
    rxPDO1->modeOfOperation = CYCLIC_SYNC_VELOCITY_MODE;
    rxPDO1->targetVelocity = 0;

    return 0;
}

int MarchSurrogate::Wheel_DisableMotor()
{
    rxPDO1->controlWord = CONTROLWORD_COMMAND_ENABLEOPERATION;
    rxPDO1->modeOfOperation = CYCLIC_SYNC_VELOCITY_MODE;
    // rxPDO1->targetVelocity = 0;

    do
    {
        if (!ProcessOneCycleCommand())
        {
            return -1;
        }
        status_now = (txPDO1->statusWord) & STATUSWORD_STATE_MASK;
        if (status_now == STATUSWORD_STATE_OPERATIONENABLED)
        {
            rxPDO1->controlWord = CONTROLWORD_COMMAND_DISABLEOPERATION;
        }
        usleep(1000);
    } while (((txPDO1->statusWord) & STATUSWORD_STATE_MASK) != STATUSWORD_STATE_SWITCHEDON);

    return 0;
}

int MarchSurrogate::Wheel_SendCommand(int _mode, int _val)
{
    if (system_ready)
    {
        rxPDO1->controlWord = CONTROLWORD_COMMAND_ENABLEOPERATION;
        rxPDO1->modeOfOperation = _mode;
        rxPDO1->targetVelocity = _val;
        // return ProcessOneCycleCommand();
        return 0;
    }
    else
        return -1;
}
#pragma endregion

#pragma region Steering (Dynamixel)
int MarchSurrogate::DXL_PrepareCommand()
{
    // instruction data settings
    int param_num = 4;
    int param_length = param_num + 3;
    int data_length = param_num + 10;

    // uint8_t instruction_data[data_length]
    //  = {0xFF, 0xFF, 0xFD, 0x00, 0x01, 0x09, 0x00, 0x03, 0x74, 0x00, 0x00, 0x02, 0x00, 0x00, 0xCA, 0x89};

    uint8_t send_data[data_length - 2];

    // stuffing (0~2) + reseved bit (3)
    send_data[0] = 0xFF;
    send_data[1] = 0xFF;
    send_data[2] = 0xFD;
    send_data[3] = 0x00;

    // id (4) + length (5~6) + instruction (7)
    send_data[4] = 1;                            // id
    send_data[5] = (param_length & 0x00FF);      // length-low
    send_data[6] = (param_length >> 8) & 0x00FF; // length-high
    send_data[7] = 0x02;                         // read instruction

    // parameter (8~)
    uint16_t param_1 = DXL_ADDR_PRESENT_POSITION; // param_1 : address
    uint16_t param_2 = DXL_SIZE_POSITION;         // param_2 : size
    send_data[8] = (param_1 & 0x00FF);
    send_data[9] = (param_1 >> 8) & 0x00FF;
    send_data[10] = (param_2 & 0x00FF);
    send_data[11] = (param_2 >> 8) & 0x00FF;

    // CRC (last 2 bits)
    uint16_t CRC = DXL_UpdateCRC(0, send_data, (5 + param_length));
    uint8_t CRC_L = (CRC & 0x00FF);
    uint8_t CRC_H = (CRC >> 8) & 0x00FF;

    // copy to rxPDO
    for (int i = 0; i < data_length - 2; i++)
    {
        rxPDO2->data[i] = send_data[i];
    }
    rxPDO2->data[data_length - 2] = CRC_L;
    rxPDO2->data[data_length - 1] = CRC_H;

    return data_length;
}

int MarchSurrogate::DXL_ParseCommand()
{
    int parsed_param_1 = (txPDO2->data[9]) + ((txPDO2->data[10]) << 8) + ((txPDO2->data[11]) << 16) + ((txPDO2->data[12]) << 24);
    return parsed_param_1;
}

int MarchSurrogate::DXL_EnableMotor()
{
    //////////////////////////////////////////////
    ///////////// packet assemble  ///////////////
    //////////////////////////////////////////////

    // instruction data settings
    int param_num = 3;
    int param_length = param_num + 3;
    int data_length = param_num + 10;

    // uint8_t instruction_data[data_length]
    //  = {0xFF, 0xFF, 0xFD, 0x00, 0x01, 0x09, 0x00, 0x03, 0x74, 0x00, 0x00, 0x02, 0x00, 0x00, 0xCA, 0x89};

    uint8_t send_data[data_length - 2];

    // stuffing (0~2) + reseved bit (3)
    send_data[0] = 0xFF;
    send_data[1] = 0xFF;
    send_data[2] = 0xFD;
    send_data[3] = 0x00;

    // id (4) + length (5~6) + instruction (7)
    send_data[4] = 1;                            // id
    send_data[5] = (param_length & 0x00FF);      // length-low
    send_data[6] = (param_length >> 8) & 0x00FF; // length-high
    send_data[7] = 0x03;                         // write instruction
                                                 //////////////////////////////////////////////
    ///////////// packet assemble  ///////////////
    ////er (8~)
    uint16_t param_1 = DXL_ADDR_TORQUE_ENABLE; // param_1 : address
    uint8_t param_2 = 0x01;                    // param_2 : enable (1:on)
    send_data[8] = (param_1 & 0x00FF);
    send_data[9] = (param_1 >> 8) & 0x00FF;
    send_data[10] = param_2;

    // CRC (last 2 bits)
    uint16_t CRC = DXL_UpdateCRC(0, send_data, (5 + param_length));
    uint8_t CRC_L = (CRC & 0x00FF);
    uint8_t CRC_H = (CRC >> 8) & 0x00FF;

    // copy to rxPDO
    for (int i = 0; i < data_length - 2; i++)
    {
        rxPDO2->data[i] = send_data[i];
    }
    rxPDO2->data[data_length - 2] = CRC_L;
    rxPDO2->data[data_length - 1] = CRC_H;

    // return data_length;

    //////////////////////////////////////////////
    ////////// EL6021 Communication  /////////////
    //////////////////////////////////////////////
    if (system_ready)
    {
        int parse_data_size = data_length;
        // Beckohff - ControlWord (transfer 24 bytes)
        int required_packet_size = 11; // write instruction always receivce 11 bytes status

        int cw_bit = (rxPDO2->controlWord) & 0x0001;
        (rxPDO2->controlWord) &= 0x00FE;
        (rxPDO2->controlWord) |= (parse_data_size << 8);
        if (cw_bit == 1)
        {
            (rxPDO2->controlWord) &= 0xFFFE;
        }
        else if (cw_bit == 0)
        {
            (rxPDO2->controlWord) += 0x0001;
        }

        // printf("check point 0 (cw: %x, sw: %x)\n", (rxPDO2->controlWord) & 0x0003, (txPDO2->statusWord) & 0x0003);

        int sw_bit = (txPDO2->statusWord) & 0x0001;
        do
        {
            ProcessOneCycleCommand();
            // printf("check point 1 (cw: %x, sw: %x)\n", (rxPDO2->controlWord) & 0x0003, (txPDO2->statusWord) & 0x0003);
        } while (((txPDO2->statusWord) & 0x0001) == sw_bit);
        // printf("check point 1-1 (cw: %x, sw: %x)\n", (rxPDO2->controlWord) & 0x0003, (txPDO2->statusWord) & 0x0003);

        /////////////////////////
        ///// wait response /////
        /////////////////////////
        int test_tick = 0;
        int arrived_data_size = 0;

        sw_bit = (txPDO2->statusWord) & 0x0002;
        cw_bit = (rxPDO2->controlWord) & 0x0002;

        int first_tick = 0;
        (txPDO2->statusWord) &= 0x00ff;
        do
        {
            ProcessOneCycleCommand();
            test_tick++;
            if (test_tick == 1)
            {
                first_tick = 1;
            }
            else
            {
                first_tick = 0;
            }
            arrived_data_size = ((txPDO2->statusWord) >> 8) + ((txPDO2->statusWord) >> 12) * 16;

            if (sw_bit != ((txPDO2->statusWord) & 0x0002)) // if previous statusword != current statusword
            {
                // controlword acknowledgement
                cw_bit = (rxPDO2->controlWord) & 0x0002;
                if (cw_bit != 0x0002) // Bit is already 0
                {
                    (rxPDO2->controlWord) |= 0x0002;
                }
                else if (cw_bit == 0x0002) // Bit is already 1
                {
                    (rxPDO2->controlWord) -= 0x0002;
                }

                // printf("[%d] check point 3 (sw: %x, data size: %d)\n", test_tick, (txPDO2->statusWord) & 0x0003, arrived_data_size);
            }
        } while (!((arrived_data_size == required_packet_size) && (sw_bit != ((txPDO2->statusWord) & 0x0002)) && (first_tick == 0)));

        int8_t error_bit = (txPDO2->data[8]);
        return error_bit;
    }
    else
        return -1;
}

int MarchSurrogate::DXL_DisableMotor()
{
    if (!system_ready)
    {
        return -1;
    }

    // 1. prepare command
    int param_num = 3;
    int param_length = param_num + 3;
    int write_size = param_num + 10;
    int read_size = 11;
    int actual_size = 0;

    uint8_t send_data[write_size];
    send_data[0] = 0xFF;                         // stuffing
    send_data[1] = 0xFF;                         // stuffing
    send_data[2] = 0xFD;                         // stuffing
    send_data[3] = 0x00;                         // reserved
    send_data[4] = 1;                            // id
    send_data[5] = (param_length & 0x00FF);      // length-low
    send_data[6] = (param_length >> 8) & 0x00FF; // length-high
    send_data[7] = 0x03;                         // write instruction

    // parameters
    uint16_t param_1 = DXL_ADDR_TORQUE_ENABLE; // param_1 : address
    uint8_t param_2 = 0x00;                    // param_2 : enable (1:on)
    send_data[8] = (param_1 & 0x00FF);
    send_data[9] = (param_1 >> 8) & 0x00FF;
    send_data[10] = param_2;

    // CRC (last 2 bits)
    uint16_t CRC = DXL_UpdateCRC(0, send_data, (5 + param_length));
    uint8_t CRC_L = (CRC & 0x00FF);
    uint8_t CRC_H = (CRC >> 8) & 0x00FF;
    send_data[11] = CRC_L;
    send_data[12] = CRC_H;

    // copy to rxPDO
    for (int i = 0; i < write_size; i++)
    {
        rxPDO2->data[i] = send_data[i];
    }

    // Beckohff - ControlWord
    int cw_bit = (rxPDO2->controlWord) & 0x0001;
    (rxPDO2->controlWord) &= 0x00FE;
    (rxPDO2->controlWord) |= (write_size << 8);
    if (cw_bit == 1)
    {
        (rxPDO2->controlWord) &= 0xFFFE;
    }
    else if (cw_bit == 0)
    {
        (rxPDO2->controlWord) += 0x0001;
    }

    int sw_bit = (txPDO2->statusWord) & 0x0001;
    do
    {
        ProcessOneCycleCommand();
    } while (((txPDO2->statusWord) & 0x0001) == sw_bit);

    /////////////////////////
    ///// wait response /////
    /////////////////////////
    int test_tick = 0;
    int arrived_data_size = 0;

    sw_bit = (txPDO2->statusWord) & 0x0002;
    cw_bit = (rxPDO2->controlWord) & 0x0002;

    int first_tick = 0;
    (txPDO2->statusWord) &= 0x00ff;
    do
    {
        ProcessOneCycleCommand();
        test_tick++;
        if (test_tick == 1)
        {
            first_tick = 1;
        }
        else
        {
            first_tick = 0;
        }
        arrived_data_size = ((txPDO2->statusWord) >> 8) + ((txPDO2->statusWord) >> 12) * 16;

        if (sw_bit != ((txPDO2->statusWord) & 0x0002)) // if previous statusword != current statusword
        {
            // controlword acknowledgement
            cw_bit = (rxPDO2->controlWord) & 0x0002;
            if (cw_bit != 0x0002) // Bit is already 0
            {
                (rxPDO2->controlWord) |= 0x0002;
            }
            else if (cw_bit == 0x0002) // Bit is already 1
            {
                (rxPDO2->controlWord) -= 0x0002;
            }
        }
    } while (!((arrived_data_size == read_size) && (sw_bit != ((txPDO2->statusWord) & 0x0002)) && (first_tick == 0)));

    int8_t error_bit = (txPDO2->data[8]);
    return error_bit;
}

int MarchSurrogate::DXL_ReadCurrentPosition(int *_current_position)
{
    if (!system_ready)
    {
        return -1;
    }

    // instruction data settings
    int param_num = 4;
    int param_length = param_num + 3;
    int write_size = param_num + 10;
    int read_size = write_size + 1; // dynamixel (+ error bit)
    int actual_read_size = 0;

    // uint8_t instruction_data[data_length]
    //  = {0xFF, 0xFF, 0xFD, 0x00, 0x01, 0x09, 0x00, 0x03, 0x74, 0x00, 0x00, 0x02, 0x00, 0x00, 0xCA, 0x89};

    uint8_t send_data[write_size];

    // stuffing (0~2) + reseved bit (3)
    send_data[0] = 0xFF;
    send_data[1] = 0xFF;
    send_data[2] = 0xFD;
    send_data[3] = 0x00;

    // id (4) + length (5~6) + instruction (7)
    send_data[4] = 1;                            // id
    send_data[5] = (param_length & 0x00FF);      // length-low
    send_data[6] = (param_length >> 8) & 0x00FF; // length-high
    send_data[7] = 0x02;                         // read instruction

    // parameter (8~)
    uint16_t param_1 = DXL_ADDR_PRESENT_POSITION; // param_1 : address
    uint16_t param_2 = DXL_SIZE_POSITION;         // param_2 : size
    send_data[8] = (param_1 & 0x00FF);
    send_data[9] = (param_1 >> 8) & 0x00FF;
    send_data[10] = (param_2 & 0x00FF);
    send_data[11] = (param_2 >> 8) & 0x00FF;

    // CRC (last 2 bits)
    uint16_t CRC = DXL_UpdateCRC(0, send_data, (5 + param_length));
    uint8_t CRC_L = (CRC & 0x00FF);
    uint8_t CRC_H = (CRC >> 8) & 0x00FF;
    send_data[write_size - 2] = CRC_L;
    send_data[write_size - 1] = CRC_H;

    // copy to rxPDO
    for (int i = 0; i < write_size; i++)
    {
        rxPDO2->data[i] = send_data[i];
    }

    ///////////////////////////
    // 2. write - update controlword
    if (write_flag[STEERING_IDX - 1] == READY)
    {
        (rxPDO2->controlWord) &= 0x00FF;            // make bit 8-15 to zero containing data size info.
        (rxPDO2->controlWord) |= (write_size << 8); // write data size
        (rxPDO2->controlWord) ^= (0x01);            // toggle bit0
        sw0 = (txPDO2->statusWord) & 0x01;
        write_flag[STEERING_IDX - 1] = ACTIVATED;
    }
    else if (write_flag[STEERING_IDX - 1] == ACTIVATED)
    {
        if (((txPDO2->statusWord) & 0x01) != sw0)
        {
            write_flag[STEERING_IDX - 1] = READY; // statusword acknowledged transmission of data
        }
    }

    int test_tick = 0;
    int arrived_data_size = 0;

    int sw_bit = (txPDO2->statusWord) & 0x0002;
    int cw_bit = (rxPDO2->controlWord) & 0x0002;

    int first_tick = 0;
    (txPDO2->statusWord) &= 0x00ff;
    do
    {
        ProcessOneCycleCommand();
        test_tick++;
        if (test_tick == 1)
        {
            first_tick = 1;
        }
        else
        {
            first_tick = 0;
        }
        arrived_data_size = ((txPDO2->statusWord) >> 8) + ((txPDO2->statusWord) >> 12) * 16;

        if (sw_bit != ((txPDO2->statusWord) & 0x0002)) // if previous statusword != current statusword
        {
            // controlword acknowledgement
            cw_bit = (rxPDO2->controlWord) & 0x0002;
            if (cw_bit != 0x0002) // Bit is already 0
            {
                (rxPDO2->controlWord) |= 0x0002;
            }
            else if (cw_bit == 0x0002) // Bit is already 1
            {
                (rxPDO2->controlWord) -= 0x0002;
            }

            // printf("[%d] check point 3 (sw: %x, data size: %d)\n", test_tick, (txPDO2->statusWord) & 0x0003, arrived_data_size);
        }
    } while (!((arrived_data_size == read_size) && (sw_bit != ((txPDO2->statusWord) & 0x0002)) && (first_tick == 0)));

    *_current_position = (txPDO2->data[9]) + ((txPDO2->data[10]) << 8) + ((txPDO2->data[11]) << 16) + ((txPDO2->data[12]) << 24);

    int8_t error_bit = (txPDO2->data[8]);
    return error_bit;
}

int MarchSurrogate::DXL_WriteTargetPosition(int _target_position)
{
    if (!system_ready)
    {
        return -1;
    }

    // instruction data settings
    int param_num = 6;
    int param_length = param_num + 3;
    int write_size = param_num + 10;
    int read_size = 11;
    int actual_read_size = 0;

    uint8_t send_data[write_size];

    // stuffing (0~2) + reseved bit (3)
    send_data[0] = 0xFF;
    send_data[1] = 0xFF;
    send_data[2] = 0xFD;
    send_data[3] = 0x00;

    // id (4) + length (5~6) + instruction (7)
    send_data[4] = 1;                            // id
    send_data[5] = (param_length & 0x00FF);      // length-low
    send_data[6] = (param_length >> 8) & 0x00FF; // length-high
    send_data[7] = 0x03;                         // write instruction

    // parameter (8~)
    int16_t param_1 = DXL_ADDR_GOAL_POSITION; // param_1 : address
    int32_t param_2 = _target_position;       // param_2 : target_position
    send_data[8] = (param_1 & 0x00FF);
    send_data[9] = (param_1 >> 8) & 0x00FF;
    send_data[10] = (param_2 & 0x00FF);
    send_data[11] = (param_2 >> 8) & 0x00FF;
    send_data[12] = (param_2 >> 16) & 0x00FF;
    send_data[13] = (param_2 >> 24) & 0x00FF;

    // CRC (last 2 bits)
    uint16_t CRC = DXL_UpdateCRC(0, send_data, (5 + param_length));
    uint8_t CRC_L = (CRC & 0x00FF);
    uint8_t CRC_H = (CRC >> 8) & 0x00FF;
    send_data[write_size - 2] = CRC_L;
    send_data[write_size - 1] = CRC_H;

    // copy to rxPDO
    for (int i = 0; i < write_size; i++)
    {
        rxPDO2->data[i] = send_data[i];
    }

    ///////////////////////////
    // 2. write - update controlword
    if (write_flag[STEERING_IDX - 1] == READY)
    {
        (rxPDO2->controlWord) &= 0x00FF;            // make bit 8-15 to zero containing data size info.
        (rxPDO2->controlWord) |= (write_size << 8); // write data size
        (rxPDO2->controlWord) ^= (0x01);            // toggle bit0
        sw0 = (txPDO2->statusWord) & 0x01;
        write_flag[STEERING_IDX - 1] = ACTIVATED;
    }
    else if (write_flag[STEERING_IDX - 1] == ACTIVATED)
    {
        if (((txPDO2->statusWord) & 0x01) != sw0)
        {
            write_flag[STEERING_IDX - 1] = READY; // statusword acknowledged transmission of data
        }
    }

    int test_tick = 0;
    int sw_bit = (txPDO2->statusWord) & 0x0002;
    int cw_bit = (rxPDO2->controlWord) & 0x0002;

    int first_tick = 0;
    (txPDO2->statusWord) &= 0x00ff;
    do
    {
        ProcessOneCycleCommand();
        test_tick++;
        if (test_tick == 1)
        {
            first_tick = 1;
        }
        else
        {
            first_tick = 0;
        }
        actual_read_size = ((txPDO2->statusWord) >> 8) + ((txPDO2->statusWord) >> 12) * 16;

        if (sw_bit != ((txPDO2->statusWord) & 0x0002)) // if previous statusword != current statusword
        {
            // controlword acknowledgement
            cw_bit = (rxPDO2->controlWord) & 0x0002;
            if (cw_bit != 0x0002) // Bit is already 0
            {
                (rxPDO2->controlWord) |= 0x0002;
            }
            else if (cw_bit == 0x0002) // Bit is already 1
            {
                (rxPDO2->controlWord) -= 0x0002;
            }

            // printf("[%d] check point 3 (sw: %x, data size: %d)\n", test_tick, (txPDO2->statusWord) & 0x0003, arrived_data_size);
        }
    } while (!((actual_read_size == read_size) && (sw_bit != ((txPDO2->statusWord) & 0x0002)) && (first_tick == 0)));

    int8_t error_bit = (txPDO2->data[8]);
    return error_bit;
}

int MarchSurrogate::DXL_ReadCurrentPosition_PrepareCommand()
{
    if (!system_ready)
    {
        return -1;
    }

    // instruction data settings
    int param_num = 4;
    int param_length = param_num + 3;
    int write_size = param_num + 10;
    int read_size = write_size + 1; // dynamixel (+ error bit)
    int actual_read_size = 0;

    // uint8_t instruction_data[data_length]
    //  = {0xFF, 0xFF, 0xFD, 0x00, 0x01, 0x09, 0x00, 0x03, 0x74, 0x00, 0x00, 0x02, 0x00, 0x00, 0xCA, 0x89};

    uint8_t send_data[write_size];

    // stuffing (0~2) + reseved bit (3)
    send_data[0] = 0xFF;
    send_data[1] = 0xFF;
    send_data[2] = 0xFD;
    send_data[3] = 0x00;

    // id (4) + length (5~6) + instruction (7)
    send_data[4] = 1;                            // id
    send_data[5] = (param_length & 0x00FF);      // length-low
    send_data[6] = (param_length >> 8) & 0x00FF; // length-high
    send_data[7] = 0x02;                         // read instruction

    // parameter (8~)
    uint16_t param_1 = DXL_ADDR_PRESENT_POSITION; // param_1 : address
    uint16_t param_2 = DXL_SIZE_POSITION;         // param_2 : size
    send_data[8] = (param_1 & 0x00FF);
    send_data[9] = (param_1 >> 8) & 0x00FF;
    send_data[10] = (param_2 & 0x00FF);
    send_data[11] = (param_2 >> 8) & 0x00FF;

    // CRC (last 2 bits)
    uint16_t CRC = DXL_UpdateCRC(0, send_data, (5 + param_length));
    uint8_t CRC_L = (CRC & 0x00FF);
    uint8_t CRC_H = (CRC >> 8) & 0x00FF;
    send_data[write_size - 2] = CRC_L;
    send_data[write_size - 1] = CRC_H;

    // copy to rxPDO
    for (int i = 0; i < write_size; i++)
    {
        rxPDO2->data[i] = send_data[i];
    }

    ///////////////////////////
    // 2. write - update controlword
    if (write_flag[STEERING_IDX - 1] == READY)
    {
        (rxPDO2->controlWord) &= 0x00FF;            // make bit 8-15 to zero containing data size info.
        (rxPDO2->controlWord) |= (write_size << 8); // write data size
        (rxPDO2->controlWord) ^= (0x01);            // toggle bit0
        sw0 = (txPDO2->statusWord) & 0x01;
        write_flag[STEERING_IDX - 1] = ACTIVATED;
    }
    else if (write_flag[STEERING_IDX - 1] == ACTIVATED)
    {
        if (((txPDO2->statusWord) & 0x01) != sw0)
        {
            write_flag[STEERING_IDX - 1] = READY; // statusword acknowledged transmission of data
        }
    }

    return 0;
}

int MarchSurrogate::DXL_ReadCurrentPosition_ParseCommand(int *_current_position)
{
    if (!system_ready)
    {
        return -1;
    }

    // instruction data settings
    int param_num = 4;
    int param_length = param_num + 3;
    int write_size = param_num + 10;
    int read_size = write_size + 1; // dynamixel (+ error bit)
    int actual_read_size = 0;

    // 4. read command
    if (read_flag[STEERING_IDX - 1] == READY)
    {
        // check whether sw2 bit is changed
        if (((txPDO2->statusWord) & 0x02) != sw1)
        {
            (rxPDO2->controlWord) ^= (0x01 << 1); // toggle cw1 bit
            actual_read_size = ((txPDO2->statusWord) >> 8) + ((txPDO2->statusWord) >> 12) * 16;
        }
        else
        {
            sw1 = (txPDO2->statusWord) & (0x01 << 1);
            actual_read_size = 0;
        }

        // 5. parse command
        if ((((txPDO2->data[0]) == 0xFF) && (txPDO2->data[1] == 0xFF) && (txPDO2->data[2] == 0xFD)))
        {
            *_current_position = (txPDO2->data[9]) + ((txPDO2->data[10]) << 8) + ((txPDO2->data[11]) << 16) + ((txPDO2->data[12]) << 24);
        }
    }

    int8_t error_bit = (txPDO2->data[8]);
    return error_bit;
}

int MarchSurrogate::DXL_WriteTargetPosition_PrepareCommand(int _target_position)
{
    if (!system_ready)
    {
        return -1;
    }

    // instruction data settings
    int param_num = 6;
    int param_length = param_num + 3;
    int write_size = param_num + 10;
    int read_size = 11;
    int actual_read_size = 0;

    uint8_t send_data[write_size];

    // stuffing (0~2) + reseved bit (3)
    send_data[0] = 0xFF;
    send_data[1] = 0xFF;
    send_data[2] = 0xFD;
    send_data[3] = 0x00;

    // id (4) + length (5~6) + instruction (7)
    send_data[4] = 1;                            // id
    send_data[5] = (param_length & 0x00FF);      // length-low
    send_data[6] = (param_length >> 8) & 0x00FF; // length-high
    send_data[7] = 0x03;                         // write instruction

    // parameter (8~)
    int16_t param_1 = DXL_ADDR_GOAL_POSITION; // param_1 : address
    int32_t param_2 = _target_position;       // param_2 : target_position
    send_data[8] = (param_1 & 0x00FF);
    send_data[9] = (param_1 >> 8) & 0x00FF;
    send_data[10] = (param_2 & 0x00FF);
    send_data[11] = (param_2 >> 8) & 0x00FF;
    send_data[12] = (param_2 >> 16) & 0x00FF;
    send_data[13] = (param_2 >> 24) & 0x00FF;

    // CRC (last 2 bits)
    uint16_t CRC = DXL_UpdateCRC(0, send_data, (5 + param_length));
    uint8_t CRC_L = (CRC & 0x00FF);
    uint8_t CRC_H = (CRC >> 8) & 0x00FF;
    send_data[write_size - 2] = CRC_L;
    send_data[write_size - 1] = CRC_H;

    // copy to rxPDO
    for (int i = 0; i < write_size; i++)
    {
        rxPDO2->data[i] = send_data[i];
    }

    ///////////////////////////
    // 2. write - update controlword
    if (write_flag[STEERING_IDX - 1] == READY)
    {
        (rxPDO2->controlWord) &= 0x00FF;            // make bit 8-15 to zero containing data size info.
        (rxPDO2->controlWord) |= (write_size << 8); // write data size
        (rxPDO2->controlWord) ^= (0x01);            // toggle bit0
        sw0 = (txPDO2->statusWord) & 0x01;
        write_flag[STEERING_IDX - 1] = ACTIVATED;
    }
    else if (write_flag[STEERING_IDX - 1] == ACTIVATED)
    {
        if (((txPDO2->statusWord) & 0x01) != sw0)
        {
            write_flag[STEERING_IDX - 1] = READY; // statusword acknowledged transmission of data
        }
    }

    return 0;
}

int MarchSurrogate::DXL_WriteTargetPosition_ParseCommand()
{
    if (!system_ready)
    {
        return -1;
    }

    // instruction data settings
    int param_num = 6;
    int param_length = param_num + 3;
    int write_size = param_num + 10;
    int read_size = 11;
    int actual_read_size = 0;

    // 4. read command
    if (read_flag[STEERING_IDX - 1] == READY)
    {
        // check whether sw2 bit is changed
        if (((txPDO2->statusWord) & 0x02) != sw1)
        {
            (rxPDO2->controlWord) ^= (0x01 << 1); // toggle cw1 bit
            actual_read_size = ((txPDO2->statusWord) >> 8) + ((txPDO2->statusWord) >> 12) * 16;
        }
        else
        {
            sw1 = (txPDO2->statusWord) & (0x01 << 1);
            actual_read_size = 0;
        }

        // 5. parse command
        if ((((txPDO2->data[0]) == 0xFF) && (txPDO2->data[1] == 0xFF) && (txPDO2->data[2] == 0xFD)))
        {
            // good
        }
    }

    int8_t error_bit = (txPDO2->data[8]);
    return error_bit;
}

unsigned short MarchSurrogate::DXL_UpdateCRC(uint16_t crc_accum, uint8_t *data_blk_ptr, uint16_t data_blk_size)
{
    uint16_t i;
    static const uint16_t crc_table[256] = {0x0000,
                                            0x8005, 0x800F, 0x000A, 0x801B, 0x001E, 0x0014, 0x8011,
                                            0x8033, 0x0036, 0x003C, 0x8039, 0x0028, 0x802D, 0x8027,
                                            0x0022, 0x8063, 0x0066, 0x006C, 0x8069, 0x0078, 0x807D,
                                            0x8077, 0x0072, 0x0050, 0x8055, 0x805F, 0x005A, 0x804B,
                                            0x004E, 0x0044, 0x8041, 0x80C3, 0x00C6, 0x00CC, 0x80C9,
                                            0x00D8, 0x80DD, 0x80D7, 0x00D2, 0x00F0, 0x80F5, 0x80FF,
                                            0x00FA, 0x80EB, 0x00EE, 0x00E4, 0x80E1, 0x00A0, 0x80A5,
                                            0x80AF, 0x00AA, 0x80BB, 0x00BE, 0x00B4, 0x80B1, 0x8093,
                                            0x0096, 0x009C, 0x8099, 0x0088, 0x808D, 0x8087, 0x0082,
                                            0x8183, 0x0186, 0x018C, 0x8189, 0x0198, 0x819D, 0x8197,
                                            0x0192, 0x01B0, 0x81B5, 0x81BF, 0x01BA, 0x81AB, 0x01AE,
                                            0x01A4, 0x81A1, 0x01E0, 0x81E5, 0x81EF, 0x01EA, 0x81FB,
                                            0x01FE, 0x01F4, 0x81F1, 0x81D3, 0x01D6, 0x01DC, 0x81D9,
                                            0x01C8, 0x81CD, 0x81C7, 0x01C2, 0x0140, 0x8145, 0x814F,
                                            0x014A, 0x815B, 0x015E, 0x0154, 0x8151, 0x8173, 0x0176,
                                            0x017C, 0x8179, 0x0168, 0x816D, 0x8167, 0x0162, 0x8123,
                                            0x0126, 0x012C, 0x8129, 0x0138, 0x813D, 0x8137, 0x0132,
                                            0x0110, 0x8115, 0x811F, 0x011A, 0x810B, 0x010E, 0x0104,
                                            0x8101, 0x8303, 0x0306, 0x030C, 0x8309, 0x0318, 0x831D,
                                            0x8317, 0x0312, 0x0330, 0x8335, 0x833F, 0x033A, 0x832B,
                                            0x032E, 0x0324, 0x8321, 0x0360, 0x8365, 0x836F, 0x036A,
                                            0x837B, 0x037E, 0x0374, 0x8371, 0x8353, 0x0356, 0x035C,
                                            0x8359, 0x0348, 0x834D, 0x8347, 0x0342, 0x03C0, 0x83C5,
                                            0x83CF, 0x03CA, 0x83DB, 0x03DE, 0x03D4, 0x83D1, 0x83F3,
                                            0x03F6, 0x03FC, 0x83F9, 0x03E8, 0x83ED, 0x83E7, 0x03E2,
                                            0x83A3, 0x03A6, 0x03AC, 0x83A9, 0x03B8, 0x83BD, 0x83B7,
                                            0x03B2, 0x0390, 0x8395, 0x839F, 0x039A, 0x838B, 0x038E,
                                            0x0384, 0x8381, 0x0280, 0x8285, 0x828F, 0x028A, 0x829B,
                                            0x029E, 0x0294, 0x8291, 0x82B3, 0x02B6, 0x02BC, 0x82B9,
                                            0x02A8, 0x82AD, 0x82A7, 0x02A2, 0x82E3, 0x02E6, 0x02EC,
                                            0x82E9, 0x02F8, 0x82FD, 0x82F7, 0x02F2, 0x02D0, 0x82D5,
                                            0x82DF, 0x02DA, 0x82CB, 0x02CE, 0x02C4, 0x82C1, 0x8243,
                                            0x0246, 0x024C, 0x8249, 0x0258, 0x825D, 0x8257, 0x0252,
                                            0x0270, 0x8275, 0x827F, 0x027A, 0x826B, 0x026E, 0x0264,
                                            0x8261, 0x0220, 0x8225, 0x822F, 0x022A, 0x823B, 0x023E,
                                            0x0234, 0x8231, 0x8213, 0x0216, 0x021C, 0x8219, 0x0208,
                                            0x820D, 0x8207, 0x0202};

    for (uint16_t j = 0; j < data_blk_size; j++)
    {
        i = ((uint16_t)(crc_accum >> 8) ^ *data_blk_ptr++) & 0xFF;
        crc_accum = (crc_accum << 8) ^ crc_table[i];
    }

    return crc_accum;
}
#pragma endregion

#pragma region Balancing motor
int MarchSurrogate::BLC_UpdateCommand_INTG(uint _command, double _torque, int *_state, double *_imu_x, double *_imu_y, double *_imu_z, double *_imu_gy, uint8_t *_read_byte_arr)
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
        rxPDO3->data[i] = command_packet[i];
    }

    // 2. write - update controlword
    if (write_flag[BALANCING_IDX - 1] == READY)
    {
        (rxPDO3->controlWord) &= 0x00FF;            // make bit 8-15 to zero containing data size info.
        (rxPDO3->controlWord) |= (write_size << 8); // write data size
        (rxPDO3->controlWord) ^= (0x01);            // toggle bit0
        sw0 = (txPDO3->statusWord) & 0x01;
        write_flag[BALANCING_IDX - 1] = ACTIVATED;
    }
    else if (write_flag[BALANCING_IDX - 1] == ACTIVATED)
    {
        if (((txPDO3->statusWord) & 0x01) != sw0)
        {
            write_flag[BALANCING_IDX - 1] = READY; // statusword acknowledged transmission of data
        }
    }

    // 3. process command
    ProcessOneCycleCommand();

    uint8_t tmp_buf[read_size];
    if (read_flag[BALANCING_IDX - 1] == READY)
    {
        actual_size = ((txPDO3->statusWord) >> 8) + ((txPDO3->statusWord) >> 12) * 16;
        // check whether sw2 bit is changed
        if (((txPDO3->statusWord) & 0x02) != sw1)
        {
            (rxPDO3->controlWord) ^= (0x01 << 1); // toggle cw1 bit
        }
        else
        {
            sw1 = (txPDO3->statusWord) & (0x01 << 1);
        }

        // 5. read - parse command
        // 5. read - parse command (ring buffer)
        if ((((txPDO3->data[0]) == 0x02) && (txPDO3->data[read_size - 1] == 0x03)) && (actual_size >= read_size))
        {
            for (int i = 0; i < read_size; i++)
            {
                tmp_buf[i] = (txPDO3->data[i]);
            }
            *_state = txPDO3->data[1];
            int16_t _imu_x_int = (tmp_buf[2]) + (tmp_buf[3] << 8);
            int16_t _imu_y_int = (tmp_buf[4]) + (tmp_buf[5] << 8);
            int16_t _imu_z_int = (tmp_buf[6]) + (tmp_buf[7] << 8);
            int16_t _imu_gy_int = (tmp_buf[8]) + (tmp_buf[9] << 8);

            *_imu_x = (double)(_imu_x_int) / 100.0;
            *_imu_y = (double)(_imu_y_int) / 100.0;
            *_imu_z = (double)(_imu_z_int) / 100.0;
            *_imu_gy = (double)(_imu_gy_int) / 131.0;
        }
        else if ((actual_size >= read_size))
        {
            for (int i = 0; i < read_size; i++)
            {
                if ((txPDO3->data[i + 1] == 0x02) && (txPDO3->data[i] == 0x03))
                {
                    int head = i + 1;
                    for (int j = 0; j < read_size; j++)
                    {
                        if ((j + 1 + head) <= read_size)
                        {
                            tmp_buf[j] = (txPDO3->data[j + head]);
                        }
                        else
                        {
                            tmp_buf[j] = (txPDO3->data[j + head - read_size]);
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
                    // break;
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
    }

    return actual_size;
}

int MarchSurrogate::BLC_PrepareCommand(uint _command, double _torque)
{
    if (!system_ready)
    {
        return -1;
    }
    int test_step = -10;

    // 0. basic settings
    int write_size = 5;
    int read_size = 7;
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
        rxPDO3->data[i] = command_packet[i];
    }

    // 2. write - update controlword
    if (write_flag[BALANCING_IDX - 1] == READY)
    {
        (rxPDO3->controlWord) &= 0x00FF;            // make bit 8-15 to zero containing data size info.
        (rxPDO3->controlWord) |= (write_size << 8); // write data size
        (rxPDO3->controlWord) ^= (0x01);            // toggle bit0
        sw0 = (txPDO3->statusWord) & 0x01;
        write_flag[BALANCING_IDX - 1] = ACTIVATED;
    }
    else if (write_flag[BALANCING_IDX - 1] == ACTIVATED)
    {
        if (((txPDO3->statusWord) & 0x01) != sw0)
        {
            write_flag[BALANCING_IDX - 1] = READY; // statusword acknowledged transmission of data
        }
    }

    return 0;
}

int MarchSurrogate::BLC_ParseCommand(int *_state, double *_imu_x, double *_imu_y, double *_imu_z, double *_imu_gy, double *_steering, int *_velocity, uint8_t *_read_byte_arr)
{
    // 0. basic settings
    int write_size = 5;
    int read_size = 7;
    int actual_size = 0;

    uint8_t tmp_buf[read_size];
    if (read_flag[BALANCING_IDX - 1] == READY)
    {
        actual_size = ((txPDO3->statusWord) >> 8) + ((txPDO3->statusWord) >> 12) * 16;
        // check whether sw2 bit is changed
        if (((txPDO3->statusWord) & 0x02) != sw1)
        {
            (rxPDO3->controlWord) ^= (0x01 << 1); // toggle cw1 bit
        }
        else
        {
            sw1 = (txPDO3->statusWord) & (0x01 << 1);
        }

        // 5. read - parse command
        // 5. read - parse command (ring buffer)
        if ((((txPDO3->data[0]) == 0x02) && (txPDO3->data[read_size - 1] == 0x03)) && (actual_size >= read_size))
        {
            for (int i = 0; i < read_size; i++)
            {
                tmp_buf[i] = (txPDO3->data[i]);
            }
            *_state = txPDO3->data[1];
            // int16_t _imu_x_int = (tmp_buf[2]) + (tmp_buf[3] << 8);
            // int16_t _imu_y_int = (tmp_buf[4]) + (tmp_buf[5] << 8);
            // int16_t _imu_z_int = (tmp_buf[6]) + (tmp_buf[7] << 8);
            // int16_t _imu_gy_int = (tmp_buf[8]) + (tmp_buf[9] << 8);
            int16_t _steering_int = (tmp_buf[2]) + (tmp_buf[3] << 8);
            int16_t _velocity_int = (tmp_buf[4]) + (tmp_buf[5] << 8);

            // *_imu_x = (double)(_imu_x_int) / 100.0;
            // *_imu_y = (double)(_imu_y_int) / 100.0;
            // *_imu_z = (double)(_imu_z_int) / 100.0;
            // *_imu_gy = (double)(_imu_gy_int) / 131.0;
            *_steering = (double)(_steering_int) / 100.0;
            *_velocity = (int)((double)_velocity_int / 100.0);

        }
        else if ((actual_size >= read_size))
        {
            for (int i = 0; i < read_size; i++)
            {
                if ((txPDO3->data[i + 1] == 0x02) && (txPDO3->data[i] == 0x03))
                {
                    int head = i + 1;
                    for (int j = 0; j < read_size; j++)
                    {
                        if ((j + 1 + head) <= read_size)
                        {
                            tmp_buf[j] = (txPDO3->data[j + head]);
                        }
                        else
                        {
                            tmp_buf[j] = (txPDO3->data[j + head - read_size]);
                        }
                        *_state = tmp_buf[1];
                        // int16_t _imu_x_int = (tmp_buf[2]) + (tmp_buf[3] << 8);
                        // int16_t _imu_y_int = (tmp_buf[4]) + (tmp_buf[5] << 8);
                        // int16_t _imu_z_int = (tmp_buf[6]) + (tmp_buf[7] << 8);
                        // int16_t _imu_gy_int = (tmp_buf[8]) + (tmp_buf[9] << 8);
                        int16_t _steering_int = (tmp_buf[2]) + (tmp_buf[3] << 8);
                        int16_t _velocity_int = (tmp_buf[4]) + (tmp_buf[5] << 8);

                        // *_imu_x = (double)(_imu_x_int) / 100.0;
                        // *_imu_y = (double)(_imu_y_int) / 100.0;
                        // *_imu_z = (double)(_imu_z_int) / 100.0;
                        // *_imu_gy = (double)(_imu_gy_int) / 131.0;
                        *_steering = (double)(_steering_int) / 100.0;
                        *_velocity = (int)((double)_velocity_int / 100.0);
                    }
                    // break;
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
    }

    return actual_size;
}

#pragma endregion