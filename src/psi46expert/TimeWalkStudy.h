//
// Author: Lukas Wehrli
//
// Time Walk Studies
//  

#ifndef TIMEWALKSTUDIES
#define TIMEWALKSTUDIES

#include "Test.h"
#include "BasePixel/GlobalConstants.h"
#include <TF1.h>

class TimeWalkStudy : public Test
{
 public: 
  TimeWalkStudy(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
  virtual void ReadTestParameters(TestParameters *testParameters);

  virtual void ModuleAction();
  virtual void RocAction();
  
  void CalDelDeltaT();
  void GetPowerSlope();
  double TimeWalk(int vcalStep);
  int FindNewVana();
  void SetThreshold(int vcal);
  int GetThreshold();

//   void TimeWalk16Chips();
//   void TimeWalk16Pixel();

  protected: 

  TF1 *fit;

  double powerSlope, calDelDT, zeroCurrent, meanShift, twBefore[MODULENUMROCS], twAfter[MODULENUMROCS];
  int vcalThreshold, vana[MODULENUMROCS];
};

#endif


