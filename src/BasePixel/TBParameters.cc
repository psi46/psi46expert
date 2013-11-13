#include <fstream>
#include <sstream>
#include <string>

#include <string.h>
#include <stdio.h>

#include "BasePixel/TBParameters.h"
#include "BasePixel/TBInterface.h"
#include "interface/Log.h"

TBParameters::TBParameters(TBInterface * aTBInterface)
{
    tbInterface = aTBInterface;

    for (int i = 0; i < NTBParameters; i++) {
        parameters[i] = -1;
        names[i] = "";
    }

    names[8] = "clk";
    names[9] = "sda";
    names[10] = "ctr";
    names[11] = "tin";
    names[12] = "rda";

    names[17] = "trc";
    names[18] = "tcc";
    names[19] = "tct";
    names[20] = "ttk";
    names[21] = "trep";
    names[22] = "cc";

    names[77] = "spd"; // dummy register for clock frequency

}

TBParameters * TBParameters::Copy()
{
    TBParameters * newParameters;
    newParameters = new TBParameters(tbInterface);
    for (int i = 0; i < NTBParameters; i++)
    {
        newParameters->_SetParameter(i, parameters[i]);
    }
    return newParameters;
}


// -- sets a parameter
void TBParameters::SetParameter(int reg, int value)
{
    parameters[reg] = value;
    if (reg == 77) tbInterface->SetClock(value);
    else if (reg > 15)
    {
        tbInterface->Set(reg, value);
    }
    else
    {
        tbInterface->SetDelay(reg, value);
    }
    //  gLog->printf("set tb parameter %i to %i\n", reg, value);
}


bool TBParameters::Execute(SysCommand command)
{
    for (int iDAC = 0; iDAC < NTBParameters; iDAC++)
    {

        if ((strcmp(names[iDAC], "") != 0) && (strcmp(command.carg[0], names[iDAC]) == 0))
        {
            SetParameter(iDAC, *command.iarg[1]);
            return true;
        }
    }
    return false;
}


// -- sets all current parameters
void TBParameters::Restore() {
    for (int i = 0; i < NTBParameters; i++) {
        if (parameters[i] != -1) {
            SetParameter(i, parameters[i]);
        }
    }
    tbInterface->Flush();
}


// == accessing =============================================================


// -- sets a testboard parameter
void TBParameters::SetParameter(const char * dacName, int value)
{
  cout << "TBPar::SetPar" << endl;
    for (int i = 0; i < NTBParameters; i++) {
        if (strcmp(names[i], dacName) == 0) {
            SetParameter(i, value);
        }
    }
}


int TBParameters::GetParameter(const char * dacName)
{
    for (int i = 0; i < NTBParameters; i++)
    {
        if (strcmp(names[i], dacName) == 0)
        {
            return parameters[i];
        }
    }
    psi::LogInfo() << "[TBParameters] Error: DAC Parameter '"
                   << dacName << "' is not found." << psi::endl;

    return 0;
}


// == file input / output ===================================================


// -- reads the parameters from a file
bool TBParameters::ReadTBParameterFile(const char * _file)
{
    std::ifstream _input(_file);
    if (!_input.is_open())
    {
        psi::LogInfo() << "[TBParameters] Error: Can not open file '" << _file
                       << "' to read TB parameters." << psi::endl;

        return false;
    }

    psi::LogInfo() << "[TBParameters] Reading TB-Parameters from '" << _file
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
		psi::LogInfo() << _line << psi::endl;
    }

    _input.close();

    tbInterface->Flush();

    return true;
}


// -- write the parameters to a file
bool TBParameters::WriteTBParameterFile(const char * _file)
{
    FILE * file = fopen(_file, "w");
    if (!file)
    {
        psi::LogInfo() << "[TBParameters] Error: Can not open file '" << _file
                       << "' to write TB parameters." << psi::endl;

        return false;
    }

    psi::LogInfo() << "[TBParameters] Writing TB-Parameters to '" << _file
                   << "'." << psi::endl;

    for (int i = 0; i < NTBParameters; i++)
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

// -- loads a parameter without setting it
void TBParameters::_SetParameter(int reg, int value)
{
    parameters[reg] = value;
}
