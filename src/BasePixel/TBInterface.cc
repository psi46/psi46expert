#include <stdio.h>
#include <string.h>
#include <iomanip>

#include "BasePixel/TBInterface.h"
#include "interface/Log.h"


TBInterface::TBInterface(ConfigParameters * configParameters)
{
    ChipId = 0;
    TBMpresent = 0;
    HUBaddress = 0;

    Initialize(configParameters);
}


TBInterface::~TBInterface() {
  delete cTestboard;
}

void TBInterface::Execute(SysCommand &command)
{
    int buf[2];
    int * value = &buf[0]; int * reg = &buf[1];
    int delay;
    if (command.Keyword("pon"))    {Pon();}
    else if (command.Keyword("poff"))   {Poff();}
    else if (command.Keyword("hvoff"))   {HVoff();}
    else if (command.Keyword("hvon"))   {HVon();}
    else if( command.Keyword( "usb" ) ) ShowUSB();
    else if( command.Keyword( "clear" ) ) ClearUSB();
    else if (command.Keyword("loop"))   {Intern(rctk_flag);}
    else if (command.Keyword("stop"))   {Single(0);}
    else if (command.Keyword("single")) {Single(rctk_flag);}
    else if (command.Keyword("setreg", &reg, &value)) {SetReg(*reg, *value);}
    else if (command.Keyword("ext"))    {Extern(rctk_flag);}
    else if (command.Keyword("GetRoCntEx")) {psi::LogInfo << cTestboard->GetRoCntEx() << psi::endl;}
    else if (command.Keyword("SetEmptyReadoutLength", &value)) {cTestboard->SetEmptyReadoutLength(*value);}
    else if (command.Keyword("TBMDisable")) {Tbmenable(false);}
    else if (command.Keyword("TBMChannel", &value)) {SetTBMChannel(*value);}
    else if (command.Keyword("TBMEnable")) {Tbmenable(true);}
    else if (command.Keyword("CountReadouts")) { psi::LogInfo << CountReadouts(10, 0) << " / 10" << psi::endl;}
    else if (command.Keyword("CountReadouts", &value)) {psi::LogInfo << CountReadouts(*value, 0) << " / " << *value << psi::endl;}
    else if( command.Keyword( "CountADCReadouts", &value ) ) psi::LogInfo << CountADCReadouts(*value ) << " / " << *value << psi::endl;

    else if (command.Keyword("dclear")) {DataCtrl(true,  false, false);} /* clear FIFO */
    else if (command.Keyword("dtrig"))  {DataCtrl(false, true,  false);} /* enable ADC gate once */
    else if (command.Keyword("dstart")) {DataCtrl(false, false, true);}  /* enable ADC gate continuously */
    else if (command.Keyword("dstop"))  {DataCtrl(false, false, false);} /* disable ADC gate */
    else if (command.Keyword("dena"))   {DataEnable(true);}  /* enable FIFO */
    else if (command.Keyword("ddis"))   {DataEnable(false);} /* disable FIFO */
    else if (command.Keyword("probe", &reg, &value)) {
        unsigned char port = *reg;
        unsigned char signal = *value;
        ProbeSelect(port, signal);
    }
    else if (command.Keyword("gate")) {ProbeSelect(0, PROBE_ADC_GATE);}
    else if (command.Keyword("version")) {
        char s[260];
        if (GetVersion(s, 260))
            psi::LogInfo << s << psi::endl;
        else
            psi::LogInfo << "Unable to aquire firmware version." << psi::endl;
    }
    else if (command.Keyword("scurve")) {
    int nTrig = 99;
        int dacReg = 25;
    int threshold = 20;
        int res[10000] = {0};
        int n = SCurve(nTrig, dacReg, threshold, res);
        psi::LogInfo << "SCurve:";
        for (int i = 0; i < n; i++)
            psi::LogInfo << " " << res[i];
        psi::LogInfo << psi::endl;
    }
    else if (command.Keyword("ia"))    {
        psi::LogInfo() << "[TBDigitalInterface] Analog current " << GetIA() << psi::endl;
    }
    else if (command.Keyword("id"))    {
        psi::LogInfo() << "[TBDigitalInterface] Digital current " << GetID() << psi::endl;
    }
    else if (command.Keyword("getvd"))    {
        psi::LogInfo() << "[TBDigitalInterface] Digital voltage " << GetVD() << psi::endl;
    }
    else if (command.Keyword("getva"))    {
        psi::LogInfo() << "[TBDigitalInterface] Analog voltage " << GetVA() << psi::endl;
    }
    else if (command.Keyword("res"))    {Single(0x08);}  //reset
    else if (command.Keyword("reseton"))    {ResetOn();}
    else if (command.Keyword("resetoff"))    {ResetOff();}
    else if (command.Keyword("dtlScan")) {DataTriggerLevelScan();}

    else if (strcmp(command.carg[0], "dv") == 0) {SetVD((double)*command.iarg[1] / 1000.);}
    else if (strcmp(command.carg[0], "av") == 0) {SetVA((double)*command.iarg[1] / 1000.);}
    else if (strcmp(command.carg[0], "dtl") == 0) {DataTriggerLevel(-*command.iarg[1]);}
    else if (strcmp(command.carg[0], "enableAll") == 0) SetEnableAll(*command.iarg[1]);

    else if (command.Keyword("t_res_cal", &value))
    {delay = RangeCheck(*value, 5, 255); SetTBParameter("trc", delay);}
    else if (command.Keyword("trc", &value))
    {delay = RangeCheck(*value, 5, 255); SetTBParameter("trc", delay);}
    else if (command.Keyword("t_cal_cal", &value))
    {delay = RangeCheck(*value, 5, 255); SetTBParameter("tcc", delay);}
    else if (command.Keyword("tcc", &value))
    {delay = RangeCheck(*value, 5, 255); SetTBParameter("tcc", delay);}
    else if (command.Keyword("t_cal_trig", &value))
    {delay = RangeCheck(*value, 5, 255); SetTBParameter("tct", delay);}
    else if (command.Keyword("tct", &value))
    {delay = RangeCheck(*value, 5, 255); SetTBParameter("tct", delay);}
    else if (command.Keyword("t_trg_tok", &value))
    {delay = RangeCheck(*value, 5, 255); SetTBParameter("ttk", delay);}
    else if (command.Keyword("ttk", &value))
    {delay = RangeCheck(*value, 5, 255); SetTBParameter("ttk", delay);}
    else if (command.Keyword("t_rep", &value))
    {delay = RangeCheck(*value, 1, 120); SetTBParameter("trep", delay);}
    else if (command.Keyword("trep", &value))
    {delay = RangeCheck(*value, 1, 120); SetTBParameter("trep", delay);}
    else if (command.Keyword("calrep", &value))
    {delay = RangeCheck(*value, 1, 120); SetTBParameter("cc", delay);}
    else if (command.Keyword("cc", &value))
    {delay = RangeCheck(*value, 1, 120); SetTBParameter("cc", delay);}
    else if (command.Keyword("seq", &value))
    {rctk_flag = RangeCheck(*value, 0, 15);}
    else if (command.Keyword("ctr", &value))
    {delay = RangeCheck(*value, 1, 120); SetTBParameter("ctr", delay);}
    else
    {
        if (!tbParameters->Execute(command))
            psi::LogInfo() << "[TBDigitalInterface] Unknown testboard command: "
                           << command.carg[0] << psi::endl;
    }

}


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

// == General functions ================================================

void TBDigitalInterface::Close()
{
    cTestboard->Close();
}

void TBInterface::Flush()
{
    cTestboard->Flush();
}


void  TBInterface::Pon()
{
    cTestboard->Pon();
    cTestboard->Flush();
}


void TBInterface::Poff()
{
    cTestboard->Poff();
    cTestboard->Flush();
}


void TBInterface::HVon()
{
    cTestboard->HVon();
    cTestboard->Flush();
}


void TBInterface::HVoff()
{
    cTestboard->HVoff();
    cTestboard->Flush();
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
