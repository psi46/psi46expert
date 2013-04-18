#include "BasePixel/TBInterface.h"
#include "interface/Log.h"


TBInterface::TBInterface()
{
    ChipId = 0;
    TBMpresent = 0;
    HUBaddress = 0;
}


TBInterface::~TBInterface() {}


// == Parameters ================================================


TBParameters * TBInterface::GetTBParameters()
{
    return tbParameters;
}


// -- Sets a testboard parameter
void TBInterface::SetTBParameter(int reg, int value)
{
    tbParameters->SetParameter(reg, value);
}


// -- Sets a testboard parameter
void TBInterface::SetTBParameter(const char * dacName, int value)
{
    tbParameters->SetParameter(dacName, value);
}


int TBInterface::GetParameter(const char * dacName)
{
    return tbParameters->GetParameter(dacName);
}


// -- Saves the testboard parameters for later use
void TBInterface::SaveTBParameters()
{
    savedTBParameters = tbParameters->Copy();
}


// -- Restores the saved testboard parameters
void TBInterface::RestoreTBParameters()
{
    tbParameters = savedTBParameters;
    tbParameters->Restore();
}


// -- Reads the testboard parameters from a file
bool TBInterface::ReadTBParameterFile(const char * _file)
{
    return tbParameters->ReadTBParameterFile(_file);
}


// -- Writes the testboard parameters to a file
bool TBInterface::WriteTBParameterFile(const char * _file)
{
    return tbParameters->WriteTBParameterFile(_file);
}


// -- Checks, if a TBM is present
bool TBInterface::TBMIsPresent()
{
    return TBMpresent;
}


// == Helper functions =============================================================

int TBInterface::COLCODE(int x)
{
    return ((x >> 1) & 0x7e) ^ x;
}


int TBInterface::ROWCODE(int x)
{
    return (x >> 1) ^ x;
}


int TBInterface::RangeCheck(int value, int min, int max)
{
    if (value < min)
    {
        psi::LogInfo() << "[TBInterface] Value too low. Register set to minimum (="
                       << min << ")." << psi::endl;
        return min;
    }
    if (value > max)
    {
        psi::LogInfo() << "[TBInterface] Value too hight. Register set to maximum (="
                       << max << ")." << psi::endl;
        return max;
    }
    return value;
}
