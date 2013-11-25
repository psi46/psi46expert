// pulse height - Vcal overview for different DACs

#ifndef DACSCAN
#define DACSCAN

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

class DACscan : public Test
{

 public:

  DACscan( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters( TestParameters *testParameters );
  virtual void RocAction();
  virtual void PixelAction();

 protected:

  int nTrig;

};

#endif
