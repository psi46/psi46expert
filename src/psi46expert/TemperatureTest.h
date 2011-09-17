//
// Author: Peter Trüb
//
// Trim functions
// 

#ifndef TEMPERATURE
#define TEMPERATURE

#include "Test.h"

class TemperatureTest : public Test
{

public:
	TemperatureTest(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	
protected:

	int nTrig;
};


#endif

