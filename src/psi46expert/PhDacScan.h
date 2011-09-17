// Pulse height dependency on different DACs

#ifndef PHDACSCAN
#define PHDACSCAN

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>


class PhDacScan : public Test
{

 public:
  PhDacScan() {};

  PhDacScan(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
  virtual void ReadTestParameters(TestParameters *testParameters);

  int FitStartPoint(TH1D *histo);
  int FitStopPoint(TH1D *histo, int fitStart);
  double FindLinearRange(TH1D *histo);
  double FindLowLinearRange(TH1D *histo);
  double QualityLowRange(TH1D *histo);
  double Quality(TH1D *histoLowRange, TH1D *histoHighRange);
  void DoDacScan();
  int PH(int vcal, TH1D *histo, TF1* fit);
	
 protected:

  int nTrig, mode, NumberOfSteps;
  double minPh;
  TF1 *fit; 
  TF1 *linFit;
  TF1 *pol2Fit;
};


#endif

