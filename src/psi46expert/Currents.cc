//DAc Sacn to find suitable range
//peller, september 2011
//
#include "Currents.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
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

void Currents::DoCurrentScan()
{
	int scanMax;
        if ((DacRegister == 1) || (DacRegister == 4) || (DacRegister == 6) || (DacRegister == 8) || (DacRegister == 14))
            scanMax = 16;
        else
            scanMax = 256;

	TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
	DACParameters * parameters = new DACParameters();
        char * dacName = parameters->GetName(DacRegister);

	//define ID hist
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
	cout << setw(10) << left << dacName << setw(10) << left << "ID (A)"  << setw(10) << left << "IA (A)" << endl;
	int loopNumber = 0;
	int defaultValue = GetDAC(DacRegister);
	// loop over different DAC values
	for (int scanValue = 0; scanValue < scanMax; scanValue+=((int)scanMax/NumberOfSteps))
	{
		loopNumber++;
		//DACParameters* parameters = new DACParameters();
		//char *dacName = parameters->GetName(DacRegister);
		//delete parameters;
		SetDAC(DacRegister, scanValue);
		//measure currents twice:
		double id = anaInterface->GetID();
		id = anaInterface->GetID();
		histo->SetBinContent(loopNumber+1, id);
		double ia = anaInterface->GetIA();
		ia = anaInterface->GetIA();
		histoA->SetBinContent(loopNumber+1, ia);
		//std::cout << "*"  << std::flush;
		cout << setw(10) << left << scanValue << setw(10) << left << id << setw(10) << left << ia << endl;
		SetDAC(DacRegister, defaultValue);
		}
		histograms->Add(histo);
	//std::cout << endl;
}
