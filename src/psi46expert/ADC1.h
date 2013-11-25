#ifndef ADC1h
#define ADC1h

#include "Test.h"
#include <TH1D.h>
#include <TH2D.h>
#include <TProfile2D.h>

class ADC1 : public Test
{

 public:
  ADC1( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );

  virtual void ReadTestParameters( TestParameters *testParameters );
  virtual void RocAction();

 protected:

};

#endif
