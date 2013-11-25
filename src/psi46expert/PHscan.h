#ifndef PHSCAN
#define PHSCAN

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>

class PHscan : public Test
{

 public:
  PHscan( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );
	
  virtual void ReadTestParameters(TestParameters *testParameters);
  virtual void RocAction();
  virtual void PixelAction();
	
  void PhDac(char *dacName);

 protected:

  int nTrig, mode;
  TH2D *map; // for mode 0

};

#endif
