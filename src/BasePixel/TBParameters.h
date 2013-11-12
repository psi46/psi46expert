// The class represents the settings of a testboard

#ifndef TBPARAMETERS
#define TBPARAMETERS

#include <stdio.h>

#include "SysCommand.h"

class TBInterface;

class TBParameters
{
public:
    TBParameters() {};
    TBParameters(TBInterface * aTBInterface);

    virtual ~TBParameters() {};

    bool Execute(SysCommand command);
    void Restore();
    virtual TBParameters * Copy();

    // == accessing =============================================================
    virtual void SetParameter(int reg, int value);
    void SetParameter(const char * tbName, int value);
    int GetParameter(const char * dacName);

    // == file input / output ===================================================
    bool ReadTBParameterFile(const char * filename);
    bool WriteTBParameterFile(const char * filename);

protected:
    void _SetParameter(int reg, int value);

protected:
    static const int NTBParameters = 256;

    int parameters[NTBParameters];
    const char * names[NTBParameters];

    TBInterface * tbInterface;
};

#endif
