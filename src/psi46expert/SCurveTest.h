// SCurve measurements

#ifndef SCURVETEST
#define SCURVETEST

#include "Test.h"
#include <TH2D.h>

class SCurveTest : public Test
{

public:
	SCurveTest(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void ModuleAction();
	virtual void RocAction();
	virtual void DoubleColumnAction();
		
protected:

	int nTrig, mode, vthr, vcal, sCurve[16*ROCNUMROWS*256];
	int dacReg;
	char *mapName;
	TH2D *map[MODULENUMROCS];
	bool testDone;
	FILE *file[MODULENUMROCS];

};


#endif

