/*******************************************************************
** File        : XE1205driver.c                                   **
********************************************************************
**                                                                **
** Version     : V 1.0                                            **
**                                                                **
** Written by   : Miguel Luis                                     **
**                                                                **
** Date        : 19-01-2004                                       **
**                                                                **
** Project     : API-1205                                         **
**                                                                **
********************************************************************
** Changes     : V 2.1 / MiL - 24-04-2004                         **
**                                                                **
**             : V 2.2 / MiL - 30-07-2004                         **
**               - Removed workaround for RX/TX switch FIFO clear **
**                 (chip correction)                              **
**                                                                **
** Changes     : V 2.3 / CRo - 06-06-2006                         **
**               - TempRFState added in ReceiveRfFrame            **
**                                                                **
** Changes     : V 2.4 / CRo - 09-01-2007                         **
**               - No change                                      **
**                                                                **
**                                                                **
********************************************************************
** Description : XE1205 transceiver drivers Implementation for the**
**               XE8000 family products (1205 buffered mode)      **
*******************************************************************/

/*******************************************************************
** Include files                                                  **
*******************************************************************/
#include "XE1205Driver.h"

/*******************************************************************
** Global variables                                               **
*******************************************************************/
static   _U8 RFState = RF_STOP;     // RF state machine
static   _U8 *pRFFrame;             // Pointer to the RF frame
static   _U8 RFFramePos;            // RF frame current position
static   _U8 RFFrameSize;           // RF frame size
static  _U16 ByteCounter = 0;       // RF frame byte counter
static   _U8 PreMode = RF_SLEEP;    // Previous chip operating mode
volatile _U8 EnableSyncByte = true; // Enables/disables the synchronization byte reception/transmission
static   _U8 SyncByte;              // RF synchronization byte counter
static   _U8 PatternSize = 4;       // Size of pattern detection
static   _U8 StartByte[4];          // Pattern detection values
static  _U32 RFFrameTimeOut = RF_FRAME_TIMEOUT(1200); // Reception counter value (full frame timeout generation)

_U16 RegistersCfg[] = { // 1205 configuration registers values
    DEF_MCPARAM1 | RF_MC1_SLEEP | RF_MC1_MODE_CHIP | RF_MC1_BUFFERED_MODE_ON | RF_MC1_DATA_UNIDIR_OFF | RF_MC1_BAND_915 | RF_MC1_FREQ_DEV_MSB_1,
    DEF_MCPARAM2 | RF_MC2_FDEV_200,
    DEF_MCPARAM3 | RF_MC3_KONNEX_OFF | RF_MC3_BAUDRATE_4800,
    DEF_MCPARAM4,
    DEF_MCPARAM5,

    DEF_IRQPARAM1 | RF_IRQ0_RX_IRQ_OFF | RF_IRQ1_RX_IRQ_OFF | RF_IRQ1_TX_IRQ_TX_STOPPED,
    DEF_IRQPARAM2 | RF_IRQ2_START_FILL_PATTERN_DET | RF_IRQ2_START_FILL_FIFO_STOP | RF_IRQ2_START_TX_FIFO_FULL | RF_IRQ2_RSSI_IRQ_OFF | RF_IRQ2_RSSI_THRES_VTHR1,

    DEF_TXPARAM1 | RF_TX1_POWER_0 | RF_TX1_MODUL_ON | RF_TX1_FILTER_OFF | RF_TX1_FIX_BSYNC_NORMAL | RF_TX1_SELECT_DEMOD_1,

    DEF_RXPARAM1 | RF_RX1_BITSYNC_ON | RF_RX1_BW_200 | RF_RX1_BW_MAX_ON | RF_RX1_REG_BW_START_UP | RF_RX1_BOSST_FILTER_START_UP,
    DEF_RXPARAM2 | RF_RX2_RSSI_OFF | RF_RX2_LOW_RANGE | RF_RX2_FEI_OFF | RF_RX2_AFC_STOP | RF_RX2_AFC_CORRECTION_ON,
    DEF_RXPARAM3 | RF_RX3_IQAMP_OFF | RF_RX3_RMODE_MODE_A | RF_RX3_PATTERN_ON | RF_RX3_P_SIZE_32 | RF_RX3_P_TOL_0,

    DEF_RXPARAM6 | 0x69, // Pattern 1
    DEF_RXPARAM7 | 0x81, // Pattern 2
    DEF_RXPARAM8 | 0x7E, // Pattern 3
    DEF_RXPARAM9 | 0x96, // Pattern 4

    DEF_OSCPARAM1 | RF_OSC1_OSC_INT | RF_OSC1_CLKOUT_OFF | RF_OSC1_CLK_FREQ_1_22_MHZ,
    DEF_OSCPARAM2 | RF_OSC2_RES_X_OSC_3800
};

/*******************************************************************
** Configuration functions                                        **
*******************************************************************/

/*******************************************************************
** InitRFChip : This routine initializes the RFChip registers     **
**              Using Pre Initialized variables                   **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitRFChip (void){
        _U16 i;
    // Initializes XE1205
    SPIInit();
    set_bit(PORTO, (SCK + NSS_DATA + NSS_CONFIG + MOSI));
    set_bit(PORTP, (SCK + NSS_DATA + NSS_CONFIG + MOSI));

    for(i = 0; (i + 2) <= REG_OSCPARAM2; i++){
        if(i < REG_RXPARAM4){
            WriteRegister(i, RegistersCfg[i]);
        }
        else{
            WriteRegister(i + 2, RegistersCfg[i]);
        }
    }

    PatternSize = ((RegistersCfg[REG_RXPARAM3] >> 2) & 0x03) + 1;
    for(i = 0; i < PatternSize; i++){
        StartByte[i] = RegistersCfg[REG_RXPARAM6 - 2 + i];
    }

    if(RegistersCfg[REG_MCPARAM3] == RF_MC3_BAUDRATE_1200){
        RFFrameTimeOut = RF_FRAME_TIMEOUT(1200);
    }
    else if(RegistersCfg[REG_MCPARAM3] == RF_MC3_BAUDRATE_2400){
        RFFrameTimeOut = RF_FRAME_TIMEOUT(2400);
    }
    else if(RegistersCfg[REG_MCPARAM3] == RF_MC3_BAUDRATE_4800){
        RFFrameTimeOut = RF_FRAME_TIMEOUT(4800);
    }
    else if(RegistersCfg[REG_MCPARAM3] == RF_MC3_BAUDRATE_9600){
        RFFrameTimeOut = RF_FRAME_TIMEOUT(9600);
    }
    else if(RegistersCfg[REG_MCPARAM3] == RF_MC3_BAUDRATE_19200){
        RFFrameTimeOut = RF_FRAME_TIMEOUT(19200);
    }
    else if(RegistersCfg[REG_MCPARAM3] == RF_MC3_BAUDRATE_38400){
        RFFrameTimeOut = RF_FRAME_TIMEOUT(38400);
    }
    else if(RegistersCfg[REG_MCPARAM3] == RF_MC3_BAUDRATE_76800){
        RFFrameTimeOut = RF_FRAME_TIMEOUT(76800);
    }
    else if(RegistersCfg[REG_MCPARAM3] == RF_MC3_BAUDRATE_153600){
        RFFrameTimeOut = RF_FRAME_TIMEOUT(153600);
    }
    else {
        RFFrameTimeOut = RF_FRAME_TIMEOUT(1200);
    }

    SetRFMode(RF_SLEEP);
#ifdef __DEBUG__
    WriteRegister(25, 0x10); // Debug copy the modulator input to the pin DATA
#endif
}

/*******************************************************************
** SetMode : Sets the XE1205 operating mode                       **
********************************************************************
** In  : mode                                                     **
** Out : -                                                        **
*******************************************************************/
void SetRFMode(_U8 mode){
    if(mode != PreMode){
        if((mode == RF_TRANSMITTER) && (PreMode == RF_SLEEP)){
            PreMode = RF_TRANSMITTER;
            WriteRegister(REG_MCPARAM1, (RegistersCfg[REG_MCPARAM1] & 0x3F) | RF_STANDBY);
            // wait TS_OS
            Wait(TS_OS);
            WriteRegister(REG_MCPARAM1, (RegistersCfg[REG_MCPARAM1] & 0x3F) | RF_TRANSMITTER);
            // wait TS_STR
            Wait(TS_STR);
        }
        else if((mode == RF_TRANSMITTER) && (PreMode == RF_RECEIVER)){
            PreMode = RF_TRANSMITTER;
            WriteRegister(REG_MCPARAM1, (RegistersCfg[REG_MCPARAM1] & 0x3F) | RF_TRANSMITTER);
            // wait TS_TR
            Wait(TS_TR);
        }
        else if((mode == RF_RECEIVER) && (PreMode == RF_SLEEP)){
            PreMode = RF_RECEIVER;
            WriteRegister(REG_MCPARAM1, (RegistersCfg[REG_MCPARAM1] & 0x3F) | RF_STANDBY);
            // wait TS_OS
            Wait(TS_OS);
            WriteRegister(REG_MCPARAM1, (RegistersCfg[REG_MCPARAM1] & 0x3F) | RF_RECEIVER);
            // wait TS_SRE
            Wait(TS_SRE);
        }
        else if((mode == RF_RECEIVER) && (PreMode == RF_TRANSMITTER)){
            PreMode = RF_RECEIVER;
            WriteRegister(REG_MCPARAM1, (RegistersCfg[REG_MCPARAM1] & 0x3F) | RF_RECEIVER);
            // wait TS_RE
            Wait(TS_RE);
        }
        else if(mode == RF_SLEEP){
            PreMode = RF_SLEEP;
            WriteRegister(REG_MCPARAM1, (RegistersCfg[REG_MCPARAM1] & 0x3F) | RF_SLEEP);
        }
        else{
            PreMode = RF_SLEEP;
            WriteRegister(REG_MCPARAM1, (RegistersCfg[REG_MCPARAM1] & 0x3F) | RF_SLEEP);
        }
    }
}

/*******************************************************************
** WriteRegister : Writes the register value at the given address **
**                  on the XE1205                                 **
********************************************************************
** In  : address, value                                           **
** Out : -                                                        **
*******************************************************************/
void WriteRegister(_U8 address, _U16 value){
    SPIInit();

    address = ((address << 1) & 0x3F) | 0x01;
    SPINssData(1);
    SPINssConfig(0);
    SpiInOut(address);
    SpiInOut(value);
    SPINssConfig(1);
}

/*******************************************************************
** ReadRegister : Reads the register value at the given address on**
**                the XE1205                                      **
********************************************************************
** In  : address                                                  **
** Out : value                                                    **
*******************************************************************/
_U16 ReadRegister(_U8 address){
    _U8 value = 0;

    SPIInit();
    SPINssData(1);
    address = ((address << 1) & 0x7F) | 0x41;
    SPINssConfig(0);
    SpiInOut(address);
    value = SpiInOut(0);
    SPINssConfig(1);

    return value;
}

/*******************************************************************
** Communication functions                                        **
*******************************************************************/

/*******************************************************************
** SendRfFrame : Sends a RF frame                                 **
********************************************************************
** In  : *buffer, size                                            **
** Out : *pReturnCode                                             **
*******************************************************************/
void SendRfFrame(_U8 *buffer, _U8 size, _U8 *pReturnCode){
    if(size > RF_BUFFER_SIZE_MAX){
        RFState |= RF_STOP;
        *pReturnCode = ERROR;
        return;
    }
    SetRFMode(RF_TRANSMITTER);

    RFState |= RF_BUSY;
    RFState &= ~RF_STOP;
    RFFrameSize = size;
    pRFFrame = buffer;

    // Select TX IRQ_1 source to FIFO full
    WriteRegister(REG_IRQPARAM1, RF_IRQ1_TX_IRQ_TX_FIFO_FULL);

    for(ByteCounter = 0; ByteCounter < 4; ){
        if(!(RegPAIn & IRQ_1)){ // If FIFO not full send a new byte
            SendByte(0xAA);
            ByteCounter++;
        }
        else{                   // If FIFO is full, waits until it is no more full
            do{
            }while((RegPAIn & IRQ_1));
        }
    }
    for(ByteCounter = 0; ByteCounter < PatternSize; ){
        if(!(RegPAIn & IRQ_1)){ // If FIFO not full send a new byte
            SendByte(StartByte[ByteCounter++]);
        }
        else{                   // If FIFO is full, waits until it is no more full
            do{
            }while((RegPAIn & IRQ_1));
        }
    }

    do{
    }while((RegPAIn & IRQ_1)); // In case the FIFO is full, waits until it is no more full

    SendByte(InvertByte(RFFrameSize));

    SyncByte = 0;

    for(ByteCounter = 0, RFFramePos = 0; ByteCounter < RFFrameSize; ){
        if(!(RegPAIn & IRQ_1)){ // If FIFO not full send a new byte
            if(SyncByte == SYNC_BYTE_FREQ){
                if(EnableSyncByte){
                    SyncByte = 0;
                    SendByte(InvertByte(0x55));
                    ByteCounter--;
                }
            }
            else{
                if(EnableSyncByte){
                    SyncByte++;
                }
                SendByte(InvertByte(pRFFrame[RFFramePos++]));
            }
            ByteCounter++;
        }
        else{                   // If FIFO is full, waits until it is no more full
            do{
            }while((RegPAIn & IRQ_1));
        }
    }

    if(!(RegPAIn & IRQ_1)){
        do{
            SendByte(0);
        }while(!(RegPAIn & IRQ_1));
    }

    // Select TX IRQ_1 source to TX stopped
    WriteRegister(REG_IRQPARAM1, RF_IRQ1_TX_IRQ_TX_STOPPED);

    do{
    }while(!(RegPAIn & IRQ_1)); // Wait for TX stopped event

    RFState |= RF_STOP;
    RFState &= ~RF_TX_DONE;
    *pReturnCode = OK;
} // void SendRfFrame(_U8 *buffer, _U8 size, _U8 *pReturnCode)

/*******************************************************************
** ReceiveRfFrame : Receives a RF frame                           **
********************************************************************
** In  : -                                                        **
** Out : *buffer, size, *pReturnCode                              **
*******************************************************************/
void ReceiveRfFrame(_U8 *buffer, _U8 *size, _U8 *pReturnCode){
    
       _U8 TempRFState;
       
       *pReturnCode = RX_RUNNING;

       TempRFState = RFState; 

    if(TempRFState & RF_STOP){
        pRFFrame = buffer;
        RFFramePos = 0;
        RFFrameSize = 2;
        SyncByte = 0;

        WriteRegister(REG_IRQPARAM1, RegistersCfg[REG_IRQPARAM1] |/**/RF_IRQ0_RX_IRQ_WRITE_BYTE /**//** /| RF_IRQ1_RX_IRQ_FIFOFULL/**/);
        WriteRegister(REG_IRQPARAM2, RegistersCfg[REG_IRQPARAM2] | RF_IRQ2_START_DETECT_CLEAR);

        RegIrqEnLow |= 0x04; // Enables Port A pin 2 interrupt IRQ_0

        SetRFMode(RF_RECEIVER);
        EnableTimeOut(true);
        RFState |= RF_BUSY;
        RFState &= ~RF_STOP;
        RFState &= ~RF_TIMEOUT;
        return;
    }
    else if(TempRFState & RF_RX_DONE){
        *size = RFFrameSize;
        *pReturnCode = OK;
        RFState |= RF_STOP;
        EnableTimeOut(false);
        RFState &= ~RF_RX_DONE;
        RegIrqEnMid &= ~0x02; // Disables Port A pin 1 interrupt IRQ_1
		RegIrqEnLow &= ~0x04; // Disables Port A pin 2 interrupt IRQ_0
        SPINssData(1);
        return;
    }
    else if(TempRFState & RF_ERROR){
        RFState |= RF_STOP;
        RFState &= ~RF_ERROR;
        *pReturnCode = ERROR;
        RegIrqEnMid &= ~0x02; // Disables Port A pin 1 interrupt IRQ_1
		RegIrqEnLow &= ~0x04; // Disables Port A pin 2 interrupt IRQ_0
        SPINssData(1);
        return;
    }
    else if(TempRFState & RF_TIMEOUT){
        RFState |= RF_STOP;
        RFState &= ~RF_TIMEOUT;
        EnableTimeOut(false);
        *pReturnCode = RX_TIMEOUT;
        RegIrqEnMid &= ~0x02; // Disables Port A pin 1 interrupt IRQ_1
		RegIrqEnLow &= ~0x04; // Disables Port A pin 2 interrupt IRQ_0
        SPINssData(1);
        return;
    }
} // void ReceiveRfFrame(_U8 *buffer, _U8 size, _U8 *pReturnCode)

/*******************************************************************
** SendByte : Sends a data to the transceiver trough the SPI      **
**            interface                                           **
********************************************************************
** In  : b                                                        **
** Out : -                                                        **
*******************************************************************/
/*******************************************************************
**  Information                                                   **
********************************************************************
** This function has been optimized to send a byte to the         **
** transceiver SPI interface as quick as possible by using        **
** IO ports.                                                      **
** If the microcontroller has an  SPI hardware interface there is **
** no need to optimize the function                               **
*******************************************************************/
void SendByte(_U8 b){
    SPIInit();
    SPINssConfig(1);
    SPINssData(0);

    SPIClock(0);
    if (b & 0x80){
        SPIMosi(1);
    }
    else{
        SPIMosi(0);
    }
    SPIClock(1);
    SPIClock(0);
    if (b & 0x40){
        SPIMosi(1);
    }
    else{
        SPIMosi(0);
    }
    SPIClock(1);
    SPIClock(0);
    if (b & 0x20){
        SPIMosi(1);
    }
    else{
        SPIMosi(0);
    }
    SPIClock(1);
    SPIClock(0);
    if (b & 0x10){
        SPIMosi(1);
    }
    else{
        SPIMosi(0);
    }
    SPIClock(1);
    SPIClock(0);
    if (b & 0x08){
        SPIMosi(1);
    }
    else{
        SPIMosi(0);
    }
    SPIClock(1);
    SPIClock(0);
    if (b & 0x04){
        SPIMosi(1);
    }
    else{
        SPIMosi(0);
    }
    SPIClock(1);
    SPIClock(0);
    if (b & 0x02){
        SPIMosi(1);
    }
    else{
        SPIMosi(0);
    }
    SPIClock(1);
    SPIClock(0);
    if (b & 0x01){
        SPIMosi(1);
    }
    else{
        SPIMosi(0);
    }
    SPIClock(1);
    SPIClock(0);
    SPIMosi(0);

    SPINssData(1);
} // void SendByte(_U8 b)

/*******************************************************************
** ReceiveByte : Receives a data from the transceiver trough the  **
**               SPI interface                                    **                                                 **
********************************************************************
** In  : -                                                        **
** Out : b                                                        **
*******************************************************************/
/*******************************************************************
**  Information                                                   **
********************************************************************
** This function has been optimized to receive a byte from the    **
** transceiver SPI interface as quick as possible by using        **
** IO ports.                                                      **
** If the microcontroller has an  SPI hardware interface there is **
** no need to optimize the function                               **
*******************************************************************/
_U8 ReceiveByte(void){
    _U8 inputByte = 0;

    SPIInit();
    SPINssConfig(1);
    SPINssData(0);

	SPIClock(0);
    SPIClock(1);
    if (SPIMisoTest()){
        inputByte |= 0x80;
    }
    SPIClock(0);
    SPIClock(1);
    if (SPIMisoTest()){
        inputByte |= 0x40;
    }
    SPIClock(0);
    SPIClock(1);
    if (SPIMisoTest()){
        inputByte |= 0x20;
    }
    SPIClock(0);
    SPIClock(1);
    if (SPIMisoTest()){
        inputByte |= 0x10;
    }
    SPIClock(0);
    SPIClock(1);
    if (SPIMisoTest()){
        inputByte |= 0x08;
    }
    SPIClock(0);
    SPIClock(1);
    if (SPIMisoTest()){
        inputByte |= 0x04;
    }
    SPIClock(0);
    SPIClock(1);
    if (SPIMisoTest()){
        inputByte |= 0x02;
    }
    SPIClock(0);
    SPIClock(1);
    if (SPIMisoTest()){
        inputByte |= 0x01;
    }
    SPIClock(0);
	SPIMosi(0);

    SPINssData(1);

	return inputByte;
}
/*******************************************************************
** Transceiver specific functions                                 **
*******************************************************************/

/*******************************************************************
** AutoFreqControl : Calibrates the receiver LO frequency to the  **
**               transmitter LO frequency                         **
********************************************************************
** In  : -                                                        **
** Out : *pReturnCode                                             **
*******************************************************************/
void AutoFreqControl(_U8 *pReturnCode){
    _S16 Result = 0;
    _S16 LoRegisterValue = 0;
    _U8 Done = false;
    _U8 TimeOut = 20;

    // Initializes the FEI to enable his reading
    InitFei();

    do{
        // FEI and LO register reading
        Result = ReadFei();
        LoRegisterValue = ReadLO();

        LoRegisterValue = LoRegisterValue - Result;

        WriteLO(LoRegisterValue);

        TimeOut --;
        if((Result >= -1) && (Result <= 1)){
            Done = true;
        }
        if(TimeOut == 0){
            *pReturnCode = RX_TIMEOUT;
            return;
        }
    }while(!Done);
    *pReturnCode = OK;
}

/*******************************************************************
** ReadLO : Reads the LO frequency value from  XE1205             **
********************************************************************
** In  : -                                                        **
** Out : value                                                    **
*******************************************************************/
_U16 ReadLO(void){
    _U16 value;

    value = ReadRegister(REG_MCPARAM4) << 8;
    value |= ReadRegister(REG_MCPARAM5) & 0xFF;

    return value;
}

/*******************************************************************
** WriteLO : Writes the LO frequency value on the XE1205          **
********************************************************************
** In  : value                                                    **
** Out : -                                                        **
*******************************************************************/
void WriteLO(_U16 value){
    WriteRegister(REG_MCPARAM4, (_U8) (value >> 8));
    WriteRegister(REG_MCPARAM5, (_U8) value);
}

/*******************************************************************
** InitFei : Initializes the XE1205 to enable the FEI reading     **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitFei(void){
    _U8 BitRate = 0;

    BitRate = ReadRegister(REG_MCPARAM3);
    Wait(TS_FEI(BitRate));
}

/*******************************************************************
** ReadFei : Reads the FEI value from  XE1205                     **
********************************************************************
** In  : -                                                        **
** Out : value                                                    **
*******************************************************************/
_S16 ReadFei(void){
	_S16 value;
	value = ReadRegister(REG_RXPARAM5);                  // Reads the FEI result LSB first (For trig)
	value = value | (ReadRegister(REG_RXPARAM4) << 8);   // Reads the FEI result MSB

	return value;
}

/*******************************************************************
** InitRssi : Initializes the XE1205 to enable the RSSI reading   **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void InitRssi(void){
    Wait(TS_RSSI);
}

/*******************************************************************
** ReadRssi : Reads the Rssi value from  XE1205                   **
********************************************************************
** In  : -                                                        **
** Out : value                                                    **
*******************************************************************/
_U16 ReadRssi(void){
	_U16 value;
	value = (ReadRegister(REG_RXPARAM2) >> 4) & 0x03;  // Reads the RSSI result
	return value;
}

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
void Wait(_U16 cntVal){
    RegCntOn &= 0xFC;                              // Disables counter A&B
    RegEvnEn &= 0x7F;                              // Disables events from the counter A&B
    RegEvn = 0x80;                                 // Clears the event from the CntA on the event register
    RegCntCtrlCk =  (RegCntCtrlCk & 0xFC) | 0x01;  // Selects RC frequency as clock source for counter A&B
    RegCntConfig1 |= 0x34;                         // A&B counters count up, counter A&B are in cascade mode
    RegCntA       = (_U8)(cntVal);                 // LSB of cntVal
    RegCntB       = (_U8)(cntVal >> 8);            // MSB of cntVal
    RegEvnEn      |= 0x80;                         // Enables events from CntA
    RegEvn        |= 0x80;                         // Clears the event from the CntA on the event register
    asm("clrb %stat, #0");                         // Clears the event on the CoolRISC status register
    RegCntOn      |= 0x03;                         // Enables counter A&B
    do{
        asm("halt");
    }while ((RegEvn & 0x80) == 0x00);              // Waits the event from counter A
    RegCntOn      &= 0xFE;                         // Disables counter A
    RegEvnEn      &= 0x7F;                         // Disables events from the counter A
    RegEvn        |= 0x80;                         // Clears the event from the CntA on the event register
    asm("clrb %stat, #0");                         // Clears the event on the CoolRISC status register
} // void Wait(_U16 cntVal)

/*******************************************************************
** EnableTimeOut : Enables/Disables the RF frame timeout          **
********************************************************************
** In  : enable                                                   **
** Out : -                                                        **
*******************************************************************/
void EnableTimeOut(_U8 enable){
    RegCntCtrlCk = (RegCntCtrlCk & 0xFC) | 0x03;        // Selects 128 Hz frequency as clock source for counter A&B
    RegCntConfig1 |=  0x34;                             // A&B counters count up, counter A&B  are in cascade mode

    RegCntA = (_U8)RFFrameTimeOut;                      // LSB of RFFrameTimeOut
    RegCntB = (_U8)(RFFrameTimeOut >> 8);               // MSB of RFFrameTimeOut

    if(enable){
        RegIrqEnHig |= 0x10;                            // Enables IRQ for the counter A&B
        RegCntOn |= 0x03;                               // Enables counter A&B
    }
    else{
        RegIrqEnHig &= ~0x10;                           // Disables IRQ for the counter A&B
        RegCntOn &= ~0x03;                              // Disables counter A&B
    }
} // void EnableTimeOut(_U8 enable)

/*******************************************************************
** InvertByte : Inverts a byte. MSB -> LSB, LSB -> MSB            **
********************************************************************
** In  : b                                                        **
** Out : b                                                        **
*******************************************************************/
_U8 InvertByte(_U8 b){
    asm("   move %r0, #0x08");
    asm("LoopInvertByte:");
    asm("   shl  %r3");
    asm("   shrc %r2");
    asm("   dec  %r0");
    asm("   jzc  LoopInvertByte");
} // _U8 InvertByte(_U8 b)

/*******************************************************************
** SpiInOut : Sends and receives a byte from the SPI bus          **
********************************************************************
** In  : outputByte                                               **
** Out : inputByte                                                **
*******************************************************************/
_U8 SpiInOut (_U8 outputByte){
    _U8 bitCounter;
    _U8 inputByte = 0;

    SPIClock(0);
    for(bitCounter = 0x80; bitCounter != 0x00; bitCounter >>= 1){
        if (outputByte & bitCounter){
            SPIMosi(1);
        }
        else{
            SPIMosi(0);
        }
        SPIClock(1);
        if (SPIMisoTest()){
            inputByte |= bitCounter;
        }
        SPIClock(0);
    }  // for(BitCounter = 0x80; BitCounter != 0x00; BitCounter >>= 1)
    SPIMosi(0);

    return inputByte;
} // _U8 SpiInOut (_U8 outputByte)

/*******************************************************************
** XE1205 Buffered interrupt handlers                             **
*******************************************************************/

/*******************************************************************
** Handle_Irq_Pa1 : Handles the interruption from the Pin 1 of    **
**                  Port A                                        **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void Handle_Irq_Pa1 (void){ // IRQ_1

} //End Handle_Irq_Pa1

/*******************************************************************
** Handle_Irq_Pa4 : Handles the interruption from the Pin 2 of    **
**                  Port A                                        **
********************************************************************
** In  : -                                                        **
** Out : -                                                        **
*******************************************************************/
void Handle_Irq_Pa2 (void){ // IRQ_0
    _U8 dummy;


    if(RFState & RF_BUSY){
        RFState |= RF_BUSY;

        if(RFFramePos < (RFFrameSize + 1)){
            if(RFFramePos == 0){
                RFFrameSize = InvertByte(ReceiveByte());
                dummy = 0;
            }
            else{
                if(SyncByte == SYNC_BYTE_FREQ){
                    if(EnableSyncByte){
                        SyncByte = 0;
                        dummy = ReceiveByte();
                        RFFramePos--;
                    }
                }
                else{
                    pRFFrame[RFFramePos-1] = InvertByte(ReceiveByte());
                    if(EnableSyncByte){
                        SyncByte++;
                    }
                }
            }
            RFFramePos++;
        }
        else{
            RFState |= RF_RX_DONE;
            RFState &= ~RF_BUSY;
        }

        if(RFFrameSize >= RF_BUFFER_SIZE_MAX){
            RFState |= RF_ERROR;
            RFState &= ~RF_BUSY;
        }
    }

} //End Handle_Irq_Pa2

/*******************************************************************
** Handle_Irq_CntA : Handles the interruption from the Counter A  **
********************************************************************
** In              : -                                            **
** Out             : -                                            **
*******************************************************************/
void Handle_Irq_CntA (void){
    RFState |= RF_TIMEOUT;
    RFState &= ~RF_BUSY;
} //End Handle_Irq_CntA

