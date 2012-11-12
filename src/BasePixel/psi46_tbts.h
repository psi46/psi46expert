// PSI46 testboard API 

#ifndef PSI46TBTS_H
#define PSI46TBTS_H


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
#define T_ResCal   0	// 4..255,  #clocks(res->cal) = T_ResCal+2
#define T_CalCal   1	// 4..255,  #clocks(cal->cal) = T_CalCal+2
#define T_CalTrg   2	// 4..255,  #clocks(cal->trg) = T_CalTrg+2
#define T_TrgTok   3	// 4..255,  #clocks(trg->tok) = T_TrgTok+0.5
#define T_Periode  4	// 1..255
#define CALREP     5	// 1..63,   #cal = CALREP

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


class CTestboard_TS
{
	CUSB usb;
	bool TBM_present;
public:
	CTestboard_TS();
	~CTestboard_TS() { Close(); }

	// === board connection methods =========================================

	bool EnumFirst(unsigned int &nDevices) { return usb.EnumFirst(nDevices); };
	bool EnumNext(char name[]) { return usb.EnumNext(name); }
	bool Open(char name[], bool init=true); // opens a connection
	void Close();				// closes the connection to the testboard
	bool IsConnected() { return usb.Connected(); }
	const char * ConnectionError() { return usb.GetErrorMsg(usb.GetLastError()); }
	bool GetVersion(char *s, unsigned int n);
	void Welcome();				// displays the welcome message
	void SetLed(unsigned char value);
	unsigned char  GetBoardId();			// reads the board number
	void Init();				// inits the testboard to default values


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

	void I2cAddr(unsigned char id);		// set testboard I2C address

	void Pon();					// switch ROC power on
	void Poff();					// switch ROC power off

	void SetVA(double V);		// set VA voltage in V
	void SetVD(double V);		// set VD voltage in V
	void SetIA(double A);		// set VA current limit in A
	void SetID(double A);		// set VD current limit in A

	double GetVA();	// get VA voltage in V
	double GetVD();	// get VD voltage in V
	double GetIA();	// get VA current in A
	double GetID();	// get VD current in A

	int GetThresh();
	bool SetLatchDelay(int delay);

	void HVon();		// switch HV relais on
	void HVoff();	// switch HV relais off

	void ResetOn();	// switch RESET-line to reset state (low)
	void ResetOff();	// switch RESET-line to not reset state (high)

	// Telescope functions
	
	void StartRun(unsigned int MemSize,unsigned int EvtNum=0); 
	void StopRun();
	void EventWriterStop();
	void EventWriterStart();

	unsigned int DaqInit(int buf);
	void DaqEnable();
	void DaqDisable();

	bool ReadyForReadout();

	unsigned int GetStatus(); // Get TB status
	int GetRawData(int MemSize);
	int GetData(unsigned short* buffer);
	int GetDaqSize();

	unsigned int GetPointer();
	void ClearRam();
 
	unsigned int Daq_Init(unsigned int size);
	void Daq_Enable();
	void Daq_Disable();
	bool Daq_Ready();
	unsigned int Daq_GetPointer();
	unsigned int Daq_GetSize();
	void Daq_Done();

	unsigned char GetBoardID();

	bool UseInternalTriggers();
	bool DisableInternalTriggers();
	bool SendCalibrationPulsesOnInternalTriggers(unsigned short delay);
	bool DisableCalPulses();
	void CalStart();
 	int DaqReady();	
        void TrigPause();
};


#endif // PSI46TBTS_H
