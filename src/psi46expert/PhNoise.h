//
// Author: Peter Trüb
//
// Measure the noise on the pulse height in the analog out signal
// 

#ifndef PHNOISE_H
#define PHNOISE_H

#include <TH1D.h>
#include "Test.h"

class PhNoise : public Test
{

public:
	PhNoise(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	virtual ~PhNoise();

        virtual void ModuleAction();
	virtual void RocAction();
	
protected:
	unsigned short count;
	short data[FIFOSIZE];

        static const int nReadouts = 1000;
	static bool debug;
};

#endif

