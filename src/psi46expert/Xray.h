// Vcal calibration with xray

#ifndef XRAY
#define XRAY

#include "Test.h"

class Xray : public Test
{

public:
  Xray(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
  	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void ModuleAction();
  virtual void RocAction();
	
protected:
  
  int nTrig, vthrCompMin, vthrCompMax;
  double maxEff;
  TH1F *histo[MODULENUMROCS];
};


#endif

