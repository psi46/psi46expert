#include "interface/Log.h"

#include "interface/Delay.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "FullTest.h"
#include "PixelAlive.h"
#include "BumpBonding.h"
#include "AddressLevels.h"
#include "AddressDecoding.h"
#include "Trim.h"
#include "SCurveTest.h"
#include "DacDependency.h"
#include "TrimBits.h"
#include "PHTest.h"
#include "ThresholdMap.h"
#include "TemperatureTest.h"
#include "TBMTest.h"
#include "AnalogReadout.h"


FullTest::FullTest(ConfigParameters * aconfigParameters, TestRange * aTestRange, TestParameters * aTestParameters, TBInterface * aTBInterface, int opt)
{
    psi::LogDebug() << "[FullTest] Initialization." << psi::endl;
    configParameters = aconfigParameters;
    testRange = aTestRange;
    tbInterface = aTBInterface;
    testParameters = aTestParameters;
    Scurve = opt;
}


void FullTest::ModuleAction()
{
    Test * test;

    if (Scurve != 0)
    {
        psi::LogInfo() << "[FullTest] Start." << psi::endl;

        //Log::Current()->printf("==>sv> Start FullTest\n");
        //Log::Current()->printf("==>sv> Start Test\n");
        gDelay->Timestamp();
    }

    //DoTemperatureTest();
    //skip iTest = 2 (DTB)
    for (int iTest = 0; iTest < 3; iTest++)
    {
        if (Scurve == 0)
        {
            test = new SCurveTest(testRange, testParameters, tbInterface);
            test->ModuleAction(module);
            break;
        }

        gDelay->Timestamp();
        if (iTest == 0) test = new SCurveTest(testRange, testParameters, tbInterface);
        if (iTest == 1 && !(configParameters->tbmEmulator)) test = new TBMTest(testRange, testParameters, tbInterface);
        else if (iTest == 1) continue;
        if (iTest == 2) test = new AnalogReadout(testRange, testParameters, tbInterface);
        test->ModuleAction(module);
        TIter next(test->GetHistos());
        while (TH1 * histo = (TH1 *)next()) histograms->Add(histo);
    }

    if (Scurve != 0)
    {
        Test::ModuleAction();
        DoTemperatureTest();

        psi::LogInfo() << "[FullTest] End." << psi::endl;
        // Log::Current()->printf("==>sv> End Test\n");
        // Log::Current()->printf("==>sv> End FullTest\n");
    }
}


void FullTest::RocAction()
{
    if (Scurve != 0)
    {
        psi::LogDebug() << "[FullTest] Chip #" << chipId << '.' << psi::endl;
        Test * test;

        histograms->Add(roc->DACHisto());

        for (int iTest = 0; iTest < 6; iTest++)
        {
            gDelay->Timestamp();
            if (iTest == 0) test = new PixelAlive(testRange, testParameters, tbInterface);
            if (iTest == 1) test = new BumpBonding(testRange, testParameters, tbInterface);
            if (iTest == 2) test = new TrimBits(testRange, testParameters, tbInterface);
            if (iTest == 3) test = new TemperatureTest(testRange, testParameters, tbInterface);
            if (iTest == 4) test = new AddressDecoding(testRange, testParameters, tbInterface);
            if (iTest == 5) test = new AddressLevels(testRange, testParameters, tbInterface);
            test->RocAction(roc);
            TIter next(test->GetHistos());
            while (TH1 * histo = (TH1 *)next()) histograms->Add(histo);
        }

        //    SaveDacParameters();
        //    Log::Current()->printf("NoiseMap\n");
        //    ThresholdMap *thresholdMap = new ThresholdMap();
        //    TH2D *noiseMap = thresholdMap->GetMap("NoiseMap", roc, testRange, 10);
        //    histograms->Add(noiseMap);
        //    delete thresholdMap;
        //    RestoreDacParameters();

        //    Log::Current()->printf("FullTest  %i\n", chipId);
        psi::LogDebug() << "[FullTest] done for chip " << chipId << '.'
                        << psi::endl;
    }
}


void FullTest::DoTemperatureTest()
{
    gDelay->Timestamp();
    psi::LogDebug() << "[FullTest] Temperature Test." << psi::endl;

    Test * test = new TemperatureTest(testRange, testParameters, tbInterface);
    test->ModuleAction(module);
    TIter next(test->GetHistos());
    while (TH1 * histo = (TH1 *)next()) histograms->Add(histo);
}
