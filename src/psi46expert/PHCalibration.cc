#include <iostream>
#include <iomanip>

#include <TSystem.h>
#include <TRandom.h>
#include <TMath.h>

#include "interface/Delay.h"
#include "BasePixel/Roc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "interface/Log.h"
#include "TestRoc.h"
#include "DacDependency.h"
#include "PHCalibration.h"
#include "pipe.h"
#include "DataFilter.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include "BasePixel/DecodedReadout.h"
#include "TestModule.h"


PHCalibration::PHCalibration()
{
  psi::LogDebug() << "[PHCalibration] Initialization." << psi::endl;

	Initialize();
}


PHCalibration::PHCalibration(TestRange *aTestRange, TestParameters *aTestParameters, TBInterface *aTBInterface)
{
	testParameters = aTestParameters;
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
	Initialize();
}


void PHCalibration::Initialize()
{
// 	printf("mode %i\n", mode);s
	if (mode == 0)
	{
		vcalSteps = 10;
		vcal[0] = 50;
		vcal[1] = 100;
		vcal[2] = 150;
		vcal[3] = 200;
		vcal[4] = 250;
		vcal[5] = 30;
		vcal[6] = 50;
		vcal[7] = 70;
		vcal[8] = 90;
		vcal[9] = 200;
		for (int i = 0; i < 5; i++)  ctrlReg[i] = 0;
		for (int i = 5; i < 10; i++)  ctrlReg[i] = 4;
	}
	else if (mode == 1)
	{	
		vcalSteps = 100;
		for (int i = 0; i < vcalSteps; i++) 
		{
			vcal[i] = 5 + i*5;
			vcal[i + vcalSteps] = 5 + i*5;
			if (i < 50) ctrlReg[i] = 0; else ctrlReg[i] = 4;
		}
	}	
	else if (mode == 2)
	{	
		vcalSteps = 102;
		for (int i = 0; i < 51; i++) 
		{
			vcal[i] = 5 + i*5;
			ctrlReg[i] = 0;
		}
		for (int i = 51; i < 102; i++) 
		{
			vcal[i] = 5 + (i-51)*5;
			ctrlReg[i] = 4;
		}
	}
	if (mode == 3)
	{
		vcalSteps = 9;
		vcal[0] = 50;
		vcal[1] = 100;
		vcal[2] = 150;
		vcal[3] = 200;
		vcal[4] = 250;
		vcal[5] = 50;
		vcal[6] = 70;
		vcal[7] = 90;
		vcal[8] = 200;
		for (int i = 0; i < 5; i++)  ctrlReg[i] = 0;
		for (int i = 5; i < 9; i++)  ctrlReg[i] = 4;
	}
}


void PHCalibration::ReadTestParameters(TestParameters *testParameters)
{
	nTrig = (*testParameters).PHCalibrationNTrig;
// 	memoryCorrection = (*testParameters).PHMemoryCorrection / 100;
	mode = (*testParameters).PHCalibrationMode;
	numPixels = (*testParameters).PHCalibrationNPixels;
	calDelVthrComp = (*testParameters).PHCalibrationCalDelVthrComp;
}


void PHCalibration::RocAction()
{
  psi::LogInfo() << "[PHCalibration] Chip #" << chipId << " Calibration: start." << psi::endl;

	gDelay->Timestamp();
	SaveDacParameters();

	// == Open file
	
	ConfigParameters *configParameters = ConfigParameters::Singleton();
	char fname[1000];
	sprintf(fname, "%s/phCalibration_C%i.dat", configParameters->directory, chipId);
	FILE *file = fopen(fname, "w");
	if (!file)
	{
    psi::LogInfo() << "[PHCalibration] Error: Can not open file '" << fname
                   << "' to write PH Calibration." << psi::endl;

		return;
	}

  psi::LogInfo() << "[PHCalibration] Writing PH Calibration to '" << fname
                 << "'." << psi::endl;
 
	fprintf(file, "Pulse heights for the following Vcal values:\n");
	fprintf(file, "Low range: ");
	for (int i = 0; i < vcalSteps; i++) 
	{
		if (ctrlReg[i] == 0) fprintf(file, "%3i ", vcal[i]);
	}
	fprintf(file, "\n");
	fprintf(file, "High range: ");
	for (int i = 0; i < vcalSteps; i++) 
	{
		if (ctrlReg[i] == 4) fprintf(file, "%3i ", vcal[i]);
	}
	fprintf(file, "\n");
	fprintf(file, "\n");

	// == Determine appropriate CalDel and VthrComp
	
	bool debug = false;

	int numFlagsRemaining = numPixels;
        TRandom u;
	bool pxlFlags[ROCNUMROWS*ROCNUMCOLS];
	if ( numPixels < 4160 )
	{
	  while ( numFlagsRemaining > 0 ){
	    int column = TMath::FloorNint(ROCNUMCOLS*u.Rndm());
	    int row    = TMath::FloorNint(ROCNUMROWS*u.Rndm());

	    if ( pxlFlags[column*ROCNUMROWS + row] == false ){ // pixel not yet included in test
	      cout << "flagging pixel in column = " << column << ", row = " << row << " for testing" << endl;
	      pxlFlags[column*ROCNUMROWS + row] = true;
	      numFlagsRemaining--;
	    }
	  }
	}
	
	if (debug) {calDel50 = 44; calDel100 = 63; calDel200 = 66; vthrComp50 = 114; vthrComp100 = 99; vthrComp200 = 85;}
	else if (calDelVthrComp)
	{
		psi::LogInfo() << "Determining CalDel values ..." << psi::endl;
		SetDAC("CtrlReg", 0);
		calDel200 = GetDAC("CalDel"); vthrComp200 = GetDAC("VthrComp"); // from Pretest
		roc->AdjustCalDelVthrComp(15, 15, 50, -0); calDel50 = GetDAC("CalDel"); vthrComp50 = GetDAC("VthrComp");
		roc->AdjustCalDelVthrComp(15, 15, 100, -0); calDel100 = GetDAC("CalDel"); vthrComp100 = GetDAC("VthrComp");
//		roc->AdjustCalDelVthrComp(15, 15, 200, -1); calDel200 = GetDAC("CalDel"); vthrComp200 = GetDAC("VthrComp");
	}
	else
	{
		calDel200 = GetDAC("CalDel"); vthrComp200 = GetDAC("VthrComp"); // from Pretest
		calDel100 = GetDAC("CalDel"); vthrComp100 = GetDAC("VthrComp"); // from Pretest
		calDel50 = GetDAC("CalDel"); vthrComp50 = GetDAC("VthrComp"); // from Pretest
	}

	// == Loop over all pixels

	int ph[vcalSteps][ROCNUMROWS*ROCNUMCOLS];
	int data[ROCNUMROWS*ROCNUMCOLS];
	int phPosition = 16 + aoutChipPosition*3;
	
	for (int i = 0; i < vcalSteps; i++)
	{
		SetDAC("CtrlReg", ctrlReg[i]);
		SetDAC("CalDel", GetCalDel(i));
		SetDAC("VthrComp", GetVthrComp(i));
		SetDAC("Vcal", vcal[i]);
		Flush();

		cout << "Calibrating with Vcal " << setw(3) << vcal[i] << (ctrlReg[i] == 4 ? "H" : "L") << " ... " << endl;

		if (numPixels >= 4160) {
			if (((TBAnalogInterface *) tbInterface)->IsAnalog())
				roc->AoutLevelChip(phPosition, nTrig, data);
			else
				PulseHeightRocDigital(data);
		} else {
			if (((TBAnalogInterface *) tbInterface)->IsAnalog())
				roc->AoutLevelPartOfChip(phPosition, nTrig, data, pxlFlags);
			else
				PulseHeightRocDigital(data);
		}

		for (int k = 0; k < ROCNUMROWS*ROCNUMCOLS; k++) ph[i][k] = data[k];
	}

	for (int col = 0; col < 52; col++)
	{	
	        for (int row = 0; row < 80; row++)
		{
			SetPixel(GetPixel(col, row));
			if (testRange->IncludesPixel(chipId, column, row))
			{
			
				for (int i = 0; i < vcalSteps; i++)
				{
					if (ph[i][col*ROCNUMROWS + row] != 7777) fprintf(file, "%5i ", ph[i][col*ROCNUMROWS + row]);
					else fprintf(file, "  N/A ");
				}
				fprintf(file, "   Pix %2i %2i\n", col, row);
			}
		}
	}

	for (int i = 0; i < vcalSteps; i++) {
		TH2F * ph_map = new TH2F(Form("ph_cal_map_vcal%i%s_C%i", vcal[i], ctrlReg[i] == 4 ? "H" : "L", chipId), Form("Pulse height calibration map ROC %i (Vcal %i%s)", chipId, vcal[i], ctrlReg[i] == 4 ? "H" : "L"), 52, 0, 52, 80, 0, 80);
		ph_map->GetXaxis()->SetTitle("Column");
		ph_map->GetYaxis()->SetTitle("Row");

		for (int col = 0; col < 52; col++) {
		        for (int row = 0; row < 80; row++) {
				SetPixel(GetPixel(col, row));
				if (testRange->IncludesPixel(chipId, column, row) && ph[i][col*ROCNUMROWS + row] != 7777) {
					ph_map->SetBinContent(col + 1, row + 1, ph[i][col * ROCNUMROWS + row]);
					//if (ph[i][col*ROCNUMROWS + row] != 7777) fprintf(file, "%5i ", ph[i][col*ROCNUMROWS + row]);
					//else fprintf(file, "  N/A ");
				}
			}
		}

		histograms->Add(ph_map);
	}

	fclose(file);
	RestoreDacParameters();
	gDelay->Timestamp();
}


int PHCalibration::GetCalDel(int vcalStep)
{
	int conversion = 1;
	if (ctrlReg[vcalStep] == 4) conversion = 7;
	if (vcal[vcalStep]*conversion < 75.) return calDel50;
	else if (vcal[vcalStep]*conversion < 125.) return calDel100;
	else return calDel200;
}


int PHCalibration::GetVthrComp(int vcalStep)
{
	int conversion = 1;
	if (ctrlReg[vcalStep] == 4) conversion = 7;
	if (vcal[vcalStep]*conversion < 75.) return vthrComp50;
	else if (vcal[vcalStep]*conversion < 125.) return vthrComp100;
	else return vthrComp200;
}

void PHCalibration::PulseHeightRocDigital(int data [])
{
	TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;
	ai->Flush();

	/* Structure that holds the decoded readout */
	DecodedReadoutModule * drm = new DecodedReadoutModule;

	/* Set local trigger and channel */
	ai->SetReg(41, 0x20 | 0x01);

	/* Enable the FIFO gate */
	ai->DataCtrl(false, false, true);

	/* Buffer for the ADC data */
	short * buffer = new short [256];
	unsigned short nwords;

	/* iterate over columns and rows to get each pixel efficiency */
	for (int col = 0; col < 52; col++) {
		for (int row = 0; row < 80; row++) {
			/* Arm the pixel */
			roc->ArmPixel(col, row);
			ai->CDelay(500);
			ai->Flush();

			/* send nTrig triggers with calibrates */
			for (int t = 0; t < nTrig; t++) {
				ai->Single(RES|CAL|TRG|TOK);
				ai->CDelay(500);
			}
			ai->Flush();

			ai->getCTestboard()->DataRead(ai->GetTBMChannel(), buffer, 256, nwords);

			/* Calculate the mean pulseheight from nTrig measurements by analysing the data */
			float ph_mean = 0.0;
			int measurement_num = 0;
			int data_pos = 0;
			for (int trig = 0; trig < nTrig; trig++) {
				int retval = decode_digital_readout(drm, buffer + data_pos, nwords, module->NRocs(), 0);
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

			data[80 * col + row] = ph_mean;

			/* Disarm the pixel */
			roc->DisarmPixel(col, row);
			ai->Flush();
		}
	}

	delete buffer;
	delete drm;
	return;
}
