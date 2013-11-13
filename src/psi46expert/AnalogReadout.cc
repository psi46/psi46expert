#include "AnalogReadout.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"


AnalogReadout::AnalogReadout(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    debug = false;
}


void AnalogReadout::ModuleAction()
{
    unsigned short counter;
    short data[FIFOSIZE];

    int emptyReadoutLengthADC = tbInterface->GetEmptyReadoutLengthADC();
    tbInterface->ADCRead(data, counter, 100);

    int max = counter;
    if (counter > emptyReadoutLengthADC) max = emptyReadoutLengthADC;
    TH1D * histo = new TH1D("AnalogReadout", "AnalogReadout", emptyReadoutLengthADC, 0, emptyReadoutLengthADC);
    for (int i = 0; i < max; i++)
    {
        histo->SetBinContent(i + 1, data[i]);
        if (debug) printf("%4i ", data[i]);
    }
    if (debug) printf("\n");
    histograms->Add(histo);
}

