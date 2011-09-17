// Ensures that the pulse height is below the ultra black

#ifndef UBCHECK_H
#define UBCKECK_H

#include "Test.h"


class UbCheck : public Test
{

 public:
 
	UbCheck();
	UbCheck(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);

	void Initialize();	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	void Measure(int nTriggers);
	int Ultrablack();
	void AdjustOpR0();

 private:

	int nTrig, minPixel;

};

#endif
