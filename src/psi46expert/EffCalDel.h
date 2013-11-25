
#ifndef EFFCALDEL
#define EFFCALDEL

#include "Test.h"

class EffCalDel : public Test
{

 public:
  EffCalDel(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();
	
 protected:

  int nTrig;
  double efficiency;

};

#endif
