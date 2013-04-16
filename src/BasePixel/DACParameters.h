// The class represents the DAC settings of a readout chip (ROC)

#ifndef DACPARAMETERS
#define DACPARAMETERS

#include <stdio.h>

#include "SysCommand.h"

class Roc;

class DACParameters
{
  public:
    DACParameters();
    DACParameters(Roc* const roc);
    void Initialize();
    DACParameters* Copy();
    bool Execute(SysCommand command);
    void Restore();
    void Print();
    
    // == accessing =============================================================
    void SetParameter(int reg, int value, bool correction = true);
    void SetParameter(const char* dacName, int value);
    int GetDAC(const char*dacName);
    int GetDAC(int reg);
    char* GetName(int reg);

    // == file input / output =================================================== 
    bool ReadDACParameterFile ( const char *filename);
    bool WriteDACParameterFile( const char *filename);

  private:
    void _SetParameter(int reg, int value);

  protected:
    static const int NDACParameters = 256;

    int parameters[NDACParameters];
    char* names[NDACParameters];

    Roc* const roc;
};

#endif
