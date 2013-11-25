#ifndef RBSCAN
#define RBSCAN

#include "Test.h"
#include <TH1D.h>
#include <TProfile.h>

class RBscan : public Test
{

 public:
  RBscan( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();

 protected:

};

#endif
