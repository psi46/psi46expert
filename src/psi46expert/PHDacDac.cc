
// G. Dolinska, I. Korol, D. Pitzl, Jan 2013
// PH vs DAC1 vs DAC2 scan, one pixel

#include "PHDacDac.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>
#include "ThresholdMap.h"
#include <TProfile2D.h>

//------------------------------------------------------------------------------
PHDacDac::PHDacDac(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void PHDacDac::ReadTestParameters(TestParameters *testParameters)
{
  dac1 = (*testParameters).Dac1;
  dac2 = (*testParameters).Dac2;
  nTrig = (*testParameters).DacNTrig;
  dacRange1 = (*testParameters).DacRange1;
  dacRange2 = (*testParameters).DacRange2;
  cout << "[PHDacDac] DAC " << dac2 << " vs " << dac1 << endl;
}

//------------------------------------------------------------------------------
void PHDacDac::PixelAction()
{
  bool ldb = 0;
  bool atb4 = 1; // atb4: compact data = 12 valid bits per word

  DACParameters* parameters = new DACParameters();
  char *dacName1 = parameters->GetName(dac1);
  char *dacName2 = parameters->GetName(dac2);

  TProfile2D *histo = new TProfile2D( Form("%s%s_c%dr%d_C%i", dacName2, dacName1, column, row, chipId),Form("%s%s_c%dr%d_C%i", dacName2, dacName1, column, row, chipId), dacRange1, 0, dacRange1, dacRange2, 0, dacRange2, 0, 256 );
  histo->GetXaxis()->SetTitle( Form( "%s [DAC units]", dacName1 ) );
  histo->GetYaxis()->SetTitle( Form( "%s [DAC units]", dacName2 ) );
  histo->GetZaxis()->SetTitle( "<PH> [ADC]" );
	
  SaveDacParameters();	
  Flush();
	
  ArmPixel();
  Flush();

  // analog ROC:

  if( tbInterface->IsAnalog() ) {
  }

  // digital ROC:

  else {

    /* Show the whole range of the ADC data in the histogram */
    histo->SetMinimum(0);
    histo->SetMaximum(255);

    /* Buffer for the ADC data */
    short * buffer = new short [16000];
    unsigned short nwords;

    /* Structure that holds the decoded readout */
    DecodedReadoutModule * drm = new DecodedReadoutModule;

    /* Set local trigger and channel */
    tbInterface->SetReg( 41, 0x20 | 0x01 );
    tbInterface->DataCtrl( false, false, true );

    EnablePixel();
    ArmPixel();

    cout << "col " << setw(2) << pixel->GetColumn();
    cout << ", row " << setw(2) << pixel->GetRow();

    int n0 = 0;
    int n255 = 0;
    int nhit = 0;

    // Loop through the whole Idac1 range:

    for( int idac1 = 0; idac1 < dacRange1; idac1++ ) {

      SetDAC( dac1, idac1 );
      tbInterface->Flush();
      tbInterface->CDelay(500); // 500 clocks = 12.5 us
      //DP tbInterface->CDelay(500);

      cout << parameters->GetName(dac1) << ": " << idac1 << endl;

      // Loop through dac2:

      for( int idac2 = 0; idac2 < dacRange2; idac2++ ) {

	SetDAC( dac2, idac2 );
	tbInterface->Flush();
	tbInterface->CDelay(500); // 500 clocks = 12.5 us
	//DP tbInterface->CDelay(500);

	// Send nTrig calibrates to the chip:

	for( int i = 0; i < nTrig; i++ ) {
	  tbInterface->Single(RES|CAL|TRG|TOK);
	  //DP tbInterface->CDelay(500);
	}
	tbInterface->Flush();
	tbInterface->CDelay(500);

	// Read the data from the FIFO on the testboard:

	tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, 16000, nwords );

	if( ldb ) cout << "Idac1 " << setw(3) << idac1
		       << ":  DataRead " << nwords << " words"
		       << endl;

	// decode:

	float ph_mean = 0.0;
	int measurement_num = 0;
	int data_pos = 0;

	for( int trig = 0; trig < nTrig; trig++ ) {

	  //cout << "  trig " << trig;

	  int retval = decode_digital_readout( drm, buffer + data_pos,
					       nwords, module->NRocs(), 0 );

	  if( retval >= 0 ) {

	    /* Successful decoding */

	    int nhits = drm->roc[roc->GetChipId()].numPixelHits;
	    nhit += nhits;
	    //cout << ": hits " << nhits;
	    if( nhits == 1 ) {
	      /* Record the pulse height and move to the next block of data */
	      int ph = drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;
	      if( ldb ) cout << " PH " << ph;
	      ph_mean += ph;
	      if( atb4 )
		data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 2; // atb4
	      else
		data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 6;
	      measurement_num++;
	      if( ldb ) cout << " at col " << drm->roc[roc->GetChipId()].pixelHit[0].columnROC;
	      if( ldb ) cout << ", row " << drm->roc[roc->GetChipId()].pixelHit[0].rowROC;
	    }
	    else if( nhits > 1 ) {
	      // More hits than expected. Move to the next block of data:
	      if( atb4 )
		data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 2; // atb4
	      else
		data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 6;
	      //cout << "|" << nhits << "|";
	      cout << "multi hits" << nhits << endl;
	    }
	    else {
	      // No hits, move to the next block of data:
	      data_pos += tbInterface->GetEmptyReadoutLengthADC();
	    }
	  }
	  else {
	    /* Decoding failed. Try next block of data. */
	    cout << ": error" << endl;
	    data_pos += tbInterface->GetEmptyReadoutLengthADC();
	  }
	if( ldb ) cout << endl;
	} // triggers

	// average:
	if( measurement_num > 0 )
	  ph_mean /= measurement_num;

	if( ph_mean < 0.5 ) n0++;
	if( ph_mean > 254.5 ) n255++;

	histo->Fill( idac1 + 1,  idac2 + 2, ph_mean );

      } // Idac2 loop

    } // Idac1 loop

    cout << ": zeroes " << n0;
    cout << ", overflows " << n255;
    cout << endl;

    /* Cleanup */
    //DP DisablePixel();
    DisarmPixel(); // Disarm = Disable + ClrCal

    delete drm;
    delete buffer;

  } // digital

  RestoreDacParameters();
  delete parameters;
  histograms->Add(histo);
}

//------------------------------------------------------------------------------
void PHDacDac::SetDacs(int d1, int d2, int range1, int range2)
{
  dac1 = d1;
  dac2 = d2;
  dacRange1 = range1;
  dacRange2 = range2;
  cout << "[PHDacDac] DAC " << dac2 << " vs " << dac1 << endl;
}

//------------------------------------------------------------------------------
void PHDacDac::SetNTrig(int aNumber)
{
  nTrig = aNumber;
}
