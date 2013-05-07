#include <fstream>
#include <sstream>
#include <string>

#include <string.h>

#include "interface/Log.h"
#include "BasePixel/DACParameters.h"
#include "BasePixel/Roc.h"
#include "BasePixel/CalibrationTable.h"
#include "BasePixel/TBAnalogInterface.h"

DACParameters::DACParameters() : roc(NULL)
{
    Initialize();
}


DACParameters::DACParameters(Roc * const aRoc) : roc(aRoc)
{
    Initialize();
}


void DACParameters::Initialize()
{
    for (int i = 0; i < NDACParameters; i++)
    {
        parameters[i] = -1;
        names[i] = (char *) "";
    }

    names[1]   = (char *) "Vdig";
    names[2]   = (char *) "Vana";
    names[3]   = (char *) "Vsf";
    names[4]   = (char *) "Vcomp";
    names[5]   = (char *) "Vleak_comp";
    names[6]   = (char *) "VrgPr";
    names[7]   = (char *) "VwllPr";
    names[8]   = (char *) "VrgSh";
    names[9]   = (char *) "VwllSh";
    names[10]  = (char *) "VhldDel";
    names[11]  = (char *) "Vtrim";
    names[12]  = (char *) "VthrComp";
    names[13]  = (char *) "VIBias_Bus";
    names[14]  = (char *) "Vbias_sf";
    names[15]  = (char *) "VoffsetOp";
    names[16]  = (char *) "VIbiasOp";
    names[17]  = (char *) "VOffsetR0";
    names[18]  = (char *) "VIon";
    names[19]  = (char *) "Vcomp_ADC"; // was VIbias_PH
    names[20]  = (char *) "VIref_ADC"; // was Ibias_DAC
    names[21]  = (char *) "VIbias_roc";
    names[22]  = (char *) "VIColOr";
    names[23]  = (char *) "Vnpix";
    names[24]  = (char *) "VSumCol";
    names[25]  = (char *) "Vcal";
    names[26]  = (char *) "CalDel";
    names[27]  = (char *) "RangeTemp";
    names[253] = (char *) "CtrlReg";
    names[254] = (char *) "WBC";
}


// -- sets all the current DAC parameters
void DACParameters::Restore()
{
    for (int i = 0; i < NDACParameters; i++)
    {
        if (parameters[i] != -1)
        {
            SetParameter(i, parameters[i], false);
        }
    }
}


DACParameters * DACParameters::Copy()
{
    DACParameters * newParameters;
    newParameters = new DACParameters(roc);
    for (int i = 0; i < NDACParameters; i++)
    {
        newParameters->_SetParameter(i, parameters[i]);
    }
    return newParameters;
}


bool DACParameters::Execute(SysCommand command)
{
    for (int iDAC = 0; iDAC < NDACParameters; iDAC++)
    {

        if ((strcmp(names[iDAC], "") != 0) && (strcmp(command.carg[0], names[iDAC]) == 0))
        {
            SetParameter(iDAC, *command.iarg[1]);
            return true;
        }
    }
    return false;
}


// == accessing =======================================================================


// -- sets a DAC parameter
void DACParameters::SetParameter(int reg, int value, bool correction)
{
    int correctValue;
    if (correction)
    {
        if (reg == 25) correctValue = gCalibrationTable->CorrectedVcalDAC(value);
        else correctValue = value;
    }
    else correctValue = value;

    parameters[reg] = correctValue;
    roc->RocSetDAC(reg, correctValue);

    if (reg == 254)  // WBC needs a reset to get active
    {
        roc->CDelay(3000);
        roc->GetTBAnalogInterface()->Single(0x08); //send a reset to set a DAC
    }

    // some DACs (especially voltages) need some time to get "active"
    if (reg == 2) {roc->CDelay(4000);}        // Vana
    else if (reg == 11) {roc->CDelay(6000);}  // VTrim
    else if (reg == 12) {roc->CDelay(3000);}  // VthrComp
    else if (reg == 25) {roc->CDelay(3000);}  // Vcal
    else {roc->CDelay(1000);}

    psi::LogDebug() << "[DACParameters] Parameter " << reg << " is set to "
                    << correctValue << '.' << psi::endl;
}


// -- sets a DAC parameter
void DACParameters::SetParameter(const char * dacName, int value)
{
    bool parameterSet = false;
    for (int i = 0; i < NDACParameters; i++)
    {
        if (strcmp(names[i], dacName) == 0)
        {
            SetParameter(i, value);
            parameterSet = true;
            i = NDACParameters;
        }
    }
    if (!parameterSet)
        psi::LogInfo() << "[DACParameters] Error: DAC Parameter " << dacName
                       << " is not found." << psi::endl;
}


// -- returns the DAC value of dacName
int DACParameters::GetDAC(const char * dacName)
{
    for (int i = 0; i < NDACParameters; i++)
    {
        if (strcmp(names[i], dacName) == 0)
        {
            return parameters[i];
        }
    }
    psi::LogInfo() << "[DACParameters] Error: DAC Parameter " << dacName
                   << " is not found." << psi::endl;
    return 0;
}


int DACParameters::GetDAC(int reg)
{
    return parameters[reg];
}


//  -- gives the name of a DAC
char * DACParameters::GetName(int reg)
{
    return names[reg];
}


// == file input / output ===================================================


// -- reads DAC parameters from a file and sets them
bool DACParameters::ReadDACParameterFile(const char * _file)
{
    std::ifstream _input(_file);
    if (!_input.is_open())
    {
        psi::LogInfo() << "[DACParameters] Can not open file " << _file
                       << " to read DAC parameters." << psi::endl;

        return false;
    }

    psi::LogInfo() << "[DACParameters] Reading DAC-Parameters from " << _file
                   << '.' << psi::endl;

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


// -- writes the DAC parameters to a file
bool DACParameters::WriteDACParameterFile(const char * filename)
{
    FILE * file = fopen(filename, "w");
    if (!file)
    {
        psi::LogInfo() << "[DACParameters] Could not open file " << filename
                       << " to write DAC parameters." << psi::endl;
        return false;
    }

    psi::LogInfo() << "[DACParameters] Writing DAC-Parameters to " << filename
                   << '.' << psi::endl;

    for (int i = 0; i < NDACParameters; i++)
    {
        if (parameters[i] != -1)
        {
            fprintf(file, "%3d %10s %3d\n", i, names[i], parameters[i]);
        }
    }

    fclose(file);
    return true;
}

void DACParameters::Print()
{
    psi::LogInfo() << "DAC\tName\t\tValue" << psi::endl;
    psi::LogInfo() << "-----------------------------" << psi::endl;
    for (int i = 0; i < NDACParameters; i++) {
        if (parameters[i] == -1)
            continue;
        if (strlen(names[i]) >= 7)
            psi::LogInfo() << i << "\t" << names[i] << ":\t" << parameters[i] << psi::endl;
        else
            psi::LogInfo() << i << "\t" << names[i] << ":\t\t" << parameters[i] << psi::endl;
    }
}

// == Private =======================================================



// -- saves the value of an parameter but doesn't set the DAC
void DACParameters::_SetParameter(int reg, int value)
{
    parameters[reg] = value;
}
