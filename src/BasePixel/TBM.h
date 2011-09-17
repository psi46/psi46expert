//
// Author: Danek Kotlinski
//
// Class provides the TBM functionalities
// 


#ifndef TBMCLASS
#define TBMCLASS

#include "TBInterface.h"
#include "TBMParameters.h"
#include "ConfigParameters.h"

// define the masks
// Reg0
const int m2040Readout     = 0x00000001;
const int mDisableClock      = 0x00000002;
const int mStackFull32       = 0x00000004;
const int mPauseReadout      = 0x00000008;
const int mIgnoreTriggers    = 0x00000010;
const int mStackReadback     = 0x00000020;
const int mDisableTriggerOut = 0x00000040;
const int mDisableAnalogOut  = 0x00000080;
// Reg1
const int mMode              = 0x000000C0;
const int mDisableCalDelTrig = 0x00000020;
// Reg2
const int mInjectTrigger     = 0x00000001;
const int mInjectSync        = 0x00000002;
const int mInjectROCReset    = 0x00000004;
const int mInjectCal         = 0x00000008;
const int mInjectTBMReset    = 0x00000010;
const int mClearStack        = 0x00000020;
const int mClearTokenOut     = 0x00000040;
const int mClearTrigCounter  = 0x00000080;
// Reg4
const int mDisableAnalogDrv  = 0x00000001;
const int mDisableTokenOutDrv= 0x00000002;
const int mForceReadoutClock = 0x00000004;

// Mode commands
const int selectModeSync         = 0x00;
const int selectModeClearCounter = 0x80;
const int selectModeCal          = 0xC0;

class TBM
{

public:
	inline TBM() {};
	TBM(ConfigParameters* configParameters, int aCNId, TBInterface *aTbInterface);
	virtual ~TBM();
	
	void Initialize( const char *tbmParametersFileName);
	void Execute(SysCommand &command);
	bool ReadTBMParameterFile( const char *filename);
	bool WriteTBMParameterFile(const char* filename);
	void SetTBMChannel(int channel);
	int ScanHubIDs();
	int GetDAC(int reg);
	void SetDAC(int reg, int value);
	bool GetReg(int reg, int &value);

	// Initilization routine. Do it at construction or nor?
	int init(void);
	// Basic access routines
	// Set a TBM1 register to a defined value,
	// destroys history(all bits replaced), stores the new value.
	int setTBM1(const int registerAddress, const int value);
	int setTBM2(const int registerAddress, const int value); //same for TBM2
	int setTBMDAC(const int DACAddress, const int value); //DACAddress=0,1,2

	// Derived access routines with masks. Preserves other bits.
	// Modify one register in TBM1, change only the selected bits set in mask
	// e.g. change=0xFF to flip all bits allowed by the mask to 1.
	inline int setTBM1Reg0(const int change, const int mask)
	{
		int newValue = replace(TBM1Reg0, change, mask);
		//cout<<"setTBM1Reg0 "<<hex<<newValue<<" "<<change<<" "<<mask<<dec<<endl;
		return setTBM1(0,newValue);
	}
	// Same for register 1, TBM1
	inline int setTBM1Reg1(const int change, const int mask)
	{
		int newValue = replace(TBM1Reg1, change, mask);
		return setTBM1(1,newValue);
	}
	// Same for register 2, TBM1
	inline int setTBM1Reg2(const int change, const int mask)
	{
		int newValue = replace(TBM1Reg2, change, mask);
		return setTBM1(2,newValue);
	}
	// Same for register 4, TBM1
	inline int setTBM1Reg4(const int change, const int mask)
	{
		int newValue = replace(TBM1Reg4, change, mask);
		return setTBM1(4,newValue);
	}
	// Same but for TBM2
	inline int setTBM2Reg0(const int change, const int mask)
	{
		int newValue = replace(TBM2Reg0, change, mask);
		return setTBM2(0,newValue);
	}
	inline int setTBM2Reg1(const int change, const int mask)
	{
		int newValue = replace(TBM2Reg1, change, mask);
		return setTBM2(1,newValue);
	}
	inline int setTBM2Reg2(const int change, const int mask)
	{
		int newValue = replace(TBM2Reg2, change, mask);
		return setTBM2(2,newValue);
	}
	inline int setTBM2Reg4(const int change, const int mask)
	{
		int newValue = replace(TBM2Reg4, change, mask);
		return setTBM2(4,newValue);
	}

	// Higher level derived methods (modifies both TBMs)
	// Flips the TBM readout speed( 0-20MHz/default, 1-40MHz)
	inline int set2040Readout(const int value)
	{    //
		int status = setTBM2Reg0(value,m2040Readout); // For both TBMs
		status = setTBM1Reg0(value,m2040Readout);
		return status;
	}
	// Ignore incomming triggers, 0-not ignore, 0x10 or 0xFF - ignore)
	inline int setIgnoreTriggers(const int value)
	{
		int status = setTBM1Reg0(value,mIgnoreTriggers);
		status = setTBM2Reg0(value,mIgnoreTriggers);
		return status;
	}
	// Disable the out trigger. Header+trailer still send.
	// Ignore incomming triggers, 0-enable, 0x40 or 0xFF - disable)
	inline int setDisableTriggers(const int value)
	{
		int status = setTBM1Reg0(value,mDisableTriggerOut);
		status = setTBM2Reg0(value,mDisableTriggerOut);
		return status;
	}
	// Set the TBM mode (0x00-SYNC mode, 0x80-ClearTriggerCounter, 0xC0-cal mode)
	inline int setMode(const int value)
	{
		int status = setTBM1Reg1(value,mMode);
		status = setTBM2Reg1(value,mMode);
		return status;
	}
	// Disable calibration delayed trigger(0x02 or 0xFF - disable, 0x00 not)
	// The TBM by default (after hard-reset) is in this mode
	// !DOES NOT SEEM TO WORK in the present TBM-v1
	inline int setDisableCalDelTrig(const int value)
	{
		int status = setTBM1Reg1(value,mDisableCalDelTrig);
		status = setTBM2Reg1(value,mDisableCalDelTrig);
		return status;
	}

	// Set the TBM to the single-tbm mode. TBM2(B) enabled.
	inline int setSingleMode2(void)
	{
//                 cout << "TBM:: setSingleMode()" << endl;
		int status = setTBM2Reg4(0x00,0x03);  // Enable TBM2
		status     = setTBM1Reg4(0x03,0x03);  // Disable TBM1
		return status;
	}
	
	// Set the TBM to the single-tbm mode. TBM1(A) enabled.
	inline int setSingleMode(void)
	{
//                 cout << "TBM:: setSingleMode()" << endl;
		int status = setTBM1Reg4(0x00,0x03);  // Enable TBM1
		status     = setTBM2Reg4(0x03,0x03);  // Disable TBM2
		return status;
	}

	inline int setDualMode(void)
	{
//              cout << "TBM:: setDualMode()" << endl;
		int status = setTBM1Reg4(0x00,0x03);  // Enable TBM1
		status = setTBM2Reg4(0x00,0x03);  // Enable TBM2
		return status;
	}

	// Switch ON selected CLEAR & INJECT bits (in register 2)
	int setBit(const int tbm, const int bit); //tbm=1,2 select tbm, bit=0-7

	// These methods just return the stored value, it is not a read.
	inline int getTBM1Reg0(void) const {return TBM1Reg0;}
	int getTBM1Reg1(void) const {return TBM1Reg1;}
	int getTBM1Reg2(void) const {return TBM1Reg2;}
	int getTBM1Reg3(void) const {return TBM1Reg3;}
	int getTBM1Reg4(void) const {return TBM1Reg4;}
	int getTBM2Reg0(void) const {return TBM2Reg0;}
	int getTBM2Reg1(void) const {return TBM2Reg1;}
	int getTBM2Reg2(void) const {return TBM2Reg2;}
	int getTBM2Reg3(void) const {return TBM2Reg3;}
	int getTBM2Reg4(void) const {return TBM2Reg4;}
	int getTBMDAC0(void) const {return TBM1Reg5;}
	int getTBMDAC1(void) const {return TBM1Reg6;}
	int getTBMDAC2(void) const {return TBM1Reg7;}


protected:
	TBMParameters *tbmParameters;

	int hubId; // Address of the control network HUB
	int controlNetworkId; // ID of the constrol network
	
	TBInterface *tbInterface;

	// TBM1 registers
	int TBM1Reg0; //Base+1/0
	int TBM1Reg1; //Base+3/2
	int TBM1Reg2; //Base+5/4
	int TBM1Reg3; //Base+7
	int TBM1Reg4; //Base+9/8
	int TBM1Reg5; // +B/A Analog Input Amplifier Bias = DAC0
	int TBM1Reg6; // +D/C Analog Amplifier Driver Bias = DAC1
	int TBM1Reg7; // +F/E TBM Analog Output Gain = DAC2
	// TBM2 registers (only first 5, no DACs)
	int TBM2Reg0; //Base+1/0
	int TBM2Reg1; //Base+3/2
	int TBM2Reg2; //Base+5/4
	int TBM2Reg3; //Base+7
	int TBM2Reg4; //Base+9/8

	// Combine the updated bits with the old value
	// e.g. change=0xF to flip all allowed bits to 1.
	inline int replace(const int oldValue, const int change, const int mask)
	const
	{
		int newValue = (oldValue & ~mask) | (change & mask);
		//cout << hex << oldValue<<" "<<change<<" "<<mask<<" "<<newValue<<endl;
		return newValue;
	}

};

#endif

