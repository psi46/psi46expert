//
// Author: Peter Trüb
//
// Trim functions
// 

#ifndef PHTEST
#define PHTEST

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>

class PHTest : public Test
{

public:
	PHTest(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	virtual void PixelAction();
	
	void PhDac(char *dacName);

	
	
protected:

	int nTrig, mode;
	TH2D *map;

};


#endif

