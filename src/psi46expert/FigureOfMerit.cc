#include "FigureOfMerit.h"
#include "TestRoc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TCanvas.h"
#include "PhDacScan.h"

FigureOfMerit::FigureOfMerit(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface, int dac1, int dac2, int crit)
    : PhDacScan(aTestRange, testParameters, aTBInterface)
{
    firstDac = dac1;
    secondDac = dac2;
    criterion = crit;
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
    fit = new TF1("Fit", "pol4");
    debug = true;
}


void FigureOfMerit::ReadTestParameters(TestParameters * testParameters)
{
    PhDacScan::ReadTestParameters(testParameters);
    dac1Start = (*testParameters).PHdac1Start;
    dac1Stop = (*testParameters).PHdac1Stop;
    dac1Step = (*testParameters).PHdac1Step;
    dac2Start = (*testParameters).PHdac2Start;
    dac2Stop = (*testParameters).PHdac2Stop;
    dac2Step = (*testParameters).PHdac2Step;
    testVcal = (*testParameters).PHtestVcal;
}


void FigureOfMerit::RocAction()
{
    SaveDacParameters();
    SetDAC("CtrlReg", 4);
    Test::RocAction();
    RestoreDacParameters();
}


void FigureOfMerit::PixelAction()
{
    ArmPixel();
    Flush();
    DoDacDacScan();
    DisarmPixel();
}


void FigureOfMerit::DoDacDacScan()
{
    if (debug) cout << " ************************* SCAN IS RUNNING **************************" << endl;

    int dacValue1Size = (dac1Stop - dac1Start) / dac1Step;
    int dacValue2Size = (dac2Stop - dac2Start) / dac2Step;
    double quality;
    int optimalDac2, optimalDac1;

    char testNameUnit[100], testName[100];

    if (criterion == 0)
    {
        strcpy(testNameUnit, "timewalk [ns]");
        strcpy(testName, "Timewalk");
    }
    if (criterion == 1)
    {
        strcpy(testNameUnit, "Linearity (high range) [DAC units]");
        strcpy(testName, "high_linear_range");
    }
    if (criterion == 2)
    {
        strcpy(testNameUnit, "pulse height [ADC units]");
        strcpy(testName, "Pulse_height");
    }
    if (criterion == 3)
    {
        strcpy(testNameUnit, "Linearity (low range) [ADC units]");
        strcpy(testName, "low_linear_range");
    }
    if (criterion == 4)
    {
        strcpy(testNameUnit, "Threshold (low range) [DAC units]");
        strcpy(testName, "threshold");
    }



    DACParameters * parameters = new DACParameters();

    TH2D * histo2 = new TH2D(Form("%s_of_c%ir%i_C%i", testName, column, row, chipId), Form("%s_of_c%ir%i_C%i", testName, column, row, chipId), dacValue1Size + 1 , dac1Start, dac1Stop + dac1Step, dacValue2Size + 1, dac2Start, dac2Stop + dac2Step);
    histo2->GetXaxis()->SetTitle(Form("%s [DAC units]", parameters->GetName(firstDac)));
    histo2->GetYaxis()->SetTitle(Form("%s [DAC units]", parameters->GetName(secondDac)));
    histo2->GetZaxis()->SetTitle(Form("%s", testNameUnit));

    TH2D * minPhHisto = new TH2D(Form("Min_PH_c%ir%i_C%i", column, row, chipId), Form("Min_PH_c%ir%i_C%i", column, row, chipId), dacValue1Size + 1 , dac1Start, dac1Stop + dac1Step, dacValue2Size + 1, dac2Start, dac2Stop + dac2Step);
    minPhHisto->GetXaxis()->SetTitle(Form("%s [DAC units]", parameters->GetName(firstDac)));
    minPhHisto->GetYaxis()->SetTitle(Form("%s [DAC units]", parameters->GetName(secondDac)));
    minPhHisto->GetZaxis()->SetTitle(Form("%s", testNameUnit));

    nor = new TH1D("numberOfReadouts", "numberOfReadouts", 250, 0, 250);

    if (criterion == 0) bestQuality = 100;
    if (criterion == 1) bestQuality = -99;
    if (criterion == 2) bestQuality = -99;
    if (criterion == 3) bestQuality = -99;
    if (criterion == 4) bestQuality = 100;

    for (int i = 0; i <= dacValue1Size; i++)
    {
        dacValue1 = dac1Start + i * dac1Step;
        SetDAC(firstDac, dacValue1);

        for (int k = 0; k <= dacValue2Size; k++)
        {
            dacValue2 = dac2Start + k * dac2Step;
            SetDAC(secondDac, dacValue2);

            if (debug) cout << parameters->GetName(firstDac) << " = " << dacValue1 << "   "  << parameters->GetName(secondDac) << " = " << dacValue2 << endl;

            if (criterion == 0) quality = Timewalk(i, k);
            if (criterion == 1) quality = LinearRange(i, k);
            if (criterion == 2) quality = PulseHeight(i, k);
            if (criterion == 3) quality = LowLinearRange(i, k);
            if (criterion == 4) quality = Threshold(i, k);

            if (debug) cout << "Quality = " << quality << endl;

            histo2->SetBinContent(i + 1, k + 1, quality);
            minPhHisto->SetBinContent(i + 1, k + 1, minPh);
        }
    }

    histograms->Add(histo2);
    histograms->Add(nor);
    if (criterion == 1) histograms->Add(minPhHisto);

    optimalDac2 = dac2Start + index2 * dac2Step;
    optimalDac1 = dac1Start + index1 * dac1Step;
    printf("bestQuality = %d @ %s = %d and %s = %d\n", bestQuality, parameters->GetName(firstDac), optimalDac1, parameters->GetName(secondDac), optimalDac2);
    if (debug) cout << "pixel column = " << pixel->GetColumn() << " pixel row = " << pixel->GetRow() << endl;
}


double FigureOfMerit::Timewalk(int i, int k)
{
    short resultA[256], resultB[256];
    DACParameters * parameters = new DACParameters();

    TH1D * histoA = new TH1D(Form("PHVhldDel_%s%d_%s%d_C%iA", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId),
                             Form("PHVhldDel_%s%d_%s%d_C%iA", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId), 256, 0, 256);
    TH1D * histoB = new TH1D(Form("PHVhldDel_%s%d_%s%d_C%iB", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId),
                             Form("PHVhldDel_%s%d_%s%d_C%iB", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId), 256, 0, 256);

    SetDAC("CtrlReg", 0);
    SetDAC("Vcal", 80);
    ((TBAnalogInterface *)tbInterface)->PHDac(26, 256, nTrig, 16 + aoutChipPosition * 3, resultA);
    SetDAC("Vcal", 250);
    ((TBAnalogInterface *)tbInterface)->PHDac(26, 256, nTrig, 16 + aoutChipPosition * 3, resultB);

    int numberOfReadoutsA = 0;
    int numberOfReadoutsB = 0;

    for (int dac = 0; dac < 256; dac++)
    {
        histoA->SetBinContent(dac + 1, resultA[dac]);
        histoB->SetBinContent(dac + 1, resultB[dac]);
        if (resultA[dac] != 7777) numberOfReadoutsA++;
        if (resultB[dac] != 7777) numberOfReadoutsB++;
    }

    nor->Fill(numberOfReadoutsA);
    nor->Fill(numberOfReadoutsB);

    if (debug) cout << "number of readoutsA = " << numberOfReadoutsA << " Number of readouts B = " << numberOfReadoutsB << endl;

    if (numberOfReadoutsA < 30) return 0;
    if (numberOfReadoutsB < 20) return 0;

    double firstCalDelA = 0.45 * (256 - FindFirstValue(resultA)) + 30;  // converts CalDel from DAC units to ns
    double firstCalDelB = 0.45 * (256 - FindFirstValue(resultB)) + 30;  // converts CalDel from DAC units to ns
    double timewalk = firstCalDelA - firstCalDelB;

    if (debug) cout << "first CalDel [ns] = " << FindFirstValue(resultA) << " second CalDel [ns] = " << FindFirstValue(resultB) << endl;
    if (debug) cout << "first CalDel [DAC units] = " << firstCalDelA << " second CalDel [DAC units] = " << firstCalDelB << endl;

    histograms->Add(histoA);
    histograms->Add(histoB);

    if (timewalk < bestQuality)
    {
        bestQuality = static_cast<int>(timewalk);
        index1 = i;
        index2 = k;
    }

    return timewalk;
}

int FigureOfMerit::LinearRange(int i, int k)
{
    short result[256];
    DACParameters * parameters = new DACParameters();

    TH1D * histo = new TH1D(Form("PHVcal_%s%d_%s%d_C%i", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId),
                            Form("PHVcal_%s%d_%s%d_C%i", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId), 256, 0, 256);

    ((TBAnalogInterface *)tbInterface)->PHDac(25, 256, nTrig, 16 + aoutChipPosition * 3, result);

    for (int dac = 0; dac < 256; dac++) histo->SetBinContent(dac + 1, result[dac]);
    histo->SetMaximum(result[255] + 100);
    histograms->Add(histo);

    int linearRange = static_cast<int>(FindLinearRange(histo));

    if (linearRange > bestQuality)
    {
        bestQuality = linearRange;
        index1 = i;
        index2 = k;
    }

    return linearRange;
}

int FigureOfMerit::PulseHeight(int i, int k)
{
    short result[256];
    DACParameters * parameters = new DACParameters();

    TH1D * histo = new TH1D(Form("PHVcal_%s%d_%s%d_C%i", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId),
                            Form("PHVcal_%s%d_%s%d_C%i", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId), 256, 0, 256);

    ((TBAnalogInterface *)tbInterface)->PHDac(25, 256, nTrig, 16 + aoutChipPosition * 3, result);

    for (int dac = 0; dac < 256; dac++) histo->SetBinContent(dac + 1, result[dac]);
    histo->SetMaximum(result[255] + 100);
    histograms->Add(histo);

    int minPh = FitStartPoint(histo);
    double pulseHeight = result[testVcal - 1] - result[minPh];

    if (pulseHeight > bestQuality)
    {
        bestQuality = static_cast<int>(pulseHeight);
        index1 = i;
        index2 = k;
    }

    return static_cast<int>(pulseHeight);
}

int FigureOfMerit::FindFirstValue(short * result)
{
    int firstCalDel = -1;
    for (int n = 255; n > 0; n--)
    {
        if (result[n] != 7777)
        {
            firstCalDel = n;
            break;
        }
    }
    return firstCalDel;
}

double FigureOfMerit::LowLinearRange(int i, int k)
{
    short result[256], resultHR[256];
    DACParameters * parameters = new DACParameters();

    TH1D * histo = new TH1D(Form("PHVcal_%s%d_%s%d_C%i", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId),
                            Form("PHVcal_%s%d_%s%d_C%i", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId), 256, 0, 256);
    TH1D * fullRangeHist = new TH1D(Form("PHVcal_%s%d_%s%d_C%iFullRange", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId),
                                    Form("PHVcal_%s%d_%s%d_C%iFullRange", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId),
                                    1792, 0, 1792);

    SetDAC("CtrlReg", 4);
    ((TBAnalogInterface *)tbInterface)->PHDac(25, 256, nTrig, 16 + aoutChipPosition * 3, resultHR);
    SetDAC("CtrlReg", 0);
    ((TBAnalogInterface *)tbInterface)->PHDac(25, 256, nTrig, 16 + aoutChipPosition * 3, result);

    int value = 0;
    for (int vcal = 0; vcal < 256; vcal++)
    {
        for (int n = 0; n < 7; n++)
        {
            fullRangeHist->SetBinContent(value + 1, resultHR[vcal]);
            value++;
        }
    }
    for (int vcal = 0; vcal < 256; vcal++) fullRangeHist->SetBinContent(vcal + 1, result[vcal]);
    fullRangeHist->GetYaxis()->SetRangeUser(fullRangeHist->GetBinContent(100) - 200, fullRangeHist->GetBinContent(1790) + 100);

    for (int dac = 0; dac < 256; dac++) histo->SetBinContent(dac + 1, result[dac]);
    histo->SetMaximum(result[255] + 100);
    histograms->Add(histo);
    histograms->Add(fullRangeHist);

    double aoverb = QualityLowRange(histo);

    if (aoverb > bestQuality)
    {
        bestQuality = static_cast<int>(aoverb);
        index1 = i;
        index2 = k;
    }

    SetDAC("CtrlReg", 4);

    return TMath::Abs(aoverb);
}

int FigureOfMerit::Threshold(int i, int k) {

    short result[256];
    DACParameters * parameters = new DACParameters();

    TH1D * histo = new TH1D(Form("PHVcal_%s%d_%s%d_C%i", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId),
                            Form("PHVcal_%s%d_%s%d_C%i", parameters->GetName(firstDac), dacValue1, parameters->GetName(secondDac), dacValue2, chipId), 256, 0, 256);

    SetDAC("CtrlReg", 0);
    ((TBAnalogInterface *)tbInterface)->PHDac(25, 256, nTrig, 16 + aoutChipPosition * 3, result);

    for (int dac = 0; dac < 256; dac++) histo->SetBinContent(dac + 1, result[dac]);
    histo->SetMaximum(result[255] + 100);
    histograms->Add(histo);

    int threshold = FitStartPoint(histo);

    if (threshold < bestQuality)
    {
        bestQuality = threshold;
        index1 = i;
        index2 = k;
    }

    return threshold;

}
