#ifndef EL6021_EL6021_H
#define EL6021_EL6021_H

#define WHEEL_NUM               2

// EL6021 StatusWord //
#define EL6021_STATUSWORD_BIT0_TRANSMIT_ACCEPTED       0
#define EL6021_STATUSWORD_BIT1_RECEIVE_REQUEST         1
#define EL6021_STATUSWORD_BIT2_INIT_ACCEPTED           2
#define EL6021_STATUSWORD_INITIALIZE_MASK              0x00FF
#define EL6021_STATUSWORD_SEND_MASK                    0x0005
#define EL6021_STATUSWORD_RECEIVE_MASK                 0x1702
#define EL6021_STATUSWORD_INITIALIZED                  0x0004
#define EL6021_STATUSWORD_READYTOEXCHANGE              0x0000

// EL6021 ControlWord //
#define EL6021_CONTROLWORD_BIT0_TRANSMIT_REQUEST       0
#define EL6021_CONTROLWORD_BIT1_RECEIVE_ACCEPTED       1
#define EL6021_CONTROLWORD_BIT2_INIT_REQUEST           2
#define EL6021_CONTROLWORD_BIT3_SEND_CONTINUOUS        3
#define EL6021_CONTROLWORD_INITIALIZE                  0x0004
#define EL6021_CONTROLWORD_PREPARE                     0x0000


// EL6021 COM Settings //
#define EL6021_COMSETTINGS_INDEX                        0x8000
#define EL6021_ENABLE_HALF_DUPLEX_SUBINDEX              0x06
#define EL6021_ENABLE_HALF_DUPLEX_SIZE                  1
#define EL6021_BAUDRATE_SUBINDEX                        0x11
#define EL6021_BAUDRATE_SIZE                            4
#define EL6021_DATAFRAME_SUBINDEX                       0x11
#define EL6021_DATAFRAME_SIZE                           4

// EL6021 Baudrate //int command, int target00                           9
#define EL6021_BAUDRATE_115200                          10
#define EL6021_DATAFRAME_8N1                            3

//////////////////////////////////////////
//////////////// Dynamixel ///////////////
//////////////////////////////////////////
// Dynamixel Command //
#define DXL_COMMAND_READ                2
#define DXL_COMMAND_WRITE               3
#define DXL_COMMAND_SET_TARGET_POSITION 4

// Dynamixel Parameters Address //
#define DXL_ADDR_TORQUE_ENABLE          64
#define DXL_ADDR_LED                    65
#define DXL_ADDR_GOAL_VELOCITY          104
#define DXL_ADDR_PROFILE_ACCELERATION   108
#define DXL_ADDR_PROFILE_VELOCITY       112
#define DXL_ADDR_GOAL_POSITION          116
#define DXL_ADDR_PRESENT_VELOCITY       128
#define DXL_ADDR_PRESENT_POSITION       132
#define DXL_ADDR_VELOCITY_TRAJECTORY    136
#define DXL_ADDR_POSITION_TRAJECTORY    140
#define DXL_ADDR_PRESENT_TEMPERATURE    146

// Dynamixel Parameters Size //
#define DXL_SIZE_TORQUE_ENABLE          1
#define DXL_SIZE_LED                    1
#define DXL_SIZE_POSITION               4
#define DXL_SIZE_TEMPERATURE            1


#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "ethercat.h"
#include "CustomDynamixel.h"

class EL6021
{
    public:
        #pragma pack(push,1)
        struct EL6021_rx
        {
            uint16_t controlWord;
            uint8_t data[22];
        };
        #pragma pack(pop)
        union rx_packet
        {
            uint8_t byte[24];
            EL6021_rx val;
        };

        #pragma pack(push,1)
        struct EL6021_tx
        {
            uint16_t statusWord;
            uint8_t data[22];
        };
        #pragma pack(pop)
        union tx_packet
        {
            uint8_t byte[24];
            EL6021_tx val;
        };

        struct EL6021_rx *rxPDO[WHEEL_NUM];
        struct EL6021_tx *txPDO[WHEEL_NUM];



        int InitSlaves(char* ifname);
        bool IsSystemReady();
        bool ProcessOneCycleCommand();

        int GetStatusWord(int slave);
        int GetControlWord(int slave);

        // EL6021
        int SetSDO();
        int SendCommand();
        int UpdateCommand(int target_position, int *current_position);
        int PrepareCommunication(int slave);

        // Dynamixel
        // CustomDynamixel dxl_write();
        // CustomDynamixel dxl_read;
        int DXL_EnableMotor();
        int DXL_DisableMotor();
        int DXL_ReadCurrentPosition(int* _current_position);
        int DXL_WriteTargetPosition(int _target_position);
        int DXL_ParseCommand();
        int DXL_PrepareCommand();
        unsigned short DXL_UpdateCRC(uint16_t crc_accum, uint8_t *data_blk_ptr, uint16_t data_blk_size);

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
