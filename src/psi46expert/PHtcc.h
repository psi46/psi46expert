#ifndef PHTCC
#define PHTCC

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TProfile.h>
#include <TProfile2D.h>

class PHtcc : public Test
{

 public:
  PHtcc( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );
	
  virtual void ReadTestParameters( TestParameters *testParameters );
  virtual void RocAction();
  virtual void PixelAction();

 protected:

  int nTrig, mode;

};

#endif
