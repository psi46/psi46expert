//
// Author: C. Veelken
//
// 
// 

#ifndef THRCOMP_H
#define THRCOMP_H

#include "Test.h"


class ThrComp : public Test
{

 public:
 
	ThrComp(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);

	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	void RocActionAuxiliary(double data[], double dataMax[]);

 private:


};

#endif
