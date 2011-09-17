//
// Author: Peter Trüb
//
// Test of the address levels
// 

#ifndef ADDRESSLEVELS_H
#define ADDRESSLEVELS_H

#include <TH1D.h>
#include "Test.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"

class AddressLevels : public Test
{

public:
	AddressLevels(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
	virtual ~AddressLevels();

        virtual void ModuleAction();
	virtual void RocAction();
	
protected:
	void TestTBM();
	void TestROC();

	void FindDecoderLevels(TH1* adcHistogram, int& numLimits, short limits[], int maxLimits, int integralLimit);

	unsigned short count;
	short data[FIFOSIZE];
	int n;

	TH1D* adcHistogramTBM;
	TH1D* adcHistogramROC;

        short fLimitsTBM[DecoderCalibrationConstants::NUM_LEVELSTBM + 1];
	short fLimitsROC[RawPacketDecoderConstants::MAX_ROCS][DecoderCalibrationConstants::NUM_LEVELSROC + 1];
	bool fTestedROC[RawPacketDecoderConstants::MAX_ROCS];

	static bool fPrintDebug;
};

#endif

