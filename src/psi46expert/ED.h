#ifndef EventDisplay
#define EventDisplay

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>

class RawPacketDecoder;

class ED : public Test
{

 public:
  ED( TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface );

  virtual void ReadTestParameters( TestParameters *testParameters );
  virtual void RocAction();
  virtual void PixelAction();

 protected:

  TH2D *map;

  //RawPacketDecoder *gDecoder;

};

#endif
