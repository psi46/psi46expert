// This class provides support for the tests on the Module level

#ifndef TESTMODULE
#define TESTMODULE

#include "BasePixel/Module.h"
#include "TestRoc.h"
#include "Test.h"
#include "TestRange.h"


class TestModule: public Module
{

public:
	TestModule(ConfigParameters *configParameters, int aCNId, TBInterface *aTBInterface, TestParameters *testParameters);
	void Execute(SysCommand &command);
	TestRoc* GetRoc(int iRoc);
	void DoTest(Test *aTest);
	TestRange *GetRange(SysCommand &command);
	TestRange *FullRange();
	
	void FullTestAndCalibration();
  void ShortTestAndCalibration();
  void ShortCalibration();
	void TestM();
	void DigiCurrent();
	void DoTBMTest();
	void AnaReadout();
	void DumpParameters();
	void DataTriggerLevelScan();
	void AdjustVana(double goalCurrent = 0.024);
	void AdjustAllDACParameters();
  void AdjustDACParameters();
	void AdjustUltraBlackLevel();
	void AdjustTBMUltraBlack();
	void AdjustSamplingPoint();
	void AdjustVOffsetOp();
  void AdjustPHRange();
	void CalibrateDecoder();
	double GetTemperature();
	void ADCHisto();
  void AdjustCalDelVthrComp();
  void MeasureCurrents();
  void VanaVariation();
	void Scurves();
	
	bool TestDACProgramming(int dacReg, int max);
	void TestDACProgramming();
	void IanaScan();
			
protected:


	TestParameters *testParameters;
};


#endif

