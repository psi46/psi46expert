
// PH distribution, one pixel, fixed Vcal

#include "PHplot.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>
#include <cmath>

//------------------------------------------------------------------------------
PHplot::PHplot( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void PHplot::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).PHNTrig;
  cout << "PHplot nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void PHplot::RocAction()
{
  SaveDacParameters();	
  Test::RocAction();	 
  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void PHplot::PixelAction()
{
  bool ldb = 1;

  int chp = roc->GetChipId();
  int col = pixel->GetColumn();
  int row = pixel->GetRow();

  TH1D *histo = 
    new TH1D( Form( "PHc%02dr%02d_C%d", col, row, chp ),
	      Form( "PHc%02dr%02d_C%d", col, row, chp ),
	      256, -0.5, 255.5 );

  histo->GetXaxis()->SetTitle( "Pulse height [ADC]" );
  histo->GetYaxis()->SetTitle( "triggers" );

  TH1D * hmulti =
    new TH1D( Form( "multiplicity_c%02dr%02d_C%d", col, row, chp ),
	      "hits per trigger;hits per trigger;triggers",
	      81, -0.5, 80.5 );

  TH1D * hwords =
    new TH1D( Form( "readoutwords_c%02dr%02d_C%d", col, row, chp ),
	      "words per readout;words per readout;readouts",
	      501, -0.5, 500.5 );

  // gain file:

  double amax[52][80];
  double Gain[52][80];
  double horz[52][80];
  double vert[52][80];
  double expo[52][80];

  string gainFileName;

  gainFileName = "/home/pitzl/psi/digi/chip301/gaindigi.dat";
  gainFileName = "/home/pitzl/psi/digi/chip205/gainls-Vdig15-Ia25-trim30.dat";

  TH1D * his4 = new TH1D( Form( "Vcal_c%02dr%02d_C%d", col, row, chp ),
			  Form( "Vcal_c%02dr%02d_C%d;PH [large Vcal DAC];events",
				col, row, chp ),
			  256, -0.5, 255.5 );

  ifstream gainFile( gainFileName.c_str() );

  bool haveGain = 0;

  if( gainFile ) {

    char ih[99];
    int icol;
    int irow;
    double am;
    double ho;
    double ga;
    double vo;
    double ex = 0.0;
    double aa = 1.0;

    cout << endl;
    cout << "gainFile: " << gainFileName << endl;
    haveGain = 1;

    while( gainFile >> ih ) {
      // Weibull coeffs from gaindigi.dat
      gainFile >> icol;
      gainFile >> irow;
      gainFile >> ho;//horz offset
      gainFile >> ga;//width
      gainFile >> ex;//exponent
      gainFile >> am;//gain
      gainFile >> vo;//vert offset

      amax[icol][irow] = am*aa;
      Gain[icol][irow] = ga;
      horz[icol][irow] = ho;
      vert[icol][irow] = vo*aa;
      expo[icol][irow] = ex;
    }
    cout << endl;

  } // gainFile

  // analog ROC:

  if( tbInterface->IsAnalog() ) {

  }
  else { // digital ROC:

    bool atb4 = 1; // atb4: compact data = 12 valid bits per word

    // Buffer for the ADC data:
    short * buffer = new short[4096];
    unsigned short nwords;

    // Structure that holds the decoded readout:
    DecodedReadoutModule * drm = new DecodedReadoutModule;

    // Set local trigger and channel:
    tbInterface->SetReg( 41, 0x20 | 0x01 );
    tbInterface->DataCtrl( false, false, true );

    EnablePixel();
    ArmPixel();

    cout << "  col " << col;
    cout << ", row " << row;

    int n0 = 0;
    int n255 = 0;
    int nhit = 0;

    // meta-triggers:

    for( int evt = 0; evt < 100; evt++ ) {

      // Send nTrig calibrates to the chip:

      for( int i = 0; i < nTrig; i++ ) {
	tbInterface->Single(RES|CAL|TRG|TOK);
	tbInterface->CDelay(500);
      }
      tbInterface->Flush();

      // Read the data from the FIFO on the testboard:

      tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, 4096, nwords );

      if( ldb ) 
	cout << "event" << setw(3) << evt
	     << ": DataRead " << nwords << " words"
	     << endl;

      hwords->Fill( nwords );

      // decode:

      int data_pos = 0;

      for( int trig = 0; trig < nTrig; trig++ ) {

	if( ldb ) cout << "  trig " << trig;

	int retval = 
	  decode_digital_readout( drm, buffer + data_pos,
				  nwords, module->NRocs(), 0 );

	if( retval >= 0 ) { // Successful decoding

	  int hits = drm->roc[roc->GetChipId()].numPixelHits;
	  nhit += hits;
	  if( ldb ) cout << ": hits " << hits;

	  hmulti->Fill( hits );

	  int ph = 0;

	  if( hits == 1 ) {

	    ph = drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;

	    //check address:

	    if( drm->roc[roc->GetChipId()].pixelHit[0].columnROC != col )
	      cout << "wrong col " << drm->roc[roc->GetChipId()].pixelHit[0].columnROC;
	    if( drm->roc[roc->GetChipId()].pixelHit[0].rowROC != row )
	      cout << ", row " << drm->roc[roc->GetChipId()].pixelHit[0].rowROC;

	    if( atb4 )
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 2; // atb4
	    else
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
	  }

	  else if( hits > 1 ) {

	    // Search for 'our' pixel:

	    for( int ipx = 0; ipx < hits; ipx++ ) {

	      if( drm->roc[roc->GetChipId()].pixelHit[ipx].columnROC == col &&
		  drm->roc[roc->GetChipId()].pixelHit[ipx].rowROC    == row ) {

		if( ldb ) cout << " with hit " << ipx;
		if( ldb ) cout << " at col " << drm->roc[roc->GetChipId()].pixelHit[ipx].columnROC;
		if( ldb ) cout << ", row "   << drm->roc[roc->GetChipId()].pixelHit[ipx].rowROC;
		ph = drm->roc[roc->GetChipId()].pixelHit[ipx].analogPulseHeight;
		if( ldb ) cout << ", PH "   << ph;

	      } // match col row

	    } // loop ipx

	    if( atb4 )
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 2; // atb4
	    else
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
	  }

	  else {
	    /* No hits, move to the next block of data. */
	    data_pos += tbInterface->GetEmptyReadoutLengthADC();
	  }

	  if( ph < 1 ) n0++;
	  if( ph > 254 ) n255++;
	  histo->Fill( ph );

	  // Gain:

	  if( haveGain ) {

	    double Aout = ph;

	    double Ared = Aout - vert[col][row];//sub vert offset

	    double ma9 = amax[col][row];

	    if( Ared >  ma9-1 ) ma9 = 1.000001* Ared;
	    if( Ared <  -ma9+1 ) ma9 = Ared - 1;

	    // inverse Weibull gives large Vcal:

	    double vcal4 = pow( -log( 1 - Ared / ma9 ), 1/expo[col][row] )
	      * Gain[col][row] + horz[col][row];

	    his4->Fill( vcal4 );

	  } // gain

	}

	else {
	  // Decoding failed. Try next block of data...
	  cout << "digital decoder error" << endl;
	  data_pos += tbInterface->GetEmptyReadoutLengthADC();
	}

	if( ldb ) cout << endl;

      } // triggers

    } // evt loop

    cout << ": zeroes " << n0;
    cout << ", overflows " << n255;
    cout << endl;

    //Cleanup:

    //DP DisablePixel();
    DisarmPixel(); // Disarm = Disable + ClrCal

    delete drm;
    delete buffer;

    histograms->Add(histo);
    if( haveGain )
      histograms->Add(his4);
    histograms->Add(hwords);
    histograms->Add(hmulti);

  } // digital
}
