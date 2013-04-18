// Class provides basic module functionality, contains a TBM and several ROCs

#ifndef MODULE
#define MODULE

#include "Roc.h"
#include "TBM.h"
#include "TBInterface.h"
#include "BasePixel/ConfigParameters.h"

class Module
{

public:
    inline Module() {};
    Module(ConfigParameters * configParameters, int aCNId, TBInterface * aTBInterface);
    virtual ~Module();

    void Initialize();
    void Execute(SysCommand &command);
    void SetDAC(int chipId, int reg, int value);
    void SetTBM(int chipId, int reg, int value);
    void WriteTrimConfiguration(char * filename);
    void ReadTrimConfiguration(char * filename);
    void WriteDACParameterFile(const char * filename);
    void ReadDACParameterFile(char * filename);
    void SetTrim(int iRoc, int iCol, int iRow, int trimBit);
    int GetTemperature();
    void AdjustDTL();

    int GetHubId();
    int GetTBM(int reg);
    int NRocs();
    Roc * GetRoc(int iRoc);
    TBM * GetTBM();
    void SetTBMSingle(int tbmChannel);

protected:

    int nRocs;
    Roc * roc[MODULENUMROCS];

    TBM * tbm;
    TBInterface * tbInterface;

    ConfigParameters * configParameters;

    int hubId;
    int controlNetworkId;

};


#endif

