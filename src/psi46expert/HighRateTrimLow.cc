#include <iostream>
using namespace std;

#include "HighRateTrimLow.h"

#include "TestModule.h"
#include "TestRoc.h"
#include "TestPixel.h"
#include "interface/Delay.h"
#include "BasePixel/RawPacketDecoder.h"
#include "DataFilter.h"
#include "interface/Log.h"

#include <TMath.h>

HRTrimLow::HRTrimLow(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
}

HRTrimLow::~HRTrimLow()
{
	
}

void HRTrimLow::RocAction(void)
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;

	/* Set trim bits all to 1111=0xf */
	roc->SetTrim(0xf);
	roc->SetDAC("Vtrim", 130);
	
	/* Unmask the ROC */
	roc->EnableAllPixels();
	ai->Flush();
	
	/* Find lowest working threshold (highest value of VthrComp) */
	const int start = 110;
	int threshold;
	int increment = 5;
	int breakdown = 130;
	float previous_rate = 0.0;
	TH1F * threshold_scan = new TH1F("trim_low_threshold_scan", "Threshold scan", breakdown - start, start, breakdown);
	for (threshold = start; threshold < breakdown; threshold += increment) {
		cout << endl;
		psi::LogInfo() << "Setting threshold (VcThr) to " << threshold << psi::endl;
		roc->SetDAC("VthrComp", threshold);

		/* Measure the map */
		MakeMap();
		
		/* Add histogram */
		map->SetNameTitle(Form("hitmap-vcthr-%i", threshold), Form("Hit map VcThr=%i", threshold));
		histograms->Add(map);
		
		/* Calculate rate */
		float rate = (map->GetEntries() / triggers) * 40e6 / 1e6 / (0.79*0.77);
		float rate_error = (TMath::Sqrt(map->GetEntries()) / triggers) * 40e6 / 1e6 / (0.79*0.77);
		threshold_scan->SetBinContent(threshold - start + 1, rate);
		threshold_scan->SetBinError(threshold - start + 1, rate_error);
		psi::LogInfo() << "Rate: " << rate;
		psi::LogInfo() << " megahits / s / cm2" << psi::endl;
		
		/* Breakdown reached? */
		if (rate < previous_rate) {
			if (increment == 5) {
				breakdown = threshold;
				threshold -= increment;
				increment = 2;
			} else if (increment == 2) {
				breakdown = threshold;
				threshold -= increment;
				increment = 1;
			} else {
				breakdown = threshold;
				break;
			}
		} else {
			previous_rate = rate;
		}
	}
	threshold_scan->GetXaxis()->SetTitle("VcThr");
	threshold_scan->GetYaxis()->SetTitle("Rate [MHz/cm2]");
	histograms->Add(threshold_scan);

	/* Set threshold to a safe distance from breakdown */
	if (threshold >= breakdown)
		threshold = breakdown - 1;
	threshold -= 2;
	roc->SetDAC("VthrComp", threshold);
	
	cout << endl;
	psi::LogInfo() << "Lowest value of threshold (VthrComp/VcThr): " << threshold << psi::endl;
	
	TH2I * trim_bits = new TH2I("trimbits", "Trim bits", 52, 0, 52, 80, 0, 80);
	for (int col = 0; col < 52; col++) {
		for (int row = 0; row < 80; row++) {
			trim_bits->SetBinContent(col + 1, row + 1, 15);
		}
	}
	
	Test::RocAction();
}

void HRTrimLow::PixelAction(void)
{
	/* Ignore all pixels except some */
	if (column != 26 || row < 40 || row > 45)
		return;
	//if (column != 26 || row != 41)
	//	return;
		
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;

	roc->EnableAllPixels();

	cout << endl;
	cout << "Testing pixel " << column << " " << row << endl;
	
	TH1F * trim_scan = new TH1F(Form("trim_scan_col%i_row%i", column, row), Form("Trim bit scan col=%i row=%i", column, row), 16, 0, 16);
	
	int trim;
	int increment = 4;
	float last = 0;
	int nlast = 0;
	int breakdown = -1;
	for (trim = 15; trim > breakdown; trim -= increment) {
		cout << endl;
		cout << "Setting trim bit to " << trim << endl;
		ai->RocPixTrim(column, row, trim);

		MakeMap();

		trim_scan->SetBinContent(trim + 1, map->GetBinContent(column + 1, row + 1));
		trim_scan->SetBinError(trim + 1, map->GetBinError(column + 1, row + 1));
		map->SetNameTitle(Form("trim_low_pixelmap_col%i_row%i_trim%x", column, row, trim), Form("Pixelmap Col=%i Row=%i Trim=%x", column, row, trim));

		int current = map->GetBinContent(column + 1, row + 1);
		cout << "Hits: " << current << " (last=" << last << ")" << endl;
		if (trim == 15) {
			last = current;
			nlast++;
		} else {
			if (current > 3 * last) {
				if (increment == 4) {
					breakdown = trim;
					trim += increment;
					increment = 2;
				} else if (increment == 2) {
					breakdown = trim;
					trim += increment;
					increment = 1;
				} else {
					breakdown = trim;
				}
			} else {
				if (increment == 4 && trim - increment <= breakdown)
					increment = 2;
				if (increment == 2 && trim - increment <= breakdown)
					increment = 1;
				last = current + nlast * last;
				last /= ++nlast;
			}
		}
	}
	
	cout << endl;
	cout << "Found trim bit: " << breakdown + 1 << endl;
	
	histograms->Add(trim_scan);
	ai->RocPixTrim(column, row, breakdown + 1);
}

/* Takes x-ray data */
void HRTrimLow::MakeMap(void)
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;
	ai->Flush();

	/* ??? */
	ai->getCTestboard()->DataBlockSize(100);
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
	
	float seconds = 1;
	for (float t = seconds; t > 0; t--) {
		cout << "\rTaking data (" << t << " seconds) ... ";
		cout.flush();
		gDelay->Mdelay(1000);
	}
	cout << "\r";
	cout.flush();
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
	
	/* Decoding chain */
	rd >> rs >> ed >> hm >> count >> end;

	/* Store histogram and number of triggers */
	map = (TH2I *) hm.getHitMap(0)->Clone();
	triggers = count.TriggerCounter;

	/* Free the memory in the RAM */
	ai->getCTestboard()->Daq_Done();

	/* Reset the chip */
	ai->Single(RES);
	ai->Flush();
}
