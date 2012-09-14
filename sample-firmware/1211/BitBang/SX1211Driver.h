/*******************************************************************
** File        : SX1211driver.h                                   **
********************************************************************
**                                                                **
** Version     : V 1.0                                            **
**                                                                **
** Written by  : Chaouki ROUAISSIA                                **
**                                                                **
** Date        : 07-12-2006                                       **
**                                                                **
** Project     : API-1211                                         **
**                                                                **
********************************************************************
**                                                                **
** Changes     : V 2.4 / CRo - 06-08-2007                         **
**               - No change                                      **
**               V 2.4.1 / CRo - 23-04-2008                       **
**               - Corrected Passive filter settings              **
**                                                                **
********************************************************************
** Description : SX1211 transceiver drivers implementation for the**
**               XE8000 family products !!! Continuous mode !!!   **
*******************************************************************/
#ifndef __SX1211DRIVER__
#define __SX1211DRIVER__

/*******************************************************************
** Include files                                                  **
*******************************************************************/
#include "Globals.h"

/*******************************************************************
** Global definitions                                             **
*******************************************************************/
/*******************************************************************
** RF packet definition                                           **
*******************************************************************/
#define RF_BUFFER_SIZE_MAX   64
#define RF_BUFFER_SIZE       64

/*******************************************************************
** RF State machine                                               **
*******************************************************************/
#define RF_STOP              0x01
#define RF_BUSY              0x02
#define RF_RX_DONE           0x04
#define RF_TX_DONE           0x08
#define RF_ERROR             0x10
#define RF_TIMEOUT           0x20

/*******************************************************************
** RF function return codes                                       **
*******************************************************************/
#define OK                   0x00
#define ERROR                0x01
#define RX_TIMEOUT           0x02
#define RX_RUNNING           0x03
#define TX_TIMEOUT           0x04
#define TX_RUNNING           0x05

/*******************************************************************
** I/O Ports Definitions                                          **
*******************************************************************/
#if defined(_XE88LC01A_) || defined(_XE88LC05A_)
	#define PORTO              RegPCOut
	#define PORTI              RegPCIn
	#define PORTDIR            RegPCDir
   #define PORTP              RegPCPullup

#endif
#if defined(_XE88LC02_) || defined(_XE88LC02_4KI_) || defined(_XE88LC02_8KI_)
	#define PORTO              RegPD1Out
	#define PORTI              RegPD1In
	#define PORTDIR            RegPD1Dir
   #define PORTP              RegPD1Pullup
	
#endif
#if defined(_XE88LC06A_) || defined(_XE88LC07A_) 
	#define PORTO              RegPDOut
	#define PORTI              RegPDIn
	#define PORTDIR            RegPDDir
   #define PORTP              RegPDPullup

#endif

/*******************************************************************
** Port A pins definitions                                        **
********************************************************************
**                                  *  uC     * SX1211    * PAx   **
*******************************************************************/
#define IRQ_1           0x02      //*  In     *  Out      * PA1 // DCLK (Rx/Tx)
#define IRQ_0           0x08      //*  In     *  Out      * PA3 // SYNC or RSSI
//#define PLL_LOCK        0x10      //*  In     *  Out      * PA4

/*******************************************************************
** Port B pins definitions                                        **
********************************************************************
**                                  *  uC     * SX1211    * PBx   **
*******************************************************************/

/*******************************************************************
** Port C pins definitions                                        **
********************************************************************
**                                  *  uC     * SX1211    * PCx   **
*******************************************************************/
#define DATA            0x01      //*  Out/In *  In/Out   * PC0 // DATA (IN/OUT)
#define NSS_CONFIG      0x08      //*  Out    *  In       * PC3
#define SCK             0x10      //*  Out    *  In       * PC4
#define MOSI            0x20      //*  Out    *  In       * PC5
#define MISO            0x40      //*  In     *  Out      * PC6

/*******************************************************************
** SX1211 SPI Macros definitions                                  **
*******************************************************************/
#define SPIInit()           (PORTDIR = (PORTDIR | SCK | NSS_CONFIG | MOSI) & (~MISO))
#define SPIClock(level)     ((level) ? (PORTO |= SCK) : (PORTO &= ~SCK))
#define SPIMosi(level)      ((level) ? (PORTO |= MOSI) : (PORTO &= ~MOSI))
#define SPINssConfig(level) ((level) ? (PORTO |= NSS_CONFIG) : (PORTO &= ~NSS_CONFIG))
#define SPIMisoTest()       (PORTI & MISO)

/*******************************************************************
** SX1211 definitions                                             **
*******************************************************************/

/*******************************************************************
** SX1211 Operating modes definition                              **
*******************************************************************/
#define RF_SLEEP                         0x00
#define RF_STANDBY                       0x20
#define RF_SYNTHESIZER                   0x40
#define RF_RECEIVER                      0x60
#define RF_TRANSMITTER                   0x80


/*******************************************************************
** SX1211 Internal registers Address                              **
*******************************************************************/
#define REG_MCPARAM1                     0x00
#define REG_MCPARAM2                     0x01
#define REG_FDEV                         0x02
#define REG_BITRATE                      0x03
#define REG_OOKFLOORTHRESH               0x04
#define REG_MCPARAM6                     0x05 //Not used in Continuous mode but kept for simplicity
#define REG_R1                           0x06
#define REG_P1                           0x07
#define REG_S1                           0x08
#define REG_R2                           0x09
#define REG_P2                           0x0A
#define REG_S2                           0x0B
#define REG_PARAMP                       0x0C

#define REG_IRQPARAM1                    0x0D
#define REG_IRQPARAM2                    0x0E
#define REG_RSSIIRQTHRESH                0x0F

#define REG_RXPARAM1                     0x10
#define REG_RXPARAM2                     0x11
#define REG_RXPARAM3                     0x12
#define REG_RES19                        0x13 
#define REG_RSSIVALUE                    0x14 
#define REG_RXPARAM5                     0x15 

#define REG_SYNCBYTE1                    0x16 
#define REG_SYNCBYTE2                    0x17 
#define REG_SYNCBYTE3                    0x18 
#define REG_SYNCBYTE4                    0x19 

#define REG_TXPARAM                      0x1A

#define REG_OSCPARAM                     0x1B

//PKTParam section not used in continuous mode

/*******************************************************************
** SX1211 initialisation register values definition               **
*******************************************************************/
#define DEF_MCPARAM1                     0x00
#define DEF_MCPARAM2                     0x00
#define DEF_FDEV                         0x00
#define DEF_BITRATE                      0x00
#define DEF_OOKFLOORTHRESH               0x00
#define DEF_MCPARAM6                     0x00
#define DEF_R1                           0x00
#define DEF_P1                           0x00
#define DEF_S1                           0x00
#define DEF_R2                           0x00
#define DEF_P2                           0x00
#define DEF_S2                           0x00
#define DEF_PARAMP                       0x20

#define DEF_IRQPARAM1                    0x00
#define DEF_IRQPARAM2                    0x08
#define DEF_RSSIIRQTHRESH                0x00

#define DEF_RXPARAM1                     0x00
#define DEF_RXPARAM2                     0x08
#define DEF_RXPARAM3                     0x00
#define DEF_RES19                        0x07 
#define DEF_RSSIVALUE                    0x00 
#define DEF_RXPARAM6                     0x00 

#define DEF_SYNCBYTE1                    0x00 
#define DEF_SYNCBYTE2                    0x00 
#define DEF_SYNCBYTE3                    0x00 
#define DEF_SYNCBYTE4                    0x00 

#define DEF_TXPARAM                      0x00

#define DEF_OSCPARAM                     0x00


/*******************************************************************
** SX1211 bit control definition                                  **
*******************************************************************/
// MC Param 1
// Chip operating mode
#define RF_MC1_SLEEP                     0x00
#define RF_MC1_STANDBY                   0x20
#define RF_MC1_SYNTHESIZER               0x40
#define RF_MC1_RECEIVER                  0x60
#define RF_MC1_TRANSMITTER               0x80

// Frequency band
#define RF_MC1_BAND_915L                 0x00
#define RF_MC1_BAND_915H                 0x08
#define RF_MC1_BAND_868                  0x10
#define RF_MC1_BAND_950                  0x10

// VCO trimming
#define RF_MC1_VCO_TRIM_00               0x00
#define RF_MC1_VCO_TRIM_01               0x02
#define RF_MC1_VCO_TRIM_10               0x04
#define RF_MC1_VCO_TRIM_11               0x06

// RF frequency selection
#define RF_MC1_RPS_SELECT_1              0x00
#define RF_MC1_RPS_SELECT_2              0x01

// MC Param 2
// Modulation scheme selection
#define RF_MC2_MODULATION_OOK            0x40
#define RF_MC2_MODULATION_FSK            0x80

// Data operation mode
#define RF_MC2_DATA_MODE_CONTINUOUS      0x00
#define RF_MC2_DATA_MODE_BUFFERED        0x20
#define RF_MC2_DATA_MODE_PACKET          0x04

// Rx OOK threshold mode selection
#define RF_MC2_OOK_THRESH_TYPE_FIXED     0x00
#define RF_MC2_OOK_THRESH_TYPE_PEAK      0x08
#define RF_MC2_OOK_THRESH_TYPE_AVERAGE   0x10

// Gain on IF chain
#define RF_MC2_GAIN_IF_00                0x00
#define RF_MC2_GAIN_IF_01                0x01
#define RF_MC2_GAIN_IF_10                0x02
#define RF_MC2_GAIN_IF_11                0x03
 

// Frequency deviation (kHz)
#define RF_FDEV_33                       0x0B
#define RF_FDEV_40                       0x09
#define RF_FDEV_50                       0x07
#define RF_FDEV_80                       0x04
#define RF_FDEV_100                      0x03
#define RF_FDEV_133                      0x02
#define RF_FDEV_200                      0x01


// Bitrate (bit/sec)  
#define RF_BITRATE_1600                  0x7C
#define RF_BITRATE_2000                  0x63
#define RF_BITRATE_2500                  0x4F
#define RF_BITRATE_5000                  0x27
#define RF_BITRATE_8000                  0x18
#define RF_BITRATE_10000                 0x13
#define RF_BITRATE_20000                 0x09
#define RF_BITRATE_25000                 0x07
#define RF_BITRATE_40000                 0x04
#define RF_BITRATE_50000                 0x03
//100000 bps not supported by API in continuous mode (CPU speed limitation)


// OOK threshold
#define RF_OOKFLOORTHRESH_VALUE          0x0C 


// MC Param 6
//!!!Not used in Continuous mode!!!

// SynthR1
#define RF_R1_VALUE                      0x77

// SynthP1
#define RF_P1_VALUE                      0x64

// SynthS1
#define RF_S1_VALUE                      0x32


// SynthR2
#define RF_R2_VALUE                      0x74

// SynthP2
#define RF_P2_VALUE                      0x62

// SynthS2
#define RF_S2_VALUE                      0x32

// PA ramp times in OOK
#define RF_PARAMP_00                     0x00
#define RF_PARAMP_01                     0x08
#define RF_PARAMP_10                     0x10
#define RF_PARAMP_11                     0x18

// IRQ Param 1
// Select RX IRQ_0 sources (Continuous mode)
#define RF_IRQ0_RX_SYNC                  0x00
#define RF_IRQ0_RX_RSSI                  0x40

// Select RX IRQ_1 sources (Continuous mode)
// !!Always DCLK, but just for lisibility!!
#define RF_IRQ1_RX_DCLK                  0x00

// Select TX IRQ_1 sources (Continuous mode)
// !!Always DCLK, but just for lisibility!!
#define RF_IRQ1_TX_DCLK                  0x00


// IRQ Param 2
// RSSI IRQ flag
#define RF_IRQ2_RSSI_IRQ_CLEAR           0x04

// PLL_Locked flag
#define RF_IRQ2_PLL_LOCK_CLEAR           0x02

// PLL_Locked pin
#define RF_IRQ2_PLL_LOCK_PIN_OFF         0x00
#define RF_IRQ2_PLL_LOCK_PIN_ON          0x01

// RSSI threshold for interrupt
#define RF_RSSIIRQTHRESH_VALUE           0x00


// RX Param 1
// Passive filter (kHz)
#define RF_RX1_PASSIVEFILT_65            0x00
#define RF_RX1_PASSIVEFILT_82            0x10
#define RF_RX1_PASSIVEFILT_109           0x20
#define RF_RX1_PASSIVEFILT_137           0x30
#define RF_RX1_PASSIVEFILT_157           0x40
#define RF_RX1_PASSIVEFILT_184           0x50
#define RF_RX1_PASSIVEFILT_211           0x60
#define RF_RX1_PASSIVEFILT_234           0x70
#define RF_RX1_PASSIVEFILT_262           0x80
#define RF_RX1_PASSIVEFILT_321           0x90
#define RF_RX1_PASSIVEFILT_378           0xA0
#define RF_RX1_PASSIVEFILT_414           0xB0
#define RF_RX1_PASSIVEFILT_458           0xC0
#define RF_RX1_PASSIVEFILT_514           0xD0
#define RF_RX1_PASSIVEFILT_676           0xE0
#define RF_RX1_PASSIVEFILT_987           0xF0

// Butterworth filter (kHz)
#define RF_RX1_FC_VALUE                  0x03
// !!! Values defined below only apply if RFCLKREF = DEFAULT VALUE = 0x07 !!!
#define RF_RX1_FC_FOPLUS25               0x00
#define RF_RX1_FC_FOPLUS50               0x01
#define RF_RX1_FC_FOPLUS75               0x02
#define RF_RX1_FC_FOPLUS100              0x03
#define RF_RX1_FC_FOPLUS125              0x04
#define RF_RX1_FC_FOPLUS150              0x05
#define RF_RX1_FC_FOPLUS175              0x06
#define RF_RX1_FC_FOPLUS200              0x07
#define RF_RX1_FC_FOPLUS225              0x08
#define RF_RX1_FC_FOPLUS250              0x09



// RX Param 2
// Polyphase filter center value (kHz)
#define RF_RX2_FO_VALUE                  0x03
// !!! Values defined below only apply if RFCLKREF = DEFAULT VALUE = 0x07 !!!
#define RF_RX2_FO_50                     0x10
#define RF_RX2_FO_75                     0x20
#define RF_RX2_FO_100                    0x30
#define RF_RX2_FO_125                    0x40
#define RF_RX2_FO_150                    0x50
#define RF_RX2_FO_175                    0x60
#define RF_RX2_FO_200                    0x70
#define RF_RX2_FO_225                    0x80
#define RF_RX2_FO_250                    0x90
#define RF_RX2_FO_275                    0xA0
#define RF_RX2_FO_300                    0xB0
#define RF_RX2_FO_325                    0xC0
#define RF_RX2_FO_350                    0xD0
#define RF_RX2_FO_375                    0xE0
#define RF_RX2_FO_400                    0xF0


// Rx Param 3
// Polyphase filter enable
#define RF_RX3_POLYPFILT_ON              0x80
#define RF_RX3_POLYPFILT_OFF             0x00

// Bit synchronizer
#define RF_RX3_BITSYNC_ON                0x00
#define RF_RX3_BITSYNC_OFF               0x40

// Sync word recognition
#define RF_RX3_SYNC_RECOG_ON             0x20
#define RF_RX3_SYNC_RECOG_OFF            0x00

// Size of the reference sync word
#define RF_RX3_SYNC_SIZE_8               0x00
#define RF_RX3_SYNC_SIZE_16              0x08
#define RF_RX3_SYNC_SIZE_24              0x10
#define RF_RX3_SYNC_SIZE_32              0x18

// Number of tolerated errors for the sync word recognition
#define RF_RX3_SYNC_TOL_0                0x00
#define RF_RX3_SYNC_TOL_1                0x02
#define RF_RX3_SYNC_TOL_2                0x04
#define RF_RX3_SYNC_TOL_3                0x06


// RSSI Value (Read only)


// Rx Param 6
// Decrement step of RSSI threshold in OOK demodulator (peak mode)
#define RF_RX6_OOK_THRESH_DECSTEP_000    0x00
#define RF_RX6_OOK_THRESH_DECSTEP_001    0x20
#define RF_RX6_OOK_THRESH_DECSTEP_010    0x40
#define RF_RX6_OOK_THRESH_DECSTEP_011    0x60
#define RF_RX6_OOK_THRESH_DECSTEP_100    0x80
#define RF_RX6_OOK_THRESH_DECSTEP_101    0xA0
#define RF_RX6_OOK_THRESH_DECSTEP_110    0xC0
#define RF_RX6_OOK_THRESH_DECSTEP_111    0xE0 

// Decrement period of RSSI threshold in OOK demodulator (peak mode)
#define RF_RX6_OOK_THRESH_DECPERIOD_000  0x00
#define RF_RX6_OOK_THRESH_DECPERIOD_001  0x04
#define RF_RX6_OOK_THRESH_DECPERIOD_010  0x08
#define RF_RX6_OOK_THRESH_DECPERIOD_011  0x0C
#define RF_RX6_OOK_THRESH_DECPERIOD_100  0x10
#define RF_RX6_OOK_THRESH_DECPERIOD_101  0x14
#define RF_RX6_OOK_THRESH_DECPERIOD_110  0x18
#define RF_RX6_OOK_THRESH_DECPERIOD_111  0x1C

// Cutoff freq of average function of RSSI threshold in OOK demodulator (average mode)
#define RF_RX6_OOK_THRESH_AVERAGING_00   0x00
#define RF_RX6_OOK_THRESH_AVERAGING_11   0x03


// TX Param 
// Interpolator filter Tx (kHz)
#define RF_TX_FC_VALUE                   0x70
// !!! Values defined below only apply if RFCLKREF = DEFAULT VALUE = 0x07 !!!
#define RF_TX_FC_25                      0x00
#define RF_TX_FC_50                      0x10
#define RF_TX_FC_75                      0x20
#define RF_TX_FC_100                     0x30
#define RF_TX_FC_125                     0x40
#define RF_TX_FC_150                     0x50
#define RF_TX_FC_175                     0x60
#define RF_TX_FC_200                     0x70
#define RF_TX_FC_225                     0x80
#define RF_TX_FC_250                     0x90
#define RF_TX_FC_275                     0xA0
#define RF_TX_FC_300                     0xB0
#define RF_TX_FC_325                     0xC0
#define RF_TX_FC_350                     0xD0
#define RF_TX_FC_375                     0xE0
#define RF_TX_FC_400                     0xF0

// Tx Output power (dBm)
#define RF_TX_POWER_MAX                  0x00
#define RF_TX_POWER_PLUS10               0x02
#define RF_TX_POWER_PLUS7                0x04
#define RF_TX_POWER_PLUS4                0x06
#define RF_TX_POWER_PLUS1                0x08
#define RF_TX_POWER_MINUS2               0x0A


// OSC Param
// CLKOUT enable
#define RF_OSC_CLKOUT_ON                 0x80
#define RF_OSC_CLKOUT_OFF                0x00

// CLKOUT frequency (kHz)
#define RF_OSC_CLKOUT_12800              0x00
#define RF_OSC_CLKOUT_6400               0x04
#define RF_OSC_CLKOUT_3200               0x08
#define RF_OSC_CLKOUT_2133               0x0C
#define RF_OSC_CLKOUT_1600               0x10
#define RF_OSC_CLKOUT_1280               0x14
#define RF_OSC_CLKOUT_1067               0x18
#define RF_OSC_CLKOUT_914                0x1C
#define RF_OSC_CLKOUT_800                0x20
#define RF_OSC_CLKOUT_711                0x24
#define RF_OSC_CLKOUT_640                0x28
#define RF_OSC_CLKOUT_582                0x2C
#define RF_OSC_CLKOUT_533                0x30
#define RF_OSC_CLKOUT_492                0x34
#define RF_OSC_CLKOUT_457                0x38
#define RF_OSC_CLKOUT_427                0x3C


/*******************************************************************
** Timings section : all timing described here are done with the  **
**                   A&B counters cascaded                        **
*******************************************************************/

/*******************************************************************
**             -- SX1211 Recommended timings --                   **
********************************************************************
** These timings depends on the RC frequency                      **
** The following values corresponds to a 2.4576 MHz RC Freq       **
** Times calculation formula                                      **
**                                                                **
** CounterA&B value = RC * wanted time  = 2 457 600  * 2 ms = 4915**
**                                                                **
*******************************************************************/
#define TS_OS          12288 // Quartz Osc wake up time, max 5 ms
#define TS_FS           1966 // Freq Synth wake-up time from OS, max 800 us
#define TS_RE           1229 // Receiver wake-up time from FS, max 500 us
#define TS_TR           1229 // Transmitter wake-up time from FS, max 500 us

/*******************************************************************
**                                                                **
**                     // RF_BUFFER_SIZE * 8 *     (4  + 1)          \          \
** CounterA&B value = || -------------------------------------------- | * 128 Hz | + 1
**                     \\                 4 * BitRate                /          /
**                                                                **
** The plus 1 at the end of formula is required for the highest   **
** baudrate as the resulting timeout is lower than the 1 / 128Hz  **
*******************************************************************/
#define RF_FRAME_TIMEOUT(BitRate) (_U16)(_F32)((((_F32)((_U32)RF_BUFFER_SIZE * (_U32)8 *((_U32)4  + (_U32)1)) / (_F32)((_U32)4 * (_U32)BitRate)) * (_F32)128) + (_F32)1)

/*******************************************************************
** Functions prototypes                                           **
*******************************************************************/

/*******************************************************************
** Configuration functions                                        **
*******************************************************************/

/*******************************************************************
** InitRFChip : This routine initializes the RFChip registers     **
**              Using Pre Initialized variable                    **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/ 
void InitRFChip (void);

/*******************************************************************
** SetRFMode : Sets the SX1211 operating mode (Sleep, Receiver,   **
**           Transmitter)                                         **
********************************************************************
** In  : mode                                                     **
** Out : -                                                        **
*******************************************************************/
void SetRFMode(_U8 mode);

/*******************************************************************
** WriteRegister : Writes the register value at the given address **
**                  on the SX1211                                 **
********************************************************************
** In  : address, value                                           **
** Out : -                                                        **
*******************************************************************/
void WriteRegister(_U8 address, _U16 value);
 
/*******************************************************************
** ReadRegister : Reads the register value at the given address on**
**                the SX1211                                      **
********************************************************************
** In  : address                                                  **
** Out : value                                                    **
*******************************************************************/
_U16 ReadRegister(_U8 address);

/*******************************************************************
** Communication functions                                        **
*******************************************************************/

/*******************************************************************
** SendRfFrame : Sends a RF frame                                 **
********************************************************************
** In  : *buffer, size                                            **
** Out : *pReturnCode                                             **
*******************************************************************/
void SendRfFrame(_U8 *buffer, _U8 size, _U8 *pReturnCode);

/*******************************************************************
** ReceiveRfFrame : Receives a RF frame                           **
********************************************************************
** In  : -                                                        **
** Out : *buffer, size, *pReturnCode                              **
*******************************************************************/
void ReceiveRfFrame(_U8 *buffer, _U8 *size, _U8 *pReturnCode);

/*******************************************************************
** SendByte : Sends a data to the transceiver LSB first           **
**                                                                **
********************************************************************
** In  : b                                                        **
** Out : -                                                        **
*******************************************************************/
void SendByte(_U8 b);

/*******************************************************************
** ReceiveByte : Receives a data from the transceiver LSB first   **
**                                                                **                                                 
********************************************************************
** In  : -                                                        **
** Out : b                                                        **
*******************************************************************/
_U8 ReceiveByte(void);

/*******************************************************************
** Transceiver specific functions                                 **
*******************************************************************/

/*******************************************************************
** ReadRssi : Reads the Rssi value from  SX1211                   **
********************************************************************
** In  : -                                                        **
** Out : value                                                    **
*******************************************************************/
_U16 ReadRssi(void);

/*******************************************************************
** Utility functions                                              **
*******************************************************************/

/*******************************************************************
** Wait : This routine uses the counter A&B to create a delay     **
**        using the RC ck source                                  **
********************************************************************
** In   : cntVal                                                  **
** Out  : -                                                       **
*******************************************************************/
void Wait(_U16 cntVal);

/*******************************************************************
** TxEventsOn : Initializes the timers and the events related to  **
**             the TX routines                                    **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void TxEventsOn(void);

/*******************************************************************
** TxEventsOff : Initializes the timers and the events related to **
**             the TX routines                                    **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void TxEventsOff(void);

/*******************************************************************
** RxEventsOn : Initializes the timers and the events related to  **
**             the RX routines                                    **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void RxEventsOn(void);

/*******************************************************************
** RxEventsOff : Initializes the timers and the events related to **
**             the RX routines                                    **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void RxEventsOff(void);

/*******************************************************************
** InvertByte : Inverts a byte. MSB -> LSB, LSB -> MSB            **
********************************************************************
** In  : b                                                        **
** Out : b                                                        **
*******************************************************************/
_U8 InvertByte(_U8 b); 
 
/*******************************************************************
** SpiInOut : Sends and receives a byte from the SPI bus          **
********************************************************************
** In  : outputByte                                               **
** Out : inputByte                                                **
*******************************************************************/
_U8 SpiInOut (_U8 outputByte);

#endif /* __SX1211DRIVER__ */
