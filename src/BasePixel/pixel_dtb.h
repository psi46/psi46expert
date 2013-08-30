/*---------------------------------------------------------------------
 *
 *  filename:    pixel_dtb.h
 *
 *  description: PSI46 testboard API for DTB
 *	author:      Beat Meier
 *	date:        15.7.2013
 *	rev:
 *
 *---------------------------------------------------------------------
 */


// ---------------------------------------------------
#define TITLE        "PSI46V2 ROC/Wafer Tester"
#define VERSION      "V1.3"
#define TIMESTAMP    "07.08.2013"
// ---------------------------------------------------

#define VERSIONINFO TITLE " " VERSION " (" TIMESTAMP ")"


#pragma once

#include "profiler.h"
#define RPC_PROFILING PROFILING

// #define RPC_MULTITHREADING
#include "rpc.h"

#ifdef _WIN32
#include "pipe.h"
#endif

#include "usb.h"

// size of ROC pixel array
#define ROC_NUMROWS  80  // # rows
#define ROC_NUMCOLS  52  // # columns
#define ROC_NUMDCOLS 26  // # double columns (= columns/2)
#define ROC_NUMDCOLS 26  // # double columns (= columns/2)

#define PIXMASK  0x80

// PUC register addresses for roc_SetDAC
#define	Vdig        0x01
#define Vana        0x02
#define	Vsh         0x03
#define	Vcomp       0x04
#define	Vleak_comp  0x05
#define	VrgPr       0x06
#define	VwllPr      0x07
#define	VrgSh       0x08
#define	VwllSh      0x09
#define	VhldDel     0x0A
#define	Vtrim       0x0B
#define	VthrComp    0x0C
#define	VIBias_Bus  0x0D
#define	Vbias_sf    0x0E
#define	VoffsetOp   0x0F
#define	VIbiasOp    0x10
#define	VoffsetRO   0x11
#define	VIon        0x12
#define	VIbias_PH   0x13
#define	Ibias_DAC   0x14
#define	VIbias_roc  0x15
#define	VIColOr     0x16
#define	Vnpix       0x17
#define	VsumCol     0x18
#define	Vcal        0x19
#define	CalDel      0x1A
#define	RangeTemp   0x1B
#define	WBC         0xFE
#define	CtrlReg     0xFD



class CTestboard
{
	RPC_DEFS
	RPC_THREAD

#ifdef _WIN32
	CPipeClient pipe;
#endif
	CUSB usb;

public:
	CRpcIo& GetIo() { return *rpc_io; }

	CTestboard() { RPC_INIT rpc_io = &usb; }
	~CTestboard() { RPC_EXIT }


	// === RPC ==============================================================

	// Don't change the following two entries
	RPC_EXPORT uint16_t GetRpcVersion();
	RPC_EXPORT int32_t  GetRpcCallId(string &callName);

	RPC_EXPORT void GetRpcTimestamp(stringR &ts);

	RPC_EXPORT int32_t GetRpcCallCount();
	RPC_EXPORT bool    GetRpcCallName(int32_t id, stringR &callName);

	// === DTB connection ====================================================

	bool EnumFirst(unsigned int &nDevices) { return usb.EnumFirst(nDevices); };
	bool EnumNext(string &name);
	bool Enum(unsigned int pos, string &name);

	bool FindDTB(string &usbId);
	bool Open(string &name, bool init=true); // opens a connection
	void Close();				// closes the connection to the testboard

#ifdef _WIN32
	bool OpenPipe(const char *name) { return pipe.Open(name); }
	void ClosePipe() { pipe.Close(); }
#endif

	bool IsConnected() { return usb.Connected(); }
	const char * ConnectionError()
	{ return usb.GetErrorMsg(usb.GetLastError()); }

	void Flush() { rpc_io->Flush(); }
	void Clear() { rpc_io->Clear(); }


	// === DTB identification ================================================

	RPC_EXPORT void GetInfo(stringR &info);
	RPC_EXPORT uint16_t GetBoardId();
	RPC_EXPORT void GetHWVersion(stringR &version);
	RPC_EXPORT uint16_t GetFWVersion();
	RPC_EXPORT uint16_t GetSWVersion();


	// === DTB service ======================================================

	// --- upgrade
	RPC_EXPORT uint16_t UpgradeGetVersion();
	RPC_EXPORT uint8_t  UpgradeStart(uint16_t version);
	RPC_EXPORT uint8_t  UpgradeData(string &record);
	RPC_EXPORT uint8_t  UpgradeError();
	RPC_EXPORT void     UpgradeErrorMsg(stringR &msg);
	RPC_EXPORT void     UpgradeExec(uint16_t recordCount);


	// === DTB functions ====================================================

	RPC_EXPORT void Init();

	RPC_EXPORT void Welcome();
	RPC_EXPORT void SetLed(uint8_t x);


	// --- Clock, Timing ----------------------------------------------------
	RPC_EXPORT void cDelay(uint16_t clocks);
	RPC_EXPORT void uDelay(uint16_t us);
	void mDelay(uint16_t ms);


	// --- Signal Delay -----------------------------------------------------
	#define SIG_CLK 0
	#define SIG_CTR 1
	#define SIG_SDA 2
	#define SIG_TIN 3

	#define SIG_MODE_NORMAL  0
	#define SIG_MODE_LO      1
	#define SIG_MODE_HI      2

	RPC_EXPORT void Sig_SetMode(uint8_t signal, uint8_t mode);
	RPC_EXPORT void Sig_SetPRBS(uint8_t signal, uint8_t speed);
	RPC_EXPORT void Sig_SetDelay(uint8_t signal, uint16_t delay, int8_t duty = 0);
	RPC_EXPORT void Sig_SetLevel(uint8_t signal, uint8_t level);
	RPC_EXPORT void Sig_SetOffset(uint8_t offset);
	RPC_EXPORT void Sig_SetLVDS();
	RPC_EXPORT void Sig_SetLCDS();


	// --- digital signal probe ---------------------------------------------
	#define PROBE_OFF     0
	#define PROBE_CLK     1
	#define PROBE_SDA     2
	#define PROBE_PGTOK   3
	#define PROBE_PGTRG   4
	#define PROBE_PGCAL   5
	#define PROBE_PGRESR  6
	#define PROBE_PGREST  7
	#define PROBE_PGSYNC  8
	#define PROBE_CTR     9
	#define PROBE_CLKP   10
	#define PROBE_CLKG   11
	#define PROBE_CRC    12

	RPC_EXPORT void SignalProbeD1(uint8_t signal);
	RPC_EXPORT void SignalProbeD2(uint8_t signal);


	// --- analog signal probe ----------------------------------------------
	#define PROBEA_TIN     0
	#define PROBEA_SDATA1  1
	#define PROBEA_SDATA2  2
	#define PROBEA_CTR     3
	#define PROBEA_CLK     4
	#define PROBEA_SDA     5
	#define PROBEA_TOUT    6
	#define PROBEA_OFF     7

	#define GAIN_1   0
	#define GAIN_2   1
	#define GAIN_3   2
	#define GAIN_4   3

	RPC_EXPORT void SignalProbeA1(uint8_t signal);
	RPC_EXPORT void SignalProbeA2(uint8_t signal);
	RPC_EXPORT void SignalProbeADC(uint8_t signal, uint8_t gain = 0);


	// --- ROC/Module power VD/VA -------------------------------------------
	RPC_EXPORT void Pon();	// switch ROC power on
	RPC_EXPORT void Poff();	// switch ROC power off

	RPC_EXPORT void _SetVD(uint16_t mV);
	RPC_EXPORT void _SetVA(uint16_t mV);
	RPC_EXPORT void _SetID(uint16_t uA100);
	RPC_EXPORT void _SetIA(uint16_t uA100);

	RPC_EXPORT uint16_t _GetVD();
	RPC_EXPORT uint16_t _GetVA();
	RPC_EXPORT uint16_t _GetID();
	RPC_EXPORT uint16_t _GetIA();

	void SetVA(double V) { _SetVA(uint16_t(V*1000)); }  // set VA voltage
	void SetVD(double V) { _SetVD(uint16_t(V*1000)); }  // set VD voltage
	void SetIA(double A) { _SetIA(uint16_t(A*10000)); }  // set VA current limit
	void SetID(double A) { _SetID(uint16_t(A*10000)); }  // set VD current limit

	double GetVA() { return _GetVA()/1000.0; }   // get VA voltage in V
	double GetVD() { return _GetVD()/1000.0; }	 // get VD voltage in V
	double GetIA() { return _GetIA()/10000.0; }  // get VA current in A
	double GetID() { return _GetID()/10000.0; }  // get VD current in A

	RPC_EXPORT void HVon();
	RPC_EXPORT void HVoff();
	RPC_EXPORT void ResetOn();
	RPC_EXPORT void ResetOff();
	RPC_EXPORT uint8_t GetStatus();
	RPC_EXPORT void SetRocAddress(uint8_t addr);


	// --- pulse pattern generator ------------------------------------------
	#define PG_TOK   0x0100
	#define PG_TRG   0x0200
	#define PG_CAL   0x0400
	#define PG_RESR  0x0800
	#define PG_REST  0x1000
	#define PG_SYNC  0x2000

	RPC_EXPORT void Pg_SetCmd(uint16_t addr, uint16_t cmd);
//	RPC_EXPORT void Pg_SetCmdAll(vector<uint16_t> &cmd);
	RPC_EXPORT void Pg_Stop();
	RPC_EXPORT void Pg_Single();
	RPC_EXPORT void Pg_Trigger();
	RPC_EXPORT void Pg_Loop(uint16_t period);

	RPC_EXPORT uint16_t GetUser1Version();


	// --- data aquisition --------------------------------------------------
	RPC_EXPORT uint32_t Daq_Open(uint32_t buffersize = 10000000); // max # of samples
	RPC_EXPORT void Daq_Close();
	RPC_EXPORT void Daq_Start();
	RPC_EXPORT void Daq_Stop();
	RPC_EXPORT uint32_t Daq_GetSize();

	RPC_EXPORT uint8_t Daq_Read(vectorR<uint16_t> &data,
			 uint16_t blocksize = 16384);

	RPC_EXPORT uint8_t Daq_Read(vectorR<uint16_t> &data,
			uint16_t blocksize, uint32_t &availsize);

	RPC_EXPORT void Daq_Select_ADC(uint16_t blocksize, uint8_t source,
			uint8_t start, uint8_t stop = 0);

	RPC_EXPORT void Daq_Select_Deser160(uint8_t shift);


	// --- ROC/module Communication -----------------------------------------
	// -- set the i2c address for the following commands
	RPC_EXPORT void roc_I2cAddr(uint8_t id);

	// -- sends "ClrCal" command to ROC
	RPC_EXPORT void roc_ClrCal();

	// -- sets a single (DAC) register
	RPC_EXPORT void roc_SetDAC(uint8_t reg, uint8_t value);

	// -- set pixel bits (count <= 60)
	//    M - - - 8 4 2 1
	RPC_EXPORT void roc_Pix(uint8_t col, uint8_t row, uint8_t value);

	// -- trimm a single pixel (count < =60)
	RPC_EXPORT void roc_Pix_Trim(uint8_t col, uint8_t row, uint8_t value);

	// -- mask a single pixel (count <= 60)
	RPC_EXPORT void roc_Pix_Mask(uint8_t col, uint8_t row);

	// -- set calibrate at specific column and row
	RPC_EXPORT void roc_Pix_Cal(uint8_t col, uint8_t row, bool sensor_cal = false);

	// -- enable/disable a double column
	RPC_EXPORT void roc_Col_Enable(uint8_t col, bool on);

	// -- mask all pixels of a column and the coresponding double column
	RPC_EXPORT void roc_Col_Mask(uint8_t col);

	// -- mask all pixels and columns of the chip
	RPC_EXPORT void roc_Chip_Mask();


// --- Wafer test functions
	RPC_EXPORT bool testColPixel(uint8_t col, uint8_t trimbit, vectorR<uint8_t> &res);

	// Ethernet test functions
	RPC_EXPORT void Ethernet_Send(string &message);
	RPC_EXPORT uint32_t Ethernet_RecvPackets();

    //TODO: Experimental
	void SetEnableAll(int32_t value){ 
        roc_I2cAddr(0);
	    SetRocAddress(0);
    }
    //TODO: Everything below needs to be implemented
    #define PROBE_ADC_GATE 12
    #define TRIGGER_OFF       0
    #define TRIGGER_FIXED     1
    #define TRIGGER_ROC       2
    #define TRIGGER_MODULE1   4
    #define TRIGGER_MODULE2   8
    #define RES  0x0800
    #define CAL  0x0400
    #define TRG  0x0200
    #define TOK  0x0100
    #define STRETCH_AFTER_CAL  2  
    // == TBM functions =====================================================

    bool TBMPresent() { return false; }

    void tbm_Enable(bool on){ return; }

    void tbm_Addr(unsigned char hub, unsigned char port){ return; }

    void mod_Addr(unsigned char hub){ return; }

    void tbm_Set(unsigned char reg, unsigned char value){ return; }

    bool tbm_Get(unsigned char reg, unsigned char &value){ return false; }

	bool tbm_GetRaw(unsigned char reg, int32_t &value){ return false; }

    bool GetVersion(char *s, uint32_t n){ return false;}


    void SetEmptyReadoutLength(int32_t emptyReadoutLength){ return; }


    void Single(unsigned char mask){ return; }

	bool SingleWait(unsigned char mask, uint16_t timeout){return false;}

    // -- enables the internal event generator
    //    mask: same as tb_Single
    void Intern(unsigned char mask){ Single(mask); }

    // -- enables the external event input
    //    mask: same as tb_Single
    void Extern(unsigned char mask){ Single(mask);}

    // -- gets the readout counter
    unsigned char GetRoCnt(){ return 1; }  

    bool SendRoCnt(){ return false;}
    unsigned char RecvRoCnt(){ return 1; }

    uint16_t GetRoCntEx(){ return 1; }
    bool SendRoCntEx(){ return false; }
	uint16_t RecvRoCntEx(){ return 1; }

	void SetTriggerMode(uint16_t mode){ return;}
    void DataCtrl(char channel, bool clear, bool trigger, bool cont){ return;}
    void DataEnable(bool on){return ;}
	uint16_t DataState(){ return 1;}
	void DataTriggerLevel(char channel, int16_t level){ return;}
	void DataBlockSize(uint16_t size){return ;}
	bool DataRead(char channel, int16_t buffer[], uint16_t buffersize,
		uint16_t &wordsread){ return false;}
	bool DataReadRaw(char channel, int16_t buffer[], uint16_t buffersize,
		uint16_t &wordsread){ return false; }
	uint16_t GetModRoCnt(uint16_t index){ return 1;}
	void GetModRoCntAll(uint16_t *counts){ return; }

	uint32_t Daq_Init(uint32_t size){ return 1; }
    void Daq_Enable(){ return; }
    void Daq_Disable(){ return; }
    bool Daq_Ready(){ return true; }
	uint16_t Daq_GetPointer(){ return 1; }
    void Daq_Done(){ return; }

    void ProbeSelect(unsigned char port, unsigned char signal){ return;}
	void SetTriggerMask(unsigned char mask){ return;}

    void TBMEmulatorOn(){ return; }
    void TBMEmulatorOff(){ return; }
	void TbmWrite(int32_t hubAddr, int32_t addr, int32_t value){ return; }
	void Tbm1Write(int32_t hubAddr, int32_t addr, int32_t value){ return; }
	void Tbm2Write(int32_t hubAddr, int32_t addr, int32_t value){ return; }
    void SetClock(unsigned char MHz){ return; }
    void SetDelay(unsigned char signal, uint16_t ns){ return; }
    void AdjustDelay(uint16_t k) { SetDelay(255, k); }

    // === module test functions ======================================
    
    // --- implemented funtions: ---
    void InitDAC();
    void Init_Reset();
    void prep_dig_test();
    void SetMHz(int MHz);
    void I2cAddr(unsigned char id){ roc_I2cAddr(id); }
    void Set(unsigned char reg, unsigned char value){ roc_SetDAC(reg, value); }
    void SetReg(unsigned char addr, uint16_t value){ roc_SetDAC(addr, value); }
    void ArmPixel(int col, int row);
    void ArmPixel(int col, int row, int trim);    
    void DisarmPixel(int col, int row);
    void DisableAllPixels();
    void EnableColumn(int col);
    void EnableAllPixels(int32_t trim[]);
    void SetChip(int iChip);    
    int32_t MaskTest(int16_t nTriggers, int16_t res[]);
	int32_t ChipEfficiency(int16_t nTriggers, int32_t trim[], double res[]); 
	void DacDac(int32_t dac1, int32_t dacRange1, int32_t dac2, int32_t dacRange2, int32_t nTrig, int32_t result[]);
	void AddressLevels(int32_t position, int32_t result[]){ return;}
    int32_t CountReadouts(int32_t nTriggers);
	int32_t CountReadouts(int32_t nTriggers, int32_t chipId);
	int32_t CountReadouts(int32_t nTriggers, int32_t dacReg, int32_t dacValue);
    int32_t Threshold(int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg);    
	int32_t PixelThreshold(int32_t col, int32_t row, int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim);
    int32_t PixelThresholdXtalk(int32_t col, int32_t row, int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim);    
	int32_t ChipThreshold(int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim[], int32_t res[]);
	void ChipThresholdIntern(int32_t start[], int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim[], int32_t res[]);
	int32_t SCurve(int32_t nTrig, int32_t dacReg, int32_t threshold, int32_t res[]);
    int32_t SCurve(int32_t nTrig, int32_t dacReg, int32_t thr[], int32_t chipId[], int32_t sCurve[]);
	int32_t SCurveColumn(int32_t column, int32_t nTrig, int32_t dacReg, int32_t thr[], int32_t trims[], int32_t chipId[], int32_t res[]);
    int32_t PH(int32_t col, int32_t row);
    // ----------------------------


	bool GetPixel(int32_t x){ return false; }
	int32_t FindLevel(){ return 1; }
    unsigned char test_PUC(unsigned char col, unsigned char row, unsigned char trim){ return 1; }
	void testColPixel(int32_t col, int32_t trimbit, unsigned char *res){ return; }
	bool GetLastDac(unsigned char count, int32_t &ldac){ return false; }
    bool ScanDac(unsigned char dac, unsigned char count,
	unsigned char min, unsigned char max, int16_t *ldac){ return false; }

	int32_t AoutLevel(int16_t position, int16_t nTriggers){ return 1; }
	int32_t AoutLevelChip(int16_t position, int16_t nTriggers, int32_t trims[],  int32_t res[]){ return 1; }
	int32_t AoutLevelPartOfChip(int16_t position, int16_t nTriggers, int32_t trims[], int32_t res[], bool pxlFlags[]){ return 1; }
	void DoubleColumnADCData(int32_t column, int16_t data[], int32_t readoutStop[]){ return; }
	void ADCRead(int16_t buffer[], uint16_t &wordsread, int16_t nTrig){ return; }
	void PHDac(int32_t dac, int32_t dacRange, int32_t nTrig, int32_t position, int16_t result[]){ return; }
	void TBMAddressLevels(int32_t result[]){ return; }
	void TrimAboveNoise(int16_t nTrigs, int16_t thr, int16_t mode, int16_t result[]){ return; }

	void ReadData(int32_t position, int32_t size, int32_t result[]){ return; }
	void ReadFPGAData(int32_t size, int32_t result[]){ return; }

	void SetEmptyReadoutLengthADC(int32_t emptyReadoutLengthADC){ return; }
	void SetTbmChannel(int32_t tbmChannel){ return; }
	void SetDTL(int32_t value){ return; }
	void SetNRocs(int32_t value){ return; }
	void SetHubID(int32_t value){ return; }
    void SetAoutChipPosition(int32_t value){ return; }
    void MemRead(uint32_t addr, uint16_t size,
                 unsigned char * s){ return; }

     // == PSI46 testboard methods ===========================================

    unsigned char isClockPresent(){ return 1; }
    void SetClockStretch(unsigned char src,
		uint16_t delay, uint16_t width){ return; }

    void ForceSignal(unsigned char pattern){ return; }

    bool ShowUSB() { return false; }; 

    bool Open(char name[], bool init = true){ return true;}


    // =======================================================================

	int32_t demo(int16_t x){ return 1; }

	void GetColPulseHeight(unsigned char col, unsigned char count,
				int16_t data[]){ return; }

	void Scan1D(unsigned char vx,
			unsigned char xmin, unsigned char xmax,	char xstep,
			unsigned char rep, uint32_t usDelay, unsigned char res[]){ return; }

	void BumpTestColPixel(unsigned char col, unsigned char res[]);
	void BumpTestColRef(unsigned char col, unsigned char res[]);
	void DacDac(int16_t dac1, int16_t dacRange1, int16_t dac2, int16_t dacRange2,
		int16_t nTrig, int16_t res[], int16_t rocpos);

	// ===================================================================

	void ScanAdac(uint16_t chip, unsigned char dac,
      unsigned char min, unsigned char max, char step,
      unsigned char rep, uint32_t usDelay, unsigned char res[]){ return; }
	void CdVc(uint16_t chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep,
          unsigned char cdinit, uint16_t &lres, uint16_t res[]){ return; }

	// === xray test =====================================================
	char CountAllReadouts(int32_t nTrig, int32_t counts[], int32_t amplitudes[]){ return 1; }



private:
    static const bool enableAll = true;
    static const int hubId = 0;
    static const int nRocs = 1;
    

};
