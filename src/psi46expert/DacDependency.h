//
// Author: Peter Trüb
//
// Trim functions
// 

#ifndef DACDEPENDENCY
#define DACDEPENDENCY

#include "Test.h"
#include <TH2D.h>

class DacDependency : public Test
{

public:
	DacDependency(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void PixelAction();
	void SetDacs(int d1, int d2, int range1, int range2);
	void SetNTrig(int nTrig);
		
protected:

	int dac1, dac2, nTrig, dacRange1, dacRange2;
	

};


#endif

