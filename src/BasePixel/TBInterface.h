// This class is an abstract superclass for the analog and digital testboard interfaces
// Provides functionality to read and set testboard parameters

#ifndef TBINTERFACE
#define TBINTERFACE

#include "SysCommand.h"
#include "TBParameters.h"

class ConfigParameters;

class TBInterface
{
public:
    TBInterface();
    virtual ~TBInterface();

    // == Parameters =======================================================

    void SetTBParameter(int reg, int value);
    void SetTBParameter(const char * dacName, int value);
    int GetParameter(const char * dacName);
    bool ReadTBParameterFile(const char * filename);
    bool WriteTBParameterFile(const char * filename);
    void SaveTBParameters();
    void RestoreTBParameters();
    TBParameters * GetTBParameters();


    // == General functions ================================================

    virtual void Pon() = 0;
    virtual void Poff() = 0;
    virtual void Set(int reg, int value) = 0;
    virtual void Single(int mask) = 0;
    virtual void Intern(int mask) = 0;
    virtual void Extern(int mask) = 0;
    virtual int GetRoCnt() = 0;
    virtual void Initialize(ConfigParameters * configParameters) = 0;
    virtual int Startup(int port) = 0;
    virtual void Cleanup() = 0;
    virtual int Present() = 0;
    virtual void I2cAddr(int id) = 0;
    virtual void Execute(SysCommand &command) = 0;
    virtual int IsPresent() = 0;
    virtual void Flush() = 0;
    virtual void Clear() = 0;
    virtual void CDelay(unsigned int n) = 0;
    virtual bool IsAnalogTB() = 0;

    // == TBM functions ======================================================
    virtual void Tbmenable(int on) = 0;
    virtual int Tbm1write(const int hubAddr, const int registerAddress, const int value) = 0;
    virtual int Tbm2write(const int hubAddr, const int registerAddress, const int value) = 0;
    bool TBMIsPresent();

    // == ROC functions ======================================================


    // == Helper functions ===================================================
    int COLCODE(int x);
    int ROWCODE(int x);
    int RangeCheck(int value, int min, int max);

protected:
    int fIsPresent;
    int rctk_flag;
    int ChipId;
    int TBMpresent;
    int HUBaddress;

    TBParameters * tbParameters;
    TBParameters * savedTBParameters;

    static const int RES = 0x08;
    static const int CAL = 0x04;
    static const int TRG = 0x02;
    static const int TOK = 0x01;

    static const unsigned char addresstbm1 = 0xE0;
    static const unsigned char addresstbm2 = 0xF0;
    static const unsigned char port4 = 0x04;
};

#endif

