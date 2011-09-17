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


DACParameters::DACParameters(Roc* const aRoc) : roc(aRoc)
{
  Initialize();
}


void DACParameters::Initialize()
{
  for (int i = 0; i < NDACParameters; i++)
  {
    parameters[i] = -1;
    names[i] = "";
  }

  names[1]="Vdig";
  names[2]="Vana";
  names[3]="Vsf";
  names[4]="Vcomp";
  names[5]="Vleak_comp";
  names[6]="VrgPr";
  names[7]="VwllPr";
  names[8]="VrgSh";
  names[9]="VwllSh";
  names[10]="VhldDel";
  names[11]="Vtrim";
  names[12]="VthrComp";
  names[13]="VIBias_Bus";
  names[14]="Vbias_sf";
  names[15]="VoffsetOp";
  names[16]="VIbiasOp";
  names[17]="VOffsetR0";
  names[18]="VIon";
  names[19]="VIbias_PH";
  names[20]="Ibias_DAC";
  names[21]="VIbias_roc";
  names[22]="VIColOr";
  names[23]="Vnpix";
  names[24]="VSumCol";
  names[25]="Vcal";
  names[26]="CalDel";
  names[27]="RangeTemp";
  names[253]="CtrlReg";
  names[254]="WBC";
}


// -- sets all the current DAC parameters
void DACParameters::Restore()
{
  for (int i = 0; i < NDACParameters; i++)
  {
    if (parameters[i] != -1)
    {
      SetParameter(i,parameters[i], false);
    }
  }
}


DACParameters* DACParameters::Copy()
{
  DACParameters* newParameters;
  newParameters = new DACParameters(roc);
  for (int i = 0; i < NDACParameters; i++)
  {
    newParameters->_SetParameter(i,parameters[i]);
  }
  return newParameters;
}


bool DACParameters::Execute(SysCommand command)
{
  for (int iDAC = 0; iDAC < NDACParameters; iDAC++)
  {

    if ( (strcmp(names[iDAC],"") != 0) && (strcmp(command.carg[0],names[iDAC]) == 0))
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
  roc->RocSetDAC(reg,correctValue);
  
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
void DACParameters::SetParameter(const char* dacName, int value)
{
  bool parameterSet = false;
  for (int i = 0; i < NDACParameters; i++)
  {
    if (strcmp(names[i],dacName) == 0)
    {
      SetParameter(i,value);
      parameterSet=true;
      i = NDACParameters;
    }
  }
  if (!parameterSet)
    psi::LogInfo() << "[DACParameters] Error: DAC Parameter " << dacName
                   << " is not found." << psi::endl;
}


// -- returns the DAC value of dacName
int DACParameters::GetDAC(const char* dacName)
{
  for (int i = 0; i < NDACParameters; i++)
  {
    if (strcmp(names[i],dacName) == 0)
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
char* DACParameters::GetName(int reg)
{
  return names[reg];
}


// == file input / output =================================================== 


// -- reads DAC parameters from a file and sets them
bool DACParameters::ReadDACParameterFile( const char *_file)
{
  std::ifstream _input( _file);
  if( !_input.is_open())
  {
    psi::LogInfo() << "[DACParameters] Can not open file " << _file
                   << " to read DAC parameters." << psi::endl;

    return false;
  }

  psi::LogInfo() << "[DACParameters] Reading DAC-Parameters from " << _file
                 << '.' << psi::endl;

  // Read file by lines
  for( std::string _line; _input.good(); )
  {
    getline( _input, _line);

    // Skip Empty Lines and Comments (starting from # or - )
    if( !_line.length()
        || '#' == _line[0]
        || '-' == _line[0] ) continue;

    std::istringstream _istring( _line);
    std::string _tmp;
    int _register;
    int _value;

    _istring >> _register >> _tmp >> _value;

    // Skip line in case any errors occured while reading parameters
    if( _istring.fail() || !_tmp.length() ) continue;

    SetParameter( _register, _value);
  }

  _input.close();

  return true;
}


// -- writes the DAC parameters to a file
bool DACParameters::WriteDACParameterFile(const char *filename)
{
  FILE *file = fopen(filename, "w");
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


// == Private =======================================================



// -- saves the value of an parameter but doesn't set the DAC
void DACParameters::_SetParameter(int reg, int value)
{
  parameters[reg] = value;
}
