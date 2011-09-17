#ifndef CALDEL_H
#define CALDEL_H

#include "Test.h"
#include "TestRange.h"


class CalDelay : public Test
{

 public:
 
	CalDelay(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);

	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();

 protected:

	TObjArray* GetEfficiency(const char* testName, TestRange* testRange);

};

#endif
