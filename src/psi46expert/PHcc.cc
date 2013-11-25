
// Daniel Pitzl, DESY, May 2013
// PH vs DAC, multiple cc

#include "PHcc.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>

//------------------------------------------------------------------------------
PHcc::PHcc( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters( testParameters );
}

//------------------------------------------------------------------------------
void PHcc::ReadTestParameters( TestParameters *testParameters )
{
  mode  = (*testParameters).PHMode;
  nTrig = (*testParameters).PHNTrig;
  cout << "PHcc mode  " << mode << endl;
  cout << "PHcc nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void PHcc::RocAction()
{
  SaveDacParameters();	

  Test::RocAction();

  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void PHcc::PixelAction()
{
  if( tbInterface->IsAnalog() ) return;

  // digital ROC:

  if( mode ==  5 ) return;
  if( mode ==  6 ) return;
  if( mode ==  8 ) return;
  if( mode == 16 ) return;
  if( mode == 21 ) return;
  if( mode == 23 ) return;
  if( mode == 24 ) return;
  if( mode >= 27 ) return;

  bool ldb = 0;
  bool atb4 = 1; // atb4: compact data = 12 valid bits per word

  DACParameters* parameters = new DACParameters();
  char *dacName = parameters->GetName( mode );
  delete parameters;

  int scanMax;
  if( mode == 1 ||
      mode == 4 ||
      mode == 6 ||
      mode == 8 ||
      mode == 14 )
    scanMax = 16; // 4-bit
  else
    scanMax = 256; // 8-bit

  int col = pixel->GetColumn();
  int row = pixel->GetRow();
  int chp = roc->GetChipId();

  cout << "  col " << setw(2) << col
       << ", row " << setw(2) << row
       << endl;

  // Buffer for the ADC data:
  short * buffer = new short[FIFOSIZE];
  unsigned short nwords;

  // Structure that holds the decoded readout:
  DecodedReadoutModule * drm = new DecodedReadoutModule;

  tbInterface->DataCtrl( false, false, true ); // continue
  ArmPixel();
  Flush();

  int oldcc = tbInterface->GetParameter( "cc" );
  cout << "tcc  " << tbInterface->GetParameter( "tcc" ) << " BC" << endl;
  cout << "tct  " << tbInterface->GetParameter( "tct" ) << " BC" << endl;

  // multiple calibrates

  for( int icc = 1; icc < 6; ++icc ) {

    TProfile *phvsdac =
      new TProfile( Form( "PHvs%s_c%02dr%02d_cc%d", dacName, col, row, icc ),
		    Form( "PHvs%s_c%02dr%02d_cc%d", dacName, col, row, icc ),
		    scanMax, -0.5, scanMax-0.5, -1, 256 );

    phvsdac->GetXaxis()->SetTitle( dacName );
    phvsdac->GetYaxis()->SetTitle( "Pulse height [ADC]" );

    phvsdac->SetMinimum(0);
    phvsdac->SetMaximum(255);

    //--------------------------------------------------------------------------

    tbInterface->SetTBParameter( "cc", icc );
    Flush();

    cout << "cc: " << icc << " calibrates" << endl;

    int n0 = 0;
    int n255 = 0;
    int nhit = 0;
    int amin = 255;
    int vmin = 0;

    // Loop through the whole dac range:

    for( int vdac = 0; vdac < scanMax; vdac++ ) {

      SetDAC( mode, vdac );
      tbInterface->Flush();
      tbInterface->CDelay(400); // 400 clocks = 10 us

      // Send nTrig calibrates to the chip:

      for( int i = 0; i < nTrig; i++ ) {
	tbInterface->Single(RES|CAL|TRG|TOK);
	tbInterface->CDelay(400); // 400 BC = 10 us
      }
      tbInterface->Flush();
      tbInterface->CDelay(4000);

      // Read the data from the FIFO on the testboard:

      tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, FIFOSIZE, nwords );

      if( ldb )
	cout << "  " << dacName
	     << setw(4) << vdac
	     << ":  DataRead " << nwords << " words"
	     << endl;

      // decode:

      float ph_mean = 0.0;
      int measurement_num = 0;
      int data_pos = 0;

      for( int trig = 0; trig < nTrig; trig++ ) {

	if( ldb ) cout << "  trig " << trig;

	int retval = decode_digital_readout( drm, buffer + data_pos,
					     nwords, module->NRocs(), 0 );
	int ph = 0;

	if( retval >= 0 ) { // Successful decoding

	  int hits = drm->roc[roc->GetChipId()].numPixelHits;
	  nhit += hits;
	  if( ldb ) cout << ": hits " << hits;


	  if( hits == 1 ) {

	    if( ldb ) cout << " at col " << drm->roc[roc->GetChipId()].pixelHit[0].columnROC;
	    if( ldb ) cout << ", row "   << drm->roc[roc->GetChipId()].pixelHit[0].rowROC;

	    ph = drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;

	    measurement_num++;

	    if( ldb ) cout << ", PH "   << ph;

	    if( atb4 )
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 2;
	    else
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6; 

	  }

	  else if( hits > 1 ) {

	    // Search for 'our' pixel:

	    for( int ipx = 0; ipx < hits; ipx++ ) {

	      if( drm->roc[roc->GetChipId()].pixelHit[ipx].columnROC == pixel->GetColumn() &&
		  drm->roc[roc->GetChipId()].pixelHit[ipx].rowROC    == pixel->GetRow() ) {

		if( ldb ) cout << " with hit " << ipx;
		if( ldb ) cout << " at col " << drm->roc[roc->GetChipId()].pixelHit[ipx].columnROC;
		if( ldb ) cout << ", row "   << drm->roc[roc->GetChipId()].pixelHit[ipx].rowROC;
		ph = drm->roc[roc->GetChipId()].pixelHit[ipx].analogPulseHeight;
		measurement_num++;
		if( ldb ) cout << ", PH "   << ph;

	      } // match col row

	    } // loop ipx

	    if( atb4 )
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 2;
	    else
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;

	  } // multi hits

	  else {
	    // No hits, move to the next block of data:
	    data_pos += tbInterface->GetEmptyReadoutLengthADC();
	  }

	  ph_mean += ph;

	  if( ph < amin ) {
	    amin = ph;
	    vmin = vdac;
	  }
	}
	else {
	  // Decoding failed, try next block of data:
	  cout << "  : decoder error " << retval << endl;
	  data_pos += tbInterface->GetEmptyReadoutLengthADC();
	}
	if( ldb ) cout << endl;

	phvsdac->Fill( vdac, ph );

      } // triggers

      // average:
      if( measurement_num > 0 )
	ph_mean /= measurement_num;

      if( ph_mean < 0.5 ) n0++;
      if( ph_mean > 254.5 ) n255++;


    } // Vdac loop

    cout << "  : zeroes " << n0;
    cout << ", overflows " << n255;
    cout << ", min " << amin;
    cout << " at " << vmin;
    cout << endl;

    histograms->Add(phvsdac);

  } // cc loop

  // Cleanup:

  DisarmPixel(); // Disarm = Disable + ClrCal

  tbInterface->SetTBParameter( "cc", oldcc );
  Flush();
  cout << "cc   " << oldcc << " calibrates" << endl;

  delete drm;
  delete buffer;

}
