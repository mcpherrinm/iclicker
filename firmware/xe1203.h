/*
 * Definitions for interfacing with the XE1203 RF Transceiver
 */

// Pin Definitions
// Port B, SPI Comms with respect to RF Transceiver
#define EN           0x04
#define SI           0x08
#define SO           0x10
#define SCLK         0x20

// Port D, RF Data Comms
#define DCLK         0x01
#define DATA         0x02
#define PATTERN      0x03
#define SWITCH       0x04

// Default Configuration
#define REG_DEFAULT  0x00

// Configuration Registers
#define REG_CONFIGSW 0x00
#define REG_RTPARAM1 0x01
#define REG_RTPARAM2 0x02
#define REG_FSPARAM1 0x03
#define REG_FSPARAM2 0x04
#define REG_FSPARAM3 0x05
#define REG_SWPARAM1 0x06
#define REG_SWPARAM2 0x07
#define REG_SWPARAM3 0x08
#define REG_SWPARAM4 0x09
#define REG_SWPARAM5 0x0A
#define REG_SWPARAM6 0x0B
#define REG_DATAOUT1 0x0C
#define REG_DATAOUT2 0x0D
#define REG_ADPARAM1 0x0E
#define REG_ADPARAM2 0x0F
#define REG_ADPARAM3 0x10
#define REG_ADPARAM4 0x11
#define REG_ADPARAM5 0x12
#define REG_PATTERN1 0x13
#define REG_PATTERN2 0x14
#define REG_PATTERN3 0x15
#define REG_PATTERN4 0x16
