#include "interface/Log.h"

#include "BumpBonding.h"
#include "TestRoc.h"
#include "Analysis.h"
#include "ThresholdMap.h"
#include "BasePixel/TBAnalogInterface.h"

BumpBonding::BumpBonding(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    psi::LogDebug() << "[BumpBonding] Initialization." << psi::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
}


void BumpBonding::ReadTestParameters(TestParameters * testParameters)
{
    nTrig = (*testParameters).BumpBondingNTrig;
}


void BumpBonding::RocAction()
{
    psi::LogInfo() << "[BumpBonding] Finding missing bump bonds for ROC #" << chipId << " ..." << psi::endl;
    TBAnalogInterface * ai = dynamic_cast<TBAnalogInterface *>(tbInterface);
    bool is_analog = roc->has_analog_readout();

    if (is_analog) {
        psi::LogInfo() << "[BumpBonding] Using method for analog ROCs ..." << psi::endl;
        ThresholdMap * thresholdMap = new ThresholdMap();

        SaveDacParameters();
        ClrCal();
        Mask();
        SetDAC("Vcal", 200);
        SetDAC("CtrlReg", 4);
        Flush();

        psi::LogInfo() << "[BumpBonding] Measuring cross talk map ..." << psi::endl;
        TH2D * calXtalk = thresholdMap->GetMap("CalXTalkMap", roc, testRange, 5);
        TH1D * calXtalkDistribution = gAnalysis->Distribution(calXtalk);
        vthrComp = static_cast<int>(calXtalkDistribution->GetMean() + 3. * calXtalkDistribution->GetRMS());

        psi::LogDebug() << "[BumpBonding] Setting VthrComp to " << vthrComp << '.'
                        << psi::endl;

        //SetDAC("VthrComp", vthrComp);
        SetDAC("VthrComp", 85);


        Flush();

        psi::LogInfo() << "[BumpBonding] Measuring Vcal threshold map (via sensor) ..." << psi::endl;
        TH2D * vcals = thresholdMap->GetMap("VcalsThresholdMap", roc, testRange, nTrig);
        psi::LogInfo() << "[BumpBonding] Measuring cross talk map ..." << psi::endl;
        TH2D * xtalk = thresholdMap->GetMap("XTalkMap", roc, testRange, nTrig);
        TH2D * difference = gAnalysis->DifferenceMap(vcals, xtalk, Form("vcals_xtalk_C%i", roc->GetChipId()));

        RestoreDacParameters();

        histograms->Add(calXtalk);
        histograms->Add(vcals);
        histograms->Add(xtalk);
        histograms->Add(difference);

        histograms->Add(gAnalysis->Distribution(calXtalk));
        histograms->Add(gAnalysis->Distribution(vcals));
        histograms->Add(gAnalysis->Distribution(xtalk));
        histograms->Add(gAnalysis->Distribution(difference));
        histograms->Add(calXtalkDistribution);
    }

    //dig chip
    else
    {
        psi::LogInfo() << "[BumpBonding] Using method for digital ROCs ..." << psi::endl;
        SaveDacParameters();
        ThresholdMap * thresholdMap = new ThresholdMap();


        SetDAC("CtrlReg", 4);
        SetDAC("Vcal", 255);
        Flush();
        psi::LogInfo() << "[BumpBonding] Measuring bump bonding map ..." << psi::endl;
        TH2D * map = thresholdMap->GetMap("BumpBondMap", roc, testRange, nTrig, 7);

        histograms->Add(map);
        histograms->Add(gAnalysis->Distribution(map));

        RestoreDacParameters();
    }
}

