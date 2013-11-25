#ifndef ADCMAP
#define ADCMAP

#include "Test.h"
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile2D.h>

class ADCmap : public Test
{

 public:
  ADCmap( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );

  virtual void ReadTestParameters( TestParameters *testParameters );
  virtual void RocAction();

 protected:

  int nTrig;

};

#endif
