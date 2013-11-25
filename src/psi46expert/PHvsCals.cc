
// PH vs Vcals: spy pad to sensor

#include "PHvsCals.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>

//------------------------------------------------------------------------------
PHvsCals::PHvsCals( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void PHvsCals::ReadTestParameters(TestParameters *testParameters)
{
  nTrig = (*testParameters).PHNTrig;
  cout << "PHvsCals nTrig " << nTrig << endl;
  mode = 25; // 25 = Vcal
}

//------------------------------------------------------------------------------
void PHvsCals::RocAction()
{
  SaveDacParameters();	

  Test::RocAction();	 

  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void PHvsCals::PixelAction()
{
  char *dacName = "Cals";
  PhDac(dacName);
}

//------------------------------------------------------------------------------
void PHvsCals::PhDac( char *dacName )
{
  bool ldb = 0;

  TH1D *histo = 
    new TH1D(
	     Form( "Ph_vs_%s_c%02dr%02d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId() ),
	     Form( "Ph_vs_%s_c%02dr%02d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId() ),
	     256, 0, 256 );

  histo->GetXaxis()->SetTitle( "Vcal (DAC units)" );
  histo->GetYaxis()->SetTitle( "Pulse height s [ADC]" );

  // analog ROC:

  if( tbInterface->IsAnalog() ) {

    TH1D * ubHist = new TH1D("ubHist", "ubHist", 256, 0, 256);
    ubHist->SetLineColor(kRed);

    EnablePixel();
    Cals();
    Flush();
    tbInterface->CDelay(500);

    short result[256], data[10000];
    int offset;
    int ubPosition = 8 + aoutChipPosition * 3;
    unsigned short count;

    if( tbInterface->TBMPresent())
      offset = 16;
    else
      offset = 9;

    tbInterface->PHDac( mode, 256, nTrig, offset + aoutChipPosition * 3, result );
    tbInterface->ADCData( data, count );

    int ubLevel = data[ubPosition];

    for( int dac = 0; dac < 256; dac++) {
      if( result[dac] == 7777)
	histo->SetBinContent(dac + 1, 0);
      else
	histo->SetBinContent(dac + 1, result[dac]);

      ubHist->SetBinContent(dac + 1, ubLevel);
    }

    roc->ClrCal();
    //DP DisablePixel();
    DisarmPixel(); // Disarm = Disable + ClrCal
    histograms->Add(histo);
    histograms->Add(ubHist);

  }//analog

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
    Cals();
    Flush();

    cout << "col " << setw(2) << pixel->GetColumn();
    cout << ", row " << setw(2) << pixel->GetRow();

    int n0 = 0;
    int n255 = 0;
    int nhit = 0;

    // Loop through the whole Vcal range:

    for( int vcal = 0; vcal < 256; vcal++ ) {

      /* Set Vcal */
      SetDAC( "Vcal", vcal );
      tbInterface->Flush();
      tbInterface->CDelay(500); // 500 clocks = 12.5 us
      tbInterface->CDelay(500);

      // Send nTrig calibrates to the chip:

      for( int i = 0; i < nTrig; i++ ) {
	tbInterface->Single(RES|CAL|TRG|TOK);
	tbInterface->CDelay(500);
      }
      tbInterface->Flush();
      tbInterface->CDelay(500);

      // Read the data from the FIFO on the testboard:

      tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, 16000, nwords );

      if( ldb ) cout << "Vcal" << setw(3) << vcal
		     << ":  DataRead " << nwords << " words"
		     << endl;

      // decode:

      float ph_mean = 0.0;
      int measurement_num = 0;
      int data_pos = 0; // not used?

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
	    ph_mean += ph;
	    //data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 6;
	    data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 2; // atb4
	    measurement_num++;
	    if( ldb ) cout << " at col " << drm->roc[roc->GetChipId()].pixelHit[0].columnROC;
	    if( ldb ) cout << ", row " << drm->roc[roc->GetChipId()].pixelHit[0].rowROC;
	  }
	  else if( nhits > 1 ) {
	    /* More hits than expected. Move to the next block of data. */
	    //data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 6;
	    data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 2; // atb4
	    //cout << "|" << nhits << "|";
	    cout << "multi hits" << nhits << endl;
	  }
	  else {
	    /* No hits, move to the next block of data. */
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

      histo->SetBinContent( vcal + 1, ph_mean );

    } // Vcal loop

    cout << ": zeroes " << n0;
    cout << ", overflows " << n255;
    cout << endl;

    /* Cleanup */
    //DP DisablePixel();
    DisarmPixel(); // Disarm = Disable + ClrCal

    delete drm;
    delete buffer;

    histograms->Add(histo);

  } // digital
}
