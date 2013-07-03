//DAc Sacn to find suitable range
//peller, september 2011
//
#include "Currents.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TCanvas.h"

Currents::Currents(TestRange *aTestRange, TestParameters * testParameters, TBInterface *aTBInterface)
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);

}

void Currents::ReadTestParameters(TestParameters * testParameters)
{
}

void Currents::RocAction()
{
  SaveDacParameters();
  DoCurrentScan();
  RestoreDacParameters();
}

void Currents::DoCurrentScan()
{
	int DacRegister = 1;
	int scanMax = 16;
	int NumberOfSteps = 16;
	TBAnalogInterface* anaInterface = (TBAnalogInterface*)tbInterface;
	DACParameters * parameters = new DACParameters();
        char * dacName = parameters->GetName(DacRegister);

	//define ID hist
	TH1D *histo = new TH1D( Form( "digital current of DAC %s", dacName)
				,Form( "digital current of DAC %s", dacName),scanMax, 0, scanMax);
	histo->GetXaxis()->SetTitle(Form( "DAC %s (DAC units)", dacName));
	histo->GetYaxis()->SetTitle("digital current (A)");
	histo->SetMarkerStyle(20);
	histo->SetMarkerSize(2);
	TH1D *histoA = new TH1D( Form( "analog current of DAC %s", dacName)
				,Form( "analog current of DAC %s", dacName),scanMax, 0, scanMax);
	histoA->GetXaxis()->SetTitle(Form( "DAC %s (DAC units)", dacName));
	histoA->GetYaxis()->SetTitle("analog current (A)");
	histoA->SetMarkerStyle(20);
	histoA->SetMarkerSize(2);
	std::cout << endl << "scanning DAC " << dacName << ": ";
	int loopNumber = 0;
	int defaultValue = GetDAC(DacRegister);
	// loop over different DAC values
	for (int scanValue = 0; scanValue < scanMax; scanValue+=((int)scanMax/NumberOfSteps))
	{
		loopNumber++;
		//DACParameters* parameters = new DACParameters();
		//char *dacName = parameters->GetName(DacRegister);
		//delete parameters;
		std::cout << "*"  << std::flush;
		SetDAC(DacRegister, scanValue);
		//measure currents twice:
		double id = anaInterface->GetID();
		id = anaInterface->GetID();
		histo->SetBinContent(scanValue+1, id);
		double ia = anaInterface->GetIA();
		ia = anaInterface->GetIA();
		histoA->SetBinContent(scanValue+1, ia);
		SetDAC(DacRegister, defaultValue);
		}
		histograms->Add(histo);
	std::cout << endl;
}
