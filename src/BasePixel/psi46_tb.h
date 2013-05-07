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

// pattern generator signal bits
#define PG_RES  0x800
#define PG_CAL  0x400
#define PG_TRG  0x200
#define PG_TOK  0x100
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

	bool EnumFirst(uint32_t &nDevices) { return usb.EnumFirst(nDevices); };
    bool EnumNext(char name[]) { return usb.EnumNext(name); }
    bool Open(char name[], bool init = true); // opens a connection
    void Close();               // closes the connection to the testboard
    bool IsConnected() { return usb.Connected(); }
    const char * ConnectionError() { return usb.GetErrorMsg(usb.GetLastError()); }
	bool GetVersion(char *s, uint32_t n);
    void Welcome();             // displays the welcome message
    void SetLed(unsigned char value);
    unsigned char  GetBoardId();            // reads the board number
    void Init();                // inits the testboard to default values

	void Bootstrap();

    // === delay methods ====================================================

	void cDelay(uint16_t clocks);
	void uDelay(uint16_t us);
	void mDelay(uint16_t ms);


    // === communication buffer methods =====================================

	bool Flush() { return usb.Flush(); }
    bool Clear() { return usb.Clear(); }
	bool ShowUSB() { return usb.Show(); };

    // == PSI46 testboard methods ===========================================

    unsigned char isClockPresent();
    void SetClock(unsigned char MHz);
    void SetClockStretch(unsigned char src,
		uint16_t delay, uint16_t width);
	void SetDelay(unsigned char signal, uint16_t ns);
	void AdjustDelay(uint16_t k) { SetDelay(255, k); }

    void ForceSignal(unsigned char pattern);

    void I2cAddr(unsigned char id);     // set testboard I2C address

    void Pon();                 // switch ROC power on
    void Poff();                    // switch ROC power off

    void SetVA(double V);       // set VA voltage in V
    void SetVD(double V);       // set VD voltage in V
	void SetVO(double V);       // set VO voltage in V
    void SetIA(double A);       // set VA current limit in A
    void SetID(double A);       // set VD current limit in A
	void SetIO(double A);       // set VO current limit in A

    double GetVA(); // get VA voltage in V
    double GetVD(); // get VD voltage in V
	double GetVO(); // get VO voltage in V
    double GetIA(); // get VA current in A
    double GetID(); // get VD current in A
	double GetIO();	// get VO current in A

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
	bool SingleWait(unsigned char mask, uint16_t timeout);

    // -- enables the internal event generator
    //    mask: same as tb_Single
    void Intern(unsigned char mask);

    // -- enables the external event input
    //    mask: same as tb_Single
    void Extern(unsigned char mask) {}

	// -- pulse pattern generator
	void pg_SetCmd(uint16_t addr, uint16_t cmd);
	void pg_SetCmdAll(uint16_t count, uint16_t *cmd);
	void pg_Disable();
	void pg_Single();
	void pg_Trigger();
	void pg_Loop(uint16_t period);

    // -- gets the readout counter
    unsigned char GetRoCnt();
    bool SendRoCnt();
    unsigned char RecvRoCnt();

	uint16_t GetRoCntEx();
    bool SendRoCntEx();
	uint16_t RecvRoCntEx();

	void SetTriggerMode(uint16_t mode);
    void DataCtrl(char channel, bool clear, bool trigger, bool cont);
    void DataEnable(bool on);
	uint16_t DataState();
	void DataTriggerLevel(char channel, int16_t level);
	void DataBlockSize(uint16_t size);
	bool DataRead(char channel, int16_t buffer[], uint16_t buffersize,
		uint16_t &wordsread);
	bool DataReadRaw(char channel, int16_t buffer[], uint16_t buffersize,
		uint16_t &wordsread);
	uint16_t GetModRoCnt(uint16_t index);
	void GetModRoCntAll(uint16_t *counts);


	uint32_t Daq_Init(uint32_t size);
    void Daq_Enable();
    void Daq_Disable();
    bool Daq_Ready();
	uint16_t Daq_GetPointer();
	uint32_t Daq_GetSize();
    void Daq_Done();

    void ProbeSelect(unsigned char port, unsigned char signal);
	void SetTriggerMask(unsigned char mask);


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

	bool tbm_GetRaw(unsigned char reg, int32_t &value);


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

	void SetReg(unsigned char addr, uint16_t value);
	uint16_t GetReg41();

    void TBMEmulatorOn();
    void TBMEmulatorOff();
	void TbmWrite(int32_t hubAddr, int32_t addr, int32_t value);
	void Tbm1Write(int32_t hubAddr, int32_t addr, int32_t value);
	void Tbm2Write(int32_t hubAddr, int32_t addr, int32_t value);

private:
	bool Write(uint32_t bytesToWrite, void *buffer)
    { return usb.Write(bytesToWrite, buffer); }

	bool Read(uint32_t bytesToRead, void * buffer, uint32_t &bytesRead)
    { return usb.Read(bytesToRead, buffer, bytesRead); }

    // === old function ==================================================
public:
	bool Mem_SetAddr(uint32_t addr) { return false; }
	bool Mem_WriteWord(uint32_t data, bool incr = false) { return false; }
	bool Mem_ReadWord(uint32_t & data, bool incr = false) { return false; }
	bool Mem_ReadBlock(uint32_t start, uint32_t size, uint16_t * buffer)
    { return false; }

	bool Mem_GetFillState(uint32_t& size) { return false; }

    void SetOrbit(unsigned int periode) {printf(">>>>>>> dummy function\n");};
    void SetTriggerScaler(unsigned int rate) {printf(">>>>>>> dummy function\n");};
    void SetTriggerScaler(double rate) {printf(">>>>>>> dummy function\n");};
    int GetTriggerRate() {printf(">>>>>>> dummy function\n"); return 0;};

    // === debug commands ================================================
	void IoRead8(uint32_t addr, uint16_t size, unsigned char step,
                 unsigned char * value);
	void IoRead16(uint32_t addr, uint16_t size, unsigned char step,
		uint16_t *value);
	void IoRead32(uint32_t addr, uint16_t size, unsigned char step,
		uint32_t *value);
	void IoWrite8(uint32_t addr, uint16_t size, unsigned char step,
                  const unsigned char * value);
	void IoWrite16(uint32_t addr, uint16_t size, unsigned char step,
		const uint16_t *value);
	void IoWrite32(uint32_t addr, uint16_t size, unsigned char step,
		const uint32_t *value);

	void MemWrite(uint32_t addr, uint16_t size,
                  unsigned char * x);
	void MemRead(uint32_t addr, uint16_t size,
                 unsigned char * s);
	void MemFill(uint32_t addr, uint16_t size,
                 unsigned char x);
	unsigned char FlashRead(uint32_t addr, uint16_t size,
                            unsigned char * x);
	unsigned char FlashWrite(uint32_t addr, uint16_t size,
                             unsigned char * x);
    // === high level functions for wafer test ===========================

    void DataRun_Init();
    void DataRun_Done();
    void DataRun_GetSample();
	int32_t  DataRun_ScanROC();
	uint32_t DataRun_GetDataSize();
	void DataRun_GetData(uint32_t pos, uint16_t size,
		int16_t *buffer);


	// === module test functions ======================================
	bool GetPixel(int32_t x);
	int32_t FindLevel();
    unsigned char test_PUC(unsigned char col, unsigned char row, unsigned char trim);
	void testColPixel(int32_t col, int32_t trimbit, unsigned char *res);
	bool GetLastDac(unsigned char count, int32_t &ldac);
    bool ScanDac(unsigned char dac, unsigned char count,
	unsigned char min, unsigned char max, int16_t *ldac);

	int32_t CountReadouts(int32_t count, int32_t chipId);
	int32_t AoutLevel(int16_t position, int16_t nTriggers);
	int32_t AoutLevelChip(int16_t position, int16_t nTriggers, int32_t trims[],  int32_t res[]);
	int32_t AoutLevelPartOfChip(int16_t position, int16_t nTriggers, int32_t trims[], int32_t res[], bool pxlFlags[]);
	int32_t ChipEfficiency(int16_t nTriggers, int32_t trim[], double res[]);
    int32_t MaskTest(int16_t nTriggers, int16_t res[]);
	void DoubleColumnADCData(int32_t column, int16_t data[], int32_t readoutStop[]);
	int32_t PixelThreshold(int32_t col, int32_t row, int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim);
	int32_t ChipThreshold(int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim[], int32_t res[]);
	int32_t SCurve(int32_t nTrig, int32_t dacReg, int32_t threshold, int32_t res[]);
	int32_t SCurveColumn(int32_t column, int32_t nTrig, int32_t dacReg, int32_t thr[], int32_t trims[], int32_t chipId[], int32_t res[]);
	void ADCRead(int16_t buffer[], uint16_t &wordsread, int16_t nTrig);
	void DacDac(int32_t dac1, int32_t dacRange1, int32_t dac2, int32_t dacRange2, int32_t nTrig, int32_t result[]);
	void PHDac(int32_t dac, int32_t dacRange, int32_t nTrig, int32_t position, int16_t result[]);
	void AddressLevels(int32_t position, int32_t result[]);
	void TBMAddressLevels(int32_t result[]);
	void TrimAboveNoise(int16_t nTrigs, int16_t thr, int16_t mode, int16_t result[]);

	void ReadData(int32_t position, int32_t size, int32_t result[]);
	void ReadFPGAData(int32_t size, int32_t result[]);

	void SetEmptyReadoutLength(int32_t emptyReadoutLength);
	void SetEmptyReadoutLengthADC(int32_t emptyReadoutLengthADC);
	void SetTbmChannel(int32_t tbmChannel);
	void SetDTL(int32_t value);
	void SetNRocs(int32_t value);
	void SetHubID(int32_t value);
	void SetEnableAll(int32_t value);
    void SetAoutChipPosition(int32_t value);

    // =======================================================================

	int32_t demo(int16_t x);

	void GetColPulseHeight(unsigned char col, unsigned char count,
				int16_t data[]);

	void Scan1D(unsigned char vx,
			unsigned char xmin, unsigned char xmax,	char xstep,
			unsigned char rep, uint32_t usDelay, unsigned char res[]);

	void BumpTestColPixel(unsigned char col, unsigned char res[]);
	void BumpTestColRef(unsigned char col, unsigned char res[]);
	void DacDac(int16_t dac1, int16_t dacRange1, int16_t dac2, int16_t dacRange2,
		int16_t nTrig, int16_t res[], int16_t rocpos);

	// ===================================================================

	void ScanAdac(uint16_t chip, unsigned char dac,
      unsigned char min, unsigned char max, char step,
      unsigned char rep, uint32_t usDelay, unsigned char res[]);
	void CdVc(uint16_t chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep,
          unsigned char cdinit, uint16_t &lres, uint16_t res[]);

	// === xray test =====================================================
	char CountAllReadouts(int32_t nTrig, int32_t counts[], int32_t amplitudes[]);


private:
	int32_t nRocs;

};


#endif // PSI46TB_H
