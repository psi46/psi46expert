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
#include <TParameter.h>

HRPixelMap::HRPixelMap(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
	this->testParameters = testParameters;
}

HRPixelMap::~HRPixelMap()
{
	
}

void HRPixelMap::ModuleAction(void)
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;
	ai->Flush();

	/* ??? */
	ai->getCTestboard()->DataBlockSize(100);
	ai->Flush();
	
	/* Unmask the ROC */
	int nroc = module->NRocs();
	for (int i = 0; i < nroc; i++)
		module->GetRoc(i)->EnableAllPixels();
	ai->Flush();

	/* Set local trigger and tbm present */
	ai->SetReg(41, 0x20 | 0x02);
	ai->Flush();
	
	/* Send a reset to the chip */
	ai->Single(RES);

	/* Set clock stretch */
	if (testParameters->HRPixelMapClockStretch > 1)
		ai->SetClockStretch(STRETCH_AFTER_CAL, testParameters->HRPixelMapStretchDelay, testParameters->HRPixelMapClockStretch);

	/* Get the digital and analog voltages / currents */
	psi::LogInfo() << "Measuring chip voltages and currents ..." << psi::endl;
	TParameter<float> vd("hr_pixelmap_digital_voltage", ai->GetVD());
	TParameter<float> id("hr_pixelmap_digital_current", ai->GetID());
	TParameter<float> va("hr_pixelmap_analog_voltage", ai->GetVA());
	TParameter<float> ia("hr_pixelmap_analog_current", ai->GetIA());
	vd.Write();
	id.Write();
	va.Write();
	ia.Write();
	
	/* Data filters */
	RawData2RawEvent rs;
	RawEventDecoder ed(nroc);
	HitMapper hm(nroc);
	EventCounter count;
	MultiplicityHistogrammer mh;
	PulseHeightHistogrammer phh;
	ConfigParameters * configParameters = ConfigParameters::Singleton();
	phh.LoadCalibration(nroc, configParameters->directory);
	
	/* Repeat measurements multiple times to collect statistics */
	for (int rep = 0; rep < testParameters->HRPixelMapRepetitions; rep++) {
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
		ai->getCTestboard()->Set(21, testParameters->HRPixelMapTriggerRate); // T_Periode has the wrong value. Should be fixed.

		/* Issue continuous Reset-(Calibrate-)Trigger-Token pattern */
		ai->Intern(CAL|TRG|TOK);

		/* Set local trigger, tbm present, and run data aquisition */
		ai->SetReg(41, 0x20 | 0x02 | 0x08);
		ai->Flush();

		/* Reset the aquisition on the testboard */
		ai->SetReg(43, (1 << 0));

		float seconds = testParameters->HRPixelMapAquisitionTime;
		for (float t = seconds; t >= 1; t--) {
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

		/* Decoding chain */
		rd >> rs >> ed >> hm >> count >> mh >> phh >> end;

		/* Free the memory in the RAM */
		ai->getCTestboard()->Daq_Done();
	}

	/* Store histograms */
	for (int i = -2; i < nroc; i++) {
		/* -2: Module with double sized edges, -1: Module */
		TH2I * map = (TH2I *) hm.getHitMap(i)->Clone();
		histograms->Add(map);
		
		/* Make dcol map and hit distribution */
		if (i >= 0) {
			TH1I * dcol_map = new TH1I(Form("dcol_map_C%i", i), Form("DCol hit map ROC %i", i), 26, 0, 26);
			int x, y, z;
			map->GetMaximumBin(x, y, z);
			z = map->GetBinContent(x, y);
			TH1I * hit_dist = new TH1I(Form("hit_dist_C%i", i), Form("Hit distribution ROC %i", i), z > 100 ? 100 : z, 0, z);
			for (int dcol = 0; dcol < 26; dcol++) {
				int sum = 0;
				for (int row = 0; row < 80; row++) {
					sum += map->GetBinContent(2 * dcol + 1, row + 1);
					sum += map->GetBinContent(2 * dcol + 2, row + 1);
					hit_dist->Fill(map->GetBinContent(2 * dcol + 1, row + 1));
					hit_dist->Fill(map->GetBinContent(2 * dcol + 2, row + 1));
				}
				dcol_map->SetBinContent(dcol + 1, sum);
			}
			dcol_map->Sumw2();
			dcol_map->SetMinimum(0);
			histograms->Add(dcol_map);
			histograms->Add(hit_dist);
			
			TH1I * multi = (TH1I *) mh.getRocMultiplicity(i)->Clone();
			histograms->Add(multi);
		}
	}
	histograms->Add((TH1I *) phh.getPulseHeightDistribution()->Clone());
	histograms->Add((TH2F *) phh.getPulseHeightMap()->Clone());
	histograms->Add((TH2F *) phh.getPulseHeightWidthMap()->Clone());
	histograms->Add((TH1I *) phh.getCalPulseHeightDistribution()->Clone());
	histograms->Add((TH2F *) phh.getCalPulseHeightMap()->Clone());
	histograms->Add((TH2F *) phh.getCalPulseHeightWidthMap()->Clone());
	
	TH2I * map = (TH2I *) hm.getHitMap(-1);
	psi::LogInfo() << "Number of triggers: " << count.TriggerCounter << psi::endl;
	psi::LogInfo() << "Number of hits: " << map->GetEntries() << psi::endl;
	psi::LogInfo() << "Rate: " << (map->GetEntries() / (count.TriggerCounter)) * 40e6 / 1e6 / (0.79*0.77 * nroc);
	psi::LogInfo() << " +/- " << (TMath::Sqrt(map->GetEntries()) / (count.TriggerCounter)) * 40e6 / 1e6 / (0.79*0.77 * nroc);
	psi::LogInfo() << " megahits / s / cm2" << psi::endl;
	psi::LogInfo() << "Number of ROC sequence errors: " << count.RocSequenceErrorCounter << psi::endl;
	
	TParameter<float> triggers("pixelmap_triggers", count.TriggerCounter);
	triggers.Write();

	/* Disable clock stretch */
	if (testParameters->HRPixelMapClockStretch > 1)
		ai->SetClockStretch(0, 0, 0);

	/* Reset the chip */
	ai->Single(RES);
	ai->Flush();
}
