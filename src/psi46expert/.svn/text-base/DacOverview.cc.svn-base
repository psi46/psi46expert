#include "DacOverview.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TCanvas.h"
#include "PhDacScan.h"

DacOverview::DacOverview(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
    : PhDacScan(aTestRange, testParameters, aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
    debug = true;
}


void DacOverview::ReadTestParameters(TestParameters * testParameters)
{
    PhDacScan::ReadTestParameters(testParameters);
    NumberOfSteps = (*testParameters).PHNumberOfSteps;
    DacType = (*testParameters).PHDacType;
}

void DacOverview::RocAction()
{
    SaveDacParameters();
    roc->EnableDoubleColumn(10);
    roc->ArmPixel(10, 13);
    Flush();
    DoDacScan();
    roc->DisarmPixel(10, 13);
    RestoreDacParameters();
}


void DacOverview::DoDacScan()
{
    cout << " ************************* DAC SCAN **************************" << endl;

    int offset;
    if (((TBAnalogInterface *)tbInterface)->TBMPresent()) offset = 16;
    else offset = 9;

    cout << "chipId = " << chipId << ", col = " << column << ", row = " << row << endl;

    int position;
    int Min;
    int Max;
    const char * Type;
    const char * dacName;

    TH1D * histoTbmUb, *histoTbmLev[4], *histoRocLev[5], *histoRocUb;

    if (DacType == 0) // ROC Dacs
    {
        Min = 1;
        Max = 28;
        Type = "ROC";
    }
    else if (DacType == 1) // TBM Dacs
    {
        Min = 2;
        Max = 5;
        Type = "TBM";
    }

    ((TBAnalogInterface *)tbInterface)->DataTriggerLevel(-100);   // xxx
    Flush();

    for (int DacRegister = Min; DacRegister < Max; DacRegister++) // loop over all possible Dacs of a DacType
    {
        printf("Min = %i, Max = %i\n", Min, Max);
        cout << "DAC set to " << DacRegister << endl;
        int scanMax = 256;
        int defaultValue;
        if (DacType == 0) defaultValue = GetDAC(DacRegister);
        else if (DacType == 1) defaultValue =  module->GetTBM(DacRegister);

        if (DacType == 0)
        {
            DACParameters * parameters = new DACParameters();
            dacName = parameters->GetName(DacRegister);
            delete parameters;
        }

        else if (DacType == 1 && DacRegister == 2) dacName = "Inputbias";
        else if (DacType == 1 && DacRegister == 3) dacName = "Outputbias";
        else if (DacType == 1 && DacRegister == 4) dacName = "Dacgain";

        printf("Using %s\n", dacName);

        int loopcount = 0;

        cout << "default value = " << defaultValue << endl;

        unsigned short count;
        short data[FIFOSIZE];

        histoTbmUb = new TH1D(Form("%sDAC%i_TbmUb", Type, DacRegister), "TbmUb", NumberOfSteps, 0, 255);
        histoTbmUb->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName));
        histoTbmUb->GetYaxis()->SetTitle("TBM Lev & UB [ADC units]");
        histoRocUb = new TH1D(Form("%sDAC%i_RocUb", Type, DacRegister), "RocUb", NumberOfSteps, 0, 255);
        histoRocUb->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName));
        histoRocUb->GetYaxis()->SetTitle("ROC Lev & UB [ADC units]");
        for (int i = 0; i < 4; i++)
        {
            histoTbmLev[i] = new TH1D(Form("%sDAC%i_TbmLev%i", Type, DacRegister, i), "TbmLev", NumberOfSteps, 0, 255);
            histoTbmLev[i]->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName));
            histoTbmLev[i]->GetYaxis()->SetTitle("TBM Lev & UB [ADC units]");
        }
        for (int i = 0; i < 5; i++)
        {
            histoRocLev[i] = new TH1D(Form("%sDAC%i_RocLev%i", Type, DacRegister, i), "RocLev", NumberOfSteps, 0, 255);
            histoRocLev[i]->GetXaxis()->SetTitle(Form("%s [DAC units]", dacName));
            histoRocLev[i]->GetYaxis()->SetTitle("ROC Lev & UB [ADC units]");
        }

        // set level so that Tbm Lev0 corresponds to lowest level
        ((TBAnalogInterface *)tbInterface)->ADCRead(data, count, 1);
        int lev1 = data[7];
        ((TBAnalogInterface *)tbInterface)->ADCRead(data, count, 1);
        int lev2 = data[7];
        while (lev2 > lev1)
        {
            lev1 = lev2;
            ((TBAnalogInterface *)tbInterface)->ADCRead(data, count, 1);
            lev2 = data[7];
        }
        for (int i = 0; i < 3; i++)
        {
            ((TBAnalogInterface *)tbInterface)->ADCRead(data, count, 1);
        }

        // loop over values for a Dac
        for (int scanValue = 0; scanValue < scanMax; scanValue += ((int)scanMax / NumberOfSteps))
        {
            if (DacType == 0) SetDAC(DacRegister, scanValue);
            else if (DacType == 1)  module->SetTBM(chipId, DacRegister, scanValue);
            loopcount++;
            int sum[4] = {0, 0, 0, 0};

            // Test TBM LEV

            position = 7;
            int readouts = 1;
            for (int j = 0; j < readouts; j++) // number of readouts per level
            {
                for (int i = 0; i < 4; i++) // loop over levels
                {
                    ((TBAnalogInterface *)tbInterface)->ADCRead(data, count, 1);
                    if (count != 70) printf("Warning! count = %i \n", count);
                    sum[i] = sum[i] + data[position];
                }
            }

            printf("loopcount = %i\n", loopcount);

            for (int i = 0; i < 4; i++)
            {
                sum[i] = sum[i] / readouts;
                if (count != 70) histoTbmLev[i]->SetBinContent(loopcount, 2500);
                else histoTbmLev[i]->SetBinContent(loopcount, sum[i]);
            }

            // Test TBM UB, ROC LEV, ROC UB

            ((TBAnalogInterface *)tbInterface)->ADCRead(data, count, 12);
            if (count != 70) printf("Warning! count = %i \n", count);

            position = 1;
            if (count != 70) histoTbmUb->SetBinContent(loopcount, 2500);
            else histoTbmUb->SetBinContent(loopcount, data[position]);

            position = 8 + aoutChipPosition * 3;
            if (count != 70) histoRocUb->SetBinContent(loopcount, 2500);
            else histoRocUb->SetBinContent(loopcount, data[position]);

            for (int i = 0; i < 5; i++)
            {
                position = 11 + i + aoutChipPosition * 3;
                if (count != 70) histoRocLev[i]->SetBinContent(loopcount, 2500);
                else histoRocLev[i]->SetBinContent(loopcount, data[position]);
            }
        }
        for (int i = 0; i < 5; i++) histograms->Add(histoRocLev[i]);
        histograms->Add(histoRocUb);
        for (int i = 0; i < 4; i++) histograms->Add(histoTbmLev[i]);
        histograms->Add(histoTbmUb);

        if (DacType == 0) SetDAC(DacRegister, defaultValue);
        else if (DacType == 1) module->SetTBM(chipId, DacRegister, defaultValue);
    }
}


