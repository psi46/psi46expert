// Trim functions

#ifndef TRIMLOW
#define TRIMLOW

#include "Test.h"
#include <TH2D.h>
#include "ThresholdMap.h"

class TrimLow : public Test
{

public:
	TrimLow(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);

	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	double MinVthrComp(char *mapName);
	int AdjustVtrim();
	void AddMap(TH2D* calMap);
	TH2D* TrimStep(int correction, TH2D *calMapOld, TestRange* aTestRange);
	void NoTrimBits(bool aBool);
	void SetVcal(int vcal);


protected:

	int vthrComp, doubleWbc, nTrig, vcal, presetVtrim, presetVthrcomp, adjustCalDel;
	bool noTrimBits;
	ThresholdMap *thresholdMap;

};


#endif

