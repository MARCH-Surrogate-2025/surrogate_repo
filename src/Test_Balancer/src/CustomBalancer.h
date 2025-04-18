#ifndef CUSTOM_BALANCER_H
#define CUSTOM_BALANCER_H

// #define SLAVES_NUM      2
// #define BALANCER_IDX    2
#define SLAVES_NUM      4
#define BALANCER_IDX    4

#pragma region Dictionary (EL6021)
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

// EL6021 COM Settings //
#define EL6021_COMSETTINGS_INDEX 0x8000
#define EL6021_ENABLE_HALF_DUPLEX_SUBINDEX 0x06
#define EL6021_ENABLE_HALF_DUPLEX_SIZE 1
#define EL6021_ENABLE_POINT_TO_POINT_SUBINDEX 0x07
#define EL6021_ENABLE_POINT_TO_POINT_SIZE 1
#define EL6021_BAUDRATE_SUBINDEX 0x11
#define EL6021_BAUDRATE_SIZE 1
#define EL6021_DATAFRAME_SUBINDEX 0x11
#define EL6021_DATAFRAME_SIZE 4

// EL6021 Baudrate //int command, int target00                           9
#define EL6021_BAUDRATE_9600    6
#define EL6021_BAUDRATE_19200   7
#define EL6021_BAUDRATE_38400   8
#define EL6021_BAUDRATE_57600   9
#define EL6021_BAUDRATE_115200  10
#define EL6021_DATAFRAME_8N1 3

#pragma endregion

#pragma region T-Motor Dictionary
typedef enum
{
    COMM_FW_VERSION = 0,
    COMM_JUMP_TO_BOOTLOADER,
    COMM_ERASE_NEW_APP,
    COMM_WRITE_NEW_APP_DATA,
    COMM_GET_VALUES,             // Get motor operating parameters
    COMM_SET_DUTY,               // Motor operates in duty cycle mode
    COMM_SET_CURRENT,            // Motor operates in current loop mode
    COMM_SET_CURRENT_BRAKE,      // Motor operates in current brake mode
    COMM_SET_RPM,                // Motor operates in speed loop mode
    COMM_SET_POS,                // Motor operates in position loop mode
    COMM_SET_HANDBRAKE,          // Motor operates in handbrake current loop mode
    COMM_SET_DETECT,             // Motor real-time feedback current position command
    COMM_ROTOR_POSITION = 22,    // Motor feedback current position
    COMM_GET_VALUES_SETUP = 50,  // Motor single or multiple parameter acquisition command
    COMM_SET_POS_SPD = 91,       // Motor operates in position-speed loop mode
    COMM_SET_POS_MULTI = 92,     // Set motor motion to single-turn mode
    COMM_SET_POS_SINGLE = 93,    // Set motor motion to multi-turn mode, range ±100 turns
    COMM_SET_POS_UNLIMITED = 94, // Reserved
    COMM_SET_POS_ORIGIN = 95,    // Set motor origin
} COMM_PACKET_ID;

typedef enum
{
    FAULT_CODE_NONE = 0,
    FAULT_CODE_OVER_VOLTAGE,                          // Overvoltage
    FAULT_CODE_UNDER_VOLTAGE,                         // Undervoltage
    FAULT_CODE_DRV,                                   // Driver fault
    FAULT_CODE_ABS_OVER_CURRENT,                      // Motor overcurrent
    FAULT_CODE_OVER_TEMP_FET,                         // MOS overtemperature
    FAULT_CODE_OVER_TEMP_MOTOR,                       // Motor overtemperature
    FAULT_CODE_GATE_DRIVER_OVER_VOLTAGE,              // Driver overvoltage
    FAULT_CODE_GATE_DRIVER_UNDER_VOLTAGE,             // Driver undervoltage
    FAULT_CODE_MCU_UNDER_VOLTAGE,                     // MCU undervoltage
    FAULT_CODE_BOOTING_FROM_WATCHDOG_RESET,           // Undervoltage
    FAULT_CODE_ENCODER_SPI,                           // SPI encoder fault
    FAULT_CODE_ENCODER_SINCOS_BELOW_MIN_AMPLITUDE,    // Encoder below minimumamplitude
    FAULT_CODE_ENCODER_SINCOS_ABOVE_MAX_AMPLITUDE,    // Encoder above maximumamplitude
    FAULT_CODE_FLASH_CORRUPTION,                      // Flash fault
    FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_1,          // Current sampling channel 1 fault
    FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_2,          // Current sampling channel 2 fault
    FAULT_CODE_HIGH_OFFSET_CURRENT_SENSOR_3,          // Current sampling channel 3 fault
    FAULT_CODE_UNBALANCED_CURRENTS,                   // Unbalanced currents
} mc_fault_code;

#pragma endregion

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <string>

#include "ethercat.h"

class CustomBalancer
{
public:
    #pragma pack(push, 1)
    struct balancer_rx
    {
        uint16_t controlWord;
        uint8_t data[22];
    };
    #pragma pack(pop)
    union rx_packet
    {
        uint8_t byte[24];
        balancer_rx val;
    };

    #pragma pack(push, 1)
    struct balancer_tx
    {
        uint16_t statusWord;
        uint8_t data[22];
    };
    #pragma pack(pop)
    union tx_packet
    {
        uint8_t byte[24];
        balancer_tx val;
    };

    struct balancer_rx *rxPDO;
    struct balancer_tx *txPDO;

    int InitSlaves(char *ifname);
    int ConfigureSDO(int _slave); // balancer
    int PrepareOperation();
    int PrepareCommunication(int _slave);

    bool IsSystemReady();
    bool ProcessOneCycleCommand();

    int GetStatusWord(int _slave);
    int GetControlWord(int _slave);

    // Dynamixel
    int BLC_GetMotorInfo();
    int BLC_UpdateCommand(uint _command, double _torque, int *_state, double *_imu_x, double *_imu_y, double *_imu_z);
    int BLC_UpdateCommand_2(uint _command, double _torque, int *_state
                            , double *_imu_x, double *_imu_y, double *_imu_z, double *_imu_gy
                            , double *_steering, int16_t *_velocity
                            , uint8_t *err1, uint8_t *err2, uint8_t *err3, uint8_t *_read_byte_arr);
    int BLC_WriteCommand(uint _command, double _torque);
    int BLC_ReadState(int *_state, double *_imu_x, double *_imu_y, double *_imu_z);

    int BLC_EnableMotor();
    int BLC_DisableMotor();
    int BLC_ReadCurrentPosition(int *_current_position);
    int BLC_WriteTargetPosition(int _target_position);
    int BLC_ParseCommand();
    int BLC_PrepareCommand();
    unsigned short BLC_UpdateCRC(unsigned char *buf, unsigned int len);

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
    uint8_t write_flag = 0;
    uint8_t read_flag = 0;
    typedef enum
    {
        READY = 0,
        ACTIVATED,
        ACKNOWLEDGED,
        INIT_REQUEST,
        INIT_DONE,
    }COMM_RES;
    int test_var_1=0;
    int error_count=0;
};

#endif