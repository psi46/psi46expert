// Pixel alive test, analog testboard version

#ifndef ADDRESSDECODING
#define ADDRESSDECODING

#include "Test.h"

class RawPacketDecoder;

class AddressDecoding : public Test
{

public:
	AddressDecoding(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface,bool debug=false);
	
	virtual void RocAction();
	virtual void DoubleColumnAction();
	void AnalyseResult(int pixel);
	
protected:

	TH2D *map;

	int readoutStop[2*ROCNUMROWS];
	short data[20000];

	RawPacketDecoder *gDecoder;
	unsigned short count;

	static bool fPrintDebug;
	bool fdebug;
};


#endif

