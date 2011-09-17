// Author  : Peter Trüb
// Modified: Samvel Khalatyan (samvel at cern dot ch)
//
// The class represents the settings of a token bit manager (TBM)

#ifndef TBMPARAMETERS
#define TBMPARAMETERS

#include <stdio.h>

#include "SysCommand.h"

class TBM;

class TBMParameters
{
  public:
    TBMParameters();
    TBMParameters(TBM* const aTBM);
    void Initialize();
    TBMParameters* Copy();
    bool Execute(SysCommand command);
    void Restore();
    
    // == accessing =============================================================
    void SetParameter(int reg, int value);
    void SetParameter(char* dacName, int value);
    int GetDAC(const char*dacName);
    int GetDAC(int reg);
    char* GetName(int reg);

    // == file input / output =================================================== 
    bool ReadTBMParameterFile ( const char *filename);
    bool WriteTBMParameterFile( const char *filename);

  private:
    void _SetParameter(int reg, int value);

  protected:
    static const int NTBMParameters = 7;

    int parameters[NTBMParameters];
    char* names[NTBMParameters];

    TBM* const tbm;
};

#endif
