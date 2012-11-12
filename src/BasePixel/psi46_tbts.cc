#include "psi46_tbts.h"
#include <stdio.h>
#include <cstring>
#include <time.h>
#include "BasePixel/GlobalConstants.h"
#include "interface/Delay.h"
#include <sys/timeb.h>
#include <TSystem.h>

#ifndef _WIN32
#include <unistd.h>
#include <iostream>
#endif

// --- begin command table -----------------------------------------------

enum
{
#include "remotecalls_ts.inc"
};

// --- end command table -------------------------------------------------


#define ESC_EXTENDED 0x8f

#define SEND_COMMAND(x) usb.Write_UCHAR(ESC_EXTENDED); usb.Write_UCHAR(x);

#define GET_CHAR(x,ret)        char x; if (!usb.Read_CHAR(x))   return (ret);
#define GET_CHARS(x,n,max,ret) char x[max]; if (!usb.Read_CHARS(x,n)) return (ret);

#define GET_UCHAR(x,ret)  unsigned char  x; if (!usb.Read_UCHAR(x))  return (ret);
#define GET_UCHARS(x,n,max,ret) unsigned char x[max]; if (!usb.Read_UCHARS(x,n)) return (ret);

#define GET_SHORT(x,ret)  short          x; if (!usb.Read_SHORT(x))  return (ret);
#define GET_SHORTS(x,n,max,ret) short x[max]; if (!usb.Read_SHORTS(x,n)) return (ret);

#define GET_USHORT(x,ret) unsigned short x; if (!usb.Read_USHORT(x)) return (ret);
#define GET_USHORTS(x,n,max,ret) unsigned short x[max]; if (!usb.Read_USHORTS(x,n)) return (ret);

#define GET_INT(x,ret)   int           x; if (!usb.Read_INT(x))   return (ret);
#define GET_INTS(x,n,max,ret) int x[max]; if (!usb.Read_INTS(x,n)) return (ret);

#define GET_UINT(x,ret)  unsigned int  x; if (!usb.Read_UINT(x))  return (ret);
#define GET_UINTS(x,n,max,ret) unsigned int x[max]; if (!usb.Read_UINTS(x,n)) return (ret);

#define GET_LONG(x,ret)  long  x; if (!usb.Read_LONG(x))  return (ret);

#define GET_STRING(x,max,ret) char x[max]; if (!usb.Read_String(x,max)) return (ret);

#define PUT_CHAR(x)      usb.Write_CHAR(x);
#define PUT_CHARS(x,n)   usb.Write_CHARS(x,n);

#define PUT_UCHAR(x)     usb.Write_UCHAR(x);
#define PUT_UCHARS(x,n)  usb.Write_UCHARS(x,n);

#define PUT_SHORT(x)     usb.Write_SHORT(x);
#define PUT_SHORTS(x,n)  usb.Write_SHORTS(x,n);

#define PUT_USHORT(x)    usb.Write_USHORT(x);
#define PUT_USHORTS(x,n) usb.Write_USHORTS(x,n);

#define PUT_INT(x)      usb.Write_INT(x);
#define PUT_INTS(x,n)   usb.Write_INTS(x,n);

#define PUT_UINT(x)     usb.Write_UINT(x);
#define PUT_UINTS(x,n)  usb.Write_UINTS(x,n);

#define PUT_STRING(x)    usb.Write_String(x);


CTestboard_TS::CTestboard_TS()
{
	TBM_present = false;
}


// === board connection methods =========================================

bool CTestboard_TS::GetVersion(char *s, unsigned int n)
{
	SEND_COMMAND(CMD_GetVersion)
	Flush();
	return usb.Read_String(s, n);
}



bool CTestboard_TS::Open(char name[], bool init)
{
	int list_boards = false;
	char actual_name [256];
	
	/* Check whether we are using a wildcard */
	if (strcmp(name, "*") == 0) {
		/* Check the number of devices connected */
		unsigned int devices;
		usb.EnumFirst(devices);
		
		if (devices == 1) {
			/* Get the name of the testboard */
			usb.EnumNext(actual_name);
			std::cout << "Using wildcard: Opening connection to testboard " << actual_name << std::endl;
		} else {
			if (devices > 1)
				std::cout << "Error using testboard wildcard: More than one testboard connected." << std::endl;
			list_boards = true;
		}
	} else {
		/* Not using the wildcard */
		strcpy(actual_name, name);
		std::cout << "Opening connection to testboard " << actual_name << std::endl;
	}
	
	/* Open and check for errors */
	if (!usb.Open(actual_name) && !list_boards) {
		int status = usb.GetLastError();
		std::cout << "USB error: " << usb.GetErrorMsg(status) << std::endl;
		list_boards = true;
	}
	
	/* List the boards connected to the computer */
	if (list_boards) {
		unsigned int devices;
		usb.EnumFirst(devices);
		if (devices == 0) {
			std::cout << "No testboards connected." << std::endl;
		} else {
			/* Iterate over the testboard names and print them */
			char name [256];
			std::cout << "Connected testboards: ";
			for (unsigned int i = 0; i < devices; i++) {
				usb.EnumNext(name);
				std::cout << name;
				if (i != devices - 1)
					std::cout << ", ";
				else
					std::cout << std:: endl;
			}
		}
		return false;
	}

	if (init) Init();
	return true;
}


void CTestboard_TS::Close()
{
	usb.Close();
}


void CTestboard_TS::Welcome()
{
	SEND_COMMAND(CMD_Welcome)
	Flush();
}


void CTestboard_TS::SetLed(unsigned char value)
{
	SEND_COMMAND(CMD_SetLed);
	PUT_UCHAR(value);
}


unsigned char CTestboard_TS::GetBoardId()
{
	SEND_COMMAND(CMD_GetBoardId)
	Flush();
	GET_UCHAR(id, 0)
	return id;
}


void CTestboard_TS::Init()
{
	SEND_COMMAND(CMD_Init)
	Flush();
}

unsigned int CTestboard_TS::GetStatus()
{
	SEND_COMMAND(CMD_GetStatus)
	Flush();
	gDelay->Mdelay(1);
	GET_UINT(status,-1)

	return status;
}

int CTestboard_TS::GetData(unsigned short* buffer)
{
	int BLOCKSIZE = 1;
	SEND_COMMAND(CMD_GetData)
	Flush();
	gDelay->Mdelay(1);
	GET_UINT(num,0)
	std::cout << "Num: " << num << std::endl;
	//GET_USHORTS(data,num-5,num+2,-1)
	//std::cout<<"I read " << iRead << " = " << iRead/sizeof(unsigned short) << " shorts"  << std::endl;
	struct timeb starttime,endtime;
	ftime(&starttime);

	for(unsigned int i = 0 ; i*BLOCKSIZE< num ;i++)
	{
		unsigned int iRead;
		//GET_USHORT(data,-1)
		//GET_USHORTS(data,BLOCKSIZE,BLOCKSIZE+1,-1)	
	        usb.Read((unsigned int)BLOCKSIZE*sizeof(unsigned short),buffer+i*BLOCKSIZE,iRead);
		//std::cout << "read: " << iRead << std::endl;
		//buffer[i] = data;
		//memcpy((void*)(buffer+i*BLOCKSIZE),data,BLOCKSIZE*sizeof(unsigned short));
	        //std::cout << data[i] << std::endl;
	//std::cout << "Read #" <<i <<": " << BLOCKSIZE << " shorts in " << (double)endtime.time-starttime.time << " seconds." << std::endl; 
		gSystem->ProcessEvents();
	}
	ftime(&endtime);
	std::cout << "Read #" <<": " << num << " shorts in " << (double)endtime.time-starttime.time << " seconds." << std::endl; 
	//Clear();
	return 1;
}


bool CTestboard_TS::ReadyForReadout()
{

	SEND_COMMAND(CMD_ReadyForReadout)
	Flush();
	gDelay->Mdelay(200);
	GET_USHORT(ready,0)
	return ready;
}

void CTestboard_TS::EventWriterStart()
{
	SEND_COMMAND(CMD_EventWriterStart)
	Flush();

}
void CTestboard_TS::EventWriterStop()
{
	SEND_COMMAND(CMD_EventWriterStop)
	Flush();

}


unsigned int CTestboard_TS::DaqInit(int buf)
{
	SEND_COMMAND(CMD_Daq_Init)
	PUT_INT(buf)
	Flush();

	GET_UINT(pointer,0)
	return pointer;
}

unsigned int CTestboard_TS::GetPointer()
{

	SEND_COMMAND(CMD_Daq_GetPointer)
        Flush();
	gDelay->Mdelay(1);	
	GET_UINT(p,0)
	return p;

}

void CTestboard_TS::DaqEnable()
{
	SEND_COMMAND(CMD_Daq_Enable)
	Flush();

}

void CTestboard_TS::DaqDisable()
{
	SEND_COMMAND(CMD_Daq_Disable)
	Flush();

}

void CTestboard_TS::ClearRam()
{
	SEND_COMMAND(CMD_TBClearRam)
	Flush();
}

// === delay methods ====================================================

void CTestboard_TS::uDelay(unsigned short us)
{
	SEND_COMMAND(CMD_uDelay)
	PUT_USHORT(us)
}


void CTestboard_TS::mDelay(unsigned short ms)
{
	Flush();
#ifdef _WIN32
	Sleep(ms);			// Windows
#else
	usleep(ms*1000);	// Linux
#endif
}


// == PSI46 testboard functions =====================================

unsigned char CTestboard_TS::isClockPresent()
{
	SEND_COMMAND(CMD_isClockPresent)
	Flush();
	GET_UCHAR(ok, 0)
	return ok;
}


void CTestboard_TS::SetClock(unsigned char MHz)
{
	SEND_COMMAND(CMD_SetClock)
	PUT_UCHAR(MHz);
}

void CTestboard_TS::Pon()
{
	SEND_COMMAND(CMD_Pon)
	Flush();
}


void CTestboard_TS::Poff()
{
	SEND_COMMAND(CMD_Poff)
}

void CTestboard_TS::StartRun(unsigned int Mem,unsigned int EvtNum)
{
	SEND_COMMAND(CMD_StartRun)
	PUT_UINT(Mem)
	PUT_UINT(EvtNum);
	Flush();
        //gDelay->Mdelay(200); 
	//GET_USHORT(p1,-1)
	//GET_USHORT(p2,-1)
	//return p1 << 16 + p2;
}

int CTestboard_TS::GetDaqSize()
{
	SEND_COMMAND(CMD_Daq_GetSize)
	Flush();
	gDelay->Mdelay(100);	
	GET_UINT(s,-1)
	return s;

}

int CTestboard_TS::GetRawData(int MemSize)
{

	SEND_COMMAND(CMD_GetRawData)
	PUT_UINT(MemSize)
	Flush();
	std::cout << "GetRaw" << std::endl;
	GET_UINT(point,-1)
	std::cout<< "pointer=" << point << std::endl;

	
	for(int i = 0 ; i < MemSize; i++)	
	{
		GET_USHORT(num,-1)
		std::cout << num << std::endl;
	}		

	//GET_USHORTS(aData,nElements,20000,-1);
	//std::cout << std::oct << aData << std::endl;
	//Flush();
	return 1;

}

void CTestboard_TS::SetVA(double V)
{
	short mV = (short)(V*1000.0);
	SEND_COMMAND(CMD_SetVA)
	PUT_SHORT(mV)
	Flush();
}

void CTestboard_TS::SetVD(double V)
{
	short mV = (short)(V*1000.0);
	SEND_COMMAND(CMD_SetVD)
	PUT_SHORT(mV)
	Flush();
}


void CTestboard_TS::SetIA(double A)
{
	short uA100 = (short)(A*10000.0);
	SEND_COMMAND(CMD_SetIA)
	PUT_SHORT(uA100)
	Flush();
}

void CTestboard_TS::SetID(double A)
{
	short uA100 = (short)(A*10000.0);
	SEND_COMMAND(CMD_SetID)
	PUT_SHORT(uA100)
	Flush();
}


double CTestboard_TS::GetVA()
{
	SEND_COMMAND(CMD_GetVA)
	Flush();
        gDelay->Mdelay(20); 
	GET_SHORT(mV,0.0)
	return mV/1000.0;
}

int CTestboard_TS::GetThresh()
{
        SEND_COMMAND(CMD_GetThresh)
        Flush();
        gDelay->Mdelay(20); 
        GET_UINT(thresh,0)
	return thresh;
}

double CTestboard_TS::GetVD()
{
	SEND_COMMAND(CMD_GetVD)
	Flush();
        gDelay->Mdelay(20); 
	GET_SHORT(mV,0.0)
	return mV/1000.0;
}


double CTestboard_TS::GetIA()
{
	SEND_COMMAND(CMD_GetIA)
	Flush();
        gDelay->Mdelay(20); 
	GET_INT(uA100,0.0)
	return uA100/10000.0;
}

double CTestboard_TS::GetID()
{
	SEND_COMMAND(CMD_GetID)
	Flush();
        gDelay->Mdelay(20); 
	GET_INT(uA100,0.0)
	return uA100/10000.0;
}


void CTestboard_TS::HVon()
{
	SEND_COMMAND(CMD_HVon)
}


void CTestboard_TS::HVoff()
{
	SEND_COMMAND(CMD_HVoff)
}


void CTestboard_TS::ResetOn()
{
	SEND_COMMAND(CMD_ResetOn)
}


void CTestboard_TS::ResetOff()
{
	SEND_COMMAND(CMD_ResetOff)
}
bool CTestboard_TS::UseInternalTriggers() {
	//0b100000000000 =  0x800
	SEND_COMMAND(CMD_GetThresh)
	Flush();
	GET_UINT(thresh,0)
	SEND_COMMAND(CMD_SetThresh)
	PUT_UINT(0x800|thresh)
	Flush();
}

bool CTestboard_TS::DisableInternalTriggers() {
	SEND_COMMAND(CMD_GetThresh)
	Flush();
	GET_UINT(thresh,0)
	SEND_COMMAND(CMD_SetThresh)
	PUT_UINT(~0x800&thresh)
	Flush();
}

bool CTestboard_TS::SendCalibrationPulsesOnInternalTriggers(unsigned short delay) {
	//0b010000000000 = 0x400
	if(delay > 15) delay = 15;
	SEND_COMMAND(CMD_GetThresh)
	Flush();
	GET_UINT(thresh,0)
	SEND_COMMAND(CMD_SetThresh)
	int mask = 0x400 + delay;
	PUT_UINT(mask|thresh)
	Flush();
}

bool CTestboard_TS::DisableCalPulses() {
	SEND_COMMAND(CMD_GetThresh)
	Flush();
	GET_UINT(thresh,0)
	SEND_COMMAND(CMD_SetThresh)
	PUT_UINT(~0x400&thresh)
	Flush();
}
bool CTestboard_TS::SetLatchDelay(int delay) {
	if(delay > 15) delay = 15;
	if(delay < 0 ) delay = 0;
        SEND_COMMAND(CMD_GetThresh)
        Flush();
        GET_UINT(thresh,0)
	thresh = thresh & 3855;
	delay = delay << 4;
        SEND_COMMAND(CMD_SetThresh)
        PUT_UINT(thresh|delay)
        Flush();
}


void CTestboard_TS::CalStart()
{
	SEND_COMMAND(CMD_Cpu_CalStart)
	Flush();
}
int CTestboard_TS::DaqReady()
{
	SEND_COMMAND(CMD_Daq_Ready)
	Flush();
	gDelay->Mdelay(2);
	GET_UCHAR(ret,-1)
	return ret;

}
void CTestboard_TS::StopRun()
{
	SEND_COMMAND(CMD_StopRun)
	Flush();

}
void CTestboard_TS::TrigPause()
{
	SEND_COMMAND(CMD_TrigPause);
	Flush();

}
