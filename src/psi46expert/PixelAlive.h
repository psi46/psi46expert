// Pixel alive test, analog testboard version

#ifndef PIXELALIVE
#define PIXELALIVE

#include "Test.h"

class PixelAlive : public Test
{

public:
	PixelAlive(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	
protected:

	int nTrig;
	double efficiency;

};


#endif

