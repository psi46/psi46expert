#ifndef PHVSCALS
#define PHVSCALS

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>

class PHvsCals : public Test
{

 public:
  PHvsCals( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );

  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();
  virtual void PixelAction();
	
  void PhDac(char *dacName);

 protected:

  int nTrig, mode;

};

#endif
