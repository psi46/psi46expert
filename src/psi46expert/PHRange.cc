#include <iostream>
#include <stdio.h>

#include "TH1D.h"

#include "interface/Log.h"
#include "BasePixel/Roc.h"
#include "BasePixel/TBInterface.h"
#include "TestRoc.h"
#include "PHRange.h"


PHRange::PHRange(TestRange * aTestRange, TestParameters * aTestParameters, TBInterface * aTBInterface)
{
    psi::LogDebug() << "[PHRange] Initialization." << psi::endl;

    testParameters = aTestParameters;
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
}


void PHRange::ReadTestParameters(TestParameters * testParameters)
{
    phSafety = (*testParameters).PHSafety; //DELTA
    tbmUbLevel = (*testParameters).TBMUbLevel;  //negative
    debug = false;

    calDelMin = 0;
    vthrCompMin = 0;
    ctrlRegMin = 0;
    vcalMin = 60;
    vtrimMin = 80;

    calDelMax = 0;
    vthrCompMax = 0;
    ctrlRegMax = 4;
    vcalMax = 200;
    vtrimMax = 0;
}


void PHRange::Init()
{
    int aoutData[ROCNUMROWS * ROCNUMCOLS], offset, minPixelPh = 2000, maxPixelPh = -2000;
    if (tbInterface->TBMPresent()) offset = 16; else offset = 9;
    int trim[ROCNUMROWS * ROCNUMCOLS];
    phPosition = offset + aoutChipPosition * 3;
    TH1D * histoMin = new TH1D(Form("PH%i_C%i", vcalMin, chipId), Form("PH%i_C%i", vcalMin, chipId), 400, -2000., 2000.);
    TH1D * histoMax = new TH1D(Form("PH%i_C%i", vcalMax, chipId), Form("PH%i_C%i", vcalMax, chipId), 400, -2000., 2000.);

    // == get settings and pixel for minimum

    roc->GetTrimValues(trim);
    SetDAC("CtrlReg", ctrlRegMin);
    SetDAC("Vtrim", vtrimMin);  //trimming the pixels helps measuring pulse heights at low vcal
    for (int k = 0; k < ROCNUMROWS * ROCNUMCOLS; k++) roc->SetTrim(k / ROCNUMROWS, k % ROCNUMROWS, 0);
    Flush();

    roc->AdjustCalDelVthrComp(5, 5, vcalMin, -0);
    roc->AoutLevelChip(phPosition, 10, aoutData);
    for (int k = 0; k < ROCNUMROWS * ROCNUMCOLS; k++) histoMin->Fill(aoutData[k]);
    for (int k = 0; k < ROCNUMROWS * ROCNUMCOLS; k++)
    {
        if ((aoutData[k] < minPixelPh) && (TMath::Abs(aoutData[k] - histoMin->GetMean()) < 4 * histoMin->GetRMS()))
        {
            minPixelPh = aoutData[k];
            minPixel = k;
        }
    }

    roc->AdjustCalDelVthrComp(minPixel / ROCNUMROWS, minPixel % ROCNUMROWS, vcalMin, -0);
    calDelMin = roc->GetDAC("CalDel");
    vthrCompMin = roc->GetDAC("VthrComp");

    for (int k = 0; k < ROCNUMROWS * ROCNUMCOLS; k++) roc->SetTrim(k / ROCNUMROWS, k % ROCNUMROWS, trim[k]);
    SetDAC("Vtrim", 0);

    if (debug) printf("MinPixel %i %i %i\n", minPixel / ROCNUMROWS, minPixel % ROCNUMROWS, minPixelPh);

    // == get settings and pixel for maximum

    SetDAC("CtrlReg", ctrlRegMax);
    Flush();

    roc->AdjustCalDelVthrComp(5, 5, vcalMax, -0);
    roc->AoutLevelChip(phPosition, 10, aoutData);
    for (int k = 0; k < ROCNUMROWS * ROCNUMCOLS; k++) histoMax->Fill(aoutData[k]);
    for (int k = 0; k < ROCNUMROWS * ROCNUMCOLS; k++)
    {
        if ((aoutData[k] > maxPixelPh) && (TMath::Abs(aoutData[k] - histoMax->GetMean()) < 4 * histoMax->GetRMS()))
        {
            maxPixelPh = aoutData[k];
            maxPixel = k;
        }
    }

    //   roc->AdjustCalDelVthrComp(maxPixel/ROCNUMROWS, maxPixel%ROCNUMROWS, vcalMax, -0);
    calDelMax = roc->GetDAC("CalDel");
    vthrCompMax = roc->GetDAC("VthrComp");

    if (debug) printf("MaxPixel %i %i %i\n", maxPixel / ROCNUMROWS, maxPixel % ROCNUMROWS, maxPixelPh);

}

int PHRange::PHMin()
{
    int ph = PH(ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel);
    if (ph == 7777) ph = PH(ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel); //second try
    if (ph == 7777) ph = PH(ctrlRegMin, vcalMin, calDelMin, vthrCompMin, vtrimMin, minPixel); //third try
    if (ph == 7777)
        psi::LogInfo() << "[PHRange] Error: Can not measure Pulse Height."
                       << psi::endl;

    return ph;
}


int PHRange::PHMax()
{
    int ph = PH(ctrlRegMax, vcalMax, calDelMax, vthrCompMax, vtrimMax, maxPixel);
    if (ph == 7777)
        psi::LogInfo() << "[PHRange] Error: Can not measure Pulse Height."
                       << psi::endl;

    return ph;
}


int PHRange::PH(int ctrlReg, int vcal, int calDel, int vthrComp, int vtrim, int pixel)
{
    //if (debug) printf("ctrlReg %i vcal %i calDel %i vthrComp %i vtrim %i\n", ctrlReg, vcal, calDel, vthrComp, vtrim);

    unsigned short count;
    short data[FIFOSIZE];
    int trim[ROCNUMROWS * ROCNUMCOLS];
    int ph = 7777;

    roc->GetTrimValues(trim);
    SetDAC("CtrlReg", ctrlReg);
    SetDAC("Vcal", vcal);
    SetDAC("CalDel", calDel);
    SetDAC("VthrComp", vthrComp);
    SetDAC("Vtrim", vtrim);

     tbInterface->DataCtrl(true, false);  //somehow needed to clear fifo buffer after AdjustCalDelVthrComp

    roc->SetTrim(pixel / ROCNUMROWS, pixel % ROCNUMROWS, 0);
    roc->ArmPixel(pixel / ROCNUMROWS, pixel % ROCNUMROWS);

    tbInterface->ADCRead(data, count, 10);
    if (count > tbInterface->GetEmptyReadoutLengthADC()) ph = data[phPosition];

    roc->DisarmPixel(pixel / ROCNUMROWS, pixel % ROCNUMROWS);
    roc->SetTrim(pixel / ROCNUMROWS, pixel % ROCNUMROWS, trim[pixel]);

    return ph;
}


void PHRange::RocAction()
{
    psi::LogDebug() << "[PHRange] Roc #" << chipId << '.' << psi::endl;

    gDelay->Timestamp();
    SaveDacParameters();
    Init();

    int vibiasPh = 90, offsetOp = 40, stepSize;
    int goalRange = 2 * TMath::Abs(tbmUbLevel) - phSafety;
    int loopnumber = 0;
    int diffRange, diffPos, diffRangeOld, diffPosOld;

    // loop to achieve best range & best position

    if (debug) printf("goalRange %i\n", goalRange);

    SetDAC("VIbias_PH", vibiasPh);
    SetDAC("VoffsetOp", offsetOp);
    Flush();

    do
    {
        if (debug) printf("loop: %i\n", loopnumber);
        if (loopnumber == 0) stepSize = 5;
        else stepSize = 1;

        diffRange = PHMax() - PHMin() - goalRange;
        if (diffRange > 0)
        {
            do
            {
                vibiasPh -= stepSize;
                SetDAC("VIbias_PH", vibiasPh);
                diffRangeOld = diffRange;
                diffRange = PHMax() - PHMin() - goalRange;
            }
            while (diffRange > 0 && vibiasPh > stepSize);
            if (TMath::Abs(diffRangeOld) < TMath::Abs(diffRange))
            {
                vibiasPh += stepSize;
                SetDAC("VIbias_PH", vibiasPh);
            }
        }
        else
        {
            do
            {
                vibiasPh += stepSize;
                SetDAC("VIbias_PH", vibiasPh);
                diffRangeOld = diffRange;
                diffRange = PHMax() - PHMin() - goalRange;
            }
            while (diffRange < 0 && vibiasPh < 230 - stepSize);
            if (TMath::Abs(diffRangeOld) < TMath::Abs(diffRange))
            {
                vibiasPh -= stepSize;
                SetDAC("VIbias_PH", vibiasPh);
            }
        }

        diffPos = TMath::Abs(tbmUbLevel) - PHMax();
        if (diffPos > 0)
        {
            do
            {
                offsetOp += stepSize;
                SetDAC("VoffsetOp", offsetOp);
                diffPosOld = diffPos;
                diffPos = TMath::Abs(tbmUbLevel) - PHMax();
            }
            while (diffPos > 0 && offsetOp < 255 - stepSize);
            if (TMath::Abs(diffPosOld) < TMath::Abs(diffPos))
            {
                offsetOp -= stepSize;
                SetDAC("VoffsetOp", offsetOp);
            }
        }
        else
        {
            do
            {
                offsetOp -= stepSize;
                SetDAC("VoffsetOp", offsetOp);
                diffPosOld = diffPos;
                diffPos = TMath::Abs(tbmUbLevel) - PHMax();
            }
            while (diffPos < 0 && offsetOp > stepSize);
            if (TMath::Abs(diffPosOld) < TMath::Abs(diffPos))
            {
                offsetOp += stepSize;
                SetDAC("VoffsetOp", offsetOp);
            }
        }

        diffRange = PHMax() - PHMin() - goalRange;
        diffPos = TMath::Abs(tbmUbLevel) - PHMax();
        if (debug) printf("diffRange %i diffPos %i\n", diffRange, diffPos);
        loopnumber++;
    }
    while ((TMath::Abs(diffRange) > 5 || TMath::Abs(diffPos) > 5) && loopnumber < 3);


    RestoreDacParameters();

    SetDAC("VIbias_PH", vibiasPh);
    SetDAC("VoffsetOp", offsetOp);

    psi::LogDebug() << "[PHRange] VIbias_PH " << vibiasPh << " VoffsetOp "
                    << offsetOp << psi::endl;

    Flush();

    ValidationPlot();
}


void PHRange::ValidationPlot()  //fast (minimal) version
{
    printf("Validation plot\n");
    TH2D * valPlot = new TH2D(Form("ValPlot_C%i", chipId), Form("ValidationPlot_C%i", chipId), 9, 0, 9, 4000, -2000, 2000);
    unsigned short count;
    short data[FIFOSIZE];

    SaveDacParameters();

    //address levels from pixel 10, 13
    int colNumber = 10, rowNumber = 13;
    roc->ArmPixel(colNumber, rowNumber);
    SetDAC("RangeTemp", 0); //maximal last dac
    tbInterface->ADCRead(data, count, 5);
    if (count == tbInterface->GetEmptyReadoutLengthADC() + 6) for (int i = 0; i < 8; i++) valPlot->Fill(i, data[8 + aoutChipPosition * 3 + i]);
    roc->DisarmPixel(colNumber, rowNumber);

    //pulse height minimum and maximum
    valPlot->Fill(8, PHMin());
    valPlot->Fill(8, PHMax());

    RestoreDacParameters();
    histograms->Add(valPlot);
}

