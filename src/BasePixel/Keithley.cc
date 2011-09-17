#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <fcntl.h>

#include <iostream>

#include <string.h>  /* String function definitions */
#include <errno.h>   /* Error number definitions */
#include <termios.h> /* POSIX terminal control definitions */

#include "BasePixel/Keithley.h"
#include "interface/Log.h"
#include "interface/Delay.h"

Keithley::Keithley()
{
}


void Keithley::Open()
{
	port = open("/dev/ttyS0", O_RDWR);
	if ( port == -1 )
	{
    psi::LogInfo() << "[Keithley] Error: Serial port can not be opened." << psi::endl;
	}

	struct termios options;
	
	// Set the baud rates to 9600...
	cfsetispeed(&options, B9600);
	cfsetospeed(&options, B9600);
	// Enable the receiver and set local mode...
	options.c_cflag |= (CLOCAL | CREAD);

	options.c_cflag &= ~PARENB; // no parity
	options.c_cflag &= ~CSTOPB; // one stop bit
	options.c_cflag &= ~CSIZE;
	options.c_cflag |= CS8; // eight data bits
	
	options.c_cc[VEOF] = char(13);/* '\n';*/
	options.c_cc[VEOL] = char(13);/* '\n';*/
	options.c_cc[VSTART] = char(13);/* '\n';*/

	tcsetattr(port, TCSANOW, &options);
}


void Keithley::Write(char *string)
{
	char buffer[1000];
	int length = strlen(string);
	for (int i = 0; i < length; i++)
	{
		buffer[i] = string[i];
	}
  psi::LogInfo() << "[Keithley] Writing: " << string << psi::endl;

	write(port, buffer, length);
	sleep(1);
}


void Keithley::Read()
{
	char buffer[1000];
	int length = read( port, buffer, 1000);
  psi::LogInfo() << "[Keithley] Read " << length << " bytes: " << buffer
                 << psi::endl;
}


void Keithley::GoLocal()
{
	char string[1000];
	sprintf(string, ":SYST:LOC\n");
	Write(string);
}


void Keithley::ShutDown()
{
	char string[1000];
	sprintf(string, ":SYST:KEY 24\n");
	Write(string);
	GoLocal();
}


void Keithley::Measure(int targetVoltage, float &voltage, float &current, int delay)
{
	char buffer[1000];
		
	//sleep(1);
        //Write("*CLR");
	
	SetVoltage(targetVoltage, delay);
        //sleep(1);
	Write("READ?\n");
	//sleep(1);
	int length = read(port, buffer, 1000);
  psi::LogInfo() << "[Keithley] Read " << length << " bytes: " << buffer
                 << psi::endl;

	sscanf(buffer, "%e,%e", &voltage, &current);
	
}


int Keithley::Tripped()
{
        int trip;
	char buffer[1000];
	Write(":SENS:CURR:PROT:TRIP?\n");
	sleep(2);
	read(port, buffer, 1000);
	sscanf(buffer, "%i", &trip);
	return trip;
}



void Keithley::ReadCurrent(float &voltage, float &current)
{
	char buffer[1000];
	Write(":READ?\n");
	sleep(2);
	read(port, buffer, 1000);
	sscanf(buffer, "%e,%e", &voltage, &current);
}


void Keithley::SetVoltage(int voltage, int delay)
{
	char string[100];
	sprintf(string, "SOUR:VOLT:IMM:AMPL -%i\n", voltage);
	Write(string);
	sleep(delay);
}

void Keithley::Command(char *commandString)
{
	Write(commandString);
	sleep(1);
	Read();
}


void Keithley::Init()
{
  psi::LogInfo() << "[Keithley] Initialization." << psi::endl;

	Write("*RST\n");
	Write(":SYST:BEEP:STAT OFF\n");
	Write(":ROUT:TERM REAR\n");
	Write(":SOUR:VOLT:MODE FIX\n");
	//Write(":FORM:ELEM VOLT,CURR\n");
	Write(":FUNC:CONC ON\n");
	Write(":SENS:AVER:TCON REP\n");
	Write(":SENS:AVER:COUNT 2\n");	
	Write(":SENS:AVER:STAT ON\n");
	Write(":CURR:PROT:LEV 100E-6\n");
	Write(":SENS:CURR:RANG 20E-6\n");
	Write(":SENS:CURR:NPLC 10\n");
	Write("SOUR:VOLT:IMM:AMPL -150\n");
	//Write(":FORM:ELEM STAT\n");
	Write(":FORM:ELEM VOLT,CURR\n");
	Write(":OUTPUT 1\n");	
	//Write("*CLS");
	
}

