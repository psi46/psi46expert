#ifndef PHMAP
#define PHMAP

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>

class PHmap : public Test
{

 public:
  PHmap( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );

  virtual void ReadTestParameters( TestParameters *testParameters );
  virtual void RocAction();

 protected:

  int nTrig;
  TH2D *map, *map4;
  TH1D *his, *his4;

};

#endif
