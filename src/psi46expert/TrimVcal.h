//
// Author: Peter Trüb
//
// Trim functions
// 

#ifndef TRIMVCAL
#define TRIMVCAL

#include "Test.h"
#include <TH2D.h>
#include "ThresholdMap.h"

class TrimVcal : public Test
{

public:
	TrimVcal(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	void AddMap(TH2D* calMap);

		
protected:

	int nTrig;
	ThresholdMap *thresholdMap;

};


#endif

