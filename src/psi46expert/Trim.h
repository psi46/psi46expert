//
// Author: Peter Trüb
//
// Trim functions
// 

#ifndef TRIM
#define TRIM

#include "Test.h"
#include <TH2D.h>
#include "ThresholdMap.h"

class Trim : public Test
{

public:
	Trim(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	int AdjustVtrim();
	void AddMap(TH2D* calMap);
	TH2D* TrimStep(int correction, TH2D *calMapOld, TestRange* aTestRange);

		
protected:

	int vthrComp, doubleWbc, nTrig, vcal;
	ThresholdMap *thresholdMap;

};


#endif

