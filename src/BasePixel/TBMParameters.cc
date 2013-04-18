#include <fstream>
#include <sstream>
#include <string>

#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "BasePixel/TBMParameters.h"
#include "BasePixel/TBM.h"
#include "interface/Log.h"

TBMParameters::TBMParameters() : tbm(NULL)
{
    Initialize();
}


TBMParameters::TBMParameters(TBM * const aTBM) : tbm(aTBM)
{
    Initialize();
}


void TBMParameters::Initialize()
{
    for (int i = 0; i < NTBMParameters; i++)
    {
        parameters[i] = -1;
        names[i] = "";
    }

    names[0] = "Single";
    names[1] = "Speed";
    names[2] = "Inputbias";
    names[3] = "Outputbias";
    names[4] = "Dacgain";
    names[5] = "Triggers";
    names[6] = "Mode";
}


bool TBMParameters::Execute(SysCommand command)
{
    for (int iDAC = 0; iDAC < NTBMParameters; iDAC++)
    {

        if ((strcmp(names[iDAC], "") != 0) && (strcmp(command.carg[0], names[iDAC]) == 0))
        {
            SetParameter(iDAC, *command.iarg[1]);
            return true;
        }
    }
    return false;
}

// -- sets all the current DAC parameters
void TBMParameters::Restore()
{
    for (int i = 0; i < NTBMParameters; i++)
    {
        if (parameters[i] != -1)
        {
            SetParameter(names[i], parameters[i]);
        }
    }
}


TBMParameters * TBMParameters::Copy()
{
    TBMParameters * newParameters;
    newParameters = new TBMParameters(tbm);
    for (int i = 0; i < NTBMParameters; i++)
    {
        newParameters->_SetParameter(i, parameters[i]);
    }
    return newParameters;
}


// == accessing =======================================================================


void TBMParameters::SetParameter(int reg, int value)
{
    if (reg > -1 && reg < NTBMParameters)
    {
        //  psi::LogInfo( "TBMParameters") << "Setting parameter "
        //                        << names[reg] << " to value: "
        //                         << value << psi::endl;
        SetParameter(names[reg], value);
    }
}


void TBMParameters::SetParameter(const char * dacName, int value)
{
    bool parameterSet = false;

    //         printf("%s %i\n", dacName, value);
    for (int i = 0; i < NTBMParameters; i++)
    {
        if (strcmp(names[i], dacName) == 0)
        {
            //    psi::LogInfo( "TBMParameters") << "Setting parameter "
            //                               << dacName << " to value: "
            //                              << value << psi::endl;
            parameters[i] = value;
            if (i == 0) //single, dual
            {
                if (value == 0) tbm->setSingleMode();
                else if (value == 1) tbm->setDualMode();
                else if (value == 2) tbm->setSingleMode2();
            }
            else if (i == 1) //speed
            {
                tbm->set2040Readout(value);
            }
            else if (i == 2) //inputbias
            {
                tbm->setTBMDAC(0, value);
            }
            else if (i == 3) //outputbias
            {
                tbm->setTBMDAC(1, value);
            }
            else if (i == 4) //dacgain
            {
                tbm->setTBMDAC(2, value);
            }
            else if (i == 5) //triggers
            {
                if (value == 0) tbm->setIgnoreTriggers(0x00);
                else if (value == 1) tbm->setIgnoreTriggers(0x10);
                else if (value == 2) tbm->setDisableTriggers(0x40);
            }
            else if (i == 6) //mode
            {
                if (value == 0) tbm->setMode(0x00);  //sync
                else if (value == 1) tbm->setMode(0xC0); //cal
                else if (value == 2) tbm->setMode(0x80); //clear
            }
            parameterSet = true;
            i = NTBMParameters;
        }
    }
    if (!parameterSet)
        psi::LogInfo() << "[TBMParameters] Error: TBM Parameter '" << dacName
                       << "' is not found." << psi::endl;
}


// -- returns the DAC value of dacName
int TBMParameters::GetDAC(const char * dacName)
{
    for (int i = 0; i < NTBMParameters; i++)
    {
        if (strcmp(names[i], dacName) == 0)
        {
            return parameters[i];
        }
    }
    psi::LogInfo() << "[TBMParameters] Error: TBM Parameter '" << dacName
                   << "' is not found." << psi::endl;

    return 0;
}


int TBMParameters::GetDAC(int reg)
{
    return parameters[reg];
}


//  -- gives the name of a DAC
const char * TBMParameters::GetName(int reg)
{
    return names[reg];
}


// == file input / output ===================================================


// -- reads TBM parameters from a file and sets them
bool TBMParameters::ReadTBMParameterFile(const char * _file)
{
    std::ifstream _input(_file);
    if (!_input.is_open())
    {
        psi::LogInfo() << "[TBMParameters] Error: Can not open file '" << _file
                       << "' to read TBM parameters." << psi::endl;

        return false;
    }

    psi::LogInfo() << "[TBMParameters] Reading TBM-Parameters from '" << _file
                   << "'." << psi::endl;

    // Read file by lines
    for (std::string _line; _input.good();)
    {
        getline(_input, _line);

        // Skip Empty Lines and Comments (starting from # or - )
        if (!_line.length()
                || '#' == _line[0]
                || '-' == _line[0]) continue;

        std::istringstream _istring(_line);
        std::string _tmp;
        int _register;
        int _value;

        _istring >> _register >> _tmp >> _value;

        // Skip line in case any errors occured while reading parameters
        if (_istring.fail() || !_tmp.length()) continue;

        SetParameter(_register, _value);
    }

    _input.close();

    return true;
}


// -- writes the TBM parameters to a file
bool TBMParameters::WriteTBMParameterFile(const char * _file)
{
    FILE * file = fopen(_file, "w");
    if (!file)
    {
        psi::LogInfo() << "[TBMParameters] Error: Can not open file '" << _file
                       << "' to write TBM parameters." << psi::endl;
        return false;
    }

    psi::LogInfo() << "[TBMParameters] Writing TBM-Parameters to '" << _file
                   << "'." << psi::endl;

    for (int i = 0; i < NTBMParameters; i++)
    {
        if (parameters[i] != -1)
        {
            fprintf(file, "%3d %10s %3d\n", i, names[i], parameters[i]);
        }
    }

    fclose(file);
    return true;
}


// == Private =======================================================
// -- saves the value of an parameter but doesn't set the DAC
void TBMParameters::_SetParameter(int reg, int value)
{
    parameters[reg] = value;
}
