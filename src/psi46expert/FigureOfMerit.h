//
// Author: Sarah Dambach
//
// 2 DACs can be scanned as a function of various criteria
// 


#ifndef FIGUREOFMERIT
#define FIGUREOFMERIT

#include "Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

class FigureOfMerit : public PhDacScan
{

 public:

  FigureOfMerit(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface, int dac1, int dac2, int crit);
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();
  virtual void PixelAction();

  void DoDacDacScan();
  double Timewalk(int i, int k);
  int LinearRange(int i, int k);
  int PulseHeight(int i, int k);
  double LowLinearRange(int i, int k);
  double DacDacDependency();
  int FindFirstValue(short *result);
  int Threshold(int i, int k);
  
 protected:

  int dac1Start, dac1Stop, dac1Step, dac2Start, dac2Stop, dac2Step, firstDac, secondDac, criterion, testVcal;
  int dacValue1, dacValue2, index1, index2, bestQuality;

  TH1D *nor;


};


#endif
