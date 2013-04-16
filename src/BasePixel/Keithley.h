// To control the Keithley high voltage source

#ifndef KEITHLEY
#define KEITHLEY

class Keithley
{

public:
	Keithley();

	void Open();
	void Init();
	void Write(const char *string);
	void Read();
	void Measure(int targetVoltage, float &voltage, float &current, int delay);
	void SetVoltage(int voltage, int delay);
	void Command(const char *commandString);
	void GoLocal();
	void ShutDown();
	void ReadCurrent(float &voltage, float &current); 
	int  Tripped();
protected:

	int port;
	
};


#endif

