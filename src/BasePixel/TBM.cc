#include <iostream>
#include <string.h>
#include <fstream>
#include <unistd.h>

#include "BasePixel/TBM.h"
#include "BasePixel/TBMParameters.h"
#include "interface/Log.h"
#include "BasePixel/TBAnalogInterface.h"

using namespace std;


TBM::TBM(ConfigParameters *configParameters, int aCNId, TBInterface *aTbInterface)
{
  tbInterface=aTbInterface;
  
  if (configParameters->hubId == -1) 
  { 
    configParameters->hubId = ScanHubIDs();
    configParameters->WriteConfigParameterFile();
  }
 
  hubId = configParameters->hubId;
  controlNetworkId = aCNId;
  tbmParameters = new TBMParameters(this);

  TBM1Reg0 = 0; //Base+1/0
  TBM1Reg1 = 0; //Base+3/2
  TBM1Reg2 = 0; //Base+5/4
  TBM1Reg3 = 0; //Base+7
  TBM1Reg4 = 0; //Base+9/8
  TBM1Reg5 = 0; // +B/A Analog Input Amplifier Bias = DAC0
  TBM1Reg6 = 0; // +D/C Analog Amplifier Driver Bias = DAC1
  TBM1Reg7 = 0; // +F/E TBM Analog Output Gain = DAC2
  // TBM2 registers (only first 5, no DACs)
  TBM2Reg0 = 0; //Base+1/0
  TBM2Reg1 = 0; //Base+3/2
  TBM2Reg2 = 0; //Base+5/4
  TBM2Reg3 = 0; //Base+7
  TBM2Reg4 = 0; //Base+9/8
}


TBM::~TBM() {}


void TBM::Initialize( const char *tbmParametersFileName) 
{
  ReadTBMParameterFile(tbmParametersFileName);
  sleep(2);
};


int TBM::GetDAC(int reg)
{
  return tbmParameters->GetDAC(reg);
}


int TBM::ScanHubIDs()
{
  if (!tbInterface->TBMIsPresent()) return -1;
  int value;
  bool result;
  for (int i = 0; i < 32; i++)
  {
     ((TBAnalogInterface*)tbInterface)->ModAddr(i);
     result = GetReg(229, value);
     if (result) 
     {
       psi::LogDebug() << "[TBM] HubID " << i << ": Module found." << psi::endl;

       return i;
     }
  }

  psi::LogInfo() << "[TBM] Error: Can not find HubID." << psi::endl;

  return -1;
}


void TBM::SetDAC(int reg, int value)
{
  return tbmParameters->SetParameter(reg, value);
}


bool TBM::ReadTBMParameterFile( const char *filename)
{
  bool result = tbmParameters->ReadTBMParameterFile( filename);
  tbInterface->Flush();
  return result;
}


bool TBM::WriteTBMParameterFile(const char* filename)
{
  return tbmParameters->WriteTBMParameterFile(filename);
}


void TBM::SetTBMChannel(int tbmChannel)
{
  ((TBAnalogInterface*)tbInterface)->SetTBMChannel(tbmChannel);
}


void TBM::Execute(SysCommand &c)
{
  if(c.narg==0) return;
  int* value;
  int* value1;
  int* value2;
  if(c.Keyword("write",&value1,&value2))
  {
    psi::LogInfo() << "[TBM] Not supported any more." << psi::endl;
/*    if(c.verbose) {cout << "TBM write 0x" << hex << *value1 << " 0x" << *value2<< dec << endl;}
    int base=*value1 & 0xF0;
    if(*value1 & 0x01)
    {
      cerr << "write to read-only address " << hex << *value1 << dec << endl;
      return;
    }
    else
    {
      int reg=*value1>>1;
      int val=*value2&0xFF;
      if(base==0xE0)
      {
        //TBMA
        if(reg<5)
        {
          // register
          setTBM1(reg, val);
        }
        else
        {
          // DAC
          setTBMDAC(reg-5,val);
        }
      }
      else
      {//TBM B
        if(reg<5)
        {
          setTBM1(reg, val);
        }
        else
        {
          // no DACs
          cerr << "invalid TBM address " << hex << c.iarg[1] << dec << endl;
        }
      }
    }*/
  }
  else if(c.Keyword("set",&value1,&value2))
  {
    psi::LogInfo() << "[TBM] Not supported any more." << psi::endl;
/*    if(c.verbose) {cout << "TBM set " << *value1 << " " << *value2<< endl;}
    setTBM1(*value1,*value2);
    setTBM2(*value1,*value2);*/
  }
  else if(c.Keyword("dual")      ){ ((TBAnalogInterface*)tbInterface)->SetTBMChannel(0); tbmParameters->SetParameter("Single", 1); }
  else if(c.Keyword("dual2")      ){ ((TBAnalogInterface*)tbInterface)->SetTBMChannel(1); tbmParameters->SetParameter("Single", 1); }
  else if(c.Keyword("single")    ){ ((TBAnalogInterface*)tbInterface)->SetTBMChannel(0); tbmParameters->SetParameter("Single", 0); }
  else if(c.Keyword("single2")    ){ ((TBAnalogInterface*)tbInterface)->SetTBMChannel(1); tbmParameters->SetParameter("Single", 2); }
  else if(c.Keyword("fullspeed") ){ tbmParameters->SetParameter("Speed", 1); }
  else if(c.Keyword("halfspeed") ){ tbmParameters->SetParameter("Speed", 0); }
  else if(c.Keyword("setA",&value1,&value2)){ setTBM1(*value1,*value2);}
  else if(c.Keyword("setB",&value1,&value2)){ setTBM2(*value1,*value2);}
  else if(c.Keyword("inputbias",&value)    ){  tbmParameters->SetParameter("Inputbias", *value); }
  else if(c.Keyword("outputbias",&value)   ){ tbmParameters->SetParameter("Outputbias", *value); }
  else if(c.Keyword("dacgain",&value)   ){ tbmParameters->SetParameter("Dacgain", *value); }
  else if(c.Keyword("mode","cal")   ){ tbmParameters->SetParameter("Mode", 1); }
  else if(c.Keyword("mode","clear") ){ tbmParameters->SetParameter("Mode", 2); }
  else if(c.Keyword("mode","sync")  ){ tbmParameters->SetParameter("Mode", 0); }
  else if(c.Keyword("ignore", "triggers") ){ tbmParameters->SetParameter("Triggers", 1); }
  else if(c.Keyword("accept", "triggers") ){ tbmParameters->SetParameter("Triggers", 0); }
  else if(c.Keyword("disable","triggers") ){ tbmParameters->SetParameter("Triggers", 2); }
  else if(c.Keyword("enable", "triggers") ){ tbmParameters->SetParameter("Triggers", 0); }
  else
  {
    if (!tbmParameters->Execute(c))
    {
      cerr << "Unknown TBM command:" << c.carg[0] << endl;
    }
  }
  
  tbInterface->Flush();

}


bool TBM::GetReg(int reg, int &value)
{
  return ((TBAnalogInterface*)tbInterface)->GetTBMReg(reg, value);
}


int TBM::init(void)
{
  if (!tbInterface->TBMIsPresent()) return 0;

  int status = 0;
  
  status = setTBM1(2,0xF0); //Clear: trigger count,token-out,stack + resetTBM
  status = setTBM2(2,0xF0); // Same for TBM2

  if (status != 0) cout << "Error in TBM init " << status << endl;
  return status;
}


// Program the TBM DACs 0,1,2
int TBM::setTBMDAC(const int DACAddress, const int value)
{
  int status = 0;
  // Check the register address
  if(DACAddress<0 || DACAddress>2 ) return -1;

  int temp = (value & 0xFF);  // keep only the lower 8 bits
  //cout << " temp " << temp << endl;
  int registerAddress = DACAddress + 5;
  // For all routines RETURN - 0 sucess, -1 - error
  status = tbInterface->Tbm1write(hubId,registerAddress,temp);
  status = tbInterface->Tbm2write(hubId,registerAddress,temp);
  if(status!=0) return -1;  // exit if error

  // Success, update the stored value
  switch(DACAddress)
  {
  case 0 :
    TBM1Reg5 = temp;
    //cout << " reg " << TBM1Reg5 << endl;
    break;
  case 1 :
    TBM1Reg6 = temp;
    break;
  case 2 :
    TBM1Reg7 = temp;
    break;
  }
  return status;
}


int TBM::setTBM1(const int registerAddress, const int value)
{
  int status = 0;
  // Check the register address
  if(registerAddress<0 || registerAddress>4 ) return -1;

  int temp = value & 0xFF;  // keep only the lower 8 bits
  // For all routines RETURN - 0 sucess, -1 - error
  status = tbInterface->Tbm1write(hubId,registerAddress,temp);
  tbInterface->CDelay(100);
  if(status!=0) return -1;  // exit if error

  // Success, update the stored value
  switch(registerAddress)
  {
  case 0 :
    TBM1Reg0 = temp;
    break;
  case 1 :
    TBM1Reg1 = temp;
    break;
  case 2 :
    TBM1Reg2 = temp;
    break;
  case 3 :
    TBM1Reg3 = temp;
    break;
  case 4 :
    TBM1Reg4 = temp;
    break;
  }
  return status;
}


int TBM::setTBM2(const int registerAddress, const int value)
{
  int status = 0;
  // Check the register address
  if(registerAddress<0 || registerAddress>4 ) return -1;

  int temp = value & 0xFF;  // keep only the lower 8 bits
  // For all routines RETURN - 0 sucess, -1 - error
  status = tbInterface->Tbm2write(hubId,registerAddress,temp);
  tbInterface->CDelay(100);
  if(status!=0) return -1;  // exit if error

  // Success, update the stored value
  switch(registerAddress)
  {
  case 0 :
    TBM2Reg0 = temp;
    break;
  case 1 :
    TBM2Reg1 = temp;
    break;
  case 2 :
    TBM2Reg2 = temp;
    break;
  case 3 :
    TBM2Reg3 = temp;
    break;
  case 4 :
    TBM2Reg4 = temp;
    break;
  }
  return status;
}

// Switch ON selected CLEAR & INJECT bits
int TBM::setBit(const int tbm, const int bit)
{
  int status = 0;
  switch(tbm)
  {
  case 1:  // TBM1
    if(bit>=0 && bit<=7)
    {
      int tmp = 0x01 << bit;
      status = setTBM1Reg2(tmp,0xFF);
      break;
    }
    else
    {
      cout << " Wrong bit selected " << bit << endl;
      return -1;
    }
    break;
  case 2:  // TBM 2
    if(bit>=0 && bit<=7)
    {
      int tmp = 0x01 << bit;
      status = setTBM2Reg2(tmp,0xFF);
      break;
    }
    else
    {
      cout << " Wrong bit selected " << bit << endl;
      return -1;
    }
    break;
  default:  // wrong TBM
    cout << " wrong TBM  selected, id = " << tbm << endl;
    return -1;
  }

  return status;
}





