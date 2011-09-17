//
// Author: Sarah Dambach
//
// pulse height - Vcal overview for different DACs
// 


#ifndef PHDACOVERVIEW
#define PHDACOVERVIEW

#include "Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

class PhDacOverview : public PhDacScan
{

 public:

  PhDacOverview(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();
  virtual void PixelAction();

  void DoDacScan();
  void DoVsfScan();

 protected:

  int NumberOfSteps;

};


#endif
