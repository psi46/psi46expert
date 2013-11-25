#ifndef EFF
#define EFF

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TProfile.h>

class Eff : public Test
{

 public:
  Eff( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();
  virtual void PixelAction();
	
 protected:

  int nTrig;

};

#endif
