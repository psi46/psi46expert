// Class provides basic functionalities to use the USB interface

#ifndef USB_H
#define USB_H

#include "ftd2xx.h"


#define USBWRITEBUFFERSIZE  150000
#define USBREADBUFFERSIZE   150000


class CUSB
{
	bool isUSB_open;
	FT_HANDLE ftHandle;
	FT_STATUS ftStatus;

	unsigned int enumPos, enumCount;

	unsigned int m_posW;
	unsigned char m_bufferW[USBWRITEBUFFERSIZE];

	unsigned int m_posR, m_sizeR;
	unsigned char m_bufferR[USBREADBUFFERSIZE];

	bool FillBuffer(unsigned int minBytesToRead);

public:
	CUSB()
	{
		m_posR = m_sizeR = m_posW = 0;
		isUSB_open = false;
		ftHandle = 0; ftStatus = 0;
		enumPos = enumCount = 0;
	}
	~CUSB() { Close(); }
	int GetLastError() { return ftStatus; }
	static const char* GetErrorMsg(int error);
	bool EnumFirst(unsigned int &nDevices);
	bool EnumNext(char name[]);
	bool Open(char serialNumber[]);
	void Close();
	bool Connected() { return isUSB_open; };
	bool Write(unsigned int bytesToWrite, const void *buffer);
	bool Flush();
	bool Read(unsigned int bytesToRead, void *buffer, unsigned int &bytesRead);
	bool _Read(void *buffer, unsigned int bytesToRead)
	{
		unsigned int bytesRead;
		if (!Read(bytesToRead, (unsigned char *)buffer, bytesRead)) return false;
		return bytesRead == bytesToRead;
	}
	bool _Write(const void *buffer, unsigned int bytesToWrite)
	{ return Write(bytesToWrite, buffer); }

	bool Clear();


	// read methods

	bool Read_CHAR(char &x) { return _Read(&x, sizeof(char)); }

	bool Read_CHARS(char *x, unsigned short count)
	{ return _Read(x, count*sizeof(char)); }

	bool Read_UCHAR(unsigned char &x)	{ return _Read(&x, sizeof(char)); }

	bool Read_UCHARS(unsigned char *x, unsigned int count)
	{ return _Read(x, count*sizeof(char)); }

	bool Read_SHORT(short &x)
	{ return _Read((unsigned char *)(&x), sizeof(short)); }

	bool Read_SHORTS(short *x, unsigned short count)
  	{ return _Read(x, count*sizeof(short)); }

	bool Read_USHORT(unsigned short &x)
	{ return _Read((unsigned char *)(&x), sizeof(short)); }

	bool Read_USHORTS(unsigned short *x, unsigned short count)
  	{ return _Read(x, count*sizeof(short)); }

	bool Read_INT(int &x)
	{ return _Read((unsigned char *)(&x), sizeof(int)); }

	bool Read_INTS(int *x, unsigned short count)
  	{ return _Read(x, count*sizeof(int)); }

	bool Read_UINT(unsigned int &x)
	{ return _Read((unsigned char *)(&x), sizeof(int)); }

	bool Read_UINTS(unsigned int *x, unsigned short count)
  	{ return _Read(x, count*sizeof(int)); }

	bool Read_String(char *s, unsigned short maxlength);


	// -- write methods

	bool Write_CHAR(char x) { return _Write(&x, sizeof(char)); }

	bool Write_CHARS(const char *x, unsigned short count)
	{ return _Write(x, count*sizeof(char)); }

	bool Write_UCHAR(const unsigned char x) { return _Write(&x, sizeof(char)); }

	bool Write_UCHARS(const unsigned char *x, unsigned int count)
	{ return _Write(x, count*sizeof(char)); }

	bool Write_SHORT(const short x) { return _Write(&x, sizeof(short)); }

	bool Write_SHORTS(const short *x, unsigned short count)
  	{ return _Write(x, count*sizeof(short)); }

	bool Write_USHORT(const unsigned short x)
	{ return _Write(&x, sizeof(short)); }

	bool Write_USHORTS(const unsigned short *x, unsigned short count)
  	{ return _Write(x, count*sizeof(short)); }

	bool Write_INT(const int x) { return _Write(&x, sizeof(int)); }

	bool Write_INTS(const int *x, unsigned short count)
  	{ return _Write(x, count*sizeof(int)); }

	bool Write_UINT(const unsigned int x) { return _Write(&x, sizeof(int)); }

	bool Write_UINTS(const unsigned int *x, unsigned short count)
  	{ return _Write(x, count*sizeof(int)); }

	bool Write_String(const char *s);
};

#endif
