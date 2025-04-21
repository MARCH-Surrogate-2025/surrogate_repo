#ifndef PDO_PDO_H
#define PDO_PDO_H

// -- Object Dictionary -- //

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
#define     STATUSWORD_SWITCHEDONDISABLED               0x0040
#define     STATUSWORD_WARNING                          0x0080
#define     STATUSWORD_MANIFACTORSPECIFIC               0x0100
#define     STATUSWORD_REMOTE                           0x0200
#define     STATUSWORD_TARGET_REACHED                   0x0400
#define     STATUSWORD_INTERNALLLIMITACTIVE             0x0800
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


// #define CONTROLWORD_
// #define CONTROLWORD_DISABLE     0x01
// #define CONTROLWORD_QUICKSTOP   0x02
// #define CONTROLWORD_SHUTDOWN    0x06
// #define CONTROLWORD_SWITCHON    0x07
// #define CONTROLWORD_
// #define CONTROLWORD_
// #define CONTROLWORD_


// #define CONTROLWORD_FAULTRESET  0x80
// #define CONTROLWORD_ENABLEOP    0x0f

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

// ---------------------- //

// #include <stdint.h>

// #pragma pack(push,1)
// struct wheel_rx
// {
//     uint16_t controlWord;
//     int8_t modeOfOperation;
//     int32_t targetVelocity;
// };

// struct wheel_tx
// {
//     uint16_t statusWord;
//     int8_t modeOfOperationDisplay;
//     int32_t velocityActualValue;
// };
// #pragma pack(pop)

// struct wheel_rx *rxPDO[WHEEL_NUM];
// struct wheel_tx *txPDO[WHEEL_NUM];



#endif