#include "ThresholdTest.h"
#include "ThresholdMap.h"
#include "Analysis.h"
#include "interface/Log.h"


ThresholdTest::ThresholdTest(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
}


void ThresholdTest::ReadTestParameters(TestParameters * testParameters)
{
    vcal = (*testParameters).ThresholdVcal;
    vthr = (*testParameters).ThresholdVthr;
    mode = (*testParameters).ThresholdMode;
    nTrig = (*testParameters).ThresholdNTrig;
}


void ThresholdTest::RocAction()
{
    SaveDacParameters();
    ThresholdMap * thresholdMap = new ThresholdMap();
    if (mode == 0)
    {
        SetDAC("Vcal", vcal);
        Flush();
        psi::LogInfo() << "[ThresholdTest] Measuring CalThresholdMap for ROC " << chipId << " ..." << psi::endl;
        map = thresholdMap->GetMap("CalThresholdMap", roc, testRange, nTrig);
    }
    else if (mode == 1)
    {
        if (vthr >= 0) SetDAC("VthrComp", vthr);
        Flush();
        psi::LogInfo() << "[ThresholdTest] Measuring VcalThresholdMap for ROC " << chipId << " ..." << psi::endl;
        map = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, nTrig);
    }
    else if (mode == 2)
    {
        if (vthr >= 0) SetDAC("VthrComp", vthr);
        SetDAC("CtrlReg", 4);
        Flush();
        psi::LogInfo() << "[ThresholdTest] Measuring VcalsThresholdMap for ROC " << chipId << " ..." << psi::endl;
        map = thresholdMap->GetMap("VcalsThresholdMap", roc, testRange, nTrig);
    }
    else if (mode == 3)
    {
        if (vcal >= 0) SetDAC("Vcal", vcal);
        Flush();
        psi::LogInfo() << "[ThresholdTest] Measuring NoiseMap for ROC " << chipId << " ..." << psi::endl;
        map = thresholdMap->GetMap("NoiseMap", roc, testRange, nTrig);
    }
    else if (mode == 4)
    {
        if (vcal >= 0) SetDAC("Vcal", vcal);
        SetDAC("CtrlReg", 4);
        Flush();
        psi::LogInfo() << "[ThresholdTest] Measuring CalXTalkMap for ROC " << chipId << " ..." << psi::endl;
        map = thresholdMap->GetMap("CalXTalkMap", roc, testRange, nTrig);
    }

    else if (mode == 5)
    {
        if (vthr >= 0) SetDAC("VthrComp", vthr);
        if (vcal >= 0) SetDAC("Vcal", vcal);
	Flush();
	map = thresholdMap->GetMap(Form("TimeWalk_vcal_%i", vcal), roc, testRange, nTrig, mode);
	histograms->Add(map);
	histograms->Add(gAnalysis->Distribution(map));
    }

    else if (mode == 6)
    {
        if (vthr >= 0) SetDAC("VthrComp", vthr);
        SetDAC("CtrlReg", 4);
        Flush();
        psi::LogInfo() << "[ThresholdTest] Measuring VcalXTalkMap for ROC " << chipId << " ..." << psi::endl;
        map = thresholdMap->GetMap("VcalXTalkMap", roc, testRange, nTrig);
    }

    else if (mode == 7)
    {
        //if (vthr >= 0) SetDAC("VthrComp", vthr);
        SetDAC("CtrlReg", 4);
        SetDAC("Vcal", 255);
        Flush();
        psi::LogInfo() << "[ThresholdTest] Measuring CalsThresholdMap for ROC " << chipId << " ..." << psi::endl;
        map = thresholdMap->GetMap("CalsThresholdMap", roc, testRange, nTrig);
    }



    histograms->Add(map);
    histograms->Add(gAnalysis->Distribution(map));

    RestoreDacParameters();
}


