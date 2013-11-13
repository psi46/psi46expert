#include <stdio.h>
#include <string.h>
#include <iomanip>

#include "BasePixel/TBInterface.h"
#include "BasePixel/settings.h"
#include "BasePixel/TBParameters.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include "interface/Log.h"
#include "interface/USBInterface.h"
#include "interface/Delay.h"


TBInterface::TBInterface()
{
    ChipId = 0;
    TBMpresent = 0;
    HUBaddress = 0;
}

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
        psi::LogInfo() << "[TBInterface] Analog current " << GetIA() << psi::endl;
    }
    else if (command.Keyword("id"))    {
        psi::LogInfo() << "[TBInterface] Digital current " << GetID() << psi::endl;
    }
    else if (command.Keyword("getvd"))    {
        psi::LogInfo() << "[TBInterface] Digital voltage " << GetVD() << psi::endl;
    }
    else if (command.Keyword("getva"))    {
        psi::LogInfo() << "[TBInterface] Analog voltage " << GetVA() << psi::endl;
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
            psi::LogInfo() << "[TBInterface] Unknown testboard command: "
                           << command.carg[0] << psi::endl;
    }

}


int TBInterface::GetRoCnt()
{
    return cTestboard->GetRoCntEx();
}

//----------------------------------------------------------------------

//------------------------------------------------------------------------------
void TBInterface::Initialize(ConfigParameters * configParameters)
{
    string usbId;
    CSettings settings;
 
    tbParameters = (TBParameters *)new TBParameters(this);
   
    cTestboard = new CTestboard();
    usbId = configParameters->testboardName;
    if (usbId == "*") cTestboard->FindDTB(usbId);
    if (cTestboard->Open(usbId)) {
      printf("\nDTB %s opened\n", usbId.c_str());
      string info;
      try {
	cTestboard->GetInfo(info);
	printf("--- DTB info-------------------------------------\n"
	       "%s"
	       "-------------------------------------------------\n",
	       info.c_str());
	cTestboard->Welcome();
	cTestboard->Flush();
      }
      catch(CRpcError &e) {
	e.What();
	printf("ERROR: DTB software version could not be identified, please update it!\n");
	cTestboard->Close();
	printf("Connection to Board %s has been cancelled\n", usbId.c_str());
      }
    }
    else {
      printf("USB error: %s\n", cTestboard->ConnectionError());
      printf("DTB: could not open port to device %s\n", settings.port_tb);
      printf("Make sure you have permission to access USB devices.\n");
    }
    cTestboard->Init();

    fIsPresent = 1;

    Pon();
    I2cAddr(0);
    rctk_flag = 15;

    SetTBMChannel(configParameters->tbmChannel);
    Tbmenable(configParameters->tbmEnable);

    SetIA(configParameters->ia);
    SetID(configParameters->id);
    SetVA(configParameters->va);
    SetVD(configParameters->vd);

    SetEmptyReadoutLength(configParameters->emptyReadoutLength);
    SetEmptyReadoutLengthADC(configParameters->emptyReadoutLengthADC);
    SetEmptyReadoutLengthADCDual(configParameters->emptyReadoutLengthADCDual);

    if (configParameters->hvOn) HVon();
    DataTriggerLevel(configParameters->dataTriggerLevel);

    cTestboard->SetHubID(configParameters->hubId);
    cTestboard->SetNRocs(configParameters->nRocs);
    cTestboard->SetEnableAll(0);

    DataEnable(true);
    cTestboard->ResetOn(); // send hard reset to connected modules / TBMs
    cTestboard->Flush();
    gDelay->Mdelay(100);
    cTestboard->ResetOff();
    cTestboard->Flush();
    cTestboard->Init_Reset();

    ReadTBParameterFile(configParameters->GetTbParametersFileName());    //only after power on

}


void TBInterface::Clear()
{
    cTestboard->Clear();
}


int TBInterface::Startup(int port)
{
    return 1;
}





int TBInterface::Present()
{
    return 1;
}


void TBInterface::I2cAddr(int id)
{
    cTestboard->I2cAddr(id);
}


void TBInterface::SetTriggerMode(unsigned short mode)
{
    cTestboard->SetTriggerMode(mode);
}


void TBInterface::SetEmptyReadoutLength(int length)
{
    emptyReadoutLength = length;
    cTestboard->SetEmptyReadoutLength(length);
}


int TBInterface::GetEmptyReadoutLength()
{
    return emptyReadoutLength;
}


void TBInterface::SetEmptyReadoutLengthADC(int length)
{
    emptyReadoutLengthADC = length;
    cTestboard->SetEmptyReadoutLengthADC(length);
}


void TBInterface::SetEmptyReadoutLengthADCDual(int length)
{
    emptyReadoutLengthADCDual = length;
}


int TBInterface::GetEmptyReadoutLengthADC()
{
    return emptyReadoutLengthADC;
}


int TBInterface::GetEmptyReadoutLengthADCDual()
{
    return emptyReadoutLengthADCDual;
}


void TBInterface::SetEnableAll(int value)
{
    cTestboard->SetEnableAll(value);
}


unsigned short TBInterface::GetModRoCnt(unsigned short index)
{
    return cTestboard->GetModRoCnt(index);
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

bool TBInterface::GetVersion(char * s, unsigned int n)
{
    return cTestboard->GetVersion(s, n);
}

void TBInterface::ShowUSB() // USB parameters
{
  cTestboard->ShowUSB(); // in psi46_tb.h
}

void TBInterface::ClearUSB() // reset USB buffer
{
  cTestboard->Clear(); // in psi46_tb.h
}

void TBInterface::Close()
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

void TBInterface::Set(int reg, int value)
{
    cTestboard->Set(reg, value);
}


void TBInterface::SetReg(int reg, int value)
{
    cTestboard->SetReg(reg, value);
}


void TBInterface::Single(int mask)
{
    cTestboard->Single(mask);
}

void TBInterface::Extern(int mask)
{
    cTestboard->Extern(mask);
}

void TBInterface::Intern(int mask)
{
    cTestboard->Intern(mask);
}



// == TBM functions ======================================================

void TBInterface::Tbmenable(int on)
{
    TBMpresent = on;
    cTestboard->tbm_Enable(on);
    SetReg41();
}


void TBInterface::ModAddr(int hub)
{
    cTestboard->mod_Addr(hub);
}


void TBInterface::TbmAddr(int hub, int port)
{
    cTestboard->tbm_Addr(hub, port);
}


int TBInterface::TbmWrite(const int hubAddr, const int addr, const int value)
{
    if (!cTestboard->TBMPresent()) return -1;
    cTestboard->TbmWrite(hubAddr, addr, value);
    return 0;
}


int TBInterface::Tbm1write(const int hubAddr, const int registerAddress, const int value)
{
    if (!cTestboard->TBMPresent()) return -1;
    cTestboard->Tbm1Write(hubAddr, registerAddress, value);
    return 0;
}


int TBInterface::Tbm2write(const int hubAddr, const int registerAddress, const int value)
{
    if (!cTestboard->TBMPresent()) return -1;
    cTestboard->Tbm2Write(hubAddr, registerAddress, value);
    return 0;
}


bool TBInterface::GetTBMReg(int reg, int &value)
{
    unsigned char v, r = (unsigned char)reg;
    bool result = cTestboard->tbm_Get(r, v);
    value = (int)v;
    return result;
}


// == ROC functions ======================================================


void TBInterface::SetChip(int chipId, int hubId, int portId, int aoutChipPosition)
{
    cTestboard->tbm_Addr(hubId, portId);
    cTestboard->roc_I2cAddr(chipId);
    cTestboard->SetAoutChipPosition(aoutChipPosition);
}

void TBInterface::RocClrCal()
{
    cTestboard->roc_ClrCal();
}


void TBInterface::RocSetDAC(int reg, int value)
{
    cTestboard->roc_SetDAC(reg, value);
}


void TBInterface::RocPixTrim(int col, int row, int value)
{
    cTestboard->roc_Pix_Trim(col, row, value);
}


void TBInterface::RocPixMask(int col, int row)
{
    cTestboard->roc_Pix_Mask(col, row);
}


void TBInterface::RocPixCal(int col, int row, int sensorcal)
{
    cTestboard->roc_Pix_Cal(col, row, sensorcal);
}


void TBInterface::RocColEnable(int col, int on)
{
    cTestboard->roc_Col_Enable(col, on);
}



void TBInterface::SetClock(int n)
{
    cTestboard->SetClock(n);
    Flush();
}


// == Analog functions =================================================


void TBInterface::DataTriggerLevel(int level)
{
    cTestboard->DataTriggerLevel(TBMChannel, level);
    cTestboard->SetDTL(level);
}


void TBInterface::DataCtrl(bool clear, bool trigger, bool cont)
{
    cTestboard->DataCtrl(TBMChannel, clear, trigger, cont);
}


void TBInterface::DataEnable(bool on)
{
    cTestboard->DataEnable(on);
}



bool TBInterface::DataRead(short buffer[], unsigned short buffersize, unsigned short &wordsread)
{
    return cTestboard->DataRead(TBMChannel, buffer, buffersize, wordsread);
}


void TBInterface::SetDelay(int signal, int ns)
{
    cTestboard->Sig_SetDelay(signal, ns);
    Flush();
}

void TBInterface::SetClockStretch(unsigned char src, unsigned short delay, unsigned short width)
{
    cTestboard->SetClockStretch(src, delay, width);
    Flush();
}


void TBInterface::CDelay(unsigned int clocks)
{
    cTestboard->cDelay(clocks);
}


bool TBInterface::SendRoCnt()
{
    //works only for trigger mode MODULE1
    //  if (signalCounter % 100 == 0) Log::Current()->printf("counter %i\n", signalCounter);
    //  Log::Current()->printf("counter %i\n", signalCounter);
    if (signalCounter == 30000) ReadBackData();
    signalCounter++;
    return cTestboard->SendRoCntEx();
}


int TBInterface::RecvRoCnt()
{
    //works only for trigger mode MODULE1
    //  Log::Current()->printf("counter %i\n", signalCounter);
    //  if (signalCounter % 100 == 0) Log::Current()->printf("counter %i\n", signalCounter);
    if (signalCounter == 0  && readPosition == writePosition)  //buffer empty and nothing to read
    {
        psi::LogInfo() << "[TBInterface] Error: no signal to read from testboard."
                       << psi::endl;
        return -1;
    }
    else if (readPosition == writePosition) {   //buffer is empty
        signalCounter--;
        return cTestboard->RecvRoCntEx();
    }
    else {
        int data = dataBuffer[readPosition];   //buffer not empty
        readPosition++;
        if (readPosition == bufferSize) readPosition = 0;
        return data;
    }
}


//------------------------------------------------------------------------------
void TBInterface::SingleCal()
{
    Single(RES | CAL | TRG | TOK);
    CDelay(500); //CDelay(100) is too short
}


int TBInterface::CountReadouts(int count, int chipId)
{
    return cTestboard->CountReadouts(count, chipId);
}


void TBInterface::SendCal(int nTrig)
{
    for (int i = 0; i < nTrig; i++)
    {
        SingleCal();
        SendRoCnt();
    }
}


int TBInterface::CountADCReadouts(int count)
{
    unsigned short counter;
    short data[FIFOSIZE];

    int n = 0;
    for (int i = 0; i < count; i++)
    {
        DataCtrl(false, true); // no clear, trigger
        Single(RES | CAL | TRG | TOK);
        CDelay(100);
        Flush();
        DataRead(data, FIFOSIZE, counter);
        // n += ((int)counter - 56) / 6; // Module
		n += ( (int)counter - 19 ) / 6; // Single ROC, with TBM emu
    }
    return n;
}


bool TBInterface::ADCData(short buffer[], unsigned short &wordsread)
{
    ADCRead(buffer, wordsread);
    return true;
}

unsigned short TBInterface::ADC()
{
    /* sends a single trigger and displays the raw readout on the console */
    unsigned short count;
    short data[FIFOSIZE];
    bool is_analog = IsAnalog();
    if (is_analog)
        ADCRead(data, count);
    else
        ADCRead_digital(data, count);

    psi::LogInfo << "[TBInterface] Count: " << count << (is_analog ? "" : " bits") << psi::endl;

    psi::LogInfo << "[TBInterface] Data: ";
    for (unsigned int n = 0; n < count; n++) {
        if (is_analog)
	 	{
    		psi::LogInfo << " " << setw(4) << data[n];
	    	if( n ==  7 ) psi::LogInfo << " :"; // after TBM header
			if( n == 10 ) psi::LogInfo << " :"; // after UB, B, lastDAC
    		if( n > 15 && n < count - 7 && (n-11)%6 == 5 ) psi::LogInfo << " :"; // after each pixel
		} else
            psi::LogInfo << ((n % 8 == 0) ? "|" : "") << ((data[n / 16] & (1 << (16 - n % 16 - 1))) ? 1 : 0);
    }

    psi::LogInfo << psi::endl;
    return count;
}

unsigned short TBInterface::ADC(int nbsize)
{

    unsigned short count;
    short data[FIFOSIZE];
    ADCRead(data, count);
    // probe with the scope the Gate and the comp output signal
    //cTestboard->ProbeSelect(0,PROBE_ADC_COMP);
    //cTestboard->ProbeSelect(1,PROBE_ADC_GATE);
    //  cout<<"&&&&&&& TBInterface::ADCData "<<endl;
    //  cout<<"start testing the reset"<<endl;
    //  cTestboard->ResetOn();
    //  cTestboard->mDelay(1000);
    //  cTestboard->ResetOff();
    //  cout<<"reset On and off done!"<<endl;
    //  cTestboard->Flush();

    if (nbsize > 0)
    {
        // run adc with fix trigger mode
        if (IsAnalog())
            cTestboard->SetReg(41, 32);
        else
            cTestboard->SetReg(41, 33);
        cTestboard->SetTriggerMode(TRIGGER_FIXED);
        cTestboard->DataBlockSize(200);
        cTestboard->DataCtrl(0, false, true, false);
        cTestboard->Single(RES | CAL | TRG);
        cTestboard->mDelay(100);
        cTestboard->DataRead(0, data, FIFOSIZE, count);
        cTestboard->mDelay(100);
        cTestboard->Intern(RES | CAL | TRG);
        cTestboard->Flush();
        //  cTestboard->Welcome();
    }

    psi::LogDebug() << "[TBInterface] Count " << count << psi::endl;
    cout << "[TBInterface] Count " << count << endl;

    //  for (unsigned int n = 0; n < count; n++) data[n] &= 0xf000;

    psi::LogDebug << "[TBInterface] Data: ";
    cout << "[TBInterface] Data: ";
    for (unsigned int n = 0; n < count; n++)
    {
        psi::LogDebug << " " << data[n];
        cout << " " << data[n];
    }
    psi::LogDebug << psi::endl;
    cout << endl;


    if (tbmenable)SetTriggerMode(TRIGGER_MODULE2);
    else SetTriggerMode(TRIGGER_ROC);

    return count;
}




// -- sends n calibrate signals and gives back the resulting ADC readout
void TBInterface::SendADCTrigs(int nTrig)
{
    for (int i = 0; i < nTrig; i++)
    {
        DataCtrl(false, true); // no clear, trigger
        Single(RES | CAL | TRG | TOK);
        CDelay(500);
    }
}


int TBInterface::LastDAC(int nTriggers, int chipId)
{
    int numRepetitions = 0;

    unsigned short count = 0;
    short data[FIFOSIZE];
    while (count == 0 && numRepetitions < 100) {
        ADCRead(data, count, nTriggers);

        //cout << "ADC = { ";
        //for ( int i = 0; i < count; i++ ){
        //  cout << data[i] << " ";
        //}
        //cout << "} " << endl;

        numRepetitions++;
    }

    if (numRepetitions >= 100) {
        cerr << "Error in <TBInterface::LastDAC>: cannot find ADC signal !" << endl;
        return 0;
    }

    return data[10 + chipId * 3];
}


void TBInterface::SendADCTrigsNoReset(int nTrig)
{
    for (int i = 0; i < nTrig; i++)
    {
        DataCtrl(false, true); // no clear, trigger
        Single(CAL | TRG);
        CDelay(500);
    }
}


bool TBInterface::GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts)
{
    RawPacketDecoder * gDecoder = RawPacketDecoder::Singleton();
    nReadouts = 0;

    while (!DataRead(buffer, buffersize, wordsread))
    {
        Clear();
        cout << "usb cleared" << endl;
        return false;
    }


    if (wordsread > 0) {
        for (int pos = 0; pos < (wordsread - 2); pos++)
        {
            if (gDecoder->isUltraBlackTBM(buffer[pos]) && gDecoder->isUltraBlackTBM(buffer[pos + 1]) && gDecoder->isUltraBlackTBM(buffer[pos + 2]))
            {
                if (nReadouts < nTrig) startBuffer[nReadouts] = pos;
                nReadouts++;
            }
        }
    }

    return (nReadouts <= nTrig);
}



bool TBInterface::DataTriggerLevelScan()
{
    unsigned short count;
    bool result = false;
    for (int delay = 0; delay < 2000; delay = delay + 50)
    {
        psi::LogDebug() << "[TBInterface] dtl: " << delay
                        << " -------------------------------------" << psi::endl;

        DataTriggerLevel(-delay);
        Flush();
        count = ADC();
        if (count == emptyReadoutLengthADC) result = true;
    }
    return result;
}




void TBInterface::SetVA(double V)
{
    cTestboard->SetVA(V);
}


void TBInterface::SetIA(double A)
{
    cTestboard->SetIA(A);
}

void TBInterface::SetVD(double V)
{
    cTestboard->SetVD(V);
}


void TBInterface::SetID(double A)
{
    return cTestboard->SetID(A);
}


double TBInterface::GetVA()
{
    return cTestboard->GetVA();
}


double TBInterface::GetIA()
{
    return cTestboard->GetIA();
}


double TBInterface::GetVD()
{
    return cTestboard->GetVD();
}


double TBInterface::GetID()
{
    return cTestboard->GetID();
}




void TBInterface::ResetOn()
{
    cTestboard->ResetOn();
}


void TBInterface::ResetOff()
{
    cTestboard->ResetOff();
}


void TBInterface::SetTBMChannel(int channel)
{
    TBMChannel = channel;
    cTestboard->SetTbmChannel(channel);
    SetReg41();
}


int TBInterface::GetTBMChannel()
{
    return TBMChannel;
}

bool TBInterface::IsAnalog()
{
    return (TBMChannel == 0);
}


// ----------------------------------------------------------------------
bool TBInterface::Mem_ReadOut(FILE * f, unsigned int addr, unsigned int size) {

	// Can be tuned to be faster. Was: unsigned short BLOCKSIZE = 32767;
    unsigned short BLOCKSIZE = 50000;
    unsigned char buffer[BLOCKSIZE];
    for (int i = 0; i < BLOCKSIZE; i++) buffer[i] = 0;

    Flush();
    Clear();
    unsigned int bound = static_cast<unsigned int>(2. * size / BLOCKSIZE);
    unsigned int start = addr;

	cout << "r/o of " << 2.*size
	     << " bytes with blocksize " << BLOCKSIZE
    	 << " starting from memory address " << addr
	     << endl;
    for (unsigned int j = 0; j < bound; ++j) {
        cTestboard->MemRead(start, BLOCKSIZE, buffer);
        start += BLOCKSIZE;
        fwrite(buffer, BLOCKSIZE, 1, f);
    	cout << "read " << (j+1)*BLOCKSIZE << " of " << 2*size << endl;
    }

    unsigned short rest = (addr + 2 * size - start);
    cTestboard->MemRead(start, rest, buffer);
    fwrite(buffer, rest, 1, f);
    Clear();
}



void TBInterface::SetReg41()
{
    int value(0);
    value = TBMChannel;
    if (TBMpresent) value += 2;
    if (triggerSource) value += 16;
    else value += 32;

    SetReg(41, value);
}


void TBInterface::StartDataTaking()
{
    DataCtrl(false, false, true); // go
    SetReg(43, 2);
    SetReg(41, 0x2A);
    Flush();
}


void TBInterface::StopDataTaking()
{
    SetReg(41, 0x22);
    DataCtrl(false, false, false); // stop
    Flush();
}


// == buffer functions ===========================================================================

void TBInterface::ReadBackData()
{
    Flush();
    //  Log::Current()->printf("reading back data\n");
    for (int i = 0; i < signalCounter; i++)
    {
        dataBuffer[writePosition] = cTestboard->RecvRoCntEx();
        writePosition++;
        if (writePosition == bufferSize) {writePosition = 0;}
        if (writePosition == readPosition)
        {
            psi::LogInfo() << "[TBInterface] Error: Signalbuffer full in "
                           << "TBInterface ! Data loss possible !!!"
                           << psi::endl;
            return;
        }
        //      Log::Current()->printf("wr %i\n", writePosition);
    }
    // //   Log::Current()->printf("reading back data done\n");
    signalCounter = 0;
}


int TBInterface::AoutLevel(int position, int nTriggers)
{
    return cTestboard->AoutLevel(position, nTriggers);
}


void TBInterface::DoubleColumnADCData(int doubleColumn, short data[], int readoutStop[])
{
    cTestboard->DoubleColumnADCData(doubleColumn, data, readoutStop);
}


int TBInterface::ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim[], int res[])
{
    DataEnable(false);
    int n =  cTestboard->ChipThreshold(start, step, thrLevel, nTrig, dacReg, xtalk, cals, trim, res);
    DataEnable(true);
    return n;
}


int TBInterface::AoutLevelChip(int position, int nTriggers, int trims[], int res[])
{
    return cTestboard->AoutLevelChip(position, nTriggers, trims, res);
}


int TBInterface::AoutLevelPartOfChip(int position, int nTriggers, int trims[], int res[], bool pxlFlags[])
{
    return cTestboard->AoutLevelPartOfChip(position, nTriggers, trims, res, pxlFlags);
}


int TBInterface::ChipEfficiency(int nTriggers, int trim[], double res[])
{
    DataEnable(false);
    int n = cTestboard->ChipEfficiency(nTriggers, trim, res);
    DataEnable(true);
    return n;
}


int TBInterface::MaskTest(short nTriggers, short res[])
{
    DataEnable(false);
    int n = cTestboard->MaskTest(nTriggers, res);
    DataEnable(true);
    return n;
}



int TBInterface::PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim)
{
    DataEnable(false);
    int n = cTestboard->PixelThreshold(col, row, start, step, thrLevel, nTrig, dacReg, xtalk, cals, trim);
    DataEnable(true);
    return n;
}


int TBInterface::SCurve(int nTrig, int dacReg, int threshold, int res[])
{
    DataEnable(false);
    int n = cTestboard->SCurve(nTrig, dacReg, threshold, res);
    DataEnable(true);
    return n;
}


int TBInterface::SCurveColumn(int column, int nTrig, int dacReg, int thr[], int trims[], int chipId[], int res[])
{
    DataEnable(false);
    int n = cTestboard->SCurveColumn(column, nTrig, dacReg, thr, trims, chipId, res);
    DataEnable(true);
    return n;
}


void TBInterface::ADCRead(short buffer[], unsigned short &wordsread, short nTrig)
{
    /* send nTrig calibrates and record the raw analog readout into buffer */
    cTestboard->ADCRead(buffer, wordsread, nTrig);
}


void TBInterface::ADCRead_digital(short buffer[], unsigned short &bitsread, short nTrig)
{
    /* send nTrig calibrates and record the raw digital readout into buffer */
    if (!buffer)
        return;

    /* Read the data as you would read the analog data */
    unsigned short wordsread;
    cTestboard->ADCRead(buffer, wordsread, nTrig);

	DecodedReadoutModule * drm = new DecodedReadoutModule;

	int retval = decode_digital_readout( drm, buffer, wordsread, 1, 0 );
	bitsread = 4 * wordsread;

	psi::LogInfo << "[TBInterface] Count: " << bitsread << " bits" << psi::endl;
	psi::LogInfo << "[TBInterface] Data: ";

    /* compactify: bit shift the data to remove the leading 12 bits in each word */
    /* data: 1000|0000|0000|XXXX (only XXXX is significant data) */
    int nibble = 0;
    for (int i = 0; i < wordsread; i++) {
        int word = i / 4;
        buffer[word] &= ~(0xf << ((4 - nibble - 1) * 4));
        buffer[word] |= (buffer[i] & 0xf) << ((4 - nibble - 1) * 4);
        nibble = (nibble + 1) % 4;
    }


	for( unsigned int n = 0; n < bitsread; n++ ) {
	  psi::LogInfo << ( (n % 8 == 0) ? "|" : "") << ( (buffer[n / 16] & (1 << (16 - n % 16 - 1) ) ) ? 1 : 0 );
	}
 	psi::LogInfo << psi::endl;

	if( retval >= 0 ) { // Successful decoding:
	    int nhits = drm->roc[0].numPixelHits;
	    cout << nhits << " pixel hits" << endl;
		for( int ii = 0; ii < nhits; ++ii ) {
	    	// Record the pulse height and move to the next block of data
	    	int ph = drm->roc[0].pixelHit[ii].analogPulseHeight;
	    	int col = drm->roc[0].pixelHit[ii].columnROC;
	    	int row = drm->roc[0].pixelHit[ii].rowROC;
	    	cout << "hit " << setw(4) << ii+1;
	    	cout << ": col " << setw(2) << col;
		    cout << ", row " << setw(2) << row;
		    cout << ", PH " << setw(3) << ph;
		    cout << endl;
		}
	}
	else {
		cout << "digital decoder error" << endl;
	}
}


void TBInterface::DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[])
{
    DataEnable(false);
    cTestboard->DacDac(dac1, dacRange1, dac2, dacRange2, nTrig, result);
    DataEnable(true);
}

int TBInterface::PH(int col, int row)
{
    return cTestboard->PH(col, row);
}

void TBInterface::PHDac(int dac, int dacRange, int nTrig, int position, short result[])
{
    cTestboard->PHDac(dac, dacRange, nTrig, position, result);
}

bool TBInterface::test_pixel_address(int col, int row)
{
    return cTestboard->test_pixel_address(col, row);
}

void TBInterface::AddressLevels(int position, int result[])
{
    cTestboard->AddressLevels(position, result);
}


void TBInterface::TBMAddressLevels(int result[])
{
    cTestboard->TBMAddressLevels(result);
}


void TBInterface::TrimAboveNoise(short nTrigs, short thr, short mode, short result[])
{
    DataEnable(false);
    cTestboard->TrimAboveNoise(nTrigs, thr, mode, result);
    DataEnable(true);
}

// --------------------------------------------------------

void TBInterface::ProbeSelect(unsigned char port, unsigned char signal) {
    cTestboard->ProbeSelect(port, signal);
}


int TBInterface::demo(short x)
{
    return cTestboard->demo(x);
}


void TBInterface::ScanAdac(unsigned short chip, unsigned char dac,
                                 unsigned char min, unsigned char max, char step,
                                 unsigned char rep, unsigned int usDelay, unsigned char res[])
{
    DataEnable(false);
    cTestboard->ScanAdac(chip, dac, min, max, step, rep, usDelay, res);
    DataEnable(true);
}

void TBInterface::CdVc(unsigned short chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep,
                             unsigned char cdinit, unsigned short &lres, unsigned short res[])
{
    DataEnable(false);
    cTestboard->CdVc(chip, wbcmin, wbcmax, vcalstep, cdinit, lres, res);
    DataEnable(true);
}

char TBInterface::CountAllReadouts(int nTrig, int counts[], int amplitudes[])
{
    return cTestboard->CountAllReadouts(nTrig, counts, amplitudes);
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
