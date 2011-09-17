#ifndef SCURVETESTBEAM
#define SCURVETESTBEAM

#include "Test.h"
#include <TH2D.h>

#include "BasePixel/DecodedReadout.h"

class RawPacketDecoder;

class SCurveTestBeam : public Test
{

public:
	SCurveTestBeam(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	virtual void PixelAction();
		
protected:
	DecodedReadoutModule decodedModuleReadout;
	int nTrig, mode, vthr, vcal, sCurve[256];
	char *dacName;
	TH2D *map;
	RawPacketDecoder *gDecoder;
};


#endif

