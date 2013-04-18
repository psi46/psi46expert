// PSI46 testboard API

#ifndef PSI46TB_H
#define PSI46TB_H


#include "interface/USBInterface.h"
#include <stdio.h>


// size of module
#define MOD_NUMROCS  16

// number of registers per ROC
#define ROC_NUMREGS  27

// size of ROC pixel array
#define ROC_NUMROWS  80  // # rows
#define ROC_NUMCOLS  52  // # columns
#define ROC_NUMDCOLS 26  // # double columns (= columns/2)


// delay cells
#define SIGNAL_CLK      0
#define SIGNAL_SDA      1
#define SIGNAL_CTR      2
#define SIGNAL_TIN      3
#define SIGNAL_TOUT     4
#define SIGNAL_RDA      4

// registers for tb_Set
#define T_ResCal   0    // 4..255,  #clocks(res->cal) = T_ResCal+2
#define T_CalCal   1    // 4..255,  #clocks(cal->cal) = T_CalCal+2
#define T_CalTrg   2    // 4..255,  #clocks(cal->trg) = T_CalTrg+2
#define T_TrgTok   3    // 4..255,  #clocks(trg->tok) = T_TrgTok+0.5
#define T_Periode  4    // 1..255
#define CALREP     5    // 1..63,   #cal = CALREP

// clock frequenz settings
#define MHZ_1_25   5
#define MHZ_2_5    4
#define MHZ_5      3
#define MHZ_10     2
#define MHZ_20     1
#define MHZ_40     0

// signal flags for tb_Single, tb_Intern, tb_Extern
#define RES  0x08
#define CAL  0x04
#define TRG  0x02
#define TOK  0x01

#define PIXMASK  0x80

#define TRIGGER_OFF       0
#define TRIGGER_FIXED     1
#define TRIGGER_ROC       2
#define TRIGGER_MODULE1   4
#define TRIGGER_MODULE2   8


// PUC register addresses for roc_SetDAC
#define Vdig        0x01
#define Vana        0x02
#define Vsh         0x03
#define Vcomp       0x04
#define Vleak_comp  0x05
#define VrgPr       0x06
#define VwllPr      0x07
#define VrgSh       0x08
#define VwllSh      0x09
#define VhldDel     0x0A
#define Vtrim       0x0B
#define VthrComp    0x0C
#define VIBias_Bus  0x0D
#define Vbias_sf    0x0E
#define VoffsetOp   0x0F
#define VIbiasOp    0x10
#define VoffsetRO   0x11
#define VIon        0x12
#define VIbias_PH   0x13
#define Ibias_DAC   0x14
#define VIbias_roc  0x15
#define VIColOr     0x16
#define Vnpix       0x17
#define VsumCol     0x18
#define Vcal        0x19
#define CalDel      0x1A
#define RangeTemp   0x1B
#define WBC         0xFE
#define CtrlReg     0xFD

// signal definions for ProbeSelect
#define PROBE_NONE      0
#define PROBE_CLK       1
#define PROBE_I2C_SEND  2
#define PROBE_SDA       3
#define PROBE_RDA       4
#define PROBE_TOUT      4
#define PROBE_TIN       5
#define PROBE_CTR       6
#define PROBE_CAL       7
#define PROBE_TRG       8
#define PROBE_RES       9
#define PROBE_TRIG_IN  10
#define PROBE_ADC_TRIG 11
#define PROBE_ADC_GATE 12
#define PROBE_ADC_COMP  13
#define PROBE_ADC_ERROR 14
#define PROBE_STRETCH   15


// sources for clock stretch trigger
#define STRETCH_AFTER_TIN  0
#define STRETCH_AFTER_TRG  1
#define STRETCH_AFTER_CAL  2
#define STRETCH_AFTER_RES  3

// constants for signal force command
#define OVW_CLK   0x10
#define OVW_SDA   0x20
#define OVW_CTR   0x40
#define OVW_TIN   0x80
#define SET_CLK   0x01
#define SET_SDA   0x02
#define SET_CTR   0x04
#define SET_TIN   0x08

// MMA
#define MMA_AOUT1   0
#define MMA_AOUT2   1
#define MMA_AOUT3   2
#define MMA_AOUT4   3


class CTestboard
{
    CUSB usb;
    bool TBM_present;
public:
    CTestboard();
    ~CTestboard() { Close(); }

    // === board connection methods =========================================

    bool EnumFirst(unsigned int &nDevices) { return usb.EnumFirst(nDevices); };
    bool EnumNext(char name[]) { return usb.EnumNext(name); }
    bool Open(char name[], bool init = true); // opens a connection
    void Close();               // closes the connection to the testboard
    bool IsConnected() { return usb.Connected(); }
    const char * ConnectionError() { return usb.GetErrorMsg(usb.GetLastError()); }
    bool GetVersion(char * s, unsigned int n);
    void Welcome();             // displays the welcome message
    void SetLed(unsigned char value);
    unsigned char  GetBoardId();            // reads the board number
    void Init();                // inits the testboard to default values


    // === delay methods ====================================================

    void cDelay(unsigned short clocks);
    void uDelay(unsigned short us);
    void mDelay(unsigned short ms);


    // === communication buffer methods =====================================

    bool Flush() { return usb.Flush(); };
    bool Clear() { return usb.Clear(); }


    // == PSI46 testboard methods ===========================================

    unsigned char isClockPresent();
    void SetClock(unsigned char MHz);
    void SetClockStretch(unsigned char src,
                         unsigned short delay, unsigned short width);
    void SetDelay(unsigned char signal, unsigned short ns);
    void AdjustDelay(unsigned short k) { SetDelay(255, k); }

    void ForceSignal(unsigned char pattern);

    void I2cAddr(unsigned char id);     // set testboard I2C address

    void Pon();                 // switch ROC power on
    void Poff();                    // switch ROC power off

    void SetVA(double V);       // set VA voltage in V
    void SetVD(double V);       // set VD voltage in V
    void SetIA(double A);       // set VA current limit in A
    void SetID(double A);       // set VD current limit in A

    double GetVA(); // get VA voltage in V
    double GetVD(); // get VD voltage in V
    double GetIA(); // get VA current in A
    double GetID(); // get VD current in A

    void HVon();        // switch HV relais on
    void HVoff();   // switch HV relais off

    void ResetOn(); // switch RESET-line to reset state (low)
    void ResetOff();    // switch RESET-line to not reset state (high)

    // set testboard register
    void Set(unsigned char reg, unsigned char value);

    // -- trigger a single RES -> CAL -> TRG -> TOK sequence
    //    sequence
    //    mask: flags selecting the presence of the different pulses
    //          RES = reset present
    //          CAL = calibrate present
    //          TRG = trigger present
    //          TOK = token present
    void Single(unsigned char mask);
    bool SingleWait(unsigned char mask, unsigned short timeout);

    // -- enables the internal event generator
    //    mask: same as tb_Single
    void Intern(unsigned char mask);

    // -- enables the external event input
    //    mask: same as tb_Single
    void Extern(unsigned char mask) {}

    // -- gets the readout counter
    unsigned char GetRoCnt();
    bool SendRoCnt();
    unsigned char RecvRoCnt();

    unsigned short GetRoCntEx();
    bool SendRoCntEx();
    unsigned short RecvRoCntEx();

    void SetTriggerMode(unsigned short mode);
    void DataCtrl(char channel, bool clear, bool trigger, bool cont);
    void DataEnable(bool on);
    unsigned short DataState();
    void DataTriggerLevel(char channel, short level);
    void DataBlockSize(unsigned short size);
    bool DataRead(char channel, short buffer[], unsigned short buffersize,
                  unsigned short &wordsread);
    bool DataReadRaw(char channel, short buffer[], unsigned short buffersize,
                     unsigned short &wordsread);
    unsigned short GetModRoCnt(unsigned short index);
    void GetModRoCntAll(unsigned short * counts);


    unsigned int Daq_Init(unsigned int size);
    void Daq_Enable();
    void Daq_Disable();
    bool Daq_Ready();
    unsigned int Daq_GetPointer();
    unsigned int Daq_GetSize();
    void Daq_Done();

    void ProbeSelect(unsigned char port, unsigned char signal);


    // == Wafer Test Adapter ================================================

    double GetVD_Reg();    // regulated VD
    double GetVD_CAP();    // unregulated VD for contact test
    double GetVDAC_CAP();  // regulated VDAC
    double GetTOUT_COM();  // TOUT common mode voltage
    double GetAOUT_COM();  // AOUT common mode voltage


    // == TBM functions =====================================================

    bool TBMPresent() { return TBM_present; }

    void tbm_Enable(bool on);

    void tbm_Addr(unsigned char hub, unsigned char port);

    void mod_Addr(unsigned char hub);

    void tbm_Set(unsigned char reg, unsigned char value);

    bool tbm_Get(unsigned char reg, unsigned char &value);

    bool tbm_GetRaw(unsigned char reg, int &value);


    // == ROC functions =====================================================

    // -- set the i2c address for the following commands
    void roc_I2cAddr(unsigned char id);

    // -- sends "ClrCal" command to ROC
    void roc_ClrCal();

    // -- sets a single (DAC) register
    void roc_SetDAC(unsigned char reg, unsigned char value);

    // -- set pixel bits (count <= 60)
    //    M - - - 8 4 2 1
    void roc_Pix(unsigned char col, unsigned char row, unsigned char value);

    // -- trimm a single pixel (count < =60)
    void roc_Pix_Trim(unsigned char col, unsigned char row, unsigned char value);

    // -- mask a single pixel (count <= 60)
    void roc_Pix_Mask(unsigned char col, unsigned char row);

    // -- set calibrate at specific column and row
    void roc_Pix_Cal(unsigned char col, unsigned char row, bool sensor_cal = false);

    // -- enable/disable a double column
    void roc_Col_Enable(unsigned char col, bool on);

    // -- mask all pixels of a column and the coresponding double column
    void roc_Col_Mask(unsigned char col);

    // -- mask all pixels and columns of the chip
    void roc_Chip_Mask();


    // === low level methodes ===========================================
    static unsigned char COLCODE(unsigned char x) { return ((x >> 1) & 0x7e) ^ x; }
    static unsigned char ROWCODE(unsigned char x) { return (x >> 1) ^ x; }

    void SetReg(unsigned char addr, unsigned short value);
    unsigned short GetReg41();

    void TBMEmulatorOn();
    void TBMEmulatorOff();
    void TbmWrite(int hubAddr, int addr, int value);
    void Tbm1Write(int hubAddr, int addr, int value);
    void Tbm2Write(int hubAddr, int addr, int value);

private:
    bool Write(unsigned int bytesToWrite, void * buffer)
    { return usb.Write(bytesToWrite, buffer); }

    bool Read(unsigned int bytesToRead, void * buffer, unsigned int &bytesRead)
    { return usb.Read(bytesToRead, buffer, bytesRead); }

    // === old function ==================================================
public:
    bool Mem_SetAddr(unsigned int addr) { return false; }
    bool Mem_WriteWord(unsigned int data, bool incr = false) { return false; }
    bool Mem_ReadWord(unsigned int &data, bool incr = false) { return false; }
    bool Mem_ReadBlock(unsigned int start, unsigned int size, unsigned short * buffer)
    { return false; }

    bool Mem_GetFillState(unsigned int &size) { return false; }

    void SetOrbit(unsigned int periode) {printf(">>>>>>> dummy function\n");};
    void SetTriggerScaler(unsigned int rate) {printf(">>>>>>> dummy function\n");};
    void SetTriggerScaler(double rate) {printf(">>>>>>> dummy function\n");};
    int GetTriggerRate() {printf(">>>>>>> dummy function\n"); return 0;};


    // === debug commands ================================================
    void IoRead8(unsigned int addr, unsigned short size, unsigned char step,
                 unsigned char * value);
    void IoRead16(unsigned int addr, unsigned short size, unsigned char step,
                  unsigned short * value);
    void IoRead32(unsigned int addr, unsigned short size, unsigned char step,
                  unsigned int * value);
    void IoWrite8(unsigned int addr, unsigned short size, unsigned char step,
                  const unsigned char * value);
    void IoWrite16(unsigned int addr, unsigned short size, unsigned char step,
                   const unsigned short * value);
    void IoWrite32(unsigned int addr, unsigned short size, unsigned char step,
                   const unsigned int * value);

    void MemWrite(unsigned int addr, unsigned short size,
                  unsigned char * x);
    void MemRead(unsigned int addr, unsigned short size,
                 unsigned char * s);
    void MemFill(unsigned int addr, unsigned short size,
                 unsigned char x);
    unsigned char FlashRead(unsigned int addr, unsigned short size,
                            unsigned char * x);
    unsigned char FlashWrite(unsigned int addr, unsigned short size,
                             unsigned char * x);
    // === high level functions for wafer test ===========================

    void DataRun_Init();
    void DataRun_Done();
    void DataRun_GetSample();
    int  DataRun_ScanROC();
    unsigned int DataRun_GetDataSize();
    void DataRun_GetData(unsigned int pos, unsigned short size,
                         short * buffer);


    // === high level atb functions ======================================
    bool GetPixel(int x);
    int FindLevel();
    unsigned char test_PUC(unsigned char col, unsigned char row, unsigned char trim);
    void testColPixel(int col, int trimbit, unsigned char * res);
    bool GetLastDac(unsigned char count, int &ldac);
    bool ScanDac(unsigned char dac, unsigned char count,
                 unsigned char min, unsigned char max, short * ldac);

    int CountReadouts(int count, int chipId);
    int AoutLevel(short position, short nTriggers);
    int AoutLevelChip(short position, short nTriggers, int trims[],  int res[]);
    int AoutLevelPartOfChip(short position, short nTriggers, int trims[], int res[], bool pxlFlags[]);
    int ChipEfficiency(short nTriggers, int trim[], double res[]);
    int MaskTest(short nTriggers, short res[]);
    void DoubleColumnADCData(int column, short data[], int readoutStop[]);
    int PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim);
    int ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim[], int res[]);
    int SCurve(int nTrig, int dacReg, int threshold, int res[]);
    int SCurveColumn(int column, int nTrig, int dacReg, int thr[], int trims[], int chipId[], int res[]);
    void ADCRead(short buffer[], unsigned short &wordsread, short nTrig);
    void DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[]);
    void PHDac(int dac, int dacRange, int nTrig, int position, short result[]);
    void AddressLevels(int position, int result[]);
    void TBMAddressLevels(int result[]);
    void TrimAboveNoise(short nTrigs, short thr, short mode, short result[]);

    void ReadData(int position, int size, int result[]);
    void ReadFPGAData(int size, int result[]);

    void SetEmptyReadoutLength(int emptyReadoutLength);
    void SetEmptyReadoutLengthADC(int emptyReadoutLengthADC);
    void SetTbmChannel(int tbmChannel);
    void SetDTL(int value);
    void SetNRocs(int value);
    void SetHubID(int value);
    void SetEnableAll(int value);
    void SetAoutChipPosition(int value);

    // =======================================================================

    int demo(short x);



    void ScanAdac(unsigned short chip, unsigned char dac, unsigned char
                  min, unsigned char max, char step, unsigned char rep,
                  unsigned int usDelay, unsigned char res[]);


    void CdVc(unsigned short chip, unsigned char wbcmin, unsigned char
              wbcmax, unsigned char vcalstep, unsigned char cdinit,
              unsigned short &lres, unsigned short res[]);

    char CountAllReadouts(int nTrig, int counts[], int amplitudes[]);


private:
    int nRocs;

};


#endif // PSI46TB_H
