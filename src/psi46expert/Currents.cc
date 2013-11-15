#include "Currents.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TCanvas.h"
#include <iomanip>

Currents::Currents(TestRange *aTestRange, TestParameters * testParameters, TBInterface *aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);

}

void Currents::ReadTestParameters(TestParameters * testParameters)
{
    DacRegister = (*testParameters).CurrentScanDac;
    NumberOfSteps = (*testParameters).CurrentScanNumberOfSteps;
}

void Currents::RocAction()
{
    SaveDacParameters();
    DoCurrentScan();
    RestoreDacParameters();
}

/**
    This fuction first calculates the Steps of the DAC setttings and then sets the DAC to each setting subsequently. For each setting the analog and digital currents are measured twice and only the later measurement is recorded. This is to avoid a bug in the Testboard. The Two currents are stored in TH1D histograms.
 */
void Currents::DoCurrentScan()
{
    // calculate steps
    int scanMax;
    if ((DacRegister == 1) || (DacRegister == 4) || (DacRegister == 6) || (DacRegister == 8) || (DacRegister == 14))
        scanMax = 16;
    else
        scanMax = 256;
    DACParameters * parameters = new DACParameters();
    char * dacName = parameters->GetName(DacRegister);

    // define current histograms
    TH1D *histo = new TH1D( Form( "dig_current_%s", dacName)
                ,Form( "digital current of DAC %s", dacName),NumberOfSteps, 0, scanMax);
    histo->GetXaxis()->SetTitle(Form( "DAC %s (DAC units)", dacName));
    histo->GetYaxis()->SetTitle("current (A)");
    histo->SetMarkerStyle(20);
    histo->SetMarkerSize(2);
    TH1D *histoA = new TH1D( Form( "ana_current_%s", dacName)
                ,Form( "analog current of DAC %s", dacName),NumberOfSteps, 0, scanMax);
    histoA->GetXaxis()->SetTitle(Form( "DAC %s (DAC units)", dacName));
    histoA->GetYaxis()->SetTitle("current (A)");
    histoA->SetMarkerStyle(20);
    histoA->SetMarkerSize(2);

    // format output
    cout << setw(10) << left << dacName << setw(10) << left << "ID (A)"  << setw(10) << left << "IA (A)" << endl;
    int loopNumber = 0;
    int defaultValue = GetDAC(DacRegister);

    // loop over different DAC values
    for (int scanValue = 0; scanValue < scanMax; scanValue+=((int)scanMax/NumberOfSteps))
    {
        loopNumber++;
        SetDAC(DacRegister, scanValue);
        //measure currents twice:
        double id = tbInterface->GetID();
        id = tbInterface->GetID();
        histo->SetBinContent(loopNumber+1, id);
        double ia = tbInterface->GetIA();
        ia = tbInterface->GetIA();
        histoA->SetBinContent(loopNumber+1, ia);
        cout << setw(10) << left << scanValue << setw(10) << left << id << setw(10) << left << ia << endl;
        SetDAC(DacRegister, defaultValue);
    }
    // add histogram
    histograms->Add(histo);
}
