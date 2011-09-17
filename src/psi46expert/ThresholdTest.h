//
// Author: Peter Trüb
//
// Trim functions
// 

#ifndef THRESHOLDTEST
#define THRESHOLDTEST

#include "Test.h"
#include <TH2D.h>

class ThresholdTest : public Test
{

public:
	ThresholdTest(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
		
protected:

	int nTrig, mode, vthr, vcal;
	char *dacName;
	TH2D *map;

};


#endif

