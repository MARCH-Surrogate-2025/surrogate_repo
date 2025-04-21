#include "CustomEtherCAT.h"

int CustomEtherCAT::InitSlaves(char *ifname)
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
        printf("[RT-SoemEcat] %d slaves are found. (name: %s)\n", ec_slavecount, ec_slave[1].name);
        return ec_slavecount;
    }
}

int CustomEtherCAT::ConfigurePDO()
{
    int retval = 0;
    int retval_PDO = 0;
    uint8_t setZero = 0x00;
    uint8_t rxPDO_size = 0x03;
    uint8_t txPDO_size = 0x03;
    for (int slave = 1; slave <= ec_slavecount; slave++)
    {
        // receive PDO
        // uint32_t map_1600[3] = {0x60400010, 0x60600008, 0x60ff0020};
        uint32_t map_1600[3] = {0x60400010, 0x60600008, 0x60710010};
        uint16_t map_1c12[2] = {0x0001, 0x1600};

        // transmit PDO
        uint32_t map_1a00[3] = {0x60410010, 0x60610008, 0x606C0020};
        uint16_t map_1c13[2] = {0x0001, 0x1a00};
        if (EnableMotor(0))
        {
            rt_printf("\n\033[1;32m[RT-SoemEcat] motors are enabled.\033[0m\n");
        }
        else
        {
            rt_printf("\033[1;31m\n[RT-SoemEcat] failed to process command.\033[0m\n");
        }

        retval += ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE * 4);
        retval_PDO += ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
        retval_PDO += ec_SDOwrite(slave, 0x1601, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
        retval_PDO += ec_SDOwrite(slave, 0x1602, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
        retval_PDO += ec_SDOwrite(slave, 0x1603, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);

        /* Define RxPdo */
        for (int i = 0; i < rxPDO_size; i++)
        {
            retval_PDO += ec_SDOwrite(slave, 0x1600, 0x01 + i, FALSE, sizeof(map_1600[i]), &(map_1600[i]), EC_TIMEOUTSAFE);
        }
        retval_PDO += ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(rxPDO_size), &rxPDO_size, EC_TIMEOUTSAFE);

        retval += ec_SDOwrite(slave, 0x1c12, 0x01, FALSE, sizeof(map_1c12[1]), &(map_1c12[1]), EC_TIMEOUTSAFE * 4);
        retval += ec_SDOwrite(slave, 0x1c12, 0x00, FALSE, sizeof(map_1c12[0]), &(map_1c12[0]), EC_TIMEOUTSAFE * 4);

        /* Clear TxPdo */
        retval += ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE * 4);
        retval_PDO += ec_SDOwrite(slave, 0x1a00, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
        retval_PDO += ec_SDOwrite(slave, 0x1a01, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
        retval_PDO += ec_SDOwrite(slave, 0x1a02, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);
        retval_PDO += ec_SDOwrite(slave, 0x1a03, 0x00, FALSE, sizeof(setZero), &setZero, EC_TIMEOUTSAFE);

        /* Define TxPdo*/
        for (int i = 0; i < txPDO_size; i++)
        {
            retval_PDO += ec_SDOwrite(slave, 0x1a00, 0x01 + i, FALSE, sizeof(map_1a00[i]), &(map_1a00[i]), EC_TIMEOUTSAFE);
        }
        retval_PDO += ec_SDOwrite(slave, 0x1a00, 0x00, FALSE, sizeof(txPDO_size), &txPDO_size, EC_TIMEOUTSAFE);

        retval += ec_SDOwrite(slave, 0x1c13, 0x01, FALSE, sizeof(map_1c13[1]), &(map_1c13[1]), EC_TIMEOUTSAFE * 4);
        retval += ec_SDOwrite(slave, 0x1c13, 0x00, FALSE, sizeof(map_1c13[0]), &(map_1c13[0]), EC_TIMEOUTSAFE * 4);
    }

    ec_config_map(&IOmap);
    ec_configdc();

    for (int slave = 1; slave <= ec_slavecount; slave++)
    {
        rxPDO[slave - 1] = (struct wheel_rx *)(ec_slave[slave].outputs);
        txPDO[slave - 1] = (struct wheel_tx *)(ec_slave[slave].inputs);
    }
    printf("\033[1;32m[RT-SoemEcat] slaves mapped, state to SAFE_OP.\033[0m\n");

    // wait for all slaves to reach SAFE_OP state
    ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE * 4);

    printf("\033[1;32m[RT-SoemEcat] request operational state for all slaves.\033[0m\n");
    expected_WKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
    printf("\033[1;32mRT-SoemEcat] calculated workcounter: %d.\033[0m\n", expected_WKC);
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

    // int state_val = ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE * 4);
    // PRINT_BLU
    // rt_printf("[RT-SoemEcat] actual state: %d \n", state_val);
    // PRINT_NRM

    if (ec_slave[0].state == EC_STATE_OPERATIONAL)
    {
        printf("\033[1;32m[RT-SoemCat] slave 1 reached operational state.\033[0m\n");
        system_ready = true;
    }

    return 0;
}

bool CustomEtherCAT::IsSystemReady()
{
    return system_ready;
}

bool CustomEtherCAT::ProcessOneCycleCommand()
{
    ec_send_processdata();
    wkc = ec_receive_processdata(EC_TIMEOUTRET);

    if (wkc >= expected_WKC)
        return true;
    else
        return false;
}

int CustomEtherCAT::SendCommand(int slave, int mode, int val)
{
    if (system_ready)
    {
        rxPDO[slave]->controlWord = CONTROLWORD_COMMAND_ENABLEOPERATION;
        rxPDO[slave]->modeOfOperation = mode;
        // rxPDO[slave]->targetVelocity = val;
        rxPDO[slave]->targetTorque = val;
        return ProcessOneCycleCommand();
    }
    else
        return -1;
}

int CustomEtherCAT::EnableMotor(int slave)
{
    do
    {
        if (!ProcessOneCycleCommand())
        {
            return -1;
        }
        status_now = (txPDO[slave]->statusWord) & STATUSWORD_STATE_MASK;
        if (status_now == STATUSWORD_STATE_SWITCHEDONDISABLED)
        {
            rxPDO[slave]->controlWord = CONTROLWORD_COMMAND_SHUTDOWN;
        }
        else if (status_now == STATUSWORD_STATE_READYTOSWTICHON)
        {
            rxPDO[slave]->controlWord = CONTROLWORD_COMMAND_SWITCHON_ENABLEOPERATION;
        }
        usleep(1000);
    } while (((txPDO[slave]->statusWord) & STATUSWORD_STATE_MASK) != STATUSWORD_STATE_OPERATIONENABLED);

    rxPDO[slave]->controlWord = CONTROLWORD_COMMAND_ENABLEOPERATION;
    // rxPDO[slave]->modeOfOperation = CYCLIC_SYNC_VELOCITY_MODE;
    rxPDO[slave]->modeOfOperation = CYCLIC_SYNC_TORQUE_MODE;
    // rxPDO[slave]->targetVelocity = 0;
    rxPDO[slave]->targetTorque = 0;

    return 1;
}

int CustomEtherCAT::DisableMotor(int slave)
{
    rxPDO[slave]->controlWord = CONTROLWORD_COMMAND_ENABLEOPERATION;
    rxPDO[slave]->modeOfOperation = CYCLIC_SYNC_VELOCITY_MODE;
    // rxPDO[slave]->targetVelocity = 0;

    do
    {
        if (!ProcessOneCycleCommand())
        {
            return -1;
        }
        status_now = (txPDO[slave]->statusWord) & STATUSWORD_STATE_MASK;
        if (status_now == STATUSWORD_STATE_OPERATIONENABLED)
        {
            rxPDO[slave]->controlWord = CONTROLWORD_COMMAND_DISABLEOPERATION;
        }
        usleep(1000);
    } while (((txPDO[slave]->statusWord) & STATUSWORD_STATE_MASK) != STATUSWORD_STATE_SWITCHEDON);

    return 1;
}

int CustomEtherCAT::GetStatusWord(int slave)
{
    return (txPDO[slave]->statusWord) & STATUSWORD_STATE_MASK;
}

int CustomEtherCAT::GetControlWord(int slave)
{
    return (rxPDO[slave]->controlWord) & CONTROLWORD_COMMAND_ENABLEOPERATION_MASK;
}