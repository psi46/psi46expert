#include <iostream>
using namespace std;

#include "HighRateSCurve.h"

#include "TestModule.h"
#include "TestRoc.h"
#include "TestPixel.h"
#include "interface/Delay.h"
#include "BasePixel/RawPacketDecoder.h"
#include "DataFilter.h"
#include "interface/Log.h"

#include <TMath.h>

HRSCurve::HRSCurve(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
}

HRSCurve::~HRSCurve()
{
	
}

void HRSCurve::RocAction(void)
{
	rough_threshold = new TH2I("rough_threshold", "Rough VCal threshold", 52, 0, 52, 80, 0, 80);
	for (int col = 0; col < 52; col++) {
		for (int row = 0; row < 80; row++) {
			rough_threshold->SetBinContent(col + 1, row + 1, -1);
		}
	}

	psi::LogInfo() << "Determining rough threshold" << psi::endl;
	for (int vcal = 82; vcal < 130; vcal += 4) {
		psi::LogInfo() << "Testing vcal " << vcal << " ..." <<  psi::endl;
		roc->SetDAC("Vcal", vcal);
		TakeEfficiencyMap(4, false, 0);
		efficiency_map->SetNameTitle(Form("effmap_vcal%i", vcal), Form("Efficiency map VCal %i", vcal));
		histograms->Add(efficiency_map);
		for (int col = 0; col < 52; col++) {
			for (int row = 0; row < 80; row++) {
				if (efficiency_map->GetBinContent(col + 1, row + 1) >= 3 && rough_threshold->GetBinContent(col + 1, row + 1) == -1)
					rough_threshold->SetBinContent(col + 1, row + 1, vcal);
			}
		}
	}
	
	
	psi::LogInfo() << "Determining SCurve" << psi::endl;
	for (int offset = -16; offset <= 16; offset++) {
		psi::LogInfo() << "Testing offset " << offset << " ..." <<  psi::endl;
		TakeEfficiencyMap(50, true, offset);
		efficiency_map->SetNameTitle(Form("effmap_offset%i", offset), Form("Efficiency map offset %i", offset));
		histograms->Add(efficiency_map);
		for (int col = 0; col < 52; col++) {
			for (int row = 0; row < 80; row++) {
				
			}
		}
	}
	

	rough_threshold->SetMinimum(81);
	rough_threshold->SetMaximum(130);
	histograms->Add(rough_threshold);
}

void HRSCurve::TakeEfficiencyMap(int ntrig, bool set_vcal, int vcal_offset)
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;
	ai->Flush();

	/* ??? */
	ai->getCTestboard()->DataBlockSize(100);
	ai->Flush();
	
	/* Unmask ROC */
	roc->EnableAllPixels();
	//roc->Mask();
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
	
	/* Set local trigger, tbm present, and run data aquisition */
	ai->SetReg(41, 0x20 | 0x02 | 0x08);
	ai->Flush();
	
	/* Reset the aquisition on the testboard */
	ai->SetReg(43, (1 << 0));
	
	/* iterate over columns and rows to get each pixel efficiency */
	for (int col = 0; col < 52; col++) {
		for (int row = 0; row < 80; row++) {
			/* Arm the pixel */
			roc->ArmPixel(col, row);
			if (set_vcal)
				roc->SetDAC("Vcal", rough_threshold->GetBinContent(col + 1, row + 1) + vcal_offset);
			ai->CDelay(5000);
			ai->Flush();

			/* send ntrig triggers with calibrates */
			for (int t = 0; t < ntrig; t++) {
				ai->Single(CAL|TRG|TOK);
				ai->CDelay(500);
			}
			ai->Flush();
			
			/* Disarm the pixel, but leave it enabled */
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
	//psi::LogInfo() << "Megabytes in RAM: " << nwords * 2. / 1024. / 1024. << psi::endl;

	/* Prepare data decoding */
	RAMRawDataReader rd(ai->getCTestboard(), (unsigned int) data_pointer, (unsigned int) data_pointer + 30000000, nwords * 2);
	RawData2RawEvent rs;
	RawEventDecoder ed(1);
	EfficiencyMapper em(ntrig);
	
	/* Decoding chain */
	rd >> rs >> ed >> em >> end;

	/* Store histograms */
	efficiency_map = (TH2I *) em.getEfficiencyMap(0)->Clone();
	efficiency_map->SetMinimum(0);
	efficiency_map->SetMaximum(ntrig);
	TH1I * effdist = (TH1I *) em.getEfficiencyDist(0);
	TH2I * bkgmap = (TH2I *) em.getBackgroundMap(0);
	//psi::LogInfo() << "Number of triggers: " << ntrig * 4160 << psi::endl;
	//psi::LogInfo() << "Number of hits: " << bkgmap->GetEntries() << psi::endl;
	psi::LogInfo() << "Rate: " << (bkgmap->GetEntries() / (ntrig * 4160)) * 40e6 / 1e6 / (0.79*0.77);
	psi::LogInfo() << " +/- " << (TMath::Sqrt(bkgmap->GetEntries()) / (ntrig * 4160)) * 40e6 / 1e6 / (0.79*0.77);
	psi::LogInfo() << " megahits / s / cm2" << psi::endl;
	psi::LogInfo() << "Overall efficiency: " << effdist->GetMean() << " %" << psi::endl;
	
	/* Free the memory in the RAM */
	ai->getCTestboard()->Daq_Done();

	/* Reset the chip */
	ai->Single(RES);
	ai->Flush();
}
