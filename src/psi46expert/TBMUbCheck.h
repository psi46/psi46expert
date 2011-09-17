//
// Author: Christian Veelken
//
// Tests ultra-black levels of tbms
// 

#ifndef TBMUBCHECK
#define TBMUBCHECK

#include "Test.h"

class TBMUbCheck : public Test
{
 public:
  TBMUbCheck(TestRange* testRange, TestParameters* testParameters, TBInterface* aTBInterface);

  virtual void ReadTestParameters(TestParameters *parameters);	
  virtual void ModuleAction();

 protected:
  int ubTarget;
};

#endif

