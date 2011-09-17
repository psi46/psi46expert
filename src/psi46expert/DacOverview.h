#ifndef DACOVERVIEW
#define DACOVERVIEW

#include "Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

class DacOverview : public PhDacScan
{

 public:

  DacOverview(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();

  void DoDacScan();

 protected:

  int NumberOfSteps, DacType;

};


#endif
