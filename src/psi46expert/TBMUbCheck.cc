#include "TBMUbCheck.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/TBM.h"
#include "TestModule.h"
#include <iostream>


TBMUbCheck::TBMUbCheck(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
    debug = false;
}


void TBMUbCheck::ReadTestParameters(TestParameters * parameters)
{
    ubTarget = parameters->TBMUbLevel;
}


void TBMUbCheck::ModuleAction()
{
    cout << "Starting TBMUbCheck" << endl;

    TBM * tbm = module->GetTBM();
    TBAnalogInterface * anaInterface = (TBAnalogInterface *)tbInterface;
    ConfigParameters * configParameters = ConfigParameters::Singleton();
    bool halfModule = !(configParameters->halfModule == 0);

    short data[FIFOSIZE];
    const int nTrig = 10;
    bool tbmChannelOk;

    int dtlOrig = configParameters->dataTriggerLevel;
    int dtl = TMath::Min(ubTarget + 300, -200);
    anaInterface->DataTriggerLevel(dtl);

    int tbmChannel_saved = anaInterface->GetTBMChannel();
    int tbmMode_saved    = tbm->GetDAC(0);
    int tbmGain_saved    = tbm->GetDAC(4);

    int tbmGain_target = 0;

    int nChannels;
    if (halfModule) nChannels = 1; else nChannels = 2;

    for (int itbm = 0; itbm < nChannels; itbm++)
    {
        if (!halfModule)
        {
            anaInterface->SetTBMChannel(itbm);
            module->SetTBMSingle(itbm);
        }
        tbmChannelOk = false;

        int tbmGain = 255;
        for (int ibit = 7; ibit >= 0; ibit--) {
            int bitValue = (ibit >= 1) ? (2 << (ibit - 1)) : 1;
            int tbmGain_new  = tbmGain - bitValue;

            tbm->SetDAC(4, tbmGain_new);

            unsigned short count = 0;
            ((TBAnalogInterface *)tbInterface)->ADCRead(data, count, nTrig);

            if (count > 0) {
                tbmChannelOk = true;
                double ubLevel = (data[0] + data[1] + data[2]) / 3.;
                if (ubLevel < ubTarget) tbmGain = tbmGain_new;
            }
        }

        unsigned short count = 0;
        ((TBAnalogInterface *)tbInterface)->ADCRead(data, count, nTrig);
        if (count > 0) {
            double ubLevel = data[0];
            cout << "tbmGain = " << tbmGain << ", ubLevel = " << ubLevel << endl;
        }

        //--- save maximum Dacgain of both TBMs
        //    (can only set one Dacgain value for both)
        if (tbmChannelOk && (tbmGain > tbmGain_target)) tbmGain_target = tbmGain;
    }

    if (tbmGain_target == 0) tbmGain_target = tbmGain_saved;

    cout << "setting tbmGain to " << tbmGain_target << endl;
    tbm->SetDAC(4, tbmGain_target);

    //--- restore previous TBM settings
    anaInterface->SetTBMChannel(tbmChannel_saved);
    tbm->SetDAC(0, tbmMode_saved);
    anaInterface->DataTriggerLevel(dtlOrig);
    Flush();

    tbm->WriteTBMParameterFile(configParameters->GetTbmParametersFileName());
}
