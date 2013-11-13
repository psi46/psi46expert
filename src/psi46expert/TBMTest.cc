#include "interface/Log.h"

#include "TBMTest.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/ConfigParameters.h"
#include <TParameter.h>


// e5 f5

TBMTest::TBMTest(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    psi::LogDebug() << "[TBMTest] Initialization." << psi::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
    debug = false;
}


void TBMTest::ReadTestParameters(TestParameters * testParameters)
{
}


void TBMTest::ModuleAction()
{
    ConfigParameters * configParameters = ConfigParameters::Singleton();

    result[0] = 0;
    result[1] = 0;

    if (!tbInterface->TBMPresent()) return;
    if (configParameters->halfModule == 0) DualModeTest();
    ReadoutTest();

    TParameter<int> * parameter0 = new TParameter<int>("TBM1", result[0]);
    TParameter<int> * parameter1 = new TParameter<int>("TBM2", result[1]);
    parameter0->Write();
    parameter1->Write();
}


void TBMTest::ReadoutTest()
{
    TBM * tbm = module->GetTBM();

    int value;
    bool res = tbm->GetReg(229, value);
    if (debug) printf("value %i\n", value);

    if (!res)
    {
        result[0] += 4;
        psi::LogInfo() << "[TBMTest] Warning: TBM1 readout test failed." << psi::endl;
    }

    res = tbm->GetReg(245, value);
    if (debug) printf("value2 %i\n", value);

    if (!res)
    {
        result[1] += 4;
        psi::LogInfo() << "[TBMTest] Warning: TBM2 readout test failed." << psi::endl;
    }


}

void TBMTest::DualModeTest()
{
    TBM * tbm = module->GetTBM();
    ConfigParameters * configParameters = ConfigParameters::Singleton();

    unsigned short count;
    short data[10000];

    psi::LogInfo() << "[TBMTest] Start." << psi::endl;

    int channel = tbInterface->GetTBMChannel();
    int singleDual = tbm->GetDAC(0);

    int dtlOrig = configParameters->dataTriggerLevel, dtl;
    tbInterface->DataTriggerLevel(dtl);

    for (int k = 0; k < 2; k++)
    {
        module->SetTBMSingle(k);
        tbInterface->SetTBMChannel(k);

        dtl = dtlOrig;
        do
        {
            tbInterface->DataTriggerLevel(dtl);
            tbInterface->Flush();
            tbInterface->ADCData(data, count);
            dtl += 50;
        }
        while ((count != tbInterface->GetEmptyReadoutLengthADC()) && (dtl < 0));

        if (count != tbInterface->GetEmptyReadoutLengthADC())
        {
            result[k] += 1;

            psi::LogInfo() << "[TBMTest] Warning: test failed for TBM #" << k
                           << ". No valid analog readout." << psi::endl;

            if (k == 1)
                psi::LogInfo() << "[TBMTest] Warning: test failed. Does testboard "
                               << "have 2 ADCs?" << psi::endl;

            for (int i = 0; i < count; i++) printf(" %i", data[i]);
            printf("\n");
        }

        tbm->setDualMode();

        dtl = dtlOrig;
        do
        {
            tbInterface->DataTriggerLevel(dtl);
            tbInterface->Flush();
            tbInterface->ADCData(data, count);
            dtl += 50;
        }
        while ((count != tbInterface->GetEmptyReadoutLengthADCDual()) && (dtl < 0));

        if (count != tbInterface->GetEmptyReadoutLengthADCDual())
        {
            result[k] += 2;

            psi::LogInfo() << "[TBMTest] Warning: test failed for TBM #" << k
                           << ". No valid dual analog readout." << psi::endl;

            if (k == 1)
                psi::LogInfo() << "[TBMTest] Warning: test failed. Does testboard "
                               << "have 2 ADCs?" << psi::endl;

            for (int i = 0; i < count; i++) printf(" %i", data[i]);
            printf("\n");
        }
    }

    tbInterface->SetTBMChannel(channel);
    tbm->SetDAC(0, singleDual);
    tbInterface->DataTriggerLevel(dtlOrig);
    Flush();
}
