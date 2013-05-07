#include "psi46_tb.h"
#include <stdio.h>
#include "BasePixel/GlobalConstants.h"
#include "interface/Delay.h"
#include <inttypes.h>
#include <cstring>

#ifndef _WIN32
#include <unistd.h>
#include <iostream>
#endif

// --- begin command table -----------------------------------------------

enum
{
#include "remotecalls.inc"
#include "remotecalls_xraytest.inc"
#include "remotecalls_chiptest.inc"
#include "remotecalls_modultest.inc"
    CMD_GetReg41,
    CMD_TBMEmulatorOn,
    CMD_TBMEmulatorOff,
    CMD_pg_SetCmd,
    CMD_pg_SetCmdAll,
    CMD_pg_Disable,
    CMD_pg_Single,
    CMD_pg_Trigger,
    CMD_pg_Loop,
    CMD_Bootstrap,
    CMD_Dummy
};

// --- end command table -------------------------------------------------


#define SEND_COMMAND(x) usb.WriteCommand(x);

#define GET_CHAR(x,ret)        char x; if (!usb.Read_CHAR(x))   return (ret);
#define GET_CHARS(x,n,max,ret) char x[max]; if (!usb.Read_CHARS(x,n)) return (ret);

#define GET_UCHAR(x,ret)  unsigned char  x; if (!usb.Read_UCHAR(x))  return (ret);
#define GET_UCHARS(x,n,max,ret) unsigned char x[max]; if (!usb.Read_UCHARS(x,n)) return (ret);

#define GET_SHORT(x,ret)  int16_t          x; if (!usb.Read_SHORT(x))  return (ret);
#define GET_SHORTS(x,n,max,ret) int16_t x[max]; if (!usb.Read_SHORTS(x,n)) return (ret);

#define GET_USHORT(x,ret) uint16_t x; if (!usb.Read_USHORT(x)) return (ret);
#define GET_USHORTS(x,n,max,ret) uint16_t x[max]; if (!usb.Read_USHORTS(x,n)) return (ret);

#define GET_LONG(x,ret)   int32_t           x; if (!usb.Read_LONG(x))   return (ret);
#define GET_LONGS(x,n,max,ret) int32_t x[max]; if (!usb.Read_LONGS(x,n)) return (ret);

#define GET_ULONG(x,ret)  uint32_t  x; if (!usb.Read_ULONG(x))  return (ret);
#define GET_ULONGS(x,n,max,ret) uint32_t x[max]; if (!usb.Read_ULONGS(x,n)) return (ret);

#define GET_STRING(x,max,ret) char x[max]; if (!usb.Read_String(x,max)) return (ret);


#define PUT_CHAR(x)      usb.Write_CHAR(x);
#define PUT_CHARS(x,n)   usb.Write_CHARS(x,n);

#define PUT_UCHAR(x)     usb.Write_UCHAR(x);
#define PUT_UCHARS(x,n)  usb.Write_UCHARS(x,n);

#define PUT_SHORT(x)     usb.Write_SHORT(x);
#define PUT_SHORTS(x,n)  usb.Write_SHORTS(x,n);

#define PUT_USHORT(x)    usb.Write_USHORT(x);
#define PUT_USHORTS(x,n) usb.Write_USHORTS(x,n);

#define PUT_LONG(x)      usb.Write_LONG(x);
#define PUT_LONGS(x,n)   usb.Write_LONGS(x,n);

#define PUT_ULONG(x)     usb.Write_ULONG(x);
#define PUT_ULONGS(x,n)  usb.Write_ULONGS(x,n);

#define PUT_STRING(x)    usb.Write_String(x);



CTestboard::CTestboard()
{
    TBM_present = false;
}


// === board connection methods =========================================

bool CTestboard::GetVersion(char *s, uint32_t n)
{
    SEND_COMMAND(CMD_GetVersion)
    Flush();
    return usb.Read_String(s, n);
}


bool CTestboard::Open(char name[], bool init)
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


void CTestboard::Close()
{
    usb.Close();
}


void CTestboard::Welcome()
{
    SEND_COMMAND(CMD_Welcome)
    Flush();
}


void CTestboard::SetLed(unsigned char value)
{
    SEND_COMMAND(CMD_SetLed);
    PUT_UCHAR(value);
}


unsigned char CTestboard::GetBoardId()
{
    SEND_COMMAND(CMD_GetBoardId)
    Flush();
    GET_UCHAR(id, 0)
    return id;
}


void CTestboard::Init()
{
    SEND_COMMAND(CMD_Init)
    Flush();
    TBM_present = false;
}

void CTestboard::Bootstrap()
{
	SEND_COMMAND(CMD_Bootstrap);
	Flush();
}


// === delay methods ====================================================

void CTestboard::cDelay(uint16_t clocks)
{
    SEND_COMMAND(CMD_cDelay)
    PUT_USHORT(clocks)
}


void CTestboard::uDelay(uint16_t us)
{
    SEND_COMMAND(CMD_uDelay)
    PUT_USHORT(us)
}


void CTestboard::mDelay(uint16_t ms)
{
    Flush();
#ifdef _WIN32
    Sleep(ms);          // Windows
#else
    usleep(ms * 1000);  // Linux
#endif
}


// == PSI46 testboard functions =====================================

unsigned char CTestboard::isClockPresent()
{
    SEND_COMMAND(CMD_isClockPresent)
    Flush();
    GET_UCHAR(ok, 0)
    return ok;
}


void CTestboard::SetClock(unsigned char MHz)
{
    SEND_COMMAND(CMD_SetClock)
    PUT_UCHAR(MHz);
}

void CTestboard::SetClockStretch(unsigned char src,
		uint16_t delay, uint16_t width)
{
    SEND_COMMAND(CMD_SetClockStretch)
    PUT_UCHAR(src)
    PUT_USHORT(delay)
    PUT_USHORT(width)
}


void CTestboard::SetDelay(unsigned char signal, uint16_t ns)
{
    SEND_COMMAND(CMD_SetDelay)
    PUT_UCHAR(signal);
    PUT_USHORT(ns);
}

void CTestboard::ForceSignal(unsigned char pattern)
{
    SEND_COMMAND(CMD_ForceSignal)
    PUT_UCHAR(pattern)
}

void CTestboard::I2cAddr(unsigned char id)
{
    SEND_COMMAND(CMD_I2cAddr)
    PUT_UCHAR(id);
}


void CTestboard::Pon()
{
    SEND_COMMAND(CMD_Pon)
}


void CTestboard::Poff()
{
    SEND_COMMAND(CMD_Poff)
}


void CTestboard::SetVA(double V)
{
	int16_t mV = (int16_t)(V*1000.0);
    SEND_COMMAND(CMD_SetVA)
    PUT_SHORT(mV)
}

void CTestboard::SetVD(double V)
{
	int16_t mV = (int16_t)(V*1000.0);
    SEND_COMMAND(CMD_SetVD)
	PUT_SHORT(mV)
}


void CTestboard::SetVO(double V)
{
	int16_t mV = (int16_t)(V*1000.0);
	SEND_COMMAND(CMD_SetVO)
    PUT_SHORT(mV)
}


void CTestboard::SetIA(double A)
{
	int16_t uA100 = (int16_t)(A*10000.0);
    SEND_COMMAND(CMD_SetIA)
    PUT_SHORT(uA100)
}

void CTestboard::SetID(double A)
{
	int16_t uA100 = (int16_t)(A*10000.0);
    SEND_COMMAND(CMD_SetID)
	PUT_SHORT(uA100)
}


void CTestboard::SetIO(double A)
{
	int16_t uA100 = (int16_t)(A*10000.0);
	SEND_COMMAND(CMD_SetIO)
    PUT_SHORT(uA100)
}


double CTestboard::GetVA()
{
    SEND_COMMAND(CMD_GetVA)
    Flush();
    gDelay->Mdelay(200);
    GET_SHORT(mV, 0.0)
    return mV / 1000.0;
}

double CTestboard::GetVD()
{
    SEND_COMMAND(CMD_GetVD)
    Flush();
	GET_SHORT(mV,0.0)
	return mV/1000.0;
}


double CTestboard::GetVO()
{
	SEND_COMMAND(CMD_GetVO)
	Flush();
    GET_SHORT(mV, 0.0)
    return mV / 1000.0;
}


double CTestboard::GetIA()
{
    SEND_COMMAND(CMD_GetIA)
    Flush();
	gDelay->Mdelay(200);
	GET_LONG(uA100,0.0)
    return uA100 / 10000.0;
}

double CTestboard::GetID()
{
    SEND_COMMAND(CMD_GetID)
    Flush();
	gDelay->Mdelay(200);
	GET_LONG(uA100,0.0)
	return uA100/10000.0;
}


double CTestboard::GetIO()
{
	SEND_COMMAND(CMD_GetIO)
	Flush();
	GET_LONG(uA100,0.0)
    return uA100 / 10000.0;
}


void CTestboard::HVon()
{
    SEND_COMMAND(CMD_HVon)
}


void CTestboard::HVoff()
{
    SEND_COMMAND(CMD_HVoff)
}


void CTestboard::ResetOn()
{
    SEND_COMMAND(CMD_ResetOn)
}


void CTestboard::ResetOff()
{
    SEND_COMMAND(CMD_ResetOff)
}


void CTestboard::Set(unsigned char reg, unsigned char value)
{
    SEND_COMMAND(CMD_Set)
    PUT_UCHAR(reg)
    PUT_UCHAR(value)
}


void CTestboard::Single(unsigned char mask)
{
    SEND_COMMAND(CMD_Single)
    PUT_UCHAR(mask)
}

bool CTestboard::SingleWait(unsigned char mask, uint16_t timeout)
{
    SEND_COMMAND(CMD_SingleWait)
    PUT_UCHAR(mask)
    PUT_USHORT(timeout)
    usb.Flush();
    GET_UCHAR(ret, 0)
    return ret != 0;
}


void CTestboard::Intern(unsigned char mask)
{
    SEND_COMMAND(CMD_Intern)
    PUT_UCHAR(mask)
}

// == pulse pattern generator ===========================================

void CTestboard::pg_SetCmd(uint16_t addr, uint16_t cmd)
{
	SEND_COMMAND(CMD_pg_SetCmd)
	PUT_USHORT(addr)
	PUT_USHORT(cmd)
}


void CTestboard::pg_SetCmdAll(uint16_t count, uint16_t *cmd)
{
	if (count > 255) return;
	SEND_COMMAND(CMD_pg_SetCmdAll)
	PUT_USHORT(count)
	PUT_USHORTS(cmd,count)
}


void CTestboard::pg_Disable()
{
	SEND_COMMAND(CMD_pg_Disable)
}


void CTestboard::pg_Single()
{
	SEND_COMMAND(CMD_pg_Single)
}


void CTestboard::pg_Trigger()
{
	SEND_COMMAND(CMD_pg_Trigger)
}


void CTestboard::pg_Loop(uint16_t period)
{
	SEND_COMMAND(CMD_pg_Loop)
	PUT_USHORT(period)
}

// -----------------------------------------------------------------------


unsigned char CTestboard::GetRoCnt()
{
    SEND_COMMAND(CMD_GetRoCnt)
    usb.Flush();
    GET_UCHAR(cnt, 0)
    return cnt;
}


bool CTestboard::SendRoCnt()
{
    SEND_COMMAND(CMD_GetRoCnt)
    return true;
}


unsigned char CTestboard::RecvRoCnt()
{
    GET_UCHAR(value, 0);
    return value;
}


uint16_t CTestboard::GetRoCntEx()
{
    SEND_COMMAND(CMD_GetRoCntEx)
    usb.Flush();
    GET_USHORT(cnt, 0)
    return cnt;
}


bool CTestboard::SendRoCntEx()
{
    SEND_COMMAND(CMD_GetRoCntEx)
    return true;
}


uint16_t CTestboard::RecvRoCntEx()
{
    GET_USHORT(value, 0)
    return value;
}


void CTestboard::SetTriggerMode(uint16_t mode)
{
    SEND_COMMAND(CMD_SetTriggerMode)
    PUT_USHORT(mode)
}


void CTestboard::DataCtrl(char channel, bool clear, bool trigger, bool cont)
{
    unsigned char flags = (clear ? 0x04 : 0) | (trigger ? 0x02 : 0) | (cont ? 0x01 : 0);
    SEND_COMMAND(CMD_DataCtrl)
    PUT_UCHAR(flags)
}


void CTestboard::DataEnable(bool on)
{
    SEND_COMMAND(CMD_DataEnable)
    PUT_UCHAR(on ? 1 : 0);
}


uint16_t CTestboard::DataState()
{
    SEND_COMMAND(CMD_DataState)
    Flush();
    GET_USHORT(state, 0);
    return state;
}


void CTestboard::DataTriggerLevel(char channel, int16_t level)
{
    SEND_COMMAND(CMD_DataTriggerLevel)
    PUT_SHORT(level)
}


void CTestboard::DataBlockSize(uint16_t size)
{
    SEND_COMMAND(CMD_DataBlockSize)
    PUT_USHORT(size)
}


bool CTestboard::DataRead(char channel, int16_t buffer[], uint16_t buffersize,
						 uint16_t &wordsread)
{
    SEND_COMMAND(CMD_DataRead)
    PUT_USHORT(buffersize)
    Flush();
    unsigned char res;
    if (!usb.Read_UCHAR(res)) { wordsread = 0; return false; }
    if (!usb.Read_USHORT(wordsread)) { wordsread = 0; return false; }
    if (wordsread > buffersize) wordsread = buffersize;
    if (wordsread) usb.Read_SHORTS(buffer, wordsread);
    return res != 0;
}


bool CTestboard::DataReadRaw(char channel, int16_t buffer[], uint16_t buffersize,
						 uint16_t &wordsread)
{
    SEND_COMMAND(CMD_DataReadRaw)
    PUT_USHORT(buffersize)
    Flush();
    unsigned char res;
    if (!usb.Read_UCHAR(res)) { wordsread = 0; return false; }
    if (!usb.Read_USHORT(wordsread)) { wordsread = 0; return false; }
    if (wordsread > buffersize) wordsread = buffersize;
    if (wordsread) usb.Read_SHORTS(buffer, wordsread);
    return res != 0;
}


uint16_t CTestboard::GetModRoCnt(uint16_t index)
{
    SEND_COMMAND(CMD_GetModRoCnt)
    PUT_USHORT(index)
    Flush();
    gDelay->Mdelay(50);
    GET_USHORT(value, 0)
    return value;
}


void CTestboard::GetModRoCntAll(uint16_t *counts)
{
    SEND_COMMAND(CMD_GetModRoCntAll)
    Flush();
    usb.Read_USHORTS(counts, 17);
}


uint32_t CTestboard::Daq_Init(uint32_t size)
{
    SEND_COMMAND(CMD_Daq_Init)
	PUT_ULONG(size)
    Flush();

	GET_ULONG(p,0)
	return (uint32_t)p;
}


void CTestboard::Daq_Enable()
{
    SEND_COMMAND(CMD_Daq_Enable)
}


void CTestboard::Daq_Disable()
{
    SEND_COMMAND(CMD_Daq_Disable)
}


bool CTestboard::Daq_Ready()
{
    SEND_COMMAND(CMD_Daq_Ready)
    Flush();
    GET_UCHAR(rdy, 0)
    return rdy != 0;
}


uint16_t CTestboard::Daq_GetPointer()
{
    SEND_COMMAND(CMD_Daq_GetPointer)
    Flush();
	GET_ULONG(p,0)
	return (uint16_t)p;
}


uint32_t CTestboard::Daq_GetSize()
{
    SEND_COMMAND(CMD_Daq_GetSize)
    Flush();
	GET_ULONG(size,0)
    return size;
}


void CTestboard::Daq_Done()
{
    SEND_COMMAND(CMD_Daq_Done)
}


// == probe signal selection ============================================

void CTestboard::ProbeSelect(unsigned char port, unsigned char signal)
{
    SEND_COMMAND(CMD_ProbeSelect)
    PUT_UCHAR(port)
    PUT_UCHAR(signal)
}

void CTestboard::SetTriggerMask(unsigned char mask)
{
  	//SEND_COMMAND(CMD_SetTrigMask)
  	//PUT_UCHAR(mask);
}

// == Wafer Test Adapter ================================================

double CTestboard::GetVD_Reg()
{
    SEND_COMMAND(CMD_GetVD_Reg)
    Flush();
    GET_SHORT(mV, 0.0)
    return mV / 1000.0;
}


double CTestboard::GetVD_CAP()
{
    SEND_COMMAND(CMD_GetVD_CAP)
    Flush();
    GET_SHORT(mV, 0.0)
    return mV / 1000.0;
}


double CTestboard::GetVDAC_CAP()
{
    SEND_COMMAND(CMD_GetVDAC_CAP)
    Flush();
    GET_SHORT(mV, 0.0)
    return mV / 1000.0;
}


double CTestboard::GetTOUT_COM()
{
    SEND_COMMAND(CMD_GetTOUT_COM)
    Flush();
    GET_SHORT(mV, 0.0)
    return mV / 1000.0;
}


double CTestboard::GetAOUT_COM()
{
    SEND_COMMAND(CMD_GetAOUT_COM)
    Flush();
    GET_SHORT(mV, 0.0)
    return mV / 1000.0;
}



// == TBM functions ======================================================

void CTestboard::tbm_Enable(bool on)
{
    TBM_present = on;
    SEND_COMMAND(CMD_tbm_Enable)
    PUT_UCHAR(on)
}


void CTestboard::tbm_Addr(unsigned char hub, unsigned char port)
{
    SEND_COMMAND(CMD_tbm_Addr)
    PUT_UCHAR(hub)
    PUT_UCHAR(port)
}


void CTestboard::mod_Addr(unsigned char hub)
{
    SEND_COMMAND(CMD_mod_Addr)
    PUT_UCHAR(hub)
}


void CTestboard::tbm_Set(unsigned char reg, unsigned char value)
{
    SEND_COMMAND(CMD_tbm_Set)
    PUT_UCHAR(reg)
    PUT_UCHAR(value)
}

bool CTestboard::tbm_Get(unsigned char reg, unsigned char &value)
{
    SEND_COMMAND(CMD_tbm_Get)
    PUT_UCHAR(reg)
    Flush();
    GET_UCHAR(res, 0);
    if (!usb.Read_UCHAR(value)) { value = 0; return false; }
    return res != 0;
}

bool CTestboard::tbm_GetRaw(unsigned char reg, int32_t &value)
{
    SEND_COMMAND(CMD_tbm_GetRaw)
    PUT_UCHAR(reg)
    Flush();
	usb.Read_LONG(value);
    return value >= 0;
}


// == ROC functions ======================================================

void CTestboard::roc_I2cAddr(unsigned char id)
{
    SEND_COMMAND(CMD_roc_I2cAddr)
    PUT_UCHAR(id)
}


void CTestboard::roc_ClrCal()
{
    SEND_COMMAND(CMD_roc_ClrCal)
}


void CTestboard::roc_SetDAC(unsigned char reg, unsigned char value)
{
    SEND_COMMAND(CMD_roc_SetDAC)
    PUT_UCHAR(reg)
    PUT_UCHAR(value)
}


void CTestboard::roc_Pix(unsigned char col, unsigned char row, unsigned char value)
{
    SEND_COMMAND(CMD_roc_Pix)
    PUT_UCHAR(col)
    PUT_UCHAR(row)
    PUT_UCHAR(value)
}


void CTestboard::roc_Pix_Trim(unsigned char col, unsigned char row, unsigned char value)
{
    SEND_COMMAND(CMD_roc_Pix_Trim)
    PUT_UCHAR(col)
    PUT_UCHAR(row)
    PUT_UCHAR(value)
}


void CTestboard::roc_Pix_Mask(unsigned char col, unsigned char row)
{
    SEND_COMMAND(CMD_roc_Pix_Mask)
    PUT_UCHAR(col)
    PUT_UCHAR(row)
}


void CTestboard::roc_Pix_Cal(unsigned char col, unsigned char row, bool sensor_cal)
{
    SEND_COMMAND(CMD_roc_Pix_Cal)
    PUT_UCHAR(col)
    PUT_UCHAR(row)
    PUT_UCHAR(sensor_cal)
}


void CTestboard::roc_Col_Enable(unsigned char col, bool on)
{
    SEND_COMMAND(CMD_roc_Col_Enable)
    PUT_UCHAR(col)
    PUT_UCHAR(on)
}


void CTestboard::roc_Col_Mask(unsigned char col)
{
    SEND_COMMAND(CMD_roc_Col_Mask)
    PUT_UCHAR(col)
}


void CTestboard::roc_Chip_Mask()
{
    SEND_COMMAND(CMD_roc_Chip_Mask)
}



// === low level methodes ================================================

void CTestboard::SetReg(unsigned char addr, uint16_t value)
{
    SEND_COMMAND(CMD_SetReg)
    PUT_UCHAR(addr)
    PUT_USHORT(value)
}


// === debug function ====================================================

void CTestboard::IoRead8(uint32_t addr, uint16_t size,
                         unsigned char step,    unsigned char * value)
{
    SEND_COMMAND(CMD_IoRead8);
	PUT_ULONG(addr)
    PUT_USHORT(size)
    PUT_USHORT(step)
    Flush();
    usb.Read_UCHARS(value, size);
}


void CTestboard::IoRead16(uint32_t addr, uint16_t size,
	unsigned char step,	uint16_t *value)
{
    SEND_COMMAND(CMD_IoRead16);
	PUT_ULONG(addr)
    PUT_USHORT(size)
    PUT_USHORT(step)
    Flush();
    usb.Read_USHORTS(value, size);
}


void CTestboard::IoRead32(uint32_t addr, uint16_t size,
	unsigned char step,	uint32_t *value)
{
    SEND_COMMAND(CMD_IoRead32);
	PUT_ULONG(addr)
    PUT_USHORT(size)
    PUT_USHORT(step)
    Flush();
	usb.Read_ULONGS(value,size);
}


void CTestboard::IoWrite8(uint32_t addr, uint16_t size,
                          unsigned char step, const unsigned char * x)
{
    SEND_COMMAND(CMD_IoWrite8)
	PUT_ULONG(addr)
    PUT_USHORT(size)
    PUT_UCHAR(step)
    PUT_UCHARS(x, size)
}


void CTestboard::IoWrite16(uint32_t addr, uint16_t size,
	unsigned char step, const uint16_t *x)
{
    SEND_COMMAND(CMD_IoWrite16)
	PUT_ULONG(addr)
    PUT_USHORT(size)
    PUT_UCHAR(step)
    PUT_USHORTS(x, size)
}

void CTestboard::IoWrite32(uint32_t addr, uint16_t size,
	unsigned char step, const uint32_t *x)
{
    SEND_COMMAND(CMD_IoWrite32)
	PUT_ULONG(addr)
    PUT_USHORT(size)
    PUT_UCHAR(step)
	PUT_ULONGS(x,size)
}


void CTestboard::MemWrite(uint32_t addr, uint16_t size,
                          unsigned char * x)
{
    SEND_COMMAND(CMD_MemWrite)
	PUT_ULONG(addr)
    PUT_USHORT(size)
    PUT_UCHARS(x, size)
}


void CTestboard::MemRead(uint32_t addr, uint16_t size,
                         unsigned char * s)
{
    SEND_COMMAND(CMD_MemRead)
	PUT_ULONG(addr)
    PUT_USHORT(size)
    Flush();
    gDelay->Mdelay(50);
    usb.Read_UCHARS(s, size);
}


void CTestboard::MemFill(uint32_t addr, uint16_t size,
                         unsigned char x)
{
    SEND_COMMAND(CMD_MemFill)
	PUT_ULONG(addr)
    PUT_USHORT(size)
    PUT_UCHAR(x)
}


unsigned char CTestboard::FlashRead(uint32_t addr, uint16_t size,
                                    unsigned char * x)
{
    SEND_COMMAND(CMD_FlashRead)
	PUT_ULONG(addr)
    PUT_USHORT(size)
    Flush();
    GET_UCHAR(ret, 0)
    usb.Read_UCHARS(x, size);
    return ret;
}

unsigned char CTestboard::FlashWrite(uint32_t addr, uint16_t size,
                                     unsigned char * x)
{
    SEND_COMMAND(CMD_FlashWrite)
	PUT_ULONG(addr)
    PUT_USHORT(size)
    usb.Write_UCHARS(x, size);
    Flush();
    GET_UCHAR(ret, 99)
    return ret;
}


void CTestboard::TbmWrite(int32_t hubAddr, int32_t addr, int32_t value)
{
    SEND_COMMAND(CMD_TbmWrite)
	PUT_ULONG(hubAddr)
	PUT_ULONG(addr)
	PUT_ULONG(value)
}


void CTestboard::Tbm1Write(int32_t hubAddr, int32_t addr, int32_t value)
{
    SEND_COMMAND(CMD_Tbm1Write)
	PUT_ULONG(hubAddr)
	PUT_ULONG(addr)
	PUT_ULONG(value)
}


void CTestboard::Tbm2Write(int32_t hubAddr, int32_t addr, int32_t value)
{
    SEND_COMMAND(CMD_Tbm2Write)
	PUT_ULONG(hubAddr)
	PUT_ULONG(addr)
	PUT_ULONG(value)
}


// === high level functions for wafer test ===============================

void CTestboard::DataRun_Init()
{
    SEND_COMMAND(CMD_DataRun_Init)
}

void CTestboard::DataRun_Done()
{
    SEND_COMMAND(CMD_DataRun_Done)
}

void CTestboard::DataRun_GetSample()
{
    SEND_COMMAND(CMD_DataRun_GetSample)
}

int32_t CTestboard::DataRun_ScanROC()
{
    SEND_COMMAND(CMD_DataRun_ScanROC);
    Flush();
    GET_UCHAR(ret, 0);
    return ret;
}

uint32_t CTestboard::DataRun_GetDataSize()
{
    SEND_COMMAND(CMD_DataRun_GetDataSize)
    Flush();
	GET_ULONG(size,0)
    return size;
}

void CTestboard::DataRun_GetData(uint32_t pos, uint16_t size,
		int16_t *buffer)
{
    SEND_COMMAND(CMD_DataRun_GetData)
	PUT_ULONG(pos);
    PUT_USHORT(size);
    Flush();
    usb.Read_SHORTS(buffer, size);
}


unsigned char CTestboard::test_PUC(unsigned char col, unsigned char row,
                                   unsigned char trim)
{
    SEND_COMMAND(CMD_test_PUC)
    PUT_UCHAR(col)
    PUT_UCHAR(row)
    PUT_UCHAR(trim)
    Flush();
    GET_UCHAR(level, 0)
    return level;
}


void CTestboard::testColPixel(int32_t col, int32_t trimbit, unsigned char *res)
{
    SEND_COMMAND(CMD_testColPixel)
    PUT_UCHAR(col)
    PUT_UCHAR(trimbit)
    Flush();
    usb.Read_UCHARS(res, ROC_NUMROWS);
}


bool CTestboard::GetLastDac(unsigned char count, int32_t &ldac)
{
    SEND_COMMAND(CMD_GetLastDac)
    PUT_UCHAR(count)
    Flush();
    GET_CHAR(res, 0)
	if (res) usb.Read_LONG(ldac);
    return res != 0;
}


bool CTestboard::ScanDac(unsigned char dac, unsigned char count,
		unsigned char min, unsigned char max, int16_t *ldac)
{
    SEND_COMMAND(CMD_ScanDac)
    PUT_UCHAR(dac)
    PUT_UCHAR(count)
    PUT_UCHAR(min)
    PUT_UCHAR(max)
    Flush();
    GET_CHAR(res, 0);
    if (res) usb.Read_SHORTS(ldac, max - min + 1);
    return res != 0;
}


// === module test functions ==========================================

bool CTestboard::GetPixel(int32_t x)
{
	uint32_t bytesRead;
    unsigned char sdata[3];
    sdata[0] = 0x8f;
    sdata[1] = 1;
    sdata[2] = (unsigned char)(x);
    Write(3, sdata);
    Flush();
    gDelay->Mdelay(50);
    Read(1, sdata, bytesRead);
    return sdata[0] != 0;
}


int32_t CTestboard::FindLevel()
{
	uint32_t bytesRead;
    unsigned char sdata[2];
    sdata[0] = 0x8f;
    sdata[1] = 2;
    Write(2, sdata);
    Flush();
    gDelay->Mdelay(50);
    Read(1, sdata, bytesRead);
    return sdata[0];
}


int32_t CTestboard::AoutLevel(int16_t position, int16_t nTriggers)
{
    SEND_COMMAND(CMD_aoutLevel)
    PUT_SHORT(position);
    PUT_SHORT(nTriggers);
    Flush();
    gDelay->Mdelay(50);
	int16_t result;
    usb.Read_SHORT(result);
    return (int)result;
}


int32_t CTestboard::CountReadouts(int32_t count, int32_t chipId)
{
    SEND_COMMAND(CMD_CountReadouts)
    PUT_SHORT(count);
    PUT_SHORT(chipId);
    Flush();
    gDelay->Mdelay(50);
    GET_SHORT(result, -1)
    return result;
}


int32_t CTestboard::AoutLevelChip(int16_t position, int16_t nTriggers, int32_t trim[], int32_t res[])
{
	int16_t trimShort[ROC_NUMROWS*ROC_NUMCOLS];
	int32_t i;
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) trimShort[i] = (int16_t)trim[i];
    SEND_COMMAND(CMD_aoutLevelChip)
    PUT_SHORT(position);
    PUT_SHORT(nTriggers);
    PUT_SHORTS(trimShort, ROC_NUMROWS * ROC_NUMCOLS);
    Flush();
    gDelay->Mdelay(50);
	int16_t sdata[ROC_NUMROWS*ROC_NUMCOLS] = {0};
    usb.Read_SHORTS(sdata, ROC_NUMROWS * ROC_NUMCOLS);
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) res[i] = sdata[i];
    return 1;
}


int32_t CTestboard::AoutLevelPartOfChip(int16_t position, int16_t nTriggers, int32_t trim[], int32_t res[], bool pxlFlags[])
{
	int16_t trimShort[ROC_NUMROWS*ROC_NUMCOLS];
	int32_t i;
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) trimShort[i] = (int16_t)trim[i];
    SEND_COMMAND(CMD_aoutLevelPartOfChip)
    PUT_SHORT(position);
    PUT_SHORT(nTriggers);
    PUT_SHORTS(trimShort, ROC_NUMROWS * ROC_NUMCOLS);
	int16_t pxlFlagsShort[ROC_NUMROWS*ROC_NUMCOLS];
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) pxlFlagsShort[i] = (pxlFlags[i] == true) ? 1 : 0;
    PUT_SHORTS(pxlFlagsShort, ROC_NUMROWS * ROC_NUMCOLS);
    Flush();
    gDelay->Mdelay(50);
	int16_t sdata[ROC_NUMROWS*ROC_NUMCOLS] = {0};
    usb.Read_SHORTS(sdata, ROC_NUMROWS * ROC_NUMCOLS);
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) res[i] = sdata[i];
    return 1;
}


int32_t CTestboard::ChipEfficiency(int16_t nTriggers, int32_t trim[], double res[])
{
	int16_t trimShort[ROC_NUMROWS*ROC_NUMCOLS];
	int32_t i;
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) trimShort[i] = (int16_t)trim[i];
    SEND_COMMAND(CMD_ChipEfficiency)
    PUT_SHORT(nTriggers);
    PUT_SHORTS(trimShort, ROC_NUMROWS * ROC_NUMCOLS);
    Flush();
    gDelay->Mdelay(50);

	int16_t sdata[ROC_NUMROWS*ROC_NUMCOLS] = {0};
    usb.Read_SHORTS(sdata, ROC_NUMROWS * ROC_NUMCOLS);
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++)
    {
        res[i] = (double)sdata[i] / nTriggers;
    }
    return 1;
}


int32_t CTestboard::MaskTest(int16_t nTriggers, int16_t res[])
{
    SEND_COMMAND(CMD_MaskTest)
    PUT_SHORT(nTriggers);
    Flush();
    gDelay->Mdelay(50);
    usb.Read_SHORTS(res, ROC_NUMROWS * ROC_NUMCOLS);
    return 1;
}


void CTestboard::TrimAboveNoise(int16_t nTrigs, int16_t thr, int16_t mode, int16_t result[])
{
    SEND_COMMAND(CMD_TrimAboveNoise);
    PUT_SHORT(nTrigs);
    PUT_SHORT(thr);
    PUT_SHORT(mode);
    Flush();
    gDelay->Mdelay(50);
    usb.Read_SHORTS(result, ROC_NUMROWS * ROC_NUMCOLS);
}


void CTestboard::DoubleColumnADCData(int32_t doubleColumn, int16_t data[], int32_t readoutStop[])
{
    SEND_COMMAND(CMD_DoubleColumnADCData)
    PUT_SHORT(doubleColumn)
    Flush();
    gDelay->Mdelay(50);

	uint16_t wordsread = 0;
    if (!usb.Read_USHORT(wordsread)) { wordsread = 0; return;}

    gDelay->Mdelay(50);
	int16_t sdata[wordsread];
    usb.Read_SHORTS(sdata, wordsread);

	int32_t i;
	for (i = 0; i < 2*ROCNUMROWS; i++) readoutStop[i] = sdata[i];
	for (i = 0; i < readoutStop[2*ROCNUMROWS - 1]; i++) data[i] = sdata[i+2*ROCNUMROWS];
}


int32_t CTestboard::PixelThreshold(int32_t col, int32_t row, int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim)
{
    SEND_COMMAND(CMD_pixelThreshold)
    PUT_SHORT(col);
    PUT_SHORT(row);
    PUT_SHORT(start);
    PUT_SHORT(step);
    PUT_SHORT(thrLevel);
    PUT_SHORT(nTrig);
    PUT_SHORT(dacReg);
    PUT_SHORT(xtalk);
    PUT_SHORT(cals);
    PUT_SHORT(trim);
    Flush();
    gDelay->Mdelay(50);
    GET_SHORT(result, 7777)
    return result;
}


int32_t CTestboard::ChipThreshold(int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim[], int32_t res[])
{
	int32_t i;
	int16_t trimShort[ROC_NUMROWS*ROC_NUMCOLS];
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) trimShort[i] = (int16_t)trim[i];
    SEND_COMMAND(CMD_chipThreshold)
    PUT_SHORT(start);
    PUT_SHORT(step);
    PUT_SHORT(thrLevel);
    PUT_SHORT(nTrig);
    PUT_SHORT(dacReg);
    PUT_SHORT(xtalk);
    PUT_SHORT(cals);
    PUT_SHORTS(trimShort, ROC_NUMROWS * ROC_NUMCOLS);
    Flush();
    gDelay->Mdelay(50);
	int16_t sdata[ROC_NUMROWS*ROC_NUMCOLS] = {0};
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) sdata[i] = -1;
    bool result = usb.Read_SHORTS(sdata, ROC_NUMROWS * ROC_NUMCOLS);
	for (i = 0; i < ROC_NUMROWS*ROC_NUMCOLS; i++) res[i] = sdata[i];
    if (result) return 1; else return 0;
}


int32_t CTestboard::SCurve(int32_t nTrig, int32_t dacReg, int32_t threshold, int32_t res[])
{
    SEND_COMMAND(CMD_SCurve)
	PUT_LONG(nTrig);
	PUT_LONG(dacReg);
	PUT_SHORT((int16_t)threshold);
    Flush();
    gDelay->Mdelay(50);
	int16_t sdata[256] = {0};
    usb.Read_SHORTS(sdata, 256);
    //  GET_SHORTS(sdata, bytesRead, 256, 0);
	for (int32_t i = 0; i < 256; i++) res[i] = sdata[i];
    return 1;
}


int32_t CTestboard::SCurveColumn(int32_t column, int32_t nTrig, int32_t dacReg, int32_t thr[], int32_t trims[], int32_t chipId[], int32_t res[])
{
	int32_t i;
    SEND_COMMAND(CMD_SCurveColumn)
	PUT_LONG(column);
	PUT_LONG(nTrig);
	PUT_LONG(dacReg);

	int16_t threshold[16*ROC_NUMROWS], trim[16*ROC_NUMROWS], chipIds[16];
	for (i = 0; i < 16*ROC_NUMROWS; i++) threshold[i] = (int16_t)thr[i];
	PUT_SHORTS(threshold, 16*ROC_NUMROWS);
	for (i = 0; i < 16*ROC_NUMROWS; i++) trim[i] = (int16_t)trims[i];
	PUT_SHORTS(trim, 16*ROC_NUMROWS);
    for (i = 0; i < 16; i++) chipIds[i] = (int16_t)chipId[i];
    PUT_SHORTS(chipIds, 16);

    Flush();
    gDelay->Mdelay(50);

    ReadFPGAData(ROCNUMROWS * 16 * 32, res);
    return 1;

}


void CTestboard::ADCRead(int16_t buffer[], uint16_t &wordsread, int16_t nTrig)
{
    SEND_COMMAND(CMD_ADCRead)
    PUT_SHORT(nTrig);
    Flush();
    gDelay->Mdelay(150);
    if (!usb.Read_USHORT(wordsread)) { wordsread = 0;}
    if (wordsread)
    {
        usb.Read_SHORTS(buffer, wordsread);
    }
}


void CTestboard::DacDac(int32_t dac1, int32_t dacRange1, int32_t dac2, int32_t dacRange2, int32_t nTrig, int32_t result[])
{
    SEND_COMMAND(CMD_DacDac)
    PUT_SHORT(dac1);
    PUT_SHORT(dacRange1);
    PUT_SHORT(dac2);
    PUT_SHORT(dacRange2);
    PUT_SHORT(nTrig);
    Flush();
    gDelay->Mdelay(50);
    ReadFPGAData(dacRange1 * dacRange2, result);
}


void CTestboard::PHDac(int32_t dac, int32_t dacRange, int32_t nTrig, int32_t position, int16_t result[])
{
    SEND_COMMAND(CMD_PHDac)
    PUT_SHORT(dac);
    PUT_SHORT(dacRange);
    PUT_SHORT(nTrig);
    PUT_SHORT(position);
    Flush();
    gDelay->Mdelay(50);
    usb.Read_SHORTS(result, dacRange);
}


void CTestboard::AddressLevels(int32_t position, int32_t result[])
{
	int16_t sdata[4000];
    SEND_COMMAND(CMD_AddressLevels)
    PUT_SHORT(position);
    Flush();
    gDelay->Mdelay(50);
    usb.Read_SHORTS(sdata, 4000);
	for (int32_t i = 0; i < 4000; i++) result[i] = sdata[i];
}


void CTestboard::TBMAddressLevels(int32_t result[])
{
	int16_t sdata[4000];
    SEND_COMMAND(CMD_TBMAddressLevels)
    Flush();
    gDelay->Mdelay(50);
    usb.Read_SHORTS(sdata, 4000);
	for (int32_t i = 0; i < 4000; i++) result[i] = sdata[i];
}


void CTestboard::ReadData(int32_t position, int32_t size, int32_t result[])
{
    if (size > 32767) size = 32767;
	int16_t sdata[32768];
    SEND_COMMAND(CMD_ReadData)
	PUT_LONG(position);
	PUT_LONG(size);
    Flush();
    gDelay->Mdelay(50);
    usb.Read_SHORTS(sdata, size);
	for (int32_t i = 0; i < size; i++) result[i] = sdata[i];
}


void CTestboard::ReadFPGAData(int32_t size, int32_t result[])
{
	int32_t toRead = size;
	int32_t data[32767];
	int32_t position = 0;
	int32_t i;
    do
    {
        if (toRead > 32767)
        {
            ReadData(position, 32767, data);
			for (i = 0; i < 32767; i++) result[position+i] = data[i];
            position += 32767;
            toRead -= 32767;
        }
        else
        {
            ReadData(position, toRead, data);
			for (i = 0; i < toRead; i++) result[position+i] = data[i];
            position += toRead;
            toRead = 0;
        }
    }
    while (toRead > 0);
//  	for (int32_t i = 0; i < size; i++) printf("%i ", result[i]);
}


void CTestboard::SetEmptyReadoutLength(int32_t emptyReadoutLength)
{
    SEND_COMMAND(CMD_SetEmptyReadoutLength)
    PUT_SHORT(emptyReadoutLength);
    Flush();
}


void CTestboard::SetEmptyReadoutLengthADC(int32_t emptyReadoutLengthADC)
{
    SEND_COMMAND(CMD_SetEmptyReadoutLengthADC)
    PUT_SHORT(emptyReadoutLengthADC);
    Flush();
}


void CTestboard::SetTbmChannel(int32_t tbmChannel)
{
    SEND_COMMAND(CMD_SetTbmChannel)
    PUT_SHORT(tbmChannel);
    Flush();
}


void CTestboard::SetDTL(int32_t value)
{
    SEND_COMMAND(CMD_SetDTL)
    PUT_SHORT(value);
    Flush();
}


void CTestboard::SetNRocs(int32_t value)
{
    nRocs = value;
    SEND_COMMAND(CMD_SetNRocs)
    PUT_SHORT(value);
    Flush();
}


void CTestboard::SetHubID(int32_t value)
{
    SEND_COMMAND(CMD_SetHubID)
    PUT_SHORT(value);
    Flush();
}


void CTestboard::SetEnableAll(int32_t value)
{
    SEND_COMMAND(CMD_SetEnableAll)
    PUT_SHORT(value);
    Flush();
}


void CTestboard::SetAoutChipPosition(int32_t value)
{
    SEND_COMMAND(CMD_SetAoutChipPosition)
    PUT_SHORT(value);
}

void CTestboard::GetColPulseHeight(unsigned char col, unsigned char count,
			int16_t data[])
{
	SEND_COMMAND(CMD_GetColPulseHeight)
	PUT_UCHAR(col)
	PUT_UCHAR(count)
	Flush();
	usb.Read_SHORTS(data,ROC_NUMROWS);
}


void CTestboard::Scan1D(unsigned char vx,
			unsigned char xmin, unsigned char xmax,	char xstep,
			unsigned char rep, uint32_t usDelay, unsigned char res[])
{
	SEND_COMMAND(CMD_Scan1D)
	PUT_UCHAR(vx)
	PUT_UCHAR(xmin)
	PUT_UCHAR(xmax)
	PUT_CHAR(xstep)
	PUT_UCHAR(rep)
	PUT_ULONG(usDelay)
	Flush();
	int32_t count = int(xmax-xmin)/xstep;
	if (count<0) return;
	if (count>256) count = 256;
	usb.Read_UCHARS(res,count);
}


void CTestboard::BumpTestColPixel(unsigned char col, unsigned char res[])
{
	SEND_COMMAND(CMD_BumpTestColPixel)
	PUT_UCHAR(col)
	Flush();
	usb.Read_UCHARS(res,ROC_NUMROWS);
}


void CTestboard::BumpTestColRef(unsigned char col, unsigned char res[])
{
	SEND_COMMAND(CMD_BumpTestColRef)
	PUT_UCHAR(col)
	Flush();
	usb.Read_UCHARS(res,ROC_NUMROWS);
}


void CTestboard::DacDac(int16_t dac1, int16_t dacRange1, int16_t dac2, int16_t dacRange2,
	int16_t nTrig, int16_t res[], int16_t rocpos)
{
	SEND_COMMAND(CMD_SetAoutChipPosition)
	PUT_SHORT(rocpos)

	SEND_COMMAND(CMD_DacDac);
	PUT_SHORT(dac1)
	PUT_SHORT(dacRange1)
	PUT_SHORT(dac2)
	PUT_SHORT(dacRange2)
	PUT_SHORT(nTrig)
	mDelay(1000);

	int32_t size = dacRange1*dacRange2;
	while (size>0)
	{
		int32_t blocksize = 4096;
		if (size < blocksize) blocksize = size;

		SEND_COMMAND(CMD_ReadData);
		PUT_LONG(0)
		PUT_LONG(blocksize)
		usb.Flush();
		usb.Read_SHORTS(res, blocksize);

		size -= blocksize;
		res  += blocksize;
	}
}

// =======================================================================

int32_t CTestboard::demo(int16_t x)
{
    SEND_COMMAND(CMD_demo)
    PUT_SHORT(x)
    usb.Flush();
	GET_LONG(ret,0)
    return ret;
}


void CTestboard::ScanAdac(uint16_t chip, unsigned char dac,
                          unsigned char min, unsigned char max, char step,
      unsigned char rep, uint32_t usDelay, unsigned char res[])
{
    SEND_COMMAND(CMD_ScanAdac)
    PUT_USHORT(chip)
    PUT_UCHAR(dac)
    PUT_UCHAR(min)
    PUT_UCHAR(max)
    PUT_CHAR(step)
    PUT_UCHAR(rep)
	PUT_ULONG(usDelay)
    Flush();
	int32_t count = int(max-min)/step;
    if (count < 0) return;
    if (count > 256) count = 256;
    gDelay->Mdelay(50);
    usb.Read_UCHARS(res, count);
}

void CTestboard::CdVc(uint16_t chip, unsigned char wbcmin, unsigned char wbcmax, unsigned char vcalstep,
          unsigned char cdinit, uint16_t &lres, uint16_t res[])
{

    SEND_COMMAND(CMD_CdVc);
    PUT_USHORT(chip)
    PUT_UCHAR(wbcmin)
    PUT_UCHAR(wbcmax)
    PUT_UCHAR(vcalstep)
    PUT_UCHAR(cdinit)
    PUT_USHORT(lres)
    Flush();
    unsigned short lres2;
    gDelay->Mdelay(50);
    if (usb.Read_USHORT(lres2)) {
        usb.Read_USHORTS(res, lres2);
    }
    lres = lres2;
}

char CTestboard::CountAllReadouts(int32_t nTrig, int32_t counts[], int32_t amplitudes[])
{
    SEND_COMMAND(CMD_CountAllReadouts)
	PUT_SHORT(nTrig)
    Flush();
	GET_CHAR(res, -1)
	if (!usb.Read_LONGS(counts,    16)) return -1;
	if (!usb.Read_LONGS(amplitudes,16)) return -1;
    return res;
}


uint16_t CTestboard::GetReg41()
{
    SEND_COMMAND(CMD_GetReg41)
    Flush();
    GET_USHORT(rvalue, 0)
    return rvalue;
}

void CTestboard::TBMEmulatorOn()
{
    SEND_COMMAND(CMD_TBMEmulatorOn)
    Flush();
}

void CTestboard::TBMEmulatorOff()
{
    SEND_COMMAND(CMD_TBMEmulatorOff)
    Flush();
}
