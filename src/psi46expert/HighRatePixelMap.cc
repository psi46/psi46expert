#include <iostream>
using namespace std;

#include "HighRatePixelMap.h"

#include "TestModule.h"
#include "TestRoc.h"
#include "TestPixel.h"
#include "interface/Delay.h"
#include "BasePixel/RawPacketDecoder.h"
#include "DataFilter.h"
#include "interface/Log.h"

#include <TMath.h>

HRPixelMap::HRPixelMap(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
}

HRPixelMap::~HRPixelMap()
{
	
}

void HRPixelMap::RocAction(void)
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;
	ai->Flush();

	/* ??? */
	ai->getCTestboard()->DataBlockSize(100);
	ai->Flush();
	
	/* Unmask the ROC */
	roc->EnableAllPixels();
	ai->Flush();

	/* Set local trigger and tbm present */
	ai->SetReg(41, 0x20 | 0x02);
	ai->Flush();
	
	/* Send a reset to the chip */
	ai->Single(RES);
	
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
	ai->getCTestboard()->Set(21, 5); // T_Periode has the wrong value. Should be fixed.
	
	/* Issue continuous Reset-(Calibrate-)Trigger-Token pattern */
	ai->Intern(TRG|TOK);
	
	/* Set local trigger, tbm present, and run data aquisition */
	ai->SetReg(41, 0x20 | 0x02 | 0x08);
	ai->Flush();
	
	/* Reset the aquisition on the testboard */
	ai->SetReg(43, (1 << 0));
	
	float seconds = 25;
	for (float t = seconds; t > 0; t--) {
		cout << "\rTaking data (" << t << " seconds) ... ";
		cout.flush();
		gDelay->Mdelay(1000);
	}
	cout << "\rTaking data (" << (seconds - (int)(seconds)) << " seconds) ... done" << endl;
	gDelay->Mdelay((int)((seconds - (int)(seconds)) * 1000));
	
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
	
	/* Number of words stored in memory */
	int nwords = (data_end - data_pointer) / 2;
	psi::LogInfo() << "Megabytes in RAM: " << nwords * 2. / 1024. / 1024. << psi::endl;

	/* Prepare data decoding */
	RAMRawDataReader rd(ai->getCTestboard(), (unsigned int) data_pointer, (unsigned int) data_pointer + 30000000, nwords * 2);
	RawData2RawEvent rs;
	RawEventDecoder ed(1);
	HitMapper hm;
	EventCounter count;
	MultiplicityHistogrammer mh;
	PulseHeightHistogrammer phh;
	
	/* Decoding chain */
	rd >> rs >> ed >> hm >> count >> mh >> phh >> end;

	/* Store histograms */
	TH2I * map = (TH2I *) hm.getHitMap(0)->Clone();
	histograms->Add(map);
	TH1I * multi = (TH1I *) mh.getRocMultiplicity(0)->Clone();
	histograms->Add(multi);
	TH1I * pulse = (TH1I *) phh.getPulseHeightHistogram()->Clone();
	histograms->Add(pulse);
	psi::LogInfo() << "Number of triggers: " << count.TriggerCounter << psi::endl;
	psi::LogInfo() << "Number of hits: " << map->GetEntries() << psi::endl;
	psi::LogInfo() << "Rate: " << (map->GetEntries() / (count.TriggerCounter)) * 40e6 / 1e6 / (0.79*0.77);
	psi::LogInfo() << " +/- " << (TMath::Sqrt(map->GetEntries()) / (count.TriggerCounter)) * 40e6 / 1e6 / (0.79*0.77);
	psi::LogInfo() << " megahits / s / cm2" << psi::endl;
	psi::LogInfo() << "Number of ROC sequence errors: " << count.RocSequenceErrorCounter << psi::endl;

	float mean = multi->GetMean();
	int entries = multi->GetEntries();
	multi = new TH1I(Form("roc_multiplicity_%i_p", 0), Form("ROC %i hit multiplicity (poisson)", 0), 40, 0, 40);
	for (int j = 0; j < 15; j++) {
		multi->SetBinContent(j + 1, TMath::Power(mean, j) * TMath::Exp(-mean) / TMath::Factorial(j) * entries);
	}
	//histograms->Add(multi);
	
	for (int i = 0; i < 26; i++) {
		TH1I * hist = (TH1I *) mh.getDColMultiplicity(0, i)->Clone();
		hist->SetMinimum(1);
		//histograms->Add(hist);
		float mean = hist->GetMean();
		int entries = hist->GetEntries();
		hist = new TH1I(Form("dcol_multiplicity_0_%i_p", i), Form("DCol %i hit multiplicity (ROC 0) (poisson)", i), 15, 0, 15);
		for (int j = 0; j < 15; j++) {
			hist->SetBinContent(j + 1, TMath::Power(mean, j) * TMath::Exp(-mean) / TMath::Factorial(j) * entries);
		}
		hist->SetMinimum(1);
		//histograms->Add(hist);
	}
	
	/* Free the memory in the RAM */
	ai->getCTestboard()->Daq_Done();

	/* Reset the chip */
	ai->Single(RES);
	ai->Flush();
}
