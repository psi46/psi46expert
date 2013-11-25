#ifndef PHCC
#define PHCC

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TProfile.h>

class PHcc : public Test
{

 public:
  PHcc( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();
  virtual void PixelAction();

 protected:

  int nTrig, mode;

};

#endif
