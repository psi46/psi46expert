#include "PHTest.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"


PHTest::PHTest(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
}


void PHTest::ReadTestParameters(TestParameters *testParameters)
{
	mode = (*testParameters).PHMode;
	nTrig = (*testParameters).PHNTrig;
}


void PHTest::RocAction()
{
	SaveDacParameters();	
	if (mode == 0)
	{
		map = new TH2D(Form("PH_C%d", chipId),Form("PH_C%d", chipId), ROCNUMCOLS, 0, ROCNUMCOLS, ROCNUMROWS, 0, ROCNUMROWS);
		int data[ROCNUMROWS*ROCNUMCOLS], offset;
		if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
		else offset = 9;
		roc->AoutLevelChip(offset + aoutChipPosition*3, nTrig, data);
		for (int col = 0; col < ROCNUMCOLS; col++)
		{	
	        	for (int row = 0; row < ROCNUMROWS; row++) map->SetBinContent(col+1, row+1, data[col*ROCNUMROWS+row]);
		}
                histograms->Add(map); 
		 
	}
	Test::RocAction();	 
	RestoreDacParameters();
}


void PHTest::PixelAction()
{
	if (mode == 0) {}
	else
	{
		DACParameters* parameters = new DACParameters();
		char *dacName = parameters->GetName(mode);
		delete parameters;
		PhDac(dacName);
	}
}


void PHTest::PhDac(char *dacName)
{
	TH1D *histo = new TH1D(Form("Ph%s_c%dr%d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId()),Form("Ph%s_c%dr%d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId()), 256, 0, 256);
	histo->GetXaxis()->SetTitle("Vcal (DAC units)");
	histo->GetYaxis()->SetTitle("Pulse height [ADC]");

	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;

	if (ai->IsAnalog()) {
		TH1D * ubHist = new TH1D("ubHist", "ubHist", 256, 0, 256);
		ubHist->SetLineColor(kRed);

		EnablePixel();
		Cal();
		Flush();

		short result[256], data[10000];
		int offset;
		int ubPosition = 8 + aoutChipPosition * 3;
		unsigned short count;

		if (ai->TBMPresent())
			offset = 16;
		else
			offset = 9;

		ai->PHDac(mode, 256, nTrig, offset + aoutChipPosition * 3, result);
		ai->ADCData(data, count);

		int ubLevel = data[ubPosition];

		for (int dac = 0; dac < 256; dac++) {
			if (result[dac] == 7777)
		    		histo->SetBinContent(dac + 1, 0);
			else
				histo->SetBinContent(dac + 1, result[dac]);

			ubHist->SetBinContent(dac + 1, ubLevel);
		}

		roc->ClrCal();
		DisablePixel();
		histograms->Add(histo);
		histograms->Add(ubHist);
	} else {
		/* Show the whole range of the ADC data in the histogram */
		histo->SetMinimum(0);
		histo->SetMaximum(255);

		/* Buffer for the ADC data */
		short * buffer = new short [256];
		unsigned short nwords;

		/* Structure that holds the decoded readout */
		DecodedReadoutModule * drm = new DecodedReadoutModule;

		/* Set local trigger and channel */
		ai->SetReg(41, 0x20 | 0x01);
		ai->DataCtrl(false, false, true);

		EnablePixel();
		ArmPixel();

		/* Loop through the whole Vcal range */
		for (int vcal = 0; vcal < 256; vcal++) {
			/* Set Vcal */
			SetDAC("Vcal", vcal);
			ai->CDelay(500);
			ai->Flush();

			/* Send nTrig calibrates to the chip */
			for (int i = 0; i < nTrig; i++) {
				ai->Single(RES|CAL|TRG|TOK);
				ai->CDelay(500);
			}
			ai->Flush();

			/* Read the data from the FIFO on the testboard */
			ai->getCTestboard()->DataRead(ai->GetTBMChannel(), buffer, 256, nwords);

			/* Calculate the mean pulseheight from nTrig measurements by analysing the data */
			float ph_mean = 0.0;
			int measurement_num = 0;
			int data_pos = 0;
			for (int trig = 0; trig < nTrig; trig++) {
				int retval = decode_digital_readout(drm, buffer + trig * (ai->GetEmptyReadoutLengthADC() + 6), nwords, module->NRocs(), 0);
				if (retval >= 0) {
					/* Successful decoding */
					int hits = drm->roc[roc->GetChipId()].numPixelHits;
					if (hits == 1) {
						/* Record the pulse height and move to the next block of data */
						ph_mean += drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;
						data_pos += ai->GetEmptyReadoutLengthADC() + hits * 6;
						measurement_num++;
					} else if (hits > 1) {
						/* More hits than expected. Move to the next block of data. */
						data_pos += ai->GetEmptyReadoutLengthADC() + hits * 6;
					} else {
						/* No hits, move to the next block of data. */
						data_pos += ai->GetEmptyReadoutLengthADC();
					}
				} else {
					/* Decoding failed. Try next block of data. */
					data_pos += ai->GetEmptyReadoutLengthADC();
				}
			}

			/* Finalize the mean value of the pulseheight */
			if (measurement_num > 0)
				ph_mean /= measurement_num;

	    		histo->SetBinContent(vcal + 1, ph_mean);
		}

		/* Cleanup */
		DisablePixel();

		delete drm;
		delete buffer;

		histograms->Add(histo);
	}
}

