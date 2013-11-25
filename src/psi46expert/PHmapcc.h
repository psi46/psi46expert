#ifndef PHMAPCC
#define PHMAPCC

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TProfile2D.h>
#include <TProfile.h>

class PHmapcc : public Test
{

 public:
  PHmapcc( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );

  virtual void ReadTestParameters( TestParameters *testParameters );
  virtual void RocAction();

 protected:

  int nTrig;

};

#endif
