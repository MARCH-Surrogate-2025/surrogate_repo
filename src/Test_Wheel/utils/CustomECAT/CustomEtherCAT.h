#ifndef CustomEtherCAT_CustomEtherCAT_H
#define CustomEtherCAT_CustomEtherCAT_H

#define WHEEL_NUM 1

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "ethercat.h"
#include "PDO.h"

class CustomEtherCAT {
public:
    #pragma pack(push,1)
    struct wheel_rx
    {
        uint16_t controlWord;
        int8_t modeOfOperation;
        // int32_t targetVelocity;
        int16_t targetTorque;
    };
    struct wheel_tx
    {
        uint16_t statusWord;
        int8_t modeOfOperationDisplay;
        int32_t velocityActualValue;
    };
    #pragma pack(pop)

    struct wheel_rx *rxPDO[WHEEL_NUM];
    struct wheel_tx *txPDO[WHEEL_NUM];


public:
    int InitSlaves(char* ifname);
    int ConfigurePDO();
    bool IsSystemReady();
    bool ProcessOneCycleCommand();

    int SendCommand(int slave, int mode, int val);
    int EnableMotor(int slave);
    int DisableMotor(int slave);

    int GetStatusWord(int slave);
    int GetControlWord(int slave);

private:
    int slave_num;
    bool system_ready;

    char IOmap[4096];
    int expected_WKC;
    bool need_lf;
    volatile int wkc;
    bool in_OP;
    int current_group = 0;

    int command_now;
    int status_now;
};


#endif