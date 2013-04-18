#include "interface/Log.h"
#include "BasePixel/Module.h"
#include "BasePixel/TBAnalogInterface.h"

Module::Module(ConfigParameters *aConfigParameters, int aCNId, TBInterface *aTBInterface)
{
  configParameters = aConfigParameters;

  if (configParameters->customModule == 0)
  {
    nRocs = configParameters->nRocs;
    controlNetworkId = aCNId;
    tbInterface = aTBInterface;

    tbm = new TBM(configParameters, aCNId, tbInterface);
    hubId = configParameters->hubId;

    int offset = 0;
    if (configParameters->halfModule == 2) offset = 8;
    for (int i = 0; i < nRocs; i++)
    {
      roc[i] = new Roc(tbInterface, i+offset, hubId, int((i+offset)/4), i);
      roc[i]->set_chip_type(aConfigParameters->roc_type);
    }
  }
  else if (configParameters->customModule == 1)
  {
    psi::LogInfo() << "[Module] Custom module constructor: Ignore nRocs, "
                   << "hubID, ... in config file." << psi::endl;
    nRocs = 4;
    hubId = 31;
    controlNetworkId = aCNId;
    tbInterface = aTBInterface;

    tbm = new TBM(configParameters, aCNId, tbInterface);
    nRocs = 4;
    roc[0] = new Roc(tbInterface, 0, hubId, 0, 0);
    roc[1] = new Roc(tbInterface, 1, hubId, 0, 1);
    roc[2] = new Roc(tbInterface, 8, hubId, 2, 2);
    roc[3] = new Roc(tbInterface, 9, hubId, 2, 3);
  }
}


Module::~Module()
{
  for (int i = 0; i < nRocs; i++)
  {
    delete roc[i];
  }
}


void Module::Initialize()
{
  tbm->Initialize(configParameters->GetTbmParametersFileName());

  for (int i = 0; i < nRocs; i++)
  {
    roc[i]->Initialize(configParameters);
  }
}


int Module::GetHubId()
{
  return hubId;
}


Roc* Module::GetRoc(int iRoc)
{
  return roc[iRoc];
}


int Module::NRocs()
{
  return nRocs;
}


TBM* Module::GetTBM()
{
  return tbm;
}

int Module::GetTBM(int reg)
{
  return tbm->GetDAC(reg);
}


void Module::SetTBMSingle(int tbmChannel)
{
  if (tbmChannel == 0) tbm->SetDAC(0, 0);
  else tbm->SetDAC(0, 2);
}


void Module::Execute(SysCommand &command)
{
  if (command.TargetIsTBM()) {tbm->Execute(command);}
  else
  { 
    bool done = false;
    for (int i = 0; i < nRocs; i++)
    {
      if (roc[i]->GetChipId() == command.roc)
      {
        roc[i]->Execute(command);
        done = true;
      }
    }
    if (!done) 
      psi::LogInfo() << "[Module] ROC " << command.roc << " doesn't exist."
                     << psi::endl;
  }
}


void Module::SetDAC(int chipId, int reg, int value)
{
  roc[chipId]->SetDAC(reg,value);
}

void Module::SetTBM(int chipId, int reg, int value)
{
  tbm->SetDAC(reg,value);
}


void Module::SetTrim(int iRoc, int iCol, int iRow, int trimBit)
{
  roc[iRoc]->SetTrim(iCol, iRow, trimBit);
}


// -- Writes the trim configuration of all rocs to disk
void Module::WriteTrimConfiguration(char* filename)
{
  for (int i = 0; i < nRocs; i++) roc[i]->WriteTrimConfiguration(filename) ;
}


// -- Write the current DAC settings of all rocs to disk
void Module::WriteDACParameterFile( const char* filename)
{
  for (int i = 0; i < nRocs; i++) roc[i]->WriteDACParameterFile(filename);
}


// -- Read the last DAC settings of all rocs to disk
void Module::ReadDACParameterFile(char* filename)
{
  for (int i = 0; i < nRocs; i++) roc[i]->ReadDACParameterFile(filename);
}


// -- Read the trim configuration for all rocs from disk
void Module::ReadTrimConfiguration(char* filename)
{
  for (int i = 0; i < nRocs; i++) roc[i]->ReadTrimConfiguration(filename) ;
}


int Module::GetTemperature()
{
  TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;
  const int nTriggers = 10;
  
  int temp[nRocs][8], calib[nRocs][8];

  for (int iRoc = 0; iRoc < nRocs; iRoc++) 
  {
    psi::LogDebug() << "[Module] -----------------------------------------------------------"
                    << psi::endl;
    psi::LogDebug() << "[Module] ROC " << iRoc << psi::endl;
    
/*    // Get range of last DAC
  
    GetRoc(iRoc)->SetDAC("Vcal", 0);
    anaInterface->Flush();
    min[iRoc] = anaInterface->LastDAC(nTriggers, iRoc);
    Log::Current()->printf("min %i\n", min[iRoc]);
    GetRoc(iRoc)->SetDAC("Vcal", 255);
    anaInterface->Flush();
    max[iRoc] = anaInterface->LastDAC(nTriggers, iRoc);
    Log::Current()->printf("max %i\n", max[iRoc]);*/
    
    // Calibrate
    
    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++)
    {
      GetRoc(iRoc)->SetDAC("RangeTemp", rangeTemp+8);
      anaInterface->Flush();
      calib[iRoc][rangeTemp] = anaInterface->LastDAC(nTriggers, iRoc);
      psi::LogDebug() << "[Module] Calib: " << calib[iRoc][rangeTemp] << psi::endl;
    }
    
    // Get temperature
  
    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++)
    {
      GetRoc(iRoc)->SetDAC("RangeTemp", rangeTemp);
      anaInterface->Flush();
      temp[iRoc][rangeTemp] = anaInterface->LastDAC(nTriggers, iRoc);
      psi::LogDebug() << "[Module] Temp: " << temp[iRoc][rangeTemp] << psi::endl;
    }
  }
  return temp[0][0];
}


void Module::AdjustDTL()
{
  TBAnalogInterface *anaInterface = (TBAnalogInterface*)tbInterface;
  int dtl = 0, emptyReadoutLength = anaInterface->GetEmptyReadoutLengthADC();
  short data[10000];
  unsigned short count;
  
  do
  {
    dtl-=50;
    anaInterface->DataTriggerLevel(dtl);
    anaInterface->ADCData(data, count);
  }
  while ((count != emptyReadoutLength) && (dtl > -2000));
  
  if (dtl == -2000) 
  {
    // try with second tbm
    TBM *tbm = GetTBM();
    int channel = anaInterface->GetTBMChannel();
    dtl = 0;
    
    psi::LogInfo() << "[Module] Problem: Can not find data trigger level. " 
                   << "Try different channel." << psi::endl;
    
    SetTBMSingle((channel + 1) % 2);
    anaInterface->SetTBMChannel((channel + 1) % 2);
    
    do
    {
      dtl-=50;
      anaInterface->DataTriggerLevel(dtl);
      anaInterface->ADCData(data, count);
    }
    while ((count != emptyReadoutLength) && (dtl > -2000));
    
    if (dtl != -2000) tbm->WriteTBMParameterFile(configParameters->GetTbmParametersFileName());
  }
  
  if (dtl == -2000) 
  {
          // still no valid readout
    psi::LogInfo() << "[Module] Problem: Can not find data trigger level." 
                   << psi::endl;
    return;
  }
  
  dtl = (data[0] + data[1] + data[2]) / 3 + 100;

  anaInterface->DataTriggerLevel(dtl);
  if (dtl < -1200) 
    psi::LogInfo() << "[Module] Warning: Very low data trigger level: "
                   << dtl << ". Check AOUT channels." << psi::endl;

  configParameters->dataTriggerLevel = dtl;
  psi::LogInfo() << "[Module] Setting data trigger level to "
                 << dtl << psi::endl;
  configParameters->WriteConfigParameterFile();
}
