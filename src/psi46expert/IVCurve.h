#ifndef IVCURVE
#define IVCURVE

#include "Test.h"

class Keithley;

class IVCurve : public Test
{

public:
	IVCurve(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void ModuleAction();
	
protected:

	int voltStep, voltStart, voltStop, delay;
	Keithley *keithley;

};


#endif

