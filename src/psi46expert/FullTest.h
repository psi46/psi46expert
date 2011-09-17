// Full test of a module

#ifndef FULLTEST
#define FULLTEST

#include "Test.h"

class FullTest : public Test
{

public:
	FullTest(ConfigParameters *configParameters, TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface,int opt);
	int Scurve;
	virtual void RocAction();
	virtual void ModuleAction();
	void DoTemperatureTest();
		
protected:

	TestParameters *testParameters;


};


#endif

