// Author  : Peter Trüb
// Modified: Samvel Khalatyan (samvel at cern dot ch)
//
// This class provides the functionality to program the analog testboard via USB
// This class is mainly a dummy class which forwards the commands to Beat's 
// testboard class CTestboard. A USB read buffer was added, because problems 
// occured under linux, when more than about 32K signals were sent without 
// reading back.

#ifndef TBANALOGINTERFACE
#define TBANALOGINTERFACE

#include "BasePixel/TBInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/psi46_tb.h"

class TBAnalogInterface: public TBInterface
{
public:
    TBAnalogInterface(ConfigParameters *configParameters);
    virtual ~TBAnalogInterface();

    // == General functions ================================================
    
    void Execute(SysCommand &command);
    void Pon();
    void Poff();
    void Set(int reg, int value);
    void SetReg(int reg, int value);
    void Single(int mask);
    void Intern(int mask);
    void Extern(int mask);
    int GetRoCnt();
    void Initialize(ConfigParameters *configParameters);
    int Startup(int port);
    void Cleanup();
    void Clear();
    int Present();
    void I2cAddr(int id);
    int IsPresent() {return fIsPresent;}
    bool IsAnalogTB() {return true;}
    int CountReadouts(int count, int chipId);
    void SingleCal();
    void SendCal(int nTrig);
    void SetEmptyReadoutLength(int length);
    int GetEmptyReadoutLength();
    void SetEmptyReadoutLengthADC(int length);
    int GetEmptyReadoutLengthADC();
    void SetEmptyReadoutLengthADCDual(int length);
    int GetEmptyReadoutLengthADCDual();
    void SetEnableAll(int value);
          unsigned short GetModRoCnt(unsigned short index);

    // == Analog functions =================================================
    
    void SetClock(int mhz);
    void DataCtrl(bool clear, bool trigger, bool cont = false);
          void DataEnable(bool on);
    bool DataRead(short buffer[], unsigned short buffersize, unsigned short &wordsread);
    void SetDelay(int signal, int ns);
          void SetClockStretch(unsigned char src, unsigned short delay, unsigned short width);
    void CDelay(unsigned int us);
    bool SendRoCnt();
    int  RecvRoCnt();
    void Flush();
    void DataTriggerLevel(int level);
          void SetTriggerMode(unsigned short mode);
    void SetTBMChannel(int channel);
    int GetTBMChannel();
    bool TBMPresent() { return cTestboard->TBMPresent(); }

    void ADCRead(short buffer[], unsigned short &wordsread, short nTrig = 1); 
    bool ADCData(short buffer[], unsigned short &wordsread);
    unsigned short ADC(int nbsize);
    unsigned short ADC();
    void SendADCTrigs(int nTrig);
    void SendADCTrigsNoReset(int nTrig);
    bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts);
    int LastDAC(int nTrig, int chipId);
    
    void SetVA(double V);   // set VA voltage in V
    void SetIA(double A);   // set VA current limit in A
    void SetVD(double V);   // set VD voltage in V
    void SetID(double A);   // set VD current limit in A

    double GetVA(); // get VA voltage in V
    double GetIA(); // get VA current in A
    double GetVD(); // get VD voltage in V
    double GetID(); // get VD current in A

    void HVon();    // switch HV relais on
    void HVoff(); // switch HV relais off

    void ResetOn(); // switch RESET-line to reset state (low)
    void ResetOff();  // switch RESET-line to not reset state (high)
    int CountADCReadouts(int count);

          bool Mem_ReadOut(FILE *file, unsigned long addr, unsigned long size);
    
    void SetReg41();

    void StartDataTaking();
    void StopDataTaking();
    
    // == TBM functions ======================================================

    void Tbmenable(int on);
          void ModAddr(int hub);
    void TbmAddr(int hub, int port);
    bool DataTriggerLevelScan();
    int TbmWrite(const int hubAddr, const int addr, const int value);
    int Tbm1write(const int hubAddr, const int registerAddress, const int value);
    int Tbm2write(const int hubAddr, const int registerAddress, const int value);
    bool GetTBMReg(int reg, int &value);

    // == ROC functions ======================================================

    void SetChip(int chipId, int hubId, int portId, int aoutChipPosition);
    void RocClrCal();
    void RocSetDAC(int reg, int value);
    void RocPixTrim(int col, int row, int value);
    void RocPixMask(int col, int row);
    void RocPixCal(int col, int row, int sensorcal);
    void RocColEnable(int col, int on);

    CTestboard *getCTestboard() {return cTestboard;}
    
    int AoutLevel(int position, int nTriggers);
    int AoutLevelChip(int position, int nTriggers, int trims[], int res[]);
    int AoutLevelPartOfChip(int position, int nTriggers, int trims[], int res[], bool pxlFlags[]);
    int ChipEfficiency(int nTriggers, int trim[], double res[]);
          int MaskTest(short nTriggers, short res[]);
    void DoubleColumnADCData(int doubleColumn, short data[], int readoutStop[]);
    int ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim[], int res[]);
    int PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim);
    int SCurve(int nTrig, int dacReg, int threshold, int res[]);
    int SCurveColumn(int column, int nTrig, int dacReg, int thr[], int trims[], int chipId[], int res[]);
    void DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[]);
    void PHDac(int dac, int dacRange, int nTrig, int position, short result[]);
    void AddressLevels(int position, int result[]);
    void TBMAddressLevels(int result[]);
    void TrimAboveNoise(short nTrigs, short thr, short mode, short result[]);
    
  // --------------------------------------------------------

    void ProbeSelect(unsigned char port, unsigned char signal);


    long demo(short x);



    void ScanAdac(unsigned short chip, unsigned char dac, unsigned char min, unsigned char max, char step,unsigned char rep, unsigned int usDelay, unsigned char res[]);
    
    
    void CdVc(unsigned short chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep, unsigned char cdinit, unsigned short &lres, unsigned short res[]);
    
    char CountAllReadouts(int nTrig, long counts[], long amplitudes[]);
	bool GetVersion(char *s, unsigned int n);

private:
    CTestboard *cTestboard;
    
    int TBMChannel;
    int emptyReadoutLength, emptyReadoutLengthADC, emptyReadoutLengthADCDual;
	bool tbmenable;

    // == data buffer ==============================================================  
    static const int bufferSize = 2500000;
    int dataBuffer[bufferSize];
    int signalCounter, readPosition, writePosition;
    void ReadBackData();

    int triggerSource;  // 0 = local, 1 = extern
};

#endif

