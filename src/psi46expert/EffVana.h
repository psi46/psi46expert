
#ifndef EFFVANA
#define EFFVANA

#include "Test.h"

class EffVana : public Test
{

 public:
  EffVana(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();
	
 protected:

  int nTrig;
  double efficiency;

};

#endif
