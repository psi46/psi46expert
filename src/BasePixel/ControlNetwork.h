//
// Author: Peter Trüb
//
// Provides ControlNetwork functionalities, mainly a module container
// 


#ifndef CONTROLNETWORK
#define CONTROLNETWORK

#include "Module.h"
#include "TBInterface.h"
#include "SysCommand.h"


class ControlNetwork
{

public:
	inline ControlNetwork() {};
	ControlNetwork(TBInterface* aTbInterface, ConfigParameters *configParameters);
	virtual ~ControlNetwork();
	void Execute(SysCommand &command);
	
	void Initialize();
	
	void WriteTrimConfiguration(char* filename);
	void ReadTrimConfiguration(char* filename);
	void WriteDACParameterFile(const char* filename);
	void ReadDACParameterFile(char* filename);

	int NModules();	
	Module* GetModule(int iModule);
	void SetDAC(int iModule, int iRoc, int reg, int value);
	void SetTBM(int iModule, int iRoc, int reg, int value);
	void SetTrim(int iModule, int iRoc, int iCol, int iRow, int trimBit);
	
protected:

	int nModules;
	Module* module[CONTROLNETWORKNUMMODULES];
	
	TBInterface *tbInterface;
	ConfigParameters *configParameters;
	
};


#endif

