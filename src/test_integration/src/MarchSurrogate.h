#ifndef MARCH_SURROGATE_H
#define MARCH_SURROGATE_H

#define SLAVES_NUM          4
#define WHEEL_IDX           1
#define ECAT_COUPLER_IDX    2
#define STEERING_IDX        3
#define BALANCING_IDX       4

#pragma region Wheel Dictionary
/*----------------------------------------
-   ControlWord
------------------------------------------*/
#define CIA402_CONTROL_WORD                             0x6040
#define     CONTROLWORD_BITO_SWITCHON                   0x0001
#define     CONTROLWORD_BIT1_ENABLE_VOLTAGE             0x0002
#define     CONTROLWORD_BIT2_QUICK_STOP                 0x0004
#define     CONTROLWORD_BIT3_ENABLE_OPERATION           0x0008

#define     CONTROLWORD_BIT4_HOMINGSTART                0x0010
#define     CONTROLWORD_BIT4_NEWSETPOINT                0x0010
#define     CONTROLWORD_BIT5_IMMEDIATELY                0x0020
#define     CONTROLWORD_BIT6_RELATIVE                   0x0040
#define     CONTROLWORD_BIT8_HALT                       0x0100

// Control Command Mask
#define CONTROLWORD_COMMAND_SHUTDOWN_MASK               0x0087
#define CONTROLWORD_COMMAND_SWITCHON_MASK               0x008F
#define CONTROLWORD_COMMAND_SWITCHON_ENABLEOP_MASK      0x008F
#define CONTROLWORD_COMMAND_DISABLEVOLTAGE_MASK         0x0082
#define CONTROLWORD_COMMAND_QUICKSTOP_MASK              0x0086
#define CONTROLWORD_COMMAND_DISABLEOPERATION_MASK       0x008F
#define CONTROLWORD_COMMAND_ENABLEOPERATION_MASK        0x008F
#define CONTROLWORD_COMMAND_FAULTRESET_MASK             0x0080

// ControlWord Command
#define CONTROLWORD_COMMAND_SHUTDOWN                    0x0006
#define CONTROLWORD_COMMAND_SWITCHON                    0x0007
#define CONTROLWORD_COMMAND_SWITCHON_ENABLEOPERATION    0x000F
#define CONTROLWORD_COMMAND_DISABLEVOLTAGE              0x0000
#define CONTROLWORD_COMMAND_QUICKSTOP                   0x0002
#define CONTROLWORD_COMMAND_DISABLEOPERATION            0x0007
#define CONTROLWORD_COMMAND_ENABLEOPERATION             0x000F
#define CONTROLWORD_COMMAND_FAULTRESET                  0x0080

/*----------------------------------------
-   StatusWord
------------------------------------------*/
#define CIA402_STATUS_WORD                              0x6041
#define     STATUSWORD_STATE_MASK                       0x006F
#define     STATUSWORD_READYTOSWITCHON                  0x0001
#define     STATUSWORD_SWITCHEDON                       0x0002
#define     STATUSWORD_OPERATIONENABLE                  0x0004
#define     STATUSWORD_FAULT                            0x0008
#define     STATUSWORD_VOLTAGE_ENABLED                  0x0010
#define     STATUSWORD_ISQUICKSTOP                      0x0020
#define     STATUSWORD_SWITCHEDONDISABLED            #define SLAVES_NUM      2
// #define BALANCER_IDX    2     0x0800
#define     STATUSWORD_MODE_MASK                        0x3000
#define     STATUSWORD_SET_POINT_ACK                    0x1000
#define     STATUSWORD_ZERO_SPEED                       0x1000
#define     STATUSWORD_HOMING_ATTAINED                  0x1000
#define     STATUSWORD_FOLLOWING_ERROR                  0x2000
#define     STATUSWORD_HOMING_ERROR                     0x2000
#define     STATUSWORD_ABS_POSITION_VALID               0x4000

#define STATUSWORD_STATE_NOTREADYTOSWITCHON             0X0000
#define STATUSWORD_STATE_SWITCHEDONDISABLED             0X0040
#define STATUSWORD_STATE_READYTOSWTICHON                0X0021
#define STATUSWORD_STATE_SWITCHEDON                     0X0023
#define STATUSWORD_STATE_OPERATIONENABLED               0X0027
#define STATUSWORD_STATE_QUICKSTOPACTIVE                0X0007
#define STATUSWORD_STATE_FAULTREACTIONACTIVE            0X000F
#define STATUSWORD_STATE_FAULT                          0X0008

#define CIA402_OPERATION_MODE                           0x6060
#define     NO_MODE                                     0
#define     PROFILE_POSITION_MODE                       1
#define     VELOCITY_MODE                               2
#define     PROFILE_VELOCITY_MODE                       3
#define     PROFILE_TORQUE_MODE                         4
#define     HOMING_MODE                                 6
#define     INTERPOLATION_POSITION_MODE                 7
#define     CYCLIC_SYNC_POSITION_MODE                   8
#define     CYCLIC_SYNC_VELOCITY_MODE                   9
#define     CYCLIC_SYNC_TORQUE_MODE                     10

#pragma endregion

#pragma region Steering Dictionary (EL6021, Dynamixel)
// EL6021 StatusWord //
#define EL6021_STATUSWORD_BIT0_TRANSMIT_ACCEPTED 0
#define EL6021_STATUSWORD_BIT1_RECEIVE_REQUEST 1
#define EL6021_STATUSWORD_BIT2_INIT_ACCEPTED 2
#define EL6021_STATUSWORD_INITIALIZE_MASK 0x00FF
#define EL6021_STATUSWORD_SEND_MASK 0x0005
#define EL6021_STATUSWORD_RECEIVE_MASK 0x1702
#define EL6021_STATUSWORD_INITIALIZED 0x0004
#define EL6021_STATUSWORD_READYTOEXCHANGE 0x0000

// EL6021 ControlWord //
#define EL6021_CONTROLWORD_BIT0_TRANSMIT_REQUEST 0
#define EL6021_CONTROLWORD_BIT1_RECEIVE_ACCEPTED 1
#define EL6021_CONTROLWORD_BIT2_INIT_REQUEST 2
#define EL6021_CONTROLWORD_BIT3_SEND_CONTINUOUS 3
#define EL6021_CONTROLWORD_INITIALIZE 0x0004
#define EL6021_CONTROLWORD_PREPARE 0x0000

// EL6021 COM Settings - Index
#define EL6021_COMSETTINGS_INDEX                0x8000
#define EL6021_BAUDRATE_INDEX                   0x4073
#define EL6021_BAUDRATE_SUBINDEX                0x0
#define EL6021_ENABLE_HALF_DUPLEX_INDEX         0x4075
#define EL6021_ENABLE_HALF_DUPLEX_SUBINDEX      0x01
#define EL6021_ENABLE_OPTIMIZATIN_INDEX         0x4075
#define EL6021_ENABLE_OPTIMIZATIN_SUBINDEX      0x06
#define EL6021_DATAFRAME_INDEX                  0x4075
#define EL6021_DATAFRAME_SUBINDEX               0x11

// EL6021 COMM Settings - Size
#define EL6021_ENABLE_HALF_DUPLEX_SIZE          1
#define EL6021_BAUDRATE_SIZE                    2
#define EL6021_OPTIMIZATION_SIZE                1
#define EL6021_DATAFRAME_SIZE                   2

// EL6021 COMM Settings - Value
#define EL6021_BAUDRATE_9600                    0x06
#define EL6021_BAUDRATE_19200                   0x07
#define EL6021_BAUDRATE_38400                   0x08
#define EL6021_BAUDRATE_57600                   0x09
#define EL6021_BAUDRATE_115200                  0x0A
#define EL6021_DATAFRAME_8N1                    3

//////////////////////////////////////////
//////////////// Dynamixel ///////////////
//////////////////////////////////////////
// Dynamixel Command //
#define DXL_COMMAND_READ 2
#define DXL_COMMAND_WRITE 3
#define DXL_COMMAND_SET_TARGET_POSITION 4

// Dynamixel Parameters Address //
#define DXL_ADDR_TORQUE_ENABLE 64
#define DXL_ADDR_LED 65
#define DXL_ADDR_GOAL_VELOCITY 104
#define DXL_ADDR_PROFILE_ACCELERATION 108
#define DXL_ADDR_PROFILE_VELOCITY 112
#define DXL_ADDR_GOAL_POSITION 116
#define DXL_ADDR_PRESENT_VELOCITY 128
#define DXL_ADDR_PRESENT_POSITION 132
#define DXL_ADDR_VELOCITY_TRAJECTORY 136
#define DXL_ADDR_POSITION_TRAJECTORY 140
#define DXL_ADDR_PRESENT_TEMPERATURE 146

// Dynamixel Parameters Size //
#define DXL_SIZE_TORQUE_ENABLE 1
#define DXL_SIZE_LED 1
#define DXL_SIZE_POSITION 4
#define DXL_SIZE_TEMPERATURE 1

#pragma endregion

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "ethercat.h"

class MarchSurrogate
{
public:
#pragma pack(push, 1)
    struct wheel_rx
    {
        uint16_t controlWord;
        int8_t modeOfOperation;
        int32_t targetVelocity;
    };
    struct wheel_tx
    {
        uint16_t statusWord;
        int8_t modeOfOperationDisplay;
        int32_t velocityActualValue;
    };
#pragma pack(pop)

    struct wheel_rx *rxPDO1;
    struct wheel_tx *txPDO1;


#pragma pack(push, 1)
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

#pragma pack(push, 1)
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

    struct EL6021_rx *rxPDO2;     // Wheel
    struct EL6021_tx *txPDO2;
    struct EL6021_rx *rxPDO3;     // Balancer
    struct EL6021_tx *txPDO3;

    int InitSlaves(char *ifname);
    int ConfigureSDO(int _slave, uint16_t _baud, bool _is_RS485, bool _tr_opt, uint16_t _data_frame); // steering
    int ConfigurePDO(int _slave); // wheel
    int PrepareOperation();
    int PrepareCommunication(int _slave);
    
    bool IsSystemReady();
    bool ProcessOneCycleCommand();

    int GetStatusWord(int _slave);
    int GetControlWord(int _slave);

    // Wheel
    int Wheel_EnableMotor();
    int Wheel_DisableMotor();
    int Wheel_SendCommand(int _mode, int _val);

    // Dynamixel
    int DXL_EnableMotor();
    int DXL_DisableMotor();
    int DXL_ReadCurrentPosition(int *_current_position);
    int DXL_WriteTargetPosition(int _target_position);
    int DXL_ReadCurrentPosition_PrepareCommand();
    int DXL_ReadCurrentPosition_ParseCommand(int *_current_position);
    int DXL_WriteTargetPosition_PrepareCommand(int _target_position);
    int DXL_WriteTargetPosition_ParseCommand();
    int DXL_ParseCommand();
    int DXL_PrepareCommand();
    unsigned short DXL_UpdateCRC(uint16_t crc_accum, uint8_t *data_blk_ptr, uint16_t data_blk_size);

    // Balancer
    int BLC_UpdateCommand_INTG(uint _command, double _torque, int *_state
                                , double *_imu_x, double *_imu_y, double *_imu_z, double *_imu_gy, uint8_t *_read_byte_arr);
    int BLC_PrepareCommand(uint _command, double _torque);
    int BLC_ParseCommand(int *_state, double *_imu_x, double *_imu_y, double *_imu_z, double *_imu_gy
                        , double *_steering, int *_velocity, uint8_t *_read_byte_arr);

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

    uint8_t cw0=0, cw1=0, cw2=0, cw3=0;
    uint8_t sw0=0, sw1=0, sw2=0, sw3=0;
    uint8_t write_flag[SLAVES_NUM] = {0, 0, 0, 0};
    uint8_t read_flag[SLAVES_NUM] = {0, 0, 0, 0};
    typedef enum
    {
        READY = 0,
        ACTIVATED,
        ACKNOWLEDGED,
        INIT_REQUEST,
        INIT_DONE,
    }COMM_RES;

};

#endif
