#include "EL6021.h"

int EL6021::InitSlaves(char *ifname)
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
        for (int i = 1; i <= ec_slavecount; i++)
        {
            printf("[RT-SoemEcat] slave %d is found. (name: %s)\n", i, ec_slave[i].name);
        }
        return ec_slavecount;
    }
}

int EL6021::SetSDO()
{
    // EL6021 - 1. COM Settings
    for (int slave = 2; slave <= ec_slavecount; slave++)
    {
        // set baudrate (115200)
        uint8_t baud_rate = EL6021_BAUDRATE_115200;
        //uint8_t initial_baud;
        //uint8_t real_baud;
        //int real_baud_size = 1;
        //ec_SDOread(slave, EL6021_COMSETTINGS_INDEX, EL6021_BAUDRATE_SUBINDEX, FALSE, &real_baud_size, &initial_baud, EC_TIMEOUTSAFE);
        ec_SDOwrite(slave, EL6021_COMSETTINGS_INDEX, EL6021_BAUDRATE_SUBINDEX, FALSE, EL6021_BAUDRATE_SIZE, &baud_rate, EC_TIMEOUTSAFE);
        //ec_SDOread(slave, EL6021_COMSETTINGS_INDEX, EL6021_BAUDRATE_SUBINDEX, FALSE, &real_baud_size, &real_baud, EC_TIMEOUTSAFE);
        //printf("[SetSDO] Baudrate is changed from %d to %d.\n", initial_baud, real_baud);

        // enable half duplex (RS485))
        bool is_RS485 = true;
        ec_SDOwrite(slave, EL6021_COMSETTINGS_INDEX, EL6021_ENABLE_HALF_DUPLEX_SUBINDEX, FALSE, EL6021_ENABLE_HALF_DUPLEX_SIZE, &is_RS485, EC_TIMEOUTSAFE);

        // data frame (8N1)
        uint8_t data_frame = EL6021_DATAFRAME_8N1;
        ec_SDOwrite(slave, EL6021_COMSETTINGS_INDEX, EL6021_DATAFRAME_SUBINDEX, FALSE, EL6021_DATAFRAME_SIZE, &data_frame, EC_TIMEOUTSAFE);

        bool com01, com02, com03, com04, com05;
        int data_frame_real;
        int data_f_size = 4;
        int bool_size = 1;
        ec_SDOread(slave, EL6021_COMSETTINGS_INDEX, 0x01, FALSE, &bool_size, &com01, EC_TIMEOUTSAFE);
        ec_SDOread(slave, EL6021_COMSETTINGS_INDEX, 0x02, FALSE, &bool_size, &com02, EC_TIMEOUTSAFE);
        ec_SDOread(slave, EL6021_COMSETTINGS_INDEX, 0x03, FALSE, &bool_size, &com03, EC_TIMEOUTSAFE);
        ec_SDOread(slave, EL6021_COMSETTINGS_INDEX, 0x04, FALSE, &bool_size, &com04, EC_TIMEOUTSAFE);
        ec_SDOread(slave, EL6021_COMSETTINGS_INDEX, 0x06, FALSE, &bool_size, &com05, EC_TIMEOUTSAFE);
        int com01_int, com02_int, com03_int, com04_int, com05_int;
        if (com01 == true) com01_int = 1;
        else com01_int = 0;
        if (com02 == true) com02_int = 1;
        else com02_int = 0;
        if (com03 == true) com03_int = 1;
        else com03_int = 0;
        if (com04 == true) com04_int = 1;
        else com04_int = 0;
        if (com05 == true) com05_int = 1;
        else com05_int = 0;
        printf("[COM Settings] 01: %d, 02: %d, 03: %d, 04: %d, 05: %d.\n", com01_int, com02_int, com03_int, com04_int, com05_int);
    }

    // EL6021 - 2. IO map
    ec_config_map(&IOmap);
    ec_configdc();

    for (int slave = 1; slave <= ec_slavecount; slave++)
    {
        rxPDO[slave - 1] = (struct EL6021_rx *)(ec_slave[slave].outputs);
        txPDO[slave - 1] = (struct EL6021_tx *)(ec_slave[slave].inputs);
    }
    // printf("\033[1;32m[RT-SoemEcat] slaves mapped, state to SAFE_OP.\033[0m\n");

    // wait for all slaves to reach SAFE_OP state
    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

    printf("\033[1;32m[RT-SoemEcat] request operational state for all slaves.\033[0m\n");
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
        printf("\033[1;32m[RT-SoemCat] slave 1 reached operational state.\033[0m\n");
        system_ready = true;
    }

    return 0;
}

int EL6021::PrepareCommunication(int slave)
{
    do
    {
        rxPDO[slave]->controlWord = EL6021_CONTROLWORD_INITIALIZE;
        ProcessOneCycleCommand();
        usleep(1000);
    } while (GetStatusWord(slave) != EL6021_STATUSWORD_INITIALIZED);
    printf("[DynamixelEtherCAT] Terminal initialization is completed (%d).\n", GetStatusWord(slave));

    do
    {
        rxPDO[slave]->controlWord = EL6021_CONTROLWORD_PREPARE;
        ProcessOneCycleCommand();
        usleep(1000);
    } while (GetStatusWord(slave) != EL6021_STATUSWORD_READYTOEXCHANGE);
    printf("[DynamixelEtherCAT] Terminal is ready for serial data exchange (%d).\n", GetStatusWord(slave));

    usleep(1000);
    return 0;
}

int EL6021::SendCommand()
{
    if (system_ready)
    {
        // instruction data settings
        int param_num = 3;
        int param_length = param_num + 3;
        int data_length = param_num + 10;

        // uint8_t instruction_data[data_length]
        //  = {0xFF, 0xFF, 0xFD, 0x00, 0x01, 0x09, 0x00, 0x03, 0x74, 0x00, 0x00, 0x02, 0x00, 0x00, 0xCA, 0x89};

        uint8_t test_data[data_length - 2];

        test_data[0] = 0xFF;
        test_data[1] = 0xFF;
        test_data[2] = 0xFD;
        test_data[3] = 0x00;

        // id & instruction
        test_data[4] = 0x01;                         // id
        test_data[5] = (param_length & 0x00FF);      // length-low
        test_data[6] = (param_length >> 8) & 0x00FF; // length-high
        test_data[7] = 0x03;                         // write instruction

        // parameter
        uint16_t param_1 = 65; // param_1 : LED
        uint16_t param_2 = 1;  // param_2 : 0x01 (on)
        test_data[8] = (param_1 & 0x00FF);
        test_data[9] = (param_1 >> 8) & 0x00FF;
        test_data[10] = (param_2 & 0x00FF);
        // test_data[11] = (param_2>>8) & 0x00FF;
        // test_data[12] = (param_2>>16) & 0x00FF;
        // test_data[13] = (param_2>>24) & 0x00FF;

        // CRC
        uint16_t CRC = DXL_UpdateCRC(0, test_data, (5 + param_length));
        uint8_t CRC_L = (CRC & 0x00FF);
        uint8_t CRC_H = (CRC >> 8) & 0x00FF;

        for (int i = 0; i < data_length - 2; i++)
        {
            rxPDO[1]->data[i] = test_data[i];
        }
        rxPDO[1]->data[data_length - 2] = CRC_L;
        rxPDO[1]->data[data_length - 1] = CRC_H;

        // Beckohff - ControlWord (transfer 24 bytes)
        int packet_size = data_length + 2;
        int cw = GetControlWord(1);
        if ((cw & 0x01) == 1)
        {
            rxPDO[1]->controlWord = 0x1800;
        }
        else if ((cw & 0x01) == 0)
        {
            rxPDO[1]->controlWord = 0x1801;
        }

        int sw_bit = (txPDO[1]->statusWord) & 0x0001;
        do
        {
            ProcessOneCycleCommand();
        } while (((txPDO[1]->statusWord) & 0x0001) == sw_bit);
        printf("[Send-Done] statusword (BIT 0): %d > %d\n", sw_bit, (txPDO[1]->statusWord) & 0x0001);

        /// wait response
        sw_bit = (txPDO[1]->statusWord) & 0x0002;
        printf("[Send] controlword: %x, data: %x, %x, %x, statusword: %x\n", rxPDO[1]->controlWord, rxPDO[1]->data[0], rxPDO[1]->data[1], rxPDO[1]->data[2], sw_bit);

        int test_tick = 0;
        int arrived_data_size = 0;
        int cw_bit = (rxPDO[1]->controlWord) & 0x0002;
        do
        {
            // usleep(1000);

            ProcessOneCycleCommand();
            test_tick++;
            arrived_data_size = ((txPDO[1]->statusWord) >> 8) + ((txPDO[1]->statusWord) >> 12) * 16;

            if (sw_bit != ((txPDO[1]->statusWord) & 0x0002))
            {
                if (((rxPDO[1]->controlWord) & 0x0002) != 0x0002)
                {
                    (rxPDO[1]->controlWord) |= 0x0002;
                }
                else if (((rxPDO[1]->controlWord) & 0x0002) == 0x0002)
                {
                    (rxPDO[1]->controlWord) -= 0x0002;
                }
                printf("[Send-Done] tick: %d, cw (BIT 1): %d > %d, sw (BIT 1): %d > %d, received_data_dize: %d\n", test_tick, cw_bit, (rxPDO[1]->controlWord) & 0x0002, sw_bit, (txPDO[1]->statusWord) & 0x0002, arrived_data_size);

                cw_bit = (rxPDO[1]->controlWord) & 0x0002;
                sw_bit = (txPDO[1]->statusWord) & 0x0002;
            }
            else
            {
                printf("[Send-Done] tick: %d, cw (BIT 1): %d > %d, sw (BIT 1): %d > %d, received_data_dize: %d\r", test_tick, cw_bit, (rxPDO[1]->controlWord) & 0x0002, sw_bit, (txPDO[1]->statusWord) & 0x0002, arrived_data_size);
            }

            // sw_bit = (txPDO[1]->statusWord) & 0x0002;

            // printf("[Send-Repeat] tick: %d, statusword: %x, arrived_data_size: %d, rData: %x, %x, %x, %x, %x, %x\r"
            // , test_tick, (txPDO[1]->statusWord), arrived_data_size
            // , txPDO[1]->data[0], txPDO[1]->data[1], txPDO[1]->data[2]
            // , txPDO[1]->data[3], txPDO[1]->data[4], txPDO[1]->data[5]);

        } while ((arrived_data_size != 11));
        // while (( ((txPDO[1]->statusWord) & 0x0002) != 0x02 ) && (arrived_data_size != 13));

        printf("\n[Send] exit while loop after %d tick (%d bytes arrived) (sw: %x) data = %x, %x, %x, %x, %x, %x, %x, %x, %x.\n", test_tick, arrived_data_size, txPDO[1]->statusWord, txPDO[1]->data[0], txPDO[1]->data[1], txPDO[1]->data[2], txPDO[1]->data[3], txPDO[1]->data[4], txPDO[1]->data[5], txPDO[1]->data[6], txPDO[1]->data[7], txPDO[1]->data[8]);

        // rxPDO[slave]->modeOfOperation = mode;
        // rxPDO[slave]->targetVelocity = val;

        return 0;
    }
    else
        return -1;
}

int EL6021::UpdateCommand(int target_position, int *current_position)
{
    if (system_ready)
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

        // parameter (8~)
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
            rxPDO[1]->data[i] = send_data[i];
        }
        rxPDO[1]->data[data_length - 2] = CRC_L;
        rxPDO[1]->data[data_length - 1] = CRC_H;

        ///////////////////////////
        // printf("[Start] cw: %x, sw: %x\n", (rxPDO[1]->controlWord) & 0x0003, (txPDO[1]->statusWord) & 0x0003);

        // Beckohff - ControlWord (transfer 24 bytes)
        int parse_data_size = data_length;
        // Beckohff - ControlWord (transfer 24 bytes)
        int required_packet_size = 11; // write instruction always receivce 11 bytes status

        int cw_bit = (rxPDO[1]->controlWord) & 0x0001;
        (rxPDO[1]->controlWord) &= 0x00FE;
        (rxPDO[1]->controlWord) |= (parse_data_size << 8);
        if (cw_bit == 1)
        {
            (rxPDO[1]->controlWord) &= 0xFFFE;
            // printf("[Send] test point 1 (cw: %x)\n", rxPDO[1]->controlWord);
        }
        else if (cw_bit == 0)
        {
            (rxPDO[1]->controlWord) += 0x0001;
            // printf("[Send] test point 2 (cw: %x)\n", rxPDO[1]->controlWord);
        }
        // printf("[Send-1] controlword (BIT 0): %x > %x\n", cw_bit, (rxPDO[1]->controlWord) & 0x0001);

        int sw_bit = (txPDO[1]->statusWord) & 0x0001;
        do
        {
            ProcessOneCycleCommand();
        } while (((txPDO[1]->statusWord) & 0x0001) == sw_bit);
        // printf("[Send-2] statusword (BIT 0): %x > %x\n", sw_bit, (txPDO[1]->statusWord) & 0x0001);
        //  printf("[Send-Done] Command send. (cw: %x, sw: %x)\n"
        //      , (rxPDO[1]->controlWord) & 0x0003
        //      , (txPDO[1]->statusWord) & 0x0003);

        /////////////////////////
        ///// wait response /////
        /////////////////////////
        int test_tick = 0;
        int arrived_data_size = 0;

        sw_bit = (txPDO[1]->statusWord) & 0x0002;
        cw_bit = (rxPDO[1]->controlWord) & 0x0002;

        int first_tick = 0;
        (txPDO[1]->statusWord) &= 0x00ff;
        do
        {
            // usleep(100);

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
            arrived_data_size = ((txPDO[1]->statusWord) >> 8) + ((txPDO[1]->statusWord) >> 12) * 16;

            if (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) // if previous statusword != current statusword
            {
                // printf("\n[Read-1] statusword (BIT 1): %x > %x\n", sw_bit, (txPDO[1]->statusWord) & 0x0002);

                // controlword acknowledgement
                cw_bit = (rxPDO[1]->controlWord) & 0x0002;
                if (cw_bit != 0x0002) // Bit is already 0
                {
                    (rxPDO[1]->controlWord) |= 0x0002;
                }
                else if (cw_bit == 0x0002) // Bit is already 1
                {
                    (rxPDO[1]->controlWord) -= 0x0002;
                }
                // printf("[Read-2] controlword (BIT 1): %x > %x\n", cw_bit, (rxPDO[1]->controlWord) & 0x0002);
            }

        } while (!((arrived_data_size == required_packet_size) && (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) && (first_tick == 0)));

        return 0;
    }
    else
        return -1;
}


int EL6021::DXL_PrepareCommand()
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
        rxPDO[1]->data[i] = send_data[i];
    }
    rxPDO[1]->data[data_length - 2] = CRC_L;
    rxPDO[1]->data[data_length - 1] = CRC_H;

    return data_length;
}

int EL6021::DXL_ParseCommand()
{
    int parsed_param_1 = (txPDO[1]->data[9]) + ((txPDO[1]->data[10]) << 8) + ((txPDO[1]->data[11]) << 16) + ((txPDO[1]->data[12]) << 24);
    int parsed_param_2 = 0;
    return parsed_param_1;
}



int EL6021::DXL_EnableMotor()
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
        rxPDO[1]->data[i] = send_data[i];
    }
    rxPDO[1]->data[data_length - 2] = CRC_L;
    rxPDO[1]->data[data_length - 1] = CRC_H;

    // return data_length;

    //////////////////////////////////////////////
    ////////// EL6021 Communication  /////////////
    //////////////////////////////////////////////
    if (system_ready)
    {
        int parse_data_size = data_length;
        // Beckohff - ControlWord (transfer 24 bytes)
        int required_packet_size = 11; // write instruction always receivce 11 bytes status

        int cw_bit = (rxPDO[1]->controlWord) & 0x0001;
        (rxPDO[1]->controlWord) &= 0x00FE;
        (rxPDO[1]->controlWord) |= (parse_data_size << 8);
        if (cw_bit == 1)
        {
            (rxPDO[1]->controlWord) &= 0xFFFE;
        }
        else if (cw_bit == 0)
        {
            (rxPDO[1]->controlWord) += 0x0001;
        }

        printf("check point 0 (cw: %x, sw: %x)\n", (rxPDO[1]->controlWord)&0x0003, (txPDO[1]->statusWord)&0x0003);

        int sw_bit = (txPDO[1]->statusWord) & 0x0001;
        do
        {
            ProcessOneCycleCommand();
            printf("check point 1 (cw: %x, sw: %x)\n", (rxPDO[1]->controlWord)&0x0003, (txPDO[1]->statusWord)&0x0003);
        } while (((txPDO[1]->statusWord) & 0x0001) == sw_bit);
        printf("check point 1-1 (cw: %x, sw: %x)\n", (rxPDO[1]->controlWord)&0x0003, (txPDO[1]->statusWord)&0x0003);
        
        /////////////////////////
        ///// wait response /////
        /////////////////////////
        int test_tick = 0;
        int arrived_data_size = 0;

        sw_bit = (txPDO[1]->statusWord) & 0x0002;
        cw_bit = (rxPDO[1]->controlWord) & 0x0002;

        int first_tick = 0;
        (txPDO[1]->statusWord) &= 0x00ff;
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
            arrived_data_size = ((txPDO[1]->statusWord) >> 8) + ((txPDO[1]->statusWord) >> 12) * 16;

            if (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) // if previous statusword != current statusword
            {
                // controlword acknowledgement
                cw_bit = (rxPDO[1]->controlWord) & 0x0002;
                if (cw_bit != 0x0002) // Bit is already 0
                {
                    (rxPDO[1]->controlWord) |= 0x0002;
                }
                else if (cw_bit == 0x0002) // Bit is already 1
                {
                    (rxPDO[1]->controlWord) -= 0x0002;
                }

                printf("[%d] check point (sw: %x, data size: %d)\n"
                , test_tick, (txPDO[1]->statusWord)&0x0003, arrived_data_size);
            }
        } while (!((arrived_data_size == required_packet_size) && (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) && (first_tick == 0)));

        int8_t error_bit = (txPDO[1]->data[8]);
        return error_bit;
    }
    else
        return -1;
}

int EL6021::DXL_DisableMotor()
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
    uint8_t param_2 = 0x00;                    // param_2 : enable (1:on)
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
        rxPDO[1]->data[i] = send_data[i];
    }
    rxPDO[1]->data[data_length - 2] = CRC_L;
    rxPDO[1]->data[data_length - 1] = CRC_H;

    // return data_length;

    //////////////////////////////////////////////
    ////////// EL6021 Communication  /////////////
    //////////////////////////////////////////////
    if (system_ready)
    {
        int parse_data_size = data_length;
        // Beckohff - ControlWord (transfer 24 bytes)
        int required_packet_size = 11; // write instruction always receivce 11 bytes status

        int cw_bit = (rxPDO[1]->controlWord) & 0x0001;
        (rxPDO[1]->controlWord) &= 0x00FE;
        (rxPDO[1]->controlWord) |= (parse_data_size << 8);
        if (cw_bit == 1)
        {
            (rxPDO[1]->controlWord) &= 0xFFFE;
        }
        else if (cw_bit == 0)
        {
            (rxPDO[1]->controlWord) += 0x0001;
        }

        int sw_bit = (txPDO[1]->statusWord) & 0x0001;
        do
        {
            ProcessOneCycleCommand();
        } while (((txPDO[1]->statusWord) & 0x0001) == sw_bit);

        /////////////////////////
        ///// wait response /////
        /////////////////////////
        int test_tick = 0;
        int arrived_data_size = 0;

        sw_bit = (txPDO[1]->statusWord) & 0x0002;
        cw_bit = (rxPDO[1]->controlWord) & 0x0002;

        int first_tick = 0;
        (txPDO[1]->statusWord) &= 0x00ff;
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
            arrived_data_size = ((txPDO[1]->statusWord) >> 8) + ((txPDO[1]->statusWord) >> 12) * 16;

            if (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) // if previous statusword != current statusword
            {
                // controlword acknowledgement
                cw_bit = (rxPDO[1]->controlWord) & 0x0002;
                if (cw_bit != 0x0002) // Bit is already 0
                {
                    (rxPDO[1]->controlWord) |= 0x0002;
                }
                else if (cw_bit == 0x0002) // Bit is already 1
                {
                    (rxPDO[1]->controlWord) -= 0x0002;
                }
            }
        } while (!((arrived_data_size == required_packet_size) && (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) && (first_tick == 0)));

        int8_t error_bit = (txPDO[1]->data[8]);
        return error_bit;
    }
    else
        return -1;
}

int EL6021::DXL_ReadCurrentPosition(int* _current_position)
{
    if (system_ready)
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
            rxPDO[1]->data[i] = send_data[i];
        }
        rxPDO[1]->data[data_length - 2] = CRC_L;
        rxPDO[1]->data[data_length - 1] = CRC_H;

        ///////////////////////////
        // Beckohff - ControlWord (transfer 24 bytes)
        int required_packet_size = data_length + 1; // dynamixel (+ error bit)

        int cw_bit = (rxPDO[1]->controlWord) & 0x0001;
        (rxPDO[1]->controlWord) &= 0x00FE;
        (rxPDO[1]->controlWord) |= (data_length << 8);
        if (cw_bit == 1)
        {
            (rxPDO[1]->controlWord) &= 0xFFFE;
        }
        else if (cw_bit == 0)
        {
            (rxPDO[1]->controlWord) += 0x0001;
        }

        int sw_bit = (txPDO[1]->statusWord) & 0x0001;
        do
        {
            ProcessOneCycleCommand();
        } while (((txPDO[1]->statusWord) & 0x0001) == sw_bit);

        /////////////////////////
        ///// wait response /////
        /////////////////////////
        int test_tick = 0;
        int arrived_data_size = 0;

        sw_bit = (txPDO[1]->statusWord) & 0x0002;
        cw_bit = (rxPDO[1]->controlWord) & 0x0002;

        int first_tick = 0;
        (txPDO[1]->statusWord) &= 0x00ff;
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
            arrived_data_size = ((txPDO[1]->statusWord) >> 8) + ((txPDO[1]->statusWord) >> 12) * 16;

            if (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) // if previous statusword != current statusword
            {
                // controlword acknowledgement
                cw_bit = (rxPDO[1]->controlWord) & 0x0002;
                if (cw_bit != 0x0002) // Bit is already 0
                {
                    (rxPDO[1]->controlWord) |= 0x0002;
                }
                else if (cw_bit == 0x0002) // Bit is already 1
                {
                    (rxPDO[1]->controlWord) -= 0x0002;
                }
            }

        } while (!((arrived_data_size == required_packet_size) && (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) && (first_tick == 0)));

        *_current_position = (txPDO[1]->data[9]) + ((txPDO[1]->data[10]) << 8) + ((txPDO[1]->data[11]) << 16) + ((txPDO[1]->data[12]) << 24);
        int8_t error_bit = (txPDO[1]->data[13]);
        return error_bit;
    }
    else
        return -1;
}

int EL6021::DXL_WriteTargetPosition(int _target_position)
{
    if (system_ready)
    {
        //////////////////////////////////////////////
        ///////////// packet assemble  ///////////////
        //////////////////////////////////////////////

        // instruction data settings
        int param_num = 6;
        int param_length = param_num + 3;
        int data_length = param_num + 10;

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

        // copy to rxPDO
        for (int i = 0; i < data_length - 2; i++)
        {
            rxPDO[1]->data[i] = send_data[i];
        }
        rxPDO[1]->data[data_length - 2] = CRC_L;
        rxPDO[1]->data[data_length - 1] = CRC_H;

        ///////////////////////////
        // printf("[Start] cw: %x, sw: %x\n", (rxPDO[1]->controlWord) & 0x0003, (txPDO[1]->statusWord) & 0x0003);

        // Beckohff - ControlWord (transfer 24 bytes)
        int parse_data_size = data_length;
        // Beckohff - ControlWord (transfer 24 bytes)
        int required_packet_size = 11; // write instruction always receivce 11 bytes status

        int cw_bit = (rxPDO[1]->controlWord) & 0x0001;
        (rxPDO[1]->controlWord) &= 0x00FE;
        (rxPDO[1]->controlWord) |= (parse_data_size << 8);
        if (cw_bit == 1)
        {
            (rxPDO[1]->controlWord) &= 0xFFFE;
            // printf("[Send] test point 1 (cw: %x)\n", rxPDO[1]->controlWord);
        }
        else if (cw_bit == 0)
        {
            (rxPDO[1]->controlWord) += 0x0001;
            // printf("[Send] test point 2 (cw: %x)\n", rxPDO[1]->controlWord);
        }
        // printf("[Send-1] controlword (BIT 0): %x > %x\n", cw_bit, (rxPDO[1]->controlWord) & 0x0001);

        int sw_bit = (txPDO[1]->statusWord) & 0x0001;
        do
        {
            ProcessOneCycleCommand();
        } while (((txPDO[1]->statusWord) & 0x0001) == sw_bit);
        // printf("[Send-2] statusword (BIT 0): %x > %x\n", sw_bit, (txPDO[1]->statusWord) & 0x0001);
        //  printf("[Send-Done] Command send. (cw: %x, sw: %x)\n"
        //      , (rxPDO[1]->controlWord) & 0x0003
        //      , (txPDO[1]->statusWord) & 0x0003);

        /////////////////////////
        ///// wait response /////
        /////////////////////////
        int test_tick = 0;
        int arrived_data_size = 0;

        sw_bit = (txPDO[1]->statusWord) & 0x0002;
        cw_bit = (rxPDO[1]->controlWord) & 0x0002;

        int first_tick = 0;
        (txPDO[1]->statusWord) &= 0x00ff;
        do
        {
            // usleep(100);

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
            arrived_data_size = ((txPDO[1]->statusWord) >> 8) + ((txPDO[1]->statusWord) >> 12) * 16;

            if (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) // if previous statusword != current statusword
            {
                // printf("\n[Read-1] statusword (BIT 1): %x > %x\n", sw_bit, (txPDO[1]->statusWord) & 0x0002);

                // controlword acknowledgement
                cw_bit = (rxPDO[1]->controlWord) & 0x0002;
                if (cw_bit != 0x0002) // Bit is already 0
                {
                    (rxPDO[1]->controlWord) |= 0x0002;
                }
                else if (cw_bit == 0x0002) // Bit is already 1
                {
                    (rxPDO[1]->controlWord) -= 0x0002;
                }
                // printf("[Read-2] controlword (BIT 1): %x > %x\n", cw_bit, (rxPDO[1]->controlWord) & 0x0002);
            }

        } while (!((arrived_data_size == required_packet_size) && (sw_bit != ((txPDO[1]->statusWord) & 0x0002)) && (first_tick == 0)));

        int8_t error_bit = (txPDO[1]->data[8]);
        return error_bit;
    }
    else
        return -1;
}

unsigned short EL6021::DXL_UpdateCRC(uint16_t crc_accum, uint8_t *data_blk_ptr, uint16_t data_blk_size)
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

bool EL6021::IsSystemReady()
{
    return system_ready;
}

bool EL6021::ProcessOneCycleCommand()
{
    ec_send_processdata();
    wkc = ec_receive_processdata(EC_TIMEOUTRET);
    // wkc = ec_receive_processdata(100000);

    if (wkc >= expected_WKC)
        return true;
    else
        return false;
}

int EL6021::GetStatusWord(int slave)
{
    return (txPDO[slave]->statusWord) & 0xffff;
}

int EL6021::GetControlWord(int slave)
{
    return (rxPDO[slave]->controlWord) & 0x000f;
}