#include <iostream>
using namespace std;

#include "HighRateEfficiency.h"

#include "TestModule.h"
#include "TestRoc.h"
#include "TestPixel.h"
#include "interface/Delay.h"
#include "BasePixel/RawPacketDecoder.h"
#include "DataFilter.h"

#include <TMath.h>

HREfficiency::HREfficiency(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
}

HREfficiency::~HREfficiency()
{
	
}

void HREfficiency::RocAction(void)
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;
	ai->Flush();

	/* ??? */
	ai->getCTestboard()->DataBlockSize(100);
	ai->Flush();
	
	/* Unmask ROC */
	roc->EnableAllPixels();
	ai->Flush();

	/* Set local trigger and tbm present */
	ai->SetReg(41, 0x20 | 0x02);
	ai->Flush();
	
	/* Send a reset to the chip */
	ai->Single(RES);
	ai->Flush();
	gDelay->Mdelay(10);
	
	/* Prepare the data aquisition (store to testboard RAM) */
	unsigned int data_pointer = ai->getCTestboard()->Daq_Init(30000000);

	/* Enable DMA (direct memory access) controller */
	ai->getCTestboard()->Daq_Enable();
	
	/* Set data aquisition to no clear buffer, multi trigger, continuous. */
	ai->DataCtrl(false, false, true);
	
	/* Reset the clock counter on the testboard */
	ai->SetReg(43, (1 << 1));
	
	/* Set the trigger frequency (f = 40000000 / (256 * n)) */
	//ai->getCTestboard()->Set(T_Periode, 5);
	//ai->getCTestboard()->Set(21, 5); // T_Periode has the wrong value. Should be fixed.
	
	/* Set local trigger, tbm present, and run data aquisition */
	ai->SetReg(41, 0x20 | 0x02 | 0x08);
	ai->Flush();
	
	/* Reset the aquisition on the testboard */
	ai->SetReg(43, (1 << 0));
	
	/* Set the number of triggers. Total triggers for all pixels: 4160 * ntrig */
	int ntrig = 100;
	
	/* iterate over columns and rows to get each pixel efficiency */
	for (int col = 0; col < 52; col++) {
		for (int row = 0; row < 80; row++) {
			/* Arm the pixel */
			roc->ArmPixel(col, row);
			ai->CDelay(5000);
			ai->Flush();

			/* send ntrig triggers with calibrates */
			for (int t = 0; t < ntrig; t++) {
				ai->Single(CAL|TRG|TOK);
				ai->CDelay(500);
			}
			ai->Flush();
			
			/* Disarm the pixel */
			roc->DisarmPixel(col, row);
			roc->EnablePixel(col, row);
			ai->Flush();
		}
	}
	
	/* Stop triggering */
	ai->Single(RES);
	ai->Flush();

	/* Wait for data aquisition to finish */
	gDelay->Mdelay(100);
	
	/* Get pointer to the end of the data block */
	int data_end = ai->getCTestboard()->Daq_GetPointer();
	ai->Flush();
	
	/* Disable data aquisition */
	ai->SetReg(41, 0x20 | 0x02);
	ai->getCTestboard()->Daq_Disable();
	ai->DataCtrl(false, false, false);
	ai->Flush();
	
	/* Number of words in memory */
	int nwords = (data_end - data_pointer) / 2;
	cout << "Megabytes in RAM: " << nwords * 2. / 1024. / 1024. << endl;

	/* Prepare data decoding */
	RAMRawDataReader rd(ai->getCTestboard(), (unsigned int) data_pointer, (unsigned int) data_pointer + 30000000, nwords * 2);
	RawData2RawEvent rs;
	RawEventDecoder ed(1);
	EfficiencyMapper em(ntrig);
	
	/* Decoding chain */
	rd >> rs >> ed >> em >> end;

	/* Store histograms */
	TH2I * effmap = (TH2I *) em.getEfficiencyMap(0)->Clone();
	effmap->SetMinimum(0);
	effmap->SetMaximum(ntrig);
	histograms->Add(effmap);
	TH1I * effdist = (TH1I *) em.getEfficiencyDist(0)->Clone();
	histograms->Add(effdist);
	TH2I * bkgmap = (TH2I *) em.getBackgroundMap(0)->Clone();
	histograms->Add(bkgmap);
	cout << "Number of triggers: " << ntrig * 4160 << endl;
	cout << "Number of hits: " << bkgmap->GetEntries() << endl;
	cout << "Rate: " << (bkgmap->GetEntries() / (ntrig * 4160)) * 40e6 / 1e6 / (0.79*0.77);
	cout << " +/- " << (TMath::Sqrt(bkgmap->GetEntries()) / (ntrig * 4160)) * 40e6 / 1e6 / (0.79*0.77);
	cout << " megahits / s / cm2" << endl;
	cout << "Overall efficiency: " << effdist->GetMean() << " %" << endl;
	
	/* Free the memory in the RAM */
	ai->getCTestboard()->Daq_Done();

	/* Reset the chip */
	ai->Single(RES);
	ai->Flush();
}
