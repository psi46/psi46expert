#include "ThresholdTest.h"
#include "ThresholdMap.h"
#include "Analysis.h"


ThresholdTest::ThresholdTest(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
	printf("Threshold test\n");
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
}


void ThresholdTest::ReadTestParameters(TestParameters *testParameters)
{
	vcal = (*testParameters).ThresholdVcal;
	vthr = (*testParameters).ThresholdVthr;
	mode = (*testParameters).ThresholdMode;
	nTrig = (*testParameters).ThresholdNTrig;
}


void ThresholdTest::RocAction()
{
	SaveDacParameters();	
	ThresholdMap *thresholdMap = new ThresholdMap();
	if (mode == 0) 
	{ 
		SetDAC("Vcal", vcal);
		Flush();
		map = thresholdMap->GetMap("CalThresholdMap", roc, testRange, nTrig);
	}
	else if (mode == 1) 
	{
		if (vthr >= 0) SetDAC("VthrComp", vthr);
		Flush();
		map = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, nTrig);
	}
	else if (mode == 2) 
	{
		if (vthr >= 0) SetDAC("VthrComp", vthr);
		SetDAC("CtrlReg", 4);
		Flush();
		map = thresholdMap->GetMap("VcalsThresholdMap", roc, testRange, nTrig);
	}
	else if (mode == 3) 
	{
		if (vcal >= 0) SetDAC("Vcal", vcal);
		Flush();
		map = thresholdMap->GetMap("NoiseMap", roc, testRange, nTrig);
	}
	else if (mode == 4) 
	{
		if (vcal >= 0) SetDAC("Vcal", vcal);
		SetDAC("CtrlReg", 4);
		Flush();
		map = thresholdMap->GetMap("CalXTalkMap", roc, testRange, nTrig);
	}
	histograms->Add(map);
	histograms->Add(gAnalysis->Distribution(map));

	RestoreDacParameters();
}


