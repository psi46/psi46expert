#include "PhDacOverview.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TCanvas.h"
#include "PhDacScan.h"
#include "BasePixel/DecodedReadout.h"
#include "BasePixel/DigitalReadoutDecoder.h"

PhDacOverview::PhDacOverview(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
  : PhDacScan(aTestRange, testParameters, aTBInterface)
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
  debug = true;
}


void PhDacOverview::ReadTestParameters(TestParameters *testParameters)
{
  PhDacScan::ReadTestParameters(testParameters);
  NumberOfSteps = (*testParameters).PHNumberOfSteps;
}

void PhDacOverview::RocAction()
{
  SaveDacParameters();
  Test::RocAction();
  RestoreDacParameters();
}


void PhDacOverview::PixelAction()
{
  ArmPixel();
  Flush();
  DoDacScan();
  //  DoVsfScan(); // xxx test me!!!
  DisarmPixel();
}

/* Test pulse height vs Vcal in the entire parameter space of the ROC and the TBM */
void PhDacOverview::DoDacScan()
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;

	int offset;
	if (ai->TBMPresent())
		offset = 16;
	else
		offset = 9;

	cout << "chipId = " << chipId << ", col = " << column << ", row = " << row << endl;

	/* Iterate over ROC DACs */
	for (int DacRegister = 1; DacRegister < 28; DacRegister++) {
		/* Exclude DACs that don't exist on the digital ROC psi46dig */
		if (!(ai->IsAnalog())) {
			switch (DacRegister) {
				case 5: case 6: case 8: case 16: case 23: case 24: case 27: case 28:
					continue;
			}
		}

		/* Get the name of the DAC */
		DACParameters * parameters = new DACParameters();
		char * dacName = parameters->GetName(DacRegister);
		delete parameters;
		cout << "Testing ROC DAC #" << DacRegister << " (" << dacName << ") ..." << endl;

		/* Set scan maximum */
		int scanMax;
		if ((DacRegister == 1) || (DacRegister == 4) || (DacRegister == 6) || (DacRegister == 8) || (DacRegister == 14))
			scanMax = 16;
		else
			scanMax = 256;

		/* Scan the DAC */
		int defaultValue = GetDAC(DacRegister);
		int loopNumber = 0;
		for (int scanValue = 0; scanValue < scanMax; scanValue+=((int)scanMax/NumberOfSteps)) {
			loopNumber++;

			TH1D * histo = new TH1D(Form("DAC%i_Value%i", DacRegister, loopNumber), Form("%s=%d", dacName, scanValue), 256, 0, 256);
			histo->GetXaxis()->SetTitle("Vcal [DAC units]");
			histo->GetYaxis()->SetTitle("Pulse height [ADC units]");
			SetDAC(DacRegister, scanValue);
			PHDac(histo);
			histograms->Add(histo);
		}
		SetDAC(DacRegister, defaultValue);
	}

	/* Stop if there is no TBM */
	if (!(ai->TBMPresent()))
		return;

	/* Iterate over TBM DACs */
	for (int DacRegister = 2; DacRegister < 5; DacRegister++) {
		const char * dacName;
		if (DacRegister == 2)
			dacName = "Inputbias";
		else if (DacRegister == 3)
			dacName = "Outputbias";
		else if (DacRegister == 4)
			dacName = "Dacgain";

		cout << "Testing TBM DAC #" << DacRegister << "(" << dacName << ") ..." << endl;

		int scanMax = 256;
		int defaultValue = module->GetTBM(DacRegister);
		int loopNumber = 0;

		for (int scanValue = 0; scanValue < scanMax; scanValue += ((int) scanMax / NumberOfSteps)) {
			loopNumber++;

			TH1D * histo = new TH1D(Form("TBM_DAC%i_Value%i", DacRegister, loopNumber), Form("%s=%d", dacName, scanValue), 256, 0, 256);
			histo->GetXaxis()->SetTitle("Vcal [DAC units]");
			histo->GetYaxis()->SetTitle("Pulse height [ADC units]");
			module->SetTBM(chipId, DacRegister, scanValue);
			PHDac(histo);
			histograms->Add(histo);
		}
		module->SetTBM(chipId,DacRegister,defaultValue);
	}
}

void PhDacOverview::PHDac(TH1D * histo)
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;

	if (ai->IsAnalog()) {
		int offset;
		if (ai->TBMPresent())
			offset = 16;
		else
			offset = 9;

		short result[256];
		ai->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result);
		for (int dac = 0; dac < 256; dac++) {
			if (result[dac] == 7777)
				histo->SetBinContent(dac+1, 0);
			else
				histo->SetBinContent(dac+1, result[dac]);
		}
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
	}
}

void PhDacOverview::DoVsfScan()
{


  int offset;
  if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
  else offset = 9;
  int nTrig = 10;

  SetDAC("CtrlReg",4);

  for (int col = 0; col < 2; col++)
    {
      printf("col = %d\n",col);
      for (int row = 0; row < 2; row++)
	{  
	  //	  printf("row = %d\n",row);
	  for (int vsf = 150; vsf < 255; vsf+=20)
	    {
	      GetDAC("Vsf");
        SetDAC("Vsf", vsf);
	      Flush();
	      short result[256];
	      ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition*3, result);
	      TH1D *histo = new TH1D(Form("Vsf%d_Col%d_Row%d",vsf,col,row), Form("Vsf%d_Col%d_Row%d",vsf,col,row), 256, 0., 256.);
	      for (int dac = 0; dac < 256; dac++)
		{
		  if (result[dac] == 7777) histo->SetBinContent(dac+1, 555);
		  else histo->SetBinContent(dac+1, result[dac]);
		}
	    }
	}
    }





}
