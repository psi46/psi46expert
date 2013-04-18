#include "interface/Log.h"

#include "TrimLow.h"
#include "TestRoc.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "Analysis.h"
#include "ThresholdMap.h"


TrimLow::TrimLow(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
    debug = false;
    noTrimBits = false;
}


void TrimLow::ReadTestParameters(TestParameters * testParameters)
{
    doubleWbc = (*testParameters).TrimDoubleWbc;
    nTrig = (*testParameters).TrimNTrig;
    vcal = (*testParameters).TrimVcal;
    presetVtrim = testParameters->TrimVtrim;
    presetVthrcomp = testParameters->TrimVthrcomp;
    adjustCalDel = testParameters->TrimAdjustCalDel;
}


void TrimLow::AddMap(TH2D * calMap)
{
    TH1D * distr = gAnalysis->Distribution(calMap, 255, 0., 255.);
    calMap->Write();
    distr->Write();
    histograms->Add(calMap);
    histograms->Add(distr);
}


void TrimLow::RocAction()
{
    TestPixel * maxPixel;
    double thr, thrMin;
    TH2D * calMap;

    thresholdMap = new ThresholdMap();
    if (doubleWbc) thresholdMap->SetDoubleWbc();

    psi::LogInfo() << "[TrimLow] ROC #" << chipId << ": Start." << psi::endl;
    SaveDacParameters();

    // Determine VthrComp, but only if it is not fixed in te test parameters
    if (presetVthrcomp < 0) {
        // VthrComp not fixed
        roc->SetTrim(15);
        SetDAC("Vtrim", 0);

        psi::LogInfo() << "[TrimLow] Trimming to Vcal " << vcal << "." << psi::endl;

        SetDAC("Vcal", vcal);
        Flush();

        psi::LogInfo() << "[TrimLow] Determining VthrComp ..." << psi::endl;

        thrMin = MinVthrComp("CalThresholdMap");
        if (thrMin == -1.)
            return;

        thresholdMap->SetSingleWbc();
        SetDAC("Vcal", 100);
        Flush();
        double thrMin2 = MinVthrComp("NoiseMap");
        if (doubleWbc)
            thresholdMap->SetDoubleWbc();

        if (thrMin2 - 10 < thrMin)
            thrMin = thrMin2 - 10;
        SetDAC("VthrComp", (int)thrMin);

        psi::LogInfo() << "[TrimLow] VthrComp is set to " << static_cast<int>(thrMin) << psi::endl;
    } else {
        // VthrComp fixed in test parameters
        SetDAC("VthrComp", (int)presetVthrcomp);
        psi::LogInfo() << "[TrimLow] VthrComp is fixed to " << static_cast<int>(presetVthrcomp) << " in testParameters.dat" << psi::endl;
    }

    Flush();
    Flush();

    psi::LogInfo() << "[TrimLow] Finding pixel with maximal Vcal threshold ... " << psi::endl;
    // Determine minimal and maximal vcal thresholds
    calMap = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, nTrig);
    AddMap(calMap);
    TH1D * distr = gAnalysis->Distribution(calMap, 255, 1., 254.);
    double vcalMaxLimit = TMath::Min(254., distr->GetMean() + 5.*distr->GetRMS());

    double vcalMin = 255., vcalMax = 0.;
    int thr255 = 0;
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            if (testRange->IncludesPixel(roc->GetChipId(), i, k))
            {
                thr = calMap->GetBinContent(i + 1, k + 1);
                if ((thr > vcalMax) && (thr < vcalMaxLimit))
                {
                    vcalMax = thr;
                    maxPixel = GetPixel(i, k);
                }
                if ((thr < vcalMin) && (thr > 1.)) vcalMin = thr;
                if (thr == 255.) thr255++;
            }
        }
    }

    psi::LogDebug() << "[TrimLow] There are " << thr255 << " pixels with " << "Vcal 255." << psi::endl;
    psi::LogDebug() << "[TrimLow] Vcal range is [ " << vcalMin << ", " << vcalMax << "]." << psi::endl;

    if (vcalMax == 0) {
        psi::LogInfo() << "[TrimLow] Error: Vcal max = 0. Abort test." << psi::endl;
        return;
    }

    // Determine Vtrim if it is not fixed
    int vtrim;
    if (presetVtrim < 0) {
        // Vtrim not fixed
        EnableDoubleColumn(maxPixel->GetColumn());
        SetPixel(maxPixel);
        vtrim = AdjustVtrim();
        psi::LogInfo() << "[TrimLow] Vtrim is set to " << vtrim << psi::endl;
        DisableDoubleColumn(maxPixel->GetColumn());
    } else {
        // Vtrim fixed in test parameters
        vtrim = presetVtrim;
        psi::LogInfo() << "[TrimLow] Vtrim is fixed to " << vtrim << " by testParameters.dat" << psi::endl;
        SetDAC("Vtrim", vtrim);
    }

    if (!noTrimBits) {
        roc->SetTrim(7);
        psi::LogInfo() << "[TrimLow] Preparing to set trim bits ..." << psi::endl;
        calMap = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, nTrig);
        AddMap(calMap);
        psi::LogInfo() << "[TrimLow] Setting trim bits (step 1 of 4) ..." << psi::endl;
        calMap = TrimStep(4, calMap, testRange);
        psi::LogInfo() << "[TrimLow] Setting trim bits (step 2 of 4) ..." << psi::endl;
        calMap = TrimStep(2, calMap, testRange);
        psi::LogInfo() << "[TrimLow] Setting trim bits (step 3 of 4) ..." << psi::endl;
        calMap = TrimStep(1, calMap, testRange);
        psi::LogInfo() << "[TrimLow] Setting trim bits (step 4 of 4) ..." << psi::endl;
        calMap = TrimStep(1, calMap, testRange);

        calMap = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, nTrig);
        AddMap(calMap);
    }

    RestoreDacParameters();

    SetDAC("Vtrim", vtrim);
    if (presetVthrcomp < 0)
        SetDAC("VthrComp", (int)thrMin);
    else
        SetDAC("VthrComp", (int)presetVthrcomp);

    /* Adjust CalDel, if requested */
    if (adjustCalDel) {
        int save_vcal = GetDAC("Vcal");
        int test_vcal = 255;
        psi::LogInfo() << "[TrimLow] Adjusting CalDel for Vcal " << test_vcal << " ..." << psi::endl;
        SetDAC("Vcal", test_vcal);
        roc->AdjustCalDel(1);
        SetDAC("Vcal", save_vcal);
    }

    ConfigParameters * configParameters = ConfigParameters::Singleton();
    char dacFileName[1000], trimFileName[1000];

    //writing files

    char dacParametersFileName[1000];
    strcpy(dacParametersFileName, configParameters->GetDacParametersFileName());
    int length = strlen(dacParametersFileName);
    if (strstr(dacParametersFileName, ".dat"))
        sprintf(dacFileName, "%s%i.dat", strncpy(dacParametersFileName, dacParametersFileName, length - 4), vcal);
    else
        sprintf(dacFileName, "%s%i_C%i.dat", strncpy(dacParametersFileName, dacParametersFileName, length - 4), vcal, chipId);
    roc->WriteDACParameterFile(dacFileName);

    char trimParametersFileName[1000];
    strcpy(trimParametersFileName, configParameters->GetTrimParametersFileName());
    length = strlen(dacParametersFileName);
    if (strstr(trimParametersFileName, ".dat"))
        sprintf(trimFileName, "%s%i.dat", strncpy(trimParametersFileName, trimParametersFileName, length - 4), vcal);
    else
        sprintf(trimFileName, "%s%i_C%i.dat", strncpy(trimParametersFileName, trimParametersFileName, length - 4), vcal, chipId);
    roc->WriteTrimConfiguration(trimFileName);

    gDelay->Timestamp();
}


double TrimLow::MinVthrComp(const char * mapName)
{
    //Find good VthrComp
    TH2D * calMap = thresholdMap->GetMap(mapName, roc, testRange, nTrig);
    AddMap(calMap);
    TH1D * distr = gAnalysis->Distribution(calMap, 255, 1., 254.);
    double thrMinLimit = TMath::Max(1., distr->GetMean() - 5. * distr->GetRMS());

    double thrMin = 255., thrMax = 0., thr;
    int thr255 = 0;
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            if (testRange->IncludesPixel(roc->GetChipId(), i, k))
            {
                thr = calMap->GetBinContent(i + 1, k + 1);
                if ((thr > thrMax) && (thr < 255.)) thrMax = thr;
                if ((thr < thrMin) && (thr > thrMinLimit)) thrMin = thr;
                if (thr == 255.) thr255++;
            }
        }
    }

    psi::LogDebug() << "[TrimLow] There are " << thr255 << " pixels with "
                    << "threshold 255." << psi::endl;
    psi::LogDebug() << "[TrimLow] Theshold range is [ " << thrMin << ", "
                    << thrMax << "]." << psi::endl;

    if (thrMax == 0.)
    {
        psi::LogInfo() << "[TrimLow] Error: Can not find maximum threshold."
                       << psi::endl;

        return -1.;
    }

    return thrMin;
}


TH2D * TrimLow::TrimStep(int correction, TH2D * calMapOld, TestRange * aTestRange)
{
    TH2D * betterCalMap = GetMap("VcalThresholdMap");
    int trim;

    //save trim map
    TH2D * trimMap = roc->TrimMap();

    //set new trim bits
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            if (aTestRange->IncludesPixel(roc->GetChipId(), i, k))
            {
                trim = (int)trimMap->GetBinContent(i + 1, k + 1);
                if ((calMapOld->GetBinContent(i + 1, k + 1) > vcal) && (calMapOld->GetBinContent(i + 1, k + 1) != 255)) trim -= correction;
                else trim += correction;

                if (trim < 0) trim = 0;
                if (trim > 15) trim = 15;
                GetPixel(i, k)->SetTrim(trim);
            }
        }
    }
    AddMap(roc->TrimMap());

    //measure new result
    TH2D * calMap = thresholdMap->GetMap("VcalThresholdMap", roc, aTestRange, nTrig);
    AddMap(calMap);

    // test if the result got better
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            if (aTestRange->IncludesPixel(roc->GetChipId(), i, k))
            {
                trim = GetPixel(i, k)->GetTrim();

                if (TMath::Abs(calMap->GetBinContent(i + 1, k + 1) - vcal) <= TMath::Abs(calMapOld->GetBinContent(i + 1, k + 1) - vcal))
                {
                    // it's better now
                    betterCalMap->SetBinContent(i + 1, k + 1, calMap->GetBinContent(i + 1, k + 1));
                }
                else
                {
                    // it's worse
                    betterCalMap->SetBinContent(i + 1, k + 1, calMapOld->GetBinContent(i + 1, k + 1));
                    GetPixel(i, k)->SetTrim((int)trimMap->GetBinContent(i + 1, k + 1));
                }
            }
        }
    }

    AddMap(roc->TrimMap());

    return betterCalMap;
}


int TrimLow::AdjustVtrim()
{
    int vtrim = -1;
    int thr = 255, thrOld;
    int wbc = GetDAC("WBC");
    psi::LogInfo() << "[TrimLow] Adjusting Vtrim for pixel " << column << ":" << row << " ..." << psi::endl;
    do
    {
        vtrim++;
        SetDAC("Vtrim", vtrim);
        Flush();
        thrOld = thr;
        thr = roc->PixelThreshold(column, row, 0, 1, nTrig, 2 * nTrig, 25, false, false, 0);
        if (debug) printf("thr %i\n", thr);
        if (doubleWbc)
        {
            SetDAC("WBC", wbc - 1);
            Flush();

            int thr2 = roc->PixelThreshold(column, row, 0, 1, nTrig, 2 * nTrig, 25, false, false, 0);
            if (debug) printf("thr 2 %i\n", thr2);
            if (thr2 < thr) thr = thr2;
            SetDAC("WBC", wbc);
            Flush();
        }
        psi::LogInfo() << "[TrimLow] Vtrim " << vtrim << " -> Thr " << thr << psi::endl;
    }
    while (((thr > vcal) || (thrOld > vcal) || (thr < 10)) && (vtrim < 250));
    vtrim += 5;
    SetDAC("Vtrim", vtrim);

    return vtrim;
}


void TrimLow::NoTrimBits(bool aBool)
{
    noTrimBits = aBool;
}


void TrimLow::SetVcal(int aValue)
{
    vcal = aValue;
}
