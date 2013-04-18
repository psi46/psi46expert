// Pulse height calibration functions

#ifndef PHCALIBRATION_H
#define PHCALIBRATION_H

#include <fstream>
#include <iostream>
#include <stdio.h>
#include "Test.h"
#include <TH2D.h>

class TF1;
class TGraph;

class PHCalibration : public Test
{

 public:
 
	PHCalibration();
	PHCalibration(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);

	void Initialize();	
	virtual void ReadTestParameters(TestParameters *testParameters);
	virtual void RocAction();
	void Measure(int nTriggers);
	int GetCalDel(int vcalStep);
	int GetVthrComp(int vcalStep);
	void PulseHeightRocDigital(int data []);

 private:

	int vcal[512], ctrlReg[512];
	int mode, vcalSteps, nTrig, numPixels, calDelVthrComp;
	int calDel50, calDel100, calDel200, vthrComp50, vthrComp100, vthrComp200;

};

#endif
