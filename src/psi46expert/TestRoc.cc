#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <TF1.h>
#include <TGraph.h>
#include <TCanvas.h>

#include "interface/Log.h"

#include "TestRoc.h"
#include "TestDoubleColumn.h"
#include "interface/Delay.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/CalibrationTable.h"
#include "PHCalibration.h"
#include "Analysis.h"
#include "PixelAlive.h"
#include "Test.h"
#include "FullTest.h"
#include "BumpBonding.h"
#include "AddressLevels.h"
#include "AddressDecoding.h"
#include "SCurveTest.h"
#include "TrimBits.h"
#include "Trim.h"
#include "TrimVcal.h"
#include "TrimLow.h"
#include "UbCheck.h"
#include "PHCalibration.h"
#include "PHRange.h"
#include "TrimBits.h"
#include "TestRange.h"
#include "DacDependency.h"
#include "TemperatureTest.h"
#include "IVCurve.h"
#include "ThresholdTest.h"
#include "OffsetOptimization.h"
#include "SCurveTest.h"
#include "HighRateTrimLow.h"
#include "PHTest.h"
#include <TFile.h>
#include <iostream>
#include <string.h>
#include <sstream>


TestRoc::TestRoc(TBInterface * const aTBInterface, TestParameters * parameters, const int aChipId, const int aHubId, const int aPortId, const int anAoutChipPosition)
    : Roc(aTBInterface, aChipId, aHubId, aPortId, anAoutChipPosition)
{
    for (int i = 0; i < ROCNUMDCOLS; i++)
    {
        delete doubleColumn[i];
        doubleColumn[i] = new TestDoubleColumn(this, i, parameters);
    }
    dacParameters = new DACParameters(this);
    testParameters = parameters;
}


TestDoubleColumn * TestRoc::GetDoubleColumn(int column)
{
    return (TestDoubleColumn *)(Roc::GetDoubleColumn(column));
}


TestPixel * TestRoc::GetPixel(int column, int row)
{
    return (TestPixel *)(Roc::GetPixel(column, row));
}


TestPixel * TestRoc::GetTestPixel()
{
    TestPixel * pixel;
    for (int i = 5; i < ROCNUMCOLS - 5; i++)  // no pixels near the edge
    {
        for (int k = 5; k < ROCNUMROWS - 5; k++) // no pixels near the edge
        {
            pixel = GetPixel(i, k);
            if (pixel->IsAlive()) {return pixel;}
        }
    }
    return 0;
}


TestParameters * TestRoc::GetTestParameters()
{
    return testParameters;
}


void TestRoc::Execute(SysCommand &command)
{
    if (Roc::Execute(command, 0)) {}
    else if (command.Keyword("Test1")) {Test1();}
    else if (command.Keyword("ThrMaps")) {ThrMaps();}
    else if (command.Keyword("ThrTest")) {DoTest(new ThresholdTest(GetRange(), testParameters, tbInterface));}
    else if (command.Keyword("dac")) {dacParameters->Print();}
    else if (command.Keyword("PhError")) {PhError();}
    else if (command.Keyword("SamplingTest")) {ADCSamplingTest();}
    else if (command.Keyword("OffsetOptimization")) {DoTest(new OffsetOptimization(GetRange(), testParameters, tbInterface));}
    else if (command.Keyword("PHRange")) {DoTest(new PHRange(GetRange(), testParameters, tbInterface));}
    //  else if (command.Keyword("dtlScan")) {DataTriggerLevelScan();}
    else if (command.Keyword("temp")) {GetTemperature();}
    else if (command.Keyword("Trim")) DoTrim();
    else if (command.Keyword("TrimVcal")) DoTrimVcal();
    else if (command.Keyword("TrimLow")) DoTrimLow();
    else if (command.Keyword("chipTest")) {ChipTest();}
    else if (command.Keyword("CalDelVthrComp")) {AdjustCalDelVthrComp();}
    else if (command.Keyword("AdjustCalDel")) {AdjustCalDel(0);}
    else if (command.Keyword("AdjustPHRange")) {AdjustPulseHeightRange();}
    else if (command.Keyword("TrimVerification")) {TrimVerification();}
    else if (strcmp("PhCalibration", command.carg[0]) == 0) DoPhCalibration();
    else if (strcmp("PulseShape", command.carg[0]) == 0) DoPulseShape();
    //  else if (command.Keyword("IV")) {DoIV(new IVCurve(GetRange(), testParameters, tbInterface));}
    else if (strcmp("HighRateTrimLow", command.carg[0]) == 0) DoTest(new HRTrimLow(GetRange(), testParameters, tbInterface));
    else {cerr << "Unknown ROC command " << command.carg[0] << endl;}
}


void TestRoc::DoTrim()
{
    DoTest(new Trim(GetRange(), testParameters, tbInterface));
}


void TestRoc::DoTrimVcal()
{
    DoTest(new TrimVcal(GetRange(), testParameters, tbInterface));
}

void TestRoc::DoTrimLow()
{
    DoTest(new TrimLow(GetRange(), testParameters, tbInterface));
}

void TestRoc::DoPhCalibration()
{
    DoTest(new PHCalibration(GetRange(), testParameters, tbInterface));
}

void TestRoc::DoIV(Test * aTest)
{
    psi::LogInfo() << "[TestRoc] IV: Start." << psi::endl;
    //  GetTestParameters()->ReadTestParameterFile(configParameters->GetTestParametersFileName());

    gDelay->Timestamp();
    aTest->ModuleAction();

    psi::LogInfo() << "[TestRoc] IV: End." << psi::endl;

    gDelay->Timestamp();
}

int TestRoc::CountReadouts(int count)
{
    //aoutChipPosition is only relevant for setup with TBM, otherwise count gives sum of all readouts
    return GetTBInterface()->CountReadouts(count, aoutChipPosition);
}


// == Tests =============================================


void TestRoc::ChipTest()
{
    //  DoTest(new PixelAlive(GetRange(), testParameters, tbInterface));
    gDelay->Timestamp();
    DoTest(new BumpBonding(GetRange(), testParameters, tbInterface));
    gDelay->Timestamp();
    DoTest(new TrimBits(GetRange(), testParameters, tbInterface));
    //  DoTest(new SCurveTest(GetRange(), testParameters, tbInterface));
    //  DoTest(new AddressLevels(GetRange(), testParameters, tbInterface));
    //  DoTest(new AddressDecoding(GetRange(), testParameters, tbInterface));
    //  DoTest(new TemperatureTest(GetRange(), testParameters, tbInterface));
    //  DACHisto();

    //  SaveDacParameters();
    //  ThresholdMap *thresholdMap = new ThresholdMap();
    //  TH2D *noiseMap = thresholdMap->GetMap("NoiseMap", this, GetRange(), 10);
    //  RestoreDacParameters();
    //
    //  testParameters->TrimVcal = 80;
    //  DoTest(new Trim(GetRange(), testParameters, tbInterface));
    //  testParameters->TrimVcal = 60;
    //  DoTest(new Trim(GetRange(), testParameters, tbInterface));
    //  testParameters->TrimVcal = 50;
    //  DoTest(new Trim(GetRange(), testParameters, tbInterface));
    //  DoTest(new PHCalibration(GetRange(), testParameters, tbInterface));
}


TestRange * TestRoc::GetRange()
{
    TestRange * range = new TestRange();
    //  range->AddPixel(chipId, 5, 5);
    range->CompleteRoc(chipId);
    return range;
}


// -- Performs a test for this roc
void TestRoc::DoTest(Test * aTest)
{
    aTest->RocAction(this);
}


void TestRoc::ADCSamplingTest()
{
    for (int delay = 0; delay < 40; ++delay)
    {
        psi::LogDebug() << "[TestRoc] Delay: " << delay << psi::endl;

        tbInterface->SetTBParameter("clk", delay);
        tbInterface->SetTBParameter("sda", 17 + delay);
        tbInterface->SetTBParameter("ctr", 15 + delay);
        tbInterface->SetTBParameter("tin", 10 + delay);
        tbInterface->Flush();
        GetTBInterface()->ADC();
    }
}


// void TestRoc::DataTriggerLevelScan()
// {
//  for (int delay = 0; delay < 2000; delay = delay + 50)
//  {
//    Log::Current()->printf("dtl: %i ------------------------------------- \n", delay);

//      (tbInterface->DataTriggerLevel(-delay);
//    tbInterface->Flush();
//    GetTBInterface()->ADC();
//  }
// }

void TestRoc::PhError()
{


    int offset;
    int nReadouts = 1000;
    short data[FIFOSIZE];
    unsigned short count;
    ArmPixel(20, 20);
    offset = chipId * 3 + 16;

    int vcal[10];
    vcal[0] = 50;
    vcal[1] = 100;
    vcal[2] = 150;
    vcal[3] = 200;
    vcal[4] = 250;
    vcal[5] = 30;
    vcal[6] = 50;
    vcal[7] = 70;
    vcal[8] = 90;
    vcal[9] = 200;

    SetDAC("VthrComp", 116);
    SetDAC("CtrlReg", 0);
    for (int Tvcal = 0; Tvcal < 10; Tvcal++)
        //for (int Tvcal = 0; Tvcal < 260; Tvcal+=50)
    {
        if (Tvcal > 4) SetDAC("CtrlReg", 4);
        SetDAC("Vcal", vcal[Tvcal]);
        //SetDAC("VIbias_roc",Tvcal);
        TH1D * phHist = new TH1D(Form("phHistVcal%d", vcal[Tvcal]), Form("phHistVcal%d", vcal[Tvcal]), 4000, -2000., 2000.);
        for (int i = 0; i < nReadouts; i++)
        {
            tbInterface->ADCRead(data, count, 1);
            if (count > offset) phHist->Fill(data[offset]);
        }

        printf("Vcal = %d, PH %.1f +- %.2f\n", vcal[Tvcal], phHist->GetMean(), phHist->GetRMS());
        //printf("VIbias_roc = %d, PH %.1f +- %.2f\n", Tvcal, phHist->GetMean(), phHist->GetRMS());

    }

}



void TestRoc::Test1()
{



    // ------------------------------------------------------------------------
    //     for (int i = 0; i < 80; i++)
    //     {
    //       int col = i;
    //       if (i > 51) col = 51;
    //       EnablePixel(col, i);
    //
    //     }
    //     Flush();

    //         GetTBInterface()->SetTriggerMode(TRIGGER_MODULE2);
    //         SendADCTrigs(1);
    //
    //         for (int i = 0; i < 17; i++)
    //         {
    //           int n = GetTBInterface()->GetModRoCnt(i);
    //           printf("%i n %i\n", i, n);
    //         }

    //         ArmPixel(5,5);
    //         ArmPixel(7,7);
    //         Flush();

    //         GetTBInterface()->SetTriggerMode(TRIGGER_MODULE2);

    // ------------------------------------------------------------------------
    //         int n;
    //         for (int i = 0; i < 16; i++)
    //         {
    //           n = GetTBInterface()->CountReadouts(1, i);
    //           printf("%i %i\n", i, n);
    //         }

    // ------------------------------------------------------------------------

    //  Rainbow2();

    //  short data[10000];
    //  unsigned short count;
    //         int noise = 0, length;
    //  int nReadouts, readoutStart[256];
    //
    //         for (int i = 0; i < 100; i++)
    //         {
    //    SendADCTrigs(10);
    //    Flush();
    //    GetADC(data, FIFOSIZE, count, 10, readoutStart, nReadouts);
    //    for (int k = 0; k < nReadouts; k++)
    //    {
    //      if (k == nReadouts - 1) length = count - readoutStart[k];
    //      else length = readoutStart[k+1] - readoutStart[k];
    //      if (length > GetTBInterface()->GetEmptyReadoutLengthADC()) noise++;
    //    }
    //    cout << "x" << flush;
    //         }
    //         printf("\n");
    //         printf("noise %i\n", noise);

    // ------------------------------------------------------------------------

    int offset;
    if (tbInterface->TBMPresent()) offset = 16;
    else offset = 9;
    int nTrig = 10;

    SetDAC("CtrlReg", 4);

    TestPixel * pixel;

    for (int col = 0; col < 2; col++)
    {
        printf("col = %d\n", col);
        for (int row = 0; row < 2; row++)
        {
            printf("row = %d\n", row);
            pixel = GetPixel(col, row);
            pixel->ArmPixel();
            for (int vsf = 150; vsf < 255; vsf += 20)
            {
                GetDAC("Vsf");
                SetDAC("Vsf", vsf);
                Flush();
                short result[256];
                tbInterface->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result);
                TH1D * histo = new TH1D(Form("Vsf%d_Col%d_Row%d", vsf, col, row), Form("Vsf%d_Col%d_Row%d", vsf, col, row), 256, 0., 256.);
                for (int dac = 0; dac < 256; dac++)
                {
                    printf("result = %i\n", result[dac]);
                    if (result[dac] == 7777) histo->SetBinContent(dac + 1, 555);
                    else histo->SetBinContent(dac + 1, result[dac]);
                }
            }
            pixel->DisarmPixel();
        }
    }
}


void TestRoc::Rainbow()
{
    TH2D * histo = new TH2D("thr", "thr", 255, 0., 255., 255, 0., 255.);
    int col = 5, row = 5, nTrig = 10, thr = 0;

    //        EnableAllPixels();
    ArmPixel(col, row);

    for (int i = 100; i < 170; i += 1)
    {
        SetDAC("VthrComp", i);
        for (int k = 0; k < 5; k += 1)
        {
            SetDAC("Vtrim", k);
            Flush();
            thr = PixelThreshold(col, row, 0, 1, nTrig, 2 * nTrig, 25, false, false, 0);
            histo->SetBinContent(i, k, thr);
            printf("vthr %i vtrim %i %i\n", i, k, thr);
        }
    }
}



void TestRoc::Rainbow2()
{
    TH2D * histo = new TH2D("alive", "alive", 255, 0., 255., 255, 0., 255.);
    double data[ROC_NUMROWS * ROC_NUMCOLS];
    int nTrig = 5, nAlive;

    //        SetTrim(0.);
    tbInterface->SetEnableAll(1);
    Flush();

    for (int i = 120; i < 121; i += 5)
    {
        SetDAC("VthrComp", i);
        for (int k = 0; k < 200; k += 2)
        {
            SetDAC("Vtrim", k);
            Flush();

            ChipEfficiency(nTrig, data);

            nAlive = 0;
            for (int l = 0; l < ROC_NUMROWS * ROC_NUMCOLS; l++)
            {
                if (data[l] == 1.) nAlive++;
            }

            histo->SetBinContent(i, k, nAlive);
            printf("vthr %i vtrim %i %i\n", i, k, nAlive);
            if (nAlive == 0) break;
        }
    }
}


// -- Function where you can quickly implement some tests
// -- This code is not persistent
void TestRoc::TestM()
{
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            EnablePixel(i, k);
        }
    }

    TestPixel * pixel;

    for (int i = 0; i < 26; i++)
    {
        EnableDoubleColumn(i * 2);
        pixel = GetPixel(i * 2, 6);
        pixel->ArmPixel();
    }
}


// -- Tests how the ROC reacts after power on
void TestRoc::PowerOnTest(int nTests)
{
    TestPixel * pixel = GetTestPixel();
    pixel->EnablePixel();
    pixel->Cal();
    GetDoubleColumn(pixel->GetColumn())->EnableDoubleColumn();
    gDelay->Timestamp();
    TH1D * histo = new TH1D("PowerOnTest", "PowerOnTest", nTests, 0., nTests);
    for (int i = 0; i < nTests; i++)
    {
        histo->SetBinContent(i + 1, pixel->FindThreshold("CalThresholdMap", 10));
        gDelay->Mdelay(20);
    }
    gDelay->Timestamp();
}

// Tries to automatically adjust Vana, may not work yet
int TestRoc::AdjustVana(double current0, double goalcurrent)
{
    int vana = 140;
    double currentMeasured, currentMeasuredOld;
    SetDAC("Vana", vana);
    Flush();
    gDelay->Mdelay(1000);
    currentMeasured = GetTBInterface()->GetIA();
    psi::LogDebug << "Vana " << vana << " current: " << currentMeasured - current0 << psi::endl;

    //guess value, slope is roughly 0.5 mA / DAC

    double currentDiff = currentMeasured - current0 - goalcurrent;
    int dacDiff = (int)(currentDiff / 0.0005);
    vana -= dacDiff;
    if (vana < 0) vana = 0;
    if (vana > 255) vana = 255;
    SetDAC("Vana", vana);
    Flush();
    gDelay->Mdelay(1000);
    currentMeasured = GetTBInterface()->GetIA();
    psi::LogDebug << "Vana " << vana << " current: " << currentMeasured - current0 << psi::endl;

    if (currentMeasured < current0 + goalcurrent)
    {
        do
        {
            vana++;
            SetDAC("Vana", vana);
            Flush();
            currentMeasuredOld = currentMeasured;
            currentMeasured = GetTBInterface()->GetIA();
            psi::LogDebug << "Vana " << vana << " current: " << currentMeasured - current0 << psi::endl;
        }
        while (currentMeasured < current0 + goalcurrent  && vana < 255);
        if (TMath::Abs(currentMeasuredOld - current0 - goalcurrent) < TMath::Abs(currentMeasured - current0 - goalcurrent))
        {
            vana--;
            currentMeasured = currentMeasuredOld;
        }
    }
    else
    {
        do
        {
            vana--;
            SetDAC("Vana", vana);
            Flush();
            currentMeasuredOld = currentMeasured;
            currentMeasured = GetTBInterface()->GetIA();
            psi::LogDebug << "Vana " << vana << " current: " << currentMeasured - current0 << psi::endl;
        }
        while (currentMeasured > current0 + goalcurrent  && vana > 0);
        if (TMath::Abs(currentMeasuredOld - current0 - goalcurrent) < TMath::Abs(currentMeasured - current0 - goalcurrent))
        {
            vana++;
            currentMeasured = currentMeasuredOld;
        }
    }

    psi::LogInfo() << Form("[TestRoc] Vana is set to %i, current is %.1f mA.", vana, (currentMeasured - current0) * 1000.0) << psi::endl;

    return vana;
}



void TestRoc::AdjustCalDelVthrComp()
{
    psi::LogInfo() << "[TestRoc] Adjusting CalDel and VthrComp ..." << psi::endl;
    int calDel = 0, vthrComp = 0;

    // make sure that ph above ub by setting VoffsetOp to maximum temporarily
    int vOffsetOp = GetDAC("VoffsetOp");
    SetDAC("VoffsetOp", 255);

    //Jan's additional Pixels
    //AdjustCalDelVthrComp(20, 05, 200, get_threshold_autoset_value());
    //AdjustCalDelVthrComp(21, 05, 200, get_threshold_autoset_value());
    //AdjustCalDelVthrComp(20, 30, 200, get_threshold_autoset_value());
    //AdjustCalDelVthrComp(21, 30, 200, get_threshold_autoset_value());
    //AdjustCalDelVthrComp(21, 20, 200, get_threshold_autoset_value());
    AdjustCalDelVthrComp(20, 20, 200, get_threshold_autoset_value());

    calDel += GetDAC("CalDel");
    vthrComp += GetDAC("VthrComp");

    SetDAC("CalDel", calDel);
    SetDAC("VthrComp", vthrComp);
    SetDAC("VoffsetOp", vOffsetOp);
    Flush();

    psi::LogInfo() << "[TestRoc] CalDel   set to " << calDel   << psi::endl;
    psi::LogInfo() << "[TestRoc] VthrComp set to " << vthrComp << psi::endl;
}


void TestRoc::AdjustCalDelVthrComp(int column, int row, int vcal, int belowNoise)
{
    bool verbose = false;
    const int max_range = 256; 
    int sCurve[max_range], nTrig = 5, nTrials = 5, n = 0, testColumn = column, testRow = row;
    int calDel, vthr;
    int oldCalDel = GetDAC("CalDel");
    int oldVthrComp = GetDAC("VthrComp");
    TH2D * histo;

    SetDAC("Vcal", vcal);
    Flush();

    do
    {
        TestPixel * pixel = GetPixel(testColumn, testRow);
        EnableDoubleColumn(pixel->GetColumn());

        TestRange * testRange = new TestRange();
        testRange->AddPixel(chipId, testColumn, testRow);
        DacDependency * dacTest = new DacDependency(testRange, testParameters, tbInterface);
        dacTest->SetDacs(26, 12, max_range, max_range);
        dacTest->SetNTrig(nTrig);
        dacTest->RocAction(this);
        histo = (TH2D *)(dacTest->GetHistos()->First());

        n++;
        testColumn = (testColumn + 1) % ROCNUMCOLS;

        double vthrMax = 0., vthrMin = max_range-1., sum ;
        vthr = max_range-1;
        do
        {
            sum = 0.;
            for (int caldel = 0; caldel < max_range; caldel++) sum += histo->GetBinContent(caldel + 1, (int)vthr + 1);
            if (sum > nTrig * 20) vthrMax = vthr;
            vthr--;
        }
        while (vthrMax == 0. && vthr > 0);

        vthr = 0;
        do
        {
            sum = 0.;
            for (int caldel = 0; caldel < max_range; caldel++) sum += histo->GetBinContent(caldel + 1, (int)vthr + 1);
            if (sum > nTrig * 20) vthrMin = vthr;
            vthr++;
        }
        while (vthrMin == max_range - 1. && vthr < max_range);

        psi::LogDebug() << "[TestRoc] vthr range [ " << vthrMin << ", " << vthrMax << "]." << psi::endl;

        if (belowNoise == 0) vthr = static_cast<int>((vthrMax + vthrMin) / 2);
        else if (belowNoise >  0) vthr = static_cast<int>(vthrMax - belowNoise);
        else if (belowNoise <  0) vthr = static_cast<int>(vthrMin - belowNoise);

        if (vthr > vthrMax - 5)
            vthr = vthrMax - 5;
        if (vthr < vthrMin + 5)
            vthr = vthrMin + 5;

        for (int caldel = 0; caldel < max_range; caldel++) sCurve[caldel] = (int)histo->GetBinContent(caldel + 1, (int)vthr + 1);

        int calDel1 = (int)Threshold(sCurve, 0, 1, 1, nTrig - 1);
        int calDel2 = (int)Threshold(sCurve, max_range-1, -1, 1, nTrig - 1);
        calDel = (calDel1 + calDel2) / 2;
    }
    while ((n < nTrials) && ((histo->GetMaximum() == 0) || (histo->GetBinContent(calDel + 1, vthr + 1) != nTrig)));
    if (n == nTrials)
    {
        psi::LogInfo() << "[TestRoc] Warning: Can not adjust VthrComp and CalDel."
                       << psi::endl;

        vthr = oldVthrComp;
        calDel = oldCalDel;
    }

    SetDAC("VthrComp", vthr);
    if (verbose)
        psi::LogDebug() << "[TestRoc] VthrComp is set to " << vthr << psi::endl;

    SetDAC("CalDel", calDel);
    if (verbose)
        psi::LogDebug() << "[TestRoc] CalDel is set to " << calDel << psi::endl;
}

int TestRoc::AdjustCalDel(int mode)
{
    /* Explanation of the modes:
        0: optimises to the center of the valid range
        1: optimises to the left edge of the valid range
        2: optimises to the right edge of the valid range
    */

    psi::LogInfo() << "[TestRoc] Determining optimal CalDel Value (mode " << mode << ") ..." << psi::endl;
    psi::LogDebug() << "[TestRoc] Using 9 pixels in a grid for CalDel optimisation." << psi::endl;

    /* Use a grid of 3x3 to measure the optimal CalDel across the chip */
    int optimal_caldel [9];
    optimal_caldel[0] = GetOptimalCalDel(0.1 * ROCNUMCOLS, 0.1 * ROCNUMROWS, mode);
    optimal_caldel[1] = GetOptimalCalDel(0.1 * ROCNUMCOLS, 0.5 * ROCNUMROWS, mode);
    optimal_caldel[2] = GetOptimalCalDel(0.1 * ROCNUMCOLS, 0.9 * ROCNUMROWS, mode);
    optimal_caldel[3] = GetOptimalCalDel(0.5 * ROCNUMCOLS, 0.1 * ROCNUMROWS, mode);
    optimal_caldel[4] = GetOptimalCalDel(0.5 * ROCNUMCOLS, 0.5 * ROCNUMROWS, mode);
    optimal_caldel[5] = GetOptimalCalDel(0.5 * ROCNUMCOLS, 0.9 * ROCNUMROWS, mode);
    optimal_caldel[6] = GetOptimalCalDel(0.9 * ROCNUMCOLS, 0.1 * ROCNUMROWS, mode);
    optimal_caldel[7] = GetOptimalCalDel(0.9 * ROCNUMCOLS, 0.5 * ROCNUMROWS, mode);
    optimal_caldel[8] = GetOptimalCalDel(0.9 * ROCNUMCOLS, 0.9 * ROCNUMROWS, mode);

    /* Sort the optimal values (Warning: very primitive sorting algorithm) */
    for (int i = 0; i < 9; i++) {
        for (int j = i; j < 9; j++) {
            if (optimal_caldel[j] < optimal_caldel[i]) {
                /* Exchange the values */
                int tmp = optimal_caldel[i];
                optimal_caldel[i] = optimal_caldel[j];
                optimal_caldel[j] = tmp;
            }
        }
    }

    /* Print debug output */
    psi::LogDebug() << "[TestRoc] The following optimal CalDel values were found:";
    for (int i = 0; i < 9; i++) {
        psi::LogDebug() << " " << optimal_caldel[i];
    }
    psi::LogDebug() << psi::endl;

    /* Determine the first non-negative (valid) CalDel */
    int first = -1;
    for (int i = 0; i < 9; i++) {
        if (optimal_caldel[i] >= 0) {
            first = i;
            break;
        }
    }
    /* Check whether any CalDel are valid */
    if (first == -1) {
        psi::LogInfo() << "[TestRoc] CalDel optimisation failed!" << psi::endl;
        return 0;
    }

    int caldel;
    if (mode == 0)
        /* Determine the median CalDel */
        caldel = optimal_caldel[(first + 8) / 2];
    else if (mode == 1)
        caldel = optimal_caldel[8];
    else if (mode == 2)
        caldel = optimal_caldel[first];
    else
        return -1;

    /* Set the CalDel */
    psi::LogInfo() << "[TestRoc] Setting CalDel to " << caldel << "." << psi::endl;
    SetDAC("CalDel", caldel);

    return 1;
}

int TestRoc::AdjustCalDel(int col, int row, int mode)
{
    psi::LogInfo() << "[TestRoc] Determining optimal CalDel Value for pixel " << col << ":" << row << " (mode " << mode << ") ..." << psi::endl;
    int caldel = GetOptimalCalDel(col, row, mode);

    if (caldel < 0) {
        psi::LogInfo() << "[TestRoc] CalDel optimisation failed!" << psi::endl;
        return 0;
    }

    psi::LogInfo() << "[TestRoc] Setting CalDel to " << caldel << "." << psi::endl;
    SetDAC("CalDel", caldel);

    return 1;
}

int TestRoc::GetOptimalCalDel(int col, int row, int mode)
{
    int ntrig, dac, xtalk, cals, trim, start, step_size, thr_level;
    int low, high;

    ntrig = 20;
    dac = CalDel;
    xtalk = 0;
    cals = 0;
    thr_level = ntrig / 2;
    trim = GetPixel(col, row)->GetTrim();

    start = 0;
    step_size = 1;
    ArmPixel(col, row);
    low = tbInterface->PixelThreshold(col, row, start, step_size, thr_level, ntrig, dac, xtalk, cals, trim);
    start = 255;
    step_size = -1;
    high = tbInterface->PixelThreshold(col, row, start, step_size, thr_level, ntrig, dac, xtalk, cals, trim);
    ClrCal();
    PixMask(col, row);

    psi::LogDebug() << "[TestRoc] Found CalDel range for pixel " << col << ":" << row << " to be [" << low << "-" << high << "]." << psi::endl;

    if (high <= low)
        return -1;
    if (mode == 0)
        return (high + low) / 2;
    else if (mode == 1)
        return low + (high - low) / 5;
    else if (mode == 2)
        return high - (high - low) / 5;

    return -1;
}

/**
    Adjusts the DACs "VOffsetR0" and "VIref_ADC" to set the pulse height
    into the valid ADC range for all pixels. It keeps a safety distance
    of 5 ADC units at the higher end and 10 ADC units at the lower end
    to avoid clipping due to noise.

    At first the minimal admissible Vcal value is found that works with
    all the pixels on the chip. This is because otherwise the Vcal might
    be below the threshold for some or all pixels.

    After that the VOffsetR0 DAC is scanned to center the pulse height
    in the ADC range. Only one pixel is used to maximise the speed of
    the process. Scans are made for the minimal and the maximal Vcal value
    and the midpoint (average between the two values) is used to find the
    optimal VOffsetR0 for this pixel.

    Next the VIref_ADC is changed with the maximum Vcal and adjusted such
    that the pixel has a maximum ADC value of 200 which is at a save
    distance from the upper limit of the ADC (255). This is because other
    pixels may differ from this one pixel that is being tested and it is
    important to avoid any of the pixels clipping the range.

    After adjusting VIref_ADC a pulse height map (all pixels) is made for
    the minimal and maximal Vcal. This determines the maximum range the
    pulse height can cover with the current settings. Using a fit of the
    VIref_ADC scan the optimal gain is calculated and set.

    After setting VIref_ADC another VOffsetR0 scan is made for the pixel
    with the highest pulse height to adjust the DAC such that this pixel
    has a pulseheight just at 250 which is 255 minus the safety distance
    of 5 ADC units.

    After setting this new VOffsetR0 DAC the pulse height maps are measured
    again to verify the new settings.

    @param range Specifies the Vcal range to be used. 0: low range, 1: high
    range (default: high range)
    @return 1: success, 0: failure
 */
int TestRoc::AdjustPulseHeightRange(int range)
{
    /* The function works only with ROCs that have digital readout. */
    if (has_analog_readout())
        return 0;

    /* Set Vcal range (use range value other than 0, 1 to use current range) */
    int Vcal_stored = GetDAC("Vcal");
    int CtrlReg_stored = GetDAC("CtrlReg");
    if (range == 1)
        SetDAC("CtrlReg", 4);
    else if (range == 0)
        SetDAC("CtrlReg", 0);

    /* Test range that includes all pixels */
    TestRange * testRangeFull = new TestRange();
    testRangeFull->CompleteRoc(chipId);
    ConfigParameters * configParameters = ConfigParameters::Singleton();
    testRangeFull->ApplyMaskFile(configParameters->GetMaskFileName());

    /* Make a Vcal threshold map to determine the lowest valid Vcal */
    psi::LogInfo() << "[TestRoc] Finding minimal working Vcal value ..." << psi::endl;
    ThresholdMap * thr_map_test = new ThresholdMap();
    TH2D * thr_map = thr_map_test->GetMap("VcalThresholdMap", this, testRangeFull, 5);
    delete thr_map_test;
    thr_map->SetName("ph_adjust_thr_map");
    int pix_col, pix_row, min_vcal;
    pix_col = pix_row = min_vcal = -1;
    /* Set the minimum Vcal */
    for (int col = 0; col < ROCNUMCOLS; col++) {
        for (int row = 0; row < ROCNUMROWS; row++) {
            if (thr_map->GetBinContent(col + 1, row + 1) < 200 && thr_map->GetBinContent(col + 1, row + 1) > min_vcal) {
                pix_col = col;
                pix_row = row;
                min_vcal = thr_map->GetBinContent(col + 1, row + 1);
            }
        }
    }
    /* Add some margin to make sure all pixels accept the Vcal */
    if (GetDAC("CtrlReg") & 4)
        min_vcal += 2;
    else
        min_vcal += 10;
    psi::LogInfo() << "[TestRoc] Found Vcal: " << min_vcal << psi::endl;

    /* Maximum Vcal is always 255 */
    const int max_vcal = 255;

    /* Test range that includes only one pixel */
    /* FIXME: check whether the default pixel is masked */
    TestRange * testRangeSingle = new TestRange();

    PHTest * ph_test = NULL;
    TH1D * voffset_ro_scan_low = NULL;
    TH1D * voffset_ro_scan_high = NULL;

    /* Use pixel 20:20 by default to make rough adjustments to the DACs, unless it's masked.
       This is automatically done by the GetValidPixel function of the test range. */
    testRangeFull->GetValidPixel(chipId, pix_col, pix_row);
    testRangeSingle->AddPixel(chipId, pix_col, pix_row);

    /* Make ADC insensitive to make it easier to find the midpoint.
       This avoids clipping. */
    SetDAC(20, 255);

    /* Scan VOffsetR0 DAC for minimal and maximal Vcal value */
    psi::LogInfo() << "[TestRoc] Finding optimal VOffsetR0 value ..." << psi::endl;
    /* Scan VOffsetR0 to find the right offset with both a low and a high Vcal.
       Use a loop to avoid duplication of code. */
    for (int i = 0; i < 2; i++) {
        SetDAC("Vcal", i == 0 ? min_vcal : max_vcal);
        ph_test = new PHTest(testRangeSingle, testParameters, tbInterface);
        ph_test->SetMode(17); // 17 = VOffsetR0 DAC
        ph_test->SetNTrig(1);
        ph_test->RocAction(this);
        TH1D * voffset_ro_scan = (TH1D *)(ph_test->GetHistos()->First());
        delete ph_test;
        voffset_ro_scan->SetName(Form("ph_adjust_VOffsetR0_scan_vcal_%s", i == 0 ? "low" : "high"));
        voffset_ro_scan->SetTitle(Form("VOffsetR0 scan pixel %i:%i Vcal=%i;VOffsetR0 [DAC units];Pulse height [ADC units]", pix_col, pix_row, GetDAC("Vcal")));
        if (i == 0)
            voffset_ro_scan_low = voffset_ro_scan;
        else
            voffset_ro_scan_high = voffset_ro_scan;
    }

    /* Find optimal VOffsetR0 value that puts the midpoint between minimal
       and maximal Vcal to 127 */
    int voffset_ro_optimal = -1;
    for (int i = 0; i < 256; i++) {
        if ((voffset_ro_scan_high->GetBinContent(i + 1) + voffset_ro_scan_low->GetBinContent(i + 1)) / 2 < 128) {
            voffset_ro_optimal = i;
            break;
        }
    }
    /* Abort if the optimal VOffsetR0 could not be found */
    if (voffset_ro_optimal == -1) {
        psi::LogInfo() << "[TestRoc] Warning: Cannot adjust pulse height range!" << psi::endl;
        return 0;
    }
    psi::LogInfo() << "[TestRoc] Found VOffsetR0 value: " << voffset_ro_optimal << psi::endl;
    SetDAC("VOffsetR0", voffset_ro_optimal);

    /* Scan VIref_ADC to stretch the pulse height. Use high Vcal only because high values are
       critical. */
    psi::LogInfo() << "[TestRoc] Finding optimal VIref_ADC value ..." << psi::endl;
    /* Find optimal VIref_ADC using a single pixel*/
    SetDAC("Vcal", max_vcal);
    ph_test = new PHTest(testRangeSingle, testParameters, tbInterface);
    ph_test->SetMode(20); // 20 = VIref_ADC DAC
    ph_test->SetNTrig(1);
    ph_test->RocAction(this);
    TH1D * viref_adc_scan = (TH1D *)(ph_test->GetHistos()->First());
    delete ph_test;
    viref_adc_scan->SetName(Form("ph_adjust_VIref_ADC_scan"));
    viref_adc_scan->SetTitle(Form("VIref_ADC scan pixel %i:%i Vcal=%i;VOffsetR0 [DAC units];Pulse height [ADC units]", pix_col, pix_row, GetDAC("Vcal")));

    /* Adjust VIref_ADC such that the highest VCal does not exceed ADC value 200 */
    int viref_adc_optimal = -1;
    for (int i = 0; i < 256; i++) {
        if (viref_adc_scan->GetBinContent(i + 1) < 200) {
            viref_adc_optimal = i;
            break;
        }
    }
    /* Abort if optimal VIref_ADC value could not be found */
    if (viref_adc_optimal == -1) {
        psi::LogInfo() << "[TestRoc] Warning: Cannot adjust pulse height range!" << psi::endl;
        return 0;
    }
    psi::LogInfo() << "[TestRoc] Found VIref_ADC value: " << viref_adc_optimal << psi::endl;
    SetDAC(20, viref_adc_optimal);

    // FIXME: find optimal fit range
    int viref_adc_fit_min = -1;
    for (int i = 0; i < viref_adc_optimal; i++) {
        if (viref_adc_scan->GetBinContent(i + 1) < 255) {
            viref_adc_fit_min = i;
            break;
        }
    }
    if (viref_adc_fit_min < 0) {
        psi::LogInfo() << "[TestRoc] Warning: Cannot fine tune VIref_ADC and VOffsetR0!" << psi::endl;
        return 0;
    }

    /* Now VOffsetR0 and VIref_ADC have been adjusted for one pixel.
       Continue with pulse height maps to fine tune the values
       to work for all pixels */
    psi::LogInfo() << "[TestRoc] Fine tuning VOffsetR0 and VIref_ADC ..." << psi::endl;
    TH2D * ph_map_low, * ph_map_high;
    /* Make pulse height maps for low and high Vcal */
    for (int i = 0; i < 2; i++) {
        SetDAC("Vcal", i == 0 ? min_vcal : max_vcal);
        ph_test = new PHTest(testRangeFull, testParameters, tbInterface);
        ph_test->SetMode(0);
        ph_test->SetNTrig(1);
        ph_test->RocAction(this);
        TH2D * ph_map = (TH2D *)(ph_test->GetHistos()->First());
        ph_map->SetName(Form("ph_adjust_map_vcal_%s", i == 0 ? "low" : "high"));
        ph_map->SetTitle(Form("Pulse height map Vcal=%i;Column;Row", GetDAC("Vcal")));
        TH1D * ph_dist = (TH1D *)(ph_test->GetHistos()->At(1));
        ph_dist->SetName(Form("ph_adjust_dist_vcal_%s", i == 0 ? "low" : "high"));
        delete ph_test;
        if (i == 0)
            ph_map_low = ph_map;
        else
            ph_map_high = ph_map;
    }

    TCanvas * c = new TCanvas("dummy");
    viref_adc_scan->Fit("pol1", "Q", "", viref_adc_fit_min, viref_adc_optimal);
    voffset_ro_scan_high->Fit("pol1", "Q");
    delete c;
    float viref_adc_slope = viref_adc_scan->GetFunction("pol1")->GetParameter(1);
    float voffset_ro_slope = voffset_ro_scan_high->GetFunction("pol1")->GetParameter(1);

    /* Leave some room (in ADC values) above and below the pulse height spectrum to avoid clipping.
       Below is a bit more because our minimal Vcal is not the exact minimum. */
    const int room_above = 5;
    const int room_below = 10;
    /* Difference between highest and lowest pulse height */
    int ph_delta = testRangeFull->GetMapMaximum(ph_map_high, chipId) - testRangeFull->GetMapMinimum(ph_map_low, chipId);
    psi::LogInfo() << "[TestRoc] Maximum pulse height difference: " << ph_delta << psi::endl;
    psi::LogInfo() << testRangeFull->GetMapMaximum(ph_map_high, chipId) << psi::endl;
    psi::LogInfo() << testRangeFull->GetMapMinimum(ph_map_low, chipId) << psi::endl;

    /* Calculate optimal VIref_ADC using the fit parameters from before */
    viref_adc_optimal = GetDAC(20) + (255 - ph_delta - room_above - room_below) / 2.0 / viref_adc_slope;
    SetDAC(20, viref_adc_optimal);

    /* Scan the pixel with the highest pulse height to fine tune VOffsetR0 */
    testRangeSingle->RemovePixel(chipId, pix_col, pix_row);
    testRangeFull->GetMapMaximum(ph_map_high, chipId, pix_col, pix_row);
    testRangeSingle->AddPixel(chipId, pix_col, pix_row);
    for (int i = 0; i < 2; i++) {
        SetDAC("Vcal", i == 0 ? min_vcal : max_vcal);
        ph_test = new PHTest(testRangeSingle, testParameters, tbInterface);
        ph_test->SetMode(17); // 17 = VOffsetR0 DAC
        ph_test->SetNTrig(5);
        ph_test->RocAction(this);
        TH1D * voffset_ro_scan = (TH1D *)(ph_test->GetHistos()->First());
        delete ph_test;
        voffset_ro_scan->SetName(Form("ph_adjust_fine_VOffsetR0_scan_vcal_%s", i == 0 ? "low" : "high"));
        voffset_ro_scan->SetTitle(Form("VOffsetR0 fine scan pixel %i:%i Vcal=%i;VOffsetR0 [DAC units];Pulse height [ADC units]", pix_col, pix_row, GetDAC("Vcal")));
        if (i == 0)
            voffset_ro_scan_low = voffset_ro_scan;
        else
            voffset_ro_scan_high = voffset_ro_scan;
    }
    /* Find the best VOffsetR0 value */
    for (int i = 0; i < 255; i++) {
        if (voffset_ro_scan_high->GetBinContent(i + 1) < 255 - room_above) {
            voffset_ro_optimal = i;
            break;
        }
    }
    SetDAC("VOffsetR0", voffset_ro_optimal);
    psi::LogInfo() << "[TestRoc] Set VOffsetR0=" << GetDAC("VOffsetR0") << " and VIref_ADC=" << GetDAC(20) << "." << psi::endl;

    /* Make pulse height maps for high and low Vcal to verify the optimisation */
    for (int i = 0; i < 2; i++) {
        SetDAC("Vcal", i == 0 ? min_vcal : max_vcal);
        ph_test = new PHTest(testRangeFull, testParameters, tbInterface);
        ph_test->SetMode(0);
        ph_test->SetNTrig(1);
        ph_test->RocAction(this);
        TH2D * ph_map = (TH2D *)(ph_test->GetHistos()->First());
        ph_map->SetName(Form("ph_adjust_final_map_vcal_%s", i == 0 ? "low" : "high"));
        ph_map->SetTitle(Form("Pulse height map (final) Vcal=%i;Column;Row", GetDAC("Vcal")));
        TH1D * ph_dist = (TH1D *)(ph_test->GetHistos()->At(1));
        ph_dist->SetName(Form("ph_adjust_dist2_vcal_%s", i == 0 ? "low" : "high"));
        /* Delete other histograms from the PH map test */
        while (ph_test->GetHistos()->GetEntries() > 2) {
            TObject * h = ph_test->GetHistos()->At(2);
            ph_test->GetHistos()->Remove(h);
            delete h;
        }
        delete ph_test;
        if (i == 0)
            ph_map_low = ph_map;
        else
            ph_map_high = ph_map;
    }

    delete testRangeSingle;
    delete testRangeFull;

    SetDAC("Vcal", Vcal_stored);
    SetDAC("CtrlReg", CtrlReg_stored);

    return 1;
}

void TestRoc::AdjustUltraBlackLevel(int ubLevel)
{
    //cout << "starting <TestRoc::AdjustUltraBlackLevel>: chipId = " << chipId << endl;

    int vibias = 90, ubPosition = 8 + aoutChipPosition * 3;
    short data[10000];
    unsigned short count;

    SetDAC("Ibias_DAC", vibias);
    Flush();
    GetTBInterface()->ADCData(data, count);

    int levelMeasured = data[ubPosition], levelMeasuredOld;
    //  cout << "Ibias = " << vibias << " (start value) : measured UB level = " << levelMeasured << "; target = " << ubLevel << endl;
    if (levelMeasured > ubLevel)
    {
        do
        {
            vibias++;
            SetDAC("Ibias_DAC", vibias);
            Flush();
            GetTBInterface()->ADCData(data, count);
            levelMeasuredOld = levelMeasured;
            levelMeasured = data[ubPosition];
            //      Log::Current()->printf("Ibias_DAC %i level: %i\n", vibias, levelMeasured);
            //cout << "Ibias DAC = " << vibias << " : measured UB level = " << levelMeasured << "; target = " << ubLevel << endl;
        }
        while (levelMeasured > ubLevel && vibias < 255);
        if (TMath::Abs(levelMeasuredOld - ubLevel) < TMath::Abs(levelMeasured - ubLevel))
        {
            vibias--;
            levelMeasured = levelMeasuredOld;
        }
    }
    else
    {
        do
        {
            vibias--;
            SetDAC("Ibias_DAC", vibias);
            Flush();
            GetTBInterface()->ADCData(data, count);
            levelMeasuredOld = levelMeasured;
            levelMeasured = data[ubPosition];
            //        Log::Current()->printf("Ibias_DAC %i level: %i\n", vibias, levelMeasured);
            //cout << "Ibias DAC = " << vibias << " : measured UB level = " << levelMeasured << "; target = " << ubLevel << endl;
        }
        while (levelMeasured < ubLevel  && vibias > 0);
        if (TMath::Abs(levelMeasuredOld - ubLevel) < TMath::Abs(levelMeasured - ubLevel))
        {
            vibias++;
            levelMeasured = levelMeasuredOld;
        }
    }

    SetDAC("Ibias_DAC", vibias);

    psi::LogDebug() << "[TestRoc] Ibias_DAC is set to " << vibias << psi::endl;
    psi::LogDebug() << "[TestRoc] ubLevel " << levelMeasured << psi::endl;
}


double TestRoc::Threshold(int sCurve[], int start, int sign, int step, double thrLevel)
{
    //  for (int i = 0; i < 256; i++) Log::Current()->printf("%i: %i ", i, sCurve[i]);
    //  Log::Current()->printf("\n");

    int threshold = start;
    //  Log::Current()->printf("start %i\n", threshold);
    double result;

    //avoid the problem, that the first entry is not zero, because the DAC was not set fast enough
    if ((sCurve[start] == thrLevel) && (sCurve[start + sign * step] == 0) && (sCurve[start + 2 * sign * step] == 0)) sCurve[start] = 0;

    if (sCurve[threshold] > thrLevel)
    {
        do
        {
            threshold -= sign * step;
        }
        while ((sCurve[threshold] > thrLevel) && (threshold > (step - 1)) && (threshold < (256 - step)));

        //    Log::Current()->printf("threshold %i\n", threshold);
        if (sCurve[threshold + sign * step] == sCurve[threshold]) result = threshold;
        else result = threshold + (double)(sign * step) * (double)(thrLevel - sCurve[threshold]) / (double)(sCurve[threshold + sign * step] - sCurve[threshold]);
    }
    else
    {
        do
        {
            threshold += sign * step;
        }
        while ((sCurve[threshold] <= thrLevel) && (threshold > (step - 1)) && (threshold < (256 - step)));

        //    Log::Current()->printf("threshold %i\n", threshold);
        if (sCurve[threshold - sign * step] == sCurve[threshold]) result = threshold;
        else result = threshold - (double)(sign * step) * (double)(thrLevel - sCurve[threshold]) / (double)(sCurve[threshold - sign * step] - sCurve[threshold]);
    }

    result = TMath::Min(result, 255.);
    result = TMath::Max(result, 0.);

    /*  if (result < 5. || result > 250.)
      {
        Log::Current()->printf("result %f, sign %i\n", result, sign);
        Log::Current()->printf("thr %i %i\n", start, threshold);
        for (int i = 0; i < 256; i++) Log::Current()->printf("%i ", sCurve[i]);
        Log::Current()->printf("\n");
      } */
    return result;
}


void TestRoc::SendSignals(int start, int stop, int step, int nTrig, char * dacName)
{
    for (int i = start; i < stop; i += step)
    {
        SetDAC(dacName, i);
        if (i == start) CDelay(1000);  // The jump from a very high value down to very low value may need more time
        SendCal(nTrig);
    }
}


void TestRoc::ReadSignals(int start, int stop, int step, int nTrig, int sCurve[])
{
    for (int i = 0; i < 256; i++) sCurve[i] = 0;
    for (int i = start; i < stop; i += step)
    {
        for (int k = 0; k < nTrig; k++)
        {
            sCurve[i] += RecvRoCnt();
        }
    }
}


// == Histos =================================================================


TH1D * TestRoc::DACHisto()
{
    int NDACParameters = 256;
    TH1D * histo = new TH1D(Form("DACHisto_C%i", chipId), Form("DACHisto_C%i", chipId), NDACParameters, 0., NDACParameters);
    int value;
    for (int i = 0; i < NDACParameters; i++)
    {   value = dacParameters->GetDAC(i);
        //    printf("value %i\n", value);
        histo->SetBinContent(i + 1, value);
    }
    return histo;
}


TH2D * TestRoc::TrimMap()
{
    TH2D * map = new TH2D(Form("TrimMap_C%d", chipId), Form("TrimMap_C%d", chipId), ROCNUMCOLS, 0, ROCNUMCOLS, ROCNUMROWS, 0, ROCNUMROWS);
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            map->SetBinContent(i + 1, k + 1, GetPixel(i, k)->GetTrim());
        }
    }
    return map;
}


Double_t Fitfcn2(Double_t * x, Double_t * par)
{
    return par[1] * x[0] + par[0];
}


double TestRoc::GetTemperature()
{
    bool debug = false;
    //  Log::Current()->printf("-----------------------------------------------------------\n");
    psi::LogInfo() << "[TestRoc] Temperature for ROC #" << chipId << psi::endl;

    const int nTriggers = 10;
    int temp[8], calib[8];

    // get black level
    unsigned short count;
    short data[10000], blackLevel;

    GetTBInterface()->ADCRead(data, count, nTriggers);
    blackLevel = data[9 + aoutChipPosition * 3];
    if (debug)
        psi::LogDebug() << "[TestRoc] blackLevel " << blackLevel << psi::endl;

    // Calibrate

    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++)
    {
        SetDAC("RangeTemp", rangeTemp + 8);
        Flush();
        calib[rangeTemp] = GetTBInterface()->LastDAC(nTriggers, aoutChipPosition);
        if (debug)
            psi::LogDebug() << "[TestRoc] Calib " << calib[rangeTemp] << psi::endl;
    }

    // Measure temperature

    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++)
    {
        SetDAC("RangeTemp", rangeTemp);
        Flush();
        temp[rangeTemp] = GetTBInterface()->LastDAC(nTriggers, aoutChipPosition);
        if (debug)
            psi::LogDebug() << "[TestRoc] Temperature " << temp[rangeTemp] << psi::endl;
    }

    // Compute voltage

    int n = 0;
    double x[8], y[8];

    for (int i = 0; i < 8; i++)
    {
        if (TMath::Abs(calib[i] - blackLevel) > 20)
        {
            x[n] = calib[i] - blackLevel;
            y[n] = -70. + (7 - i) * 164. / 7.;
            n++;
        }
    }
    if (n == 0) return -9999.;
    if (debug)
        psi::LogDebug() << "[TestRoc] n = " << n << psi::endl;

    if (debug)
        for (int i = 0; i < n; i++)
            psi::LogDebug() << "[TestRoc] x " << x[i] << ", y " << y[i] << psi::endl;

    TGraph * graph = new TGraph(n, x, y);
    TF1 * fit = new TF1("fit", Fitfcn2, 0., 2000., 2);
    fit->SetParameter(0, 50.);
    fit->SetParameter(1, 0.1);
    graph->Fit("fit", "NRQ");

    double voltage = 400. + fit->Eval(temp[0] - blackLevel);
    if (debug)
        psi::LogDebug() << "[TestRoc] Voltage " << voltage << psi::endl;

    //Convert to temperature
    double temperature = (voltage - 410.) * 5. / 8.;
    psi::LogDebug() << "[TestRoc] Temperature " << temperature << psi::endl;

    delete graph;
    delete fit;

    return temperature;
}

/**
    Measures a threshold map in terms of Vcal with the current VthrComp
    setting. The threshold map takes timewalk into account by testing
    two WBCs.
 */
void TestRoc::TrimVerification()
{
    gDelay->Timestamp();
    psi::LogInfo() << "[TestRoc] Measuring VcalThresholdMap for ROC " << chipId << " with double WBC ..." << psi::endl;

    SaveDacParameters();

    ThresholdMap * thresholdMap = new ThresholdMap();
    thresholdMap->SetDoubleWbc();
    TH2D * map = thresholdMap->GetMap("VcalThresholdMap", this, GetRange(), 5);
    TH1D * dist = gAnalysis->Distribution(map, 255, 0., 255.);
    bool overflow = dist->GetBinContent(0) > 0 || dist->GetBinContent(256) > 0;
    psi::LogInfo() << "[TestRoc] Mean Vcal = " << dist->GetMean() << ", RMS = " << dist->GetRMS() << (overflow ? " (overflow!)" : "") << psi::endl;

    RestoreDacParameters();

    gDelay->Timestamp();
}

void TestRoc::ThrMaps()
{
    gDelay->Timestamp();
    psi::LogInfo() << "[TestRoc] Measuring VcalThresholdMap for ROC " << chipId << " with double WBC ..." << psi::endl;
    ThresholdMap * thresholdMap = new ThresholdMap();
    thresholdMap->SetDoubleWbc(); //absolute threshold (not in-time)

    SaveDacParameters();

    TestRange * testRange = new TestRange();
    testRange->CompleteRoc(chipId);

    TH2D * vcalMap = thresholdMap->GetMap("VcalThresholdMap", this, testRange, 5);
    vcalMap->SetNameTitle(Form("VcalThresholdMap_C%i", chipId), Form("VcalThresholdMap_C%i", chipId));
    vcalMap->Write();
    TH1D * dist = gAnalysis->Distribution(vcalMap);
    dist->Write();

    bool overflow = dist->GetBinContent(0) > 0 || dist->GetBinContent(256) > 0;
    psi::LogInfo() << "[TestRoc] Mean Vcal = " << dist->GetMean() << ", RMS = " << dist->GetRMS() << (overflow ? " (overflow!)" : "") << psi::endl;

    RestoreDacParameters();

    gDelay->Timestamp();
}

    

/*void TestRoc::Scurves(){
TestRange *fullRange = new TestRange();
for (int iCol = 0; iCol <= 51; iCol++)
                {
                for (int iRow = 0; iRow <= 79; iRow++)
                    {
                    fullRange->AddPixel(0, iCol, iRow);
                        }
                    }
  Test *testSc;
    gDelay->Timestamp();
testSc = new SCurveTest(fullRange, testParameters, tbInterface);
testSc->ModuleAction();
 Flush();
}*/

// added by Tilman Oct. 2009 for Pulseshape determination
//
void TestRoc::DoPulseShape()
{
    cout << "==================================================================================\n";
    cout << "=                                                                                =\n";
    cout << "=    Third try of an implementation of the rise time estimation.                 =\n";
    cout << "=    .                                                                           =\n";
    cout << "=                                                                                =\n";
    cout << "=    At the moment this program loops over every 4th pixel in each direction     =\n";
    cout << "=    at Vcal 120                                                                 =\n";
    cout << "=                                                          Hank, March 2010     =\n";
    cout << "==================================================================================\n";

    //  DoPulseShape(5,5,120);//



    int c_skip = 4;
    int r_skip = 4;
    int c_num = 52;
    int r_num =  80;
    int c_offset = 0;
    int r_offset = 0;
    int i_max = c_num / c_skip;
    int j_max = r_num / r_skip;

    TH2D * risetime = new TH2D("RiseTime", "Risetime", c_num + 1, -1, c_num, r_num + 1, -1, r_num);
    double time;
    for (int i = 0; i < (i_max); i++) {
        for (int j = 0; j < (j_max); j++) {
            time = DoPulseShape(c_skip * i + c_offset, r_skip * j + r_offset, 120);
            risetime->SetBinContent(c_skip * i + c_offset + 1, r_skip * j + r_offset + 1, time);
        }
    }
    risetime->Write();
}

double TestRoc::DoPulseShape(int column, int row, int vcal)
{


    bool verbose = false;
    int nTrig = 5, testColumn = column, testRow = row, testVcal = vcal;
    psi::LogInfo() << "Find pulse shape pixel column " << column << " row " << row << psi::endl;

    // 1st Step scan Vthr vs CalDel

    psi::LogInfo() << "Scan Vthr vs CalDel, Vcal = " << testVcal << psi::endl;
    int calDel, vthr;
    int oldCalDel = GetDAC("CalDel");
    int oldVthrComp = GetDAC("VthrComp");
    ;
    TH2D * ptVthrVsCalDel;
    SetDAC("Vcal", testVcal);
    Flush();

    TestPixel * pixel = GetPixel(testColumn, testRow);
    EnableDoubleColumn(pixel->GetColumn());

    TestRange * testRange = new TestRange();
    testRange->AddPixel(chipId, testColumn, testRow);
    DacDependency * dacTest = new DacDependency(testRange, testParameters, tbInterface);
    dacTest->SetDacs(26, 12, 256, 256);
    dacTest->SetNTrig(nTrig);
    dacTest->RocAction(this);

    SetDAC("CalDel", oldCalDel); // restore old CalDel value

    ptVthrVsCalDel = (TH2D *)(dacTest->GetHistos()->First());
    ptVthrVsCalDel->Write();

    psi::LogInfo() << "Scan Vthr vs CalDel finished" << psi::endl;
    psi::LogInfo() << "===" << psi::endl;
    psi::LogInfo() << "Find lowest threshold for Vcal vs CalDel scan" << psi::endl;

    // Find Lowest Threshold: move from top of plot (in Oy)
    int minThreshold = 0;
    for (int binCounterY = ptVthrVsCalDel->GetNbinsY(); binCounterY > 0; binCounterY--) {
        int nBins = 0;
        for (int binCounterX = 1; binCounterX < ptVthrVsCalDel->GetNbinsX(); binCounterX++) {
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) nBins++;
        }
        if ((nBins > 15) && (nBins < 80)) { // if too many bins are "effective" this is due to noise
            minThreshold = binCounterY; // in all those histograms binNumber = value
            break;
        }
    }

    minThreshold -= 10 ; // just for safety

    psi::LogInfo() << "Lowest threshold - 10 is " << minThreshold << psi::endl;
    psi::LogInfo() << "============================" << psi::endl;


    //=======================================================

    psi::LogInfo() << "Scan Vthr vs Vcal, CalDel = " << oldCalDel << psi::endl;

    TH2D * ptVthrVsVcal; //pointer

    DacDependency * dacTest2 = new DacDependency(testRange, testParameters, tbInterface);
    dacTest2->SetDacs(25, 12, 256, 256);
    dacTest2->SetNTrig(nTrig);
    dacTest2->RocAction(this);

    ptVthrVsVcal = (TH2D *)(dacTest2->GetHistos()->First());
    TH2D hVthrVsVcal(*ptVthrVsVcal);//histogram
    // rename
    char hisName[100], hisNameBase[100];
    strcpy(hisNameBase, hVthrVsVcal.GetName());
    sprintf(hisName, "%s_WBC", hisNameBase);
    //cout << "New his name: " << hisName << endl;

    hVthrVsVcal.SetName(hisName);
    //  hVthrVsVcal.Write();

    //Log::Current()->printf("Scan Vthr vs Vcal finished\n");
    psi::LogInfo() << "===" << psi::endl;


    // ===

    int oldWBC = GetDAC("WBC");
    SetDAC("WBC", oldWBC - 1);
    psi::LogInfo() << "Scan Vthr vs Vcal at WBC-1, CalDel = " << oldCalDel << psi::endl;
    psi::LogInfo() << "Old WBC= " << oldWBC << " now use WBC-1 =  " << oldWBC - 1 << psi::endl;

    TH2D * ptVthrVsVcalWBCm1; //pointer

    DacDependency * dacTest3 = new DacDependency(testRange, testParameters, tbInterface);
    dacTest3->SetDacs(25, 12, 256, 256);
    dacTest3->SetNTrig(nTrig);
    dacTest3->RocAction(this);
    SetDAC("WBC", oldWBC); // restore old WBC value


    ptVthrVsVcalWBCm1 = (TH2D *)(dacTest3->GetHistos()->First());
    TH2D hVthrVsVcalWBCm1(*ptVthrVsVcalWBCm1);

    sprintf(hisName, "%s_WBCm1", hisNameBase);
    //cout << "New his name: " << hisName << endl;
    hVthrVsVcalWBCm1.SetName(hisName);
    //  hVthrVsVcalWBCm1.Write();

    //Log::Current()->printf("Scan Vthr vs Vcal at WBC-1 finished\n");
    psi::LogInfo() << "===" << psi::endl;
    psi::LogInfo() << "Add the histograms at WBC and WBC-1 " << psi::endl;

    // ===


    SetDAC("WBC", oldWBC - 2);
    psi::LogInfo() << "Scan Vthr vs Vcal at WBC-1, CalDel = " << oldCalDel << psi::endl;
    psi::LogInfo() << "Old WBC= " << oldWBC << " now use WBC-1 = " << oldWBC - 2 << psi::endl;

    TH2D * ptVthrVsVcalWBCm2; //pointer

    DacDependency * dacTest5 = new DacDependency(testRange, testParameters, tbInterface);
    dacTest5->SetDacs(25, 12, 256, 256);
    dacTest5->SetNTrig(nTrig);
    dacTest5->RocAction(this);
    SetDAC("WBC", oldWBC); // restore old WBC value


    ptVthrVsVcalWBCm2 = (TH2D *)(dacTest5->GetHistos()->First());
    TH2D hVthrVsVcalWBCm2(*ptVthrVsVcalWBCm2);

    sprintf(hisName, "%s_WBCm2", hisNameBase);
    //cout << "New his name: " << hisName << endl;
    hVthrVsVcalWBCm2.SetName(hisName);
    //  hVthrVsVcalWBCm2.Write();

    //Log::Current()->printf("Scan Vthr vs Vcal at WBC-1 finished\n");
    psi::LogInfo() << "===\n" << psi::endl;
    psi::LogInfo() << "Add the histograms at WBC and WBC-1 " << psi::endl;

    TH2D * hVthrVsVcal_tot(&hVthrVsVcal);
    hVthrVsVcal_tot->Add(&hVthrVsVcalWBCm1);
    hVthrVsVcal_tot->Add(&hVthrVsVcalWBCm2);
    sprintf(hisName, "%s_tot", hisNameBase);
    //  hVthrVsVcal;
    //  hVthrVsVcalWBCm1;
    //  hVthrVsVcalWBCm2;
    hVthrVsVcal_tot->Write(hisName);



    psi::LogInfo() << "into " << hisName << psi::endl;
    psi::LogInfo() << "============================" << psi::endl;


    // Scan Vcal vs CalDel at Vthr=min to define time == 0
    psi::LogInfo() << "Scan Vcal vs CalDel at Vthr = " << minThreshold << psi::endl;
    SetDAC("VthrComp", minThreshold);

    TH2D * ptVcalVsCalDel;

    DacDependency * dacTest4 = new DacDependency(testRange, testParameters, tbInterface);
    dacTest4->SetDacs(26, 25, 256, 256);
    dacTest4->SetNTrig(nTrig);
    dacTest4->RocAction(this);

    ptVcalVsCalDel = (TH2D *)(dacTest4->GetHistos()->First());

    SetDAC("VthrComp", oldVthrComp); // restore old Vthr value

    ptVcalVsCalDel = (TH2D *)(dacTest4->GetHistos()->First());
    //  ptVcalVsCalDel->Write();

    psi::LogInfo() << "Scan Vthr vs CalDel finished\n" << psi::endl;
    psi::LogInfo() << "============================\n" << psi::endl;


    //This section includes the supplementals necessary for offline processing of
    //The data, and should be commented out for online running.
    /*  int nTrig = 5;
        int minThreshold = 117;
    */  //----This is the end of the supplemental code.
    char name[50];
    char pixelname[15];

    sprintf(pixelname, "PH_C%i_R%i", testColumn, testRow);
    //supplemental for offline
    /*  sprintf(name,"VthrCompCalDel_c%ir%i_C0",testColumn,testRow);
        TH2D *ptVthrVsCalDel = (TH2D*)f.Get(name);

        sprintf(name,"VthrCompVcal_c%ir%i_C0_tot",testColumn,testRow);
        TH2D *hVthrVsVcal_tot = (TH2D*)f.Get(name);

        sprintf(name,"VcalCalDel_c%ir%i_C0",testColumn,testRow);
        TH2D *ptVcalVsCalDel = (TH2D*)f.Get(name);
    */  //end of supplemental
    sprintf(name, "CalDelcal_C%i_R%i", testColumn, testRow);
    TH2D * ptCalDelcalib = new TH2D(name, name, 256, 0, 256, 256, 0, 256);

    sprintf(name, "CalDelwidth_C%i_R%i", testColumn, testRow);
    TH1D * ptCalDelwidth = new TH1D(name, name, 256, 0, 256);

    sprintf(name, "Vthrcalib_C%i_R%i", testColumn, testRow);
    TH2D * ptVthrcalib = new TH2D(name, name, 256, 0, 256, 256, 0, 256);

    sprintf(name, "Vthrline_C%i_R%i", testColumn, testRow);
    TH1D * ptVthrline = new TH1D(name, name, 256, 0, 256);

    sprintf(name, "PHdataPoints_C%i_R%i", testColumn, testRow);
    TH2D * ptPHdataPoints = new TH2D(name, name, 256, 0, 256, 256, 0, 256);

    sprintf(name, "PHintcurve_C%i_R%i", testColumn, testRow);
    TH1D * ptPHintcurve = new TH1D(name, name, 256, 0, 256);

    int pulseHeight;

    psi::LogInfo() << "The Pixel is : " << pixelname << psi::endl;
    psi::LogInfo() << "The number of bins is " << ptVcalVsCalDel->GetNbinsX() << psi::endl;
    psi::LogInfo() << "The nTrig is " << nTrig << psi::endl;
    psi::LogInfo() << "The minThreshold is " << minThreshold << psi::endl;


    printf("Find t_0 from Vthr vs CalDel Scan\n");

    //
    //  Vcal
    //  |         .________.
    //  |         |        |
    //  |         |        |
    //  |         |        |
    //  |         |        |
    //  |         |        |
    //  |         |        |
    //  |        /         |
    //  |      _/         /
    //  |   __/         _/
    //  |__/       ____/
    //  |_________/
    //  |
    //  +-------------------------------- CalDel

    // Find Right Edge (~ Time of Vcal injection)
    // CalDel for large signals and small thresholds = tZero
    int tZero = 0;

    for (int binCounterX = ptVcalVsCalDel->GetNbinsX(); binCounterX > 0; binCounterX--) {
        int nBins = 0;
        for (int binCounterY = ptVcalVsCalDel->GetNbinsY(); binCounterY > 0; binCounterY--) {
            if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) nBins++;
        }
        if (nBins > 2) {
            tZero = binCounterX; // in all those histograms binNumber = value
            break;
        }
    }
    psi::LogInfo() << "  t_0 is at CalDel " << tZero << psi::endl;



    // scan the Vcal vs CalDel tornado from
    // top (minThreshold) to bottom and measure
    // right edge (time) and width of the tornado
    // use a very simplistic method:
    // right edge is defined as (last bin with value zero) - (first bin with value nTrig) / 2
    // last bin with zero is defined a the 1st bin from the right with 0 followed by >=3 bins >0
    // first bin with nTrig is defined as the first bin from the right with nTrig followed by >= 2
    //  bins with nTrig.
    // left edge is defined accordingly
    // only lines with > 30 bins with value nTrig are considered.
    cout << "Find CalDel cal width from Vcal vs CalDel Scan" << endl;
    double widthavg = 0., widthtot = 0.;
    int nLinesUsed = 0;
    for (int binCounterY = ptVcalVsCalDel->GetNbinsY(); binCounterY > 0; binCounterY--) {

        double rightEdge = 256., leftEdge = 0., width = 0.;
        int lastBinZero = 256, nBinNotZero = 0, firstBinnTrig = 0, nBinnTrig = 0;
        // right edge
        for (int binCounterX = ptVcalVsCalDel->GetNbinsX(); binCounterX > 0; binCounterX--) {
            if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) > 0) {
                nBinNotZero++;
                if (nBinNotZero == 3) {
                    lastBinZero = binCounterX - 3;

                    //              cout << lastBinZero << endl;
                }
            }
            else {
                nBinNotZero = 0;
            }
            if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) {
                nBinnTrig++;
                if (nBinnTrig == 3) {
                    firstBinnTrig = binCounterX - 3;
                    rightEdge = (lastBinZero + firstBinnTrig) / 2;
                    //              cout << firstBinnTrig << "    " << lastBinZero << "    " << rightEdge << endl;
                    ptCalDelcalib->SetBinContent(lastBinZero, binCounterY, 1);
                    ptCalDelcalib->SetBinContent(firstBinnTrig, binCounterY, 3);
                    break;
                }
            } else {
                nBinnTrig = 0;
            }
        }

        // left edge is only searched if a right edge was found
        int nBinZero = 0;
        int lastBinNotZero = 256,  firstBinnNotTrig = 0, nBinnNotTrig = 0;
        nBinNotZero = 0;
        if (rightEdge < 256.) {
            for (int binCounterX = rightEdge; binCounterX > 0; binCounterX--) {
                if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) < 1) {
                    nBinZero++;
                    if (nBinZero == 3) lastBinNotZero = binCounterX - 3;
                } else {
                    nBinNotZero = 0;
                }
                if (ptVcalVsCalDel->GetBinContent(binCounterX, binCounterY) == 0) {
                    nBinnNotTrig++;
                    if (nBinnNotTrig == 3) {
                        firstBinnNotTrig = binCounterX - 3;
                        leftEdge = (lastBinNotZero + firstBinnNotTrig) / 2.0;
                        //                  cout << firstBinnNotTrig << "    " << lastBinNotZero << "    " << leftEdge << endl;
                        ptCalDelcalib->SetBinContent(lastBinNotZero, binCounterY, 2);
                        ptCalDelcalib->SetBinContent(firstBinnNotTrig, binCounterY, 4);
                        break;
                    }
                } else {
                    nBinnNotTrig = 0;
                }
            }
            width = rightEdge - leftEdge;
            ptCalDelwidth->SetBinContent(binCounterY, width);
            //      cout << width << endl;
            if (width > 30. && width < 80) {
                widthtot += width;
                nLinesUsed++;
                //          cout << width << endl;
            }
        }
    }
    ptCalDelcalib->Write();
    //  ptCalDelwidth->Write();
    widthavg = widthtot / nLinesUsed;
    psi::LogInfo() << "  cal width is " << widthavg << psi::endl;


    //scan Vthrcomp vs Vcal
    //The purpose of this test is to scan the Vthrcomp vs. VCal
    //curve to get the linear relation between Vthrcomp and Vcal.
    //It currently takes in the WBC and WBC summed file and
    //Searches from the upper right to bottom left for points
    //where the signal goes from nTrig to 0.  It uses the same
    //matching function as above where it takes the location of
    //nTrig as the last place 3 nTrigs were seen in a row and then
    //The first place a 3 0's are seen and averages the two of them
    //To get the location for that point.  Once the two have been
    //Found for all Vthrcomps, it will fit to a curve and output the
    //calibration factor for Vthrcomp.

    // Vthrcomp
    //  |
    //  |   \------\---------------------
    //  |    \------\--------------------
    //  |     \------\-------------------
    //  |      \------\------------------
    //  |       \-------\----------------
    //  |        \---------\-------------
    //  |         \-----------\----------
    //  |          \-------------\-------
    //  |           \---------------\----
    //  +-------------------------------- Vcal
    //  We are looking for the linear line that is at the end, whether it comes from
    //  WBC n or if we have to look lower at WBC n-1.  This has already been added
    //  Togehter and the curved line should be a point where spurious counts from 5
    //  to 10 can occur, so we can assume that the searcher will pass over them and
    //  go to the end at the left side for each point.

    cout << "Find the VthrComp calibration from Vthrcomp vs. Vcal" << endl;
    int Vthrmin = 0; //This is the starting threshold for the Vthrcomp vs Vcal calb.
    double leftEdge = 0;
    int numBins;
    for (int binCounterY = hVthrVsVcal_tot->GetNbinsY(); binCounterY > 0; binCounterY--) {
        numBins = 0;
        for (int binCounterX = hVthrVsVcal_tot->GetNbinsX(); binCounterX > 150; binCounterX--) {
            if (hVthrVsVcal_tot->GetBinContent(binCounterX, binCounterY) != 0) {
                numBins ++;
                if (numBins > 10) {
                    Vthrmin = binCounterY - 6;  //Arbitary drop to clear the noise
                    break;
                }
            }
        }
        if (Vthrmin != 0) {
            break;
        }
    }
    for (int binCounterY = Vthrmin; binCounterY > 0; binCounterY--) {
        int nBinZero = 0;
        int lastBinNotZero = 256, firstBinnNotTrig = 0, nBinnNotTrig = 0;
        for (int binCounterX = hVthrVsVcal_tot->GetNbinsX(); binCounterX > 0; binCounterX--) {
            if (hVthrVsVcal_tot->GetBinContent(binCounterX, binCounterY) < nTrig) {
                nBinnNotTrig++;
                if (nBinnNotTrig == 3) {
                    firstBinnNotTrig = binCounterX - 3;
                }
            } else {
                nBinnNotTrig = 0;
            }

            if (hVthrVsVcal_tot->GetBinContent(binCounterX, binCounterY) < 1) {
                nBinZero++;
                if (nBinZero == 3) {
                    lastBinNotZero = binCounterX - 3;
                    leftEdge = (lastBinNotZero + firstBinnNotTrig) / 2.0;
                    ptVthrcalib->SetBinContent(lastBinNotZero, binCounterY, 1);
                    ptVthrcalib->SetBinContent(firstBinnNotTrig, binCounterY, 2);
                    ptVthrline->SetBinContent(binCounterY, leftEdge);
                    //                          cout << firstBinnNotTrig << "    " << lastBinNotZero << "    " << leftEdge << endl;
                    break;
                }
            } else {
                nBinZero = 0;
            }
        }
    }
    //  ptVthrcalib->Write();
    ptVthrline->Write();
    ptVthrline->Draw("A*");
    ptVthrline->Fit("pol1", "Q");
    TF1 * VthrlineParam = ptVthrline->GetFunction("pol1");
    psi::LogInfo() << "  The y interscept of the Vthrcomp vs. Vcal is " << VthrlineParam->GetParameter(0) << psi::endl;
    psi::LogInfo() << "  The slope of the V thrcomp vs. Vcal is " << VthrlineParam->GetParameter(1) << psi::endl;

    /* Scan vthrcomp vs CalDel
        //
        //  Vthrcomp
        //  |         .________.
        //  |         |        |
        //  |         |        |
        //  |         |        |
        //  |         |        |
        //  |         |        |
        //  |         |        |
        //  |        /         |
        //  |      _/         /
        //  |   __/         _/
        //  |__/       ____/
        //  |_________/
        //  |
        //  +-------------------------------- CalDel
        Here we will be using the Vthrcomp vs CalDel graph to actually
        extract the pulse height curve.  This corresponds to the two edges
        tornado.  For this curve, we will find both the left and right edge
        of the graph, and then we will do a linear transformation of the right
        edge and overlay it on the left to get extra points of data for the
        curve fit.  This will use the same routines that have been implemented
        before and the only difference will be that we are now going to sum
        them sides instead of finding the width.  The widthavg will be used as
        the number for the linear transformation to shift by.  This part should
        output the inital edges, the average edges, the linear transformed edges,
        and finally the totally calibration pulse height curve using all of the
        data collected above.
    */

    cout << "Find Pulse Height Curves from Vthrcomp vs. CalDel" << endl;

    for (int binCounterY = minThreshold; binCounterY > 0; binCounterY--) {

        double rightEdge = 256., leftEdge = 0.;
        int lastBinZero = 256, nBinNotZero = 0, firstBinnTrig = 0, nBinnTrig = 0;
        // right edge
        for (int binCounterX = ptVthrVsCalDel->GetNbinsX(); binCounterX > 0; binCounterX--) {
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) > 0) {
                nBinNotZero++;
                if (nBinNotZero == 3) {
                    lastBinZero = binCounterX - 3;

                    //              cout << lastBinZero << endl;
                }
            }
            else {
                nBinNotZero = 0;
            }
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) {
                nBinnTrig++;
                if (nBinnTrig == 3) {
                    firstBinnTrig = binCounterX - 3;
                    rightEdge = (lastBinZero + firstBinnTrig) / 2;
                    //              cout << firstBinnTrig << "    " << lastBinZero << "    " << rightEdge << endl;
                    ptPHdataPoints->SetBinContent(lastBinZero, binCounterY, 1);
                    ptPHdataPoints->SetBinContent(firstBinnTrig, binCounterY, 3);
                    break;
                }
            } else {
                nBinnTrig = 0;
            }
        }

        // left edge is only searched if a right edge was found
        int nBinZero = 0;
        int lastBinNotZero = 256, firstBinnNotTrig = 0, nBinnNotTrig = 0;
        nBinNotZero = 0;
        if (rightEdge < 256.) {
            for (int binCounterX = rightEdge; binCounterX > 0; binCounterX--) {
                if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) < 1) {
                    nBinZero++;
                    if (nBinZero == 3) lastBinNotZero = binCounterX - 3;
                } else {
                    nBinNotZero = 0;
                }
                if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) == 0) {
                    nBinnNotTrig++;
                    if (nBinnNotTrig == 3) {
                        firstBinnNotTrig = binCounterX - 3;
                        leftEdge = (lastBinNotZero + firstBinnNotTrig) / 2.0;
                        //                  cout << firstBinnNotTrig << "    " << lastBinNotZero << "    " << leftEdge << endl;
                        ptPHdataPoints->SetBinContent(lastBinNotZero, binCounterY, 2);
                        ptPHdataPoints->SetBinContent(firstBinnNotTrig, binCounterY, 4);
                        pulseHeight = (rightEdge + leftEdge + widthavg) / 2;
                        //                  cout << rightEdge << " " << leftEdge << "  " << pulseHeight << endl;
                        ptPHintcurve->SetBinContent(pulseHeight, binCounterY);
                        break;
                    }
                } else {
                    nBinnNotTrig = 0;
                }
            }
        }
    }
    for (int binCounterX = 150; binCounterX > 0; binCounterX--) {

        double rightEdge = 256.;
        int lastBinZero = 256, nBinNotZero = 0, firstBinnTrig = 0, nBinnTrig = 0;
        // right edge
        for (int binCounterY = 0; binCounterY < ptVthrVsCalDel->GetNbinsY(); binCounterY++) {
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) > 0) {
                nBinNotZero++;
                if (nBinNotZero == 3) {
                    lastBinZero = binCounterY - 3;

                    //              cout << lastBinZero << endl;
                }
            }
            else {
                nBinNotZero = 0;
            }
            if (ptVthrVsCalDel->GetBinContent(binCounterX, binCounterY) == nTrig) {
                nBinnTrig++;
                if (nBinnTrig == 3) {
                    firstBinnTrig = binCounterY - 3;
                    rightEdge = (lastBinZero + firstBinnTrig) / 2;
                    //              cout << firstBinnTrig << "    " << lastBinZero << "    " << rightEdge << endl;
                    ptPHdataPoints->SetBinContent(binCounterX, lastBinZero, 5);
                    ptPHdataPoints->SetBinContent(binCounterX, firstBinnTrig, 6);
                    double edgeavg;
                    if (ptPHintcurve->GetBinContent(binCounterX) != 0) {
                        edgeavg = (rightEdge + ptPHintcurve->GetBinContent(binCounterX)) / 2.0;
                    } else {
                        edgeavg = rightEdge;
                    }
                    ptPHintcurve->SetBinContent(binCounterX, edgeavg);
                    break;
                }
            } else {
                nBinnTrig = 0;
            }
        }
    }
    ptPHdataPoints->Write();
    ptPHintcurve->Write();
    double x[256];
    double pHlevel[256];
    double pHuncal;
    double slope = VthrlineParam->GetParameter(1);
    double yinter = VthrlineParam->GetParameter(0);
    int counter = 0;
    for (int i = 0; i < 256; i++) {
        pHuncal = ptPHintcurve->GetBinContent(i);
        if (pHuncal != 0) {
            x[counter] = (25.0 / widthavg * (tZero - i)); //Set units of ns
            pHlevel[counter] = yinter + pHuncal * slope;
            //          cout << x[counter] << "  " << pHlevel[counter] << endl;
            counter++;
        }
    }

    TGraph * PHcurve = new TGraph(counter, x, pHlevel);
    PHcurve->SetTitle(pixelname);
    PHcurve->Write(pixelname);

    //This section will find the rise time for the pixel.  It does this first, by searching for the maximum value
    //of the curve.  It then looks for the point that has 90% of that height, and then outputs the time for this.
    double pHmax = 0;
    double riseTime = 0;
    for (int i = counter; i > 0; i--) {
        if (pHlevel[i] > pHmax) {
            pHmax = pHlevel[i];
        }
    }
    for (int i = counter; i > 0; i--) {
        if (pHlevel[i] > 0.9 * pHmax) {
            riseTime = x[i];
            break;
        }
    }
    psi::LogInfo() << "The rise time is: " << riseTime << psi::endl;
    psi::LogInfo() << "==============================" << psi::endl;

    delete ptVcalVsCalDel;
    delete ptCalDelwidth;
    delete ptVthrVsCalDel;
    return riseTime;
}


// utilities for analizing the historgams in DoPulseShape
//  int fitLeft(TH2D *his, int line);

int fitRight(TH2D * his, int line, int maxEntries) {


}
