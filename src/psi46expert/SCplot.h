#ifndef SCPLOT
#define SCPLOT

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>

class SCplot : public Test
{

 public:
  SCplot( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );
	
  virtual void ReadTestParameters( TestParameters *testParameters );
  virtual void RocAction();
  virtual void PixelAction();

 protected:

  int nTrig;

};

#endif
