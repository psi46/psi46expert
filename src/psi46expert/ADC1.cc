
// Daniel Pitzl, DESY, May 2013

// user: arm i:j k:l

#include "ADC1.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>
#include <cmath>

//------------------------------------------------------------------------------
ADC1::ADC1( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters( testParameters );
}

//------------------------------------------------------------------------------
void ADC1::ReadTestParameters( TestParameters *testParameters )
{
}

//------------------------------------------------------------------------------
void ADC1::RocAction()
{
  if( tbInterface->IsAnalog() ) {

    cout << "not implemented for analog ROC" << endl;
    return;

  }//analog

  SaveDacParameters();

  int vcal = GetDAC( "Vcal" );
  int creg = GetDAC( "CtrlReg" );

  cout << "Vcal is " << vcal << " at CtrlReg " << creg << endl;

  TH2D * hitmap =
    new TH2D( Form( "HitmapCR%iVcal%i_C%d", creg, vcal, chipId ),
	      Form( "Hit map ROC%d, CtrlReg %d, Vcal %d;col;row;hits",
		    chipId, creg, vcal ),
	      ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
	      ROCNUMROWS, -0.5, ROCNUMROWS-0.5 );

  TProfile2D * adcmap =
    new TProfile2D( Form( "ADCmapCR%iVcal%i_C%d", creg, vcal, chipId ),
		    Form( "ADC map ROC%d, CtrlReg %d, Vcal %d;col;row;<PH> [ADC]",
			  chipId, creg, vcal ),
		    ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
		    ROCNUMROWS, -0.5, ROCNUMROWS-0.5,
		    0, 256 );

  adcmap->SetMinimum(0);
  adcmap->SetMaximum(256);

  TH1D * adchst =
    new TH1D( Form( "ADChistCR%iVcal%i_C%d", creg, vcal, chipId ),
	      Form( "ADC ROC%d, CtrlReg %d, Vcal %d;PH [ADC]",
		    chipId, creg, vcal ),
	      256, -0.5, 255.5 );

  TProfile2D * map4 =
    new TProfile2D( Form( "VcalmapCR%iVcal%i_C%d", creg, vcal, chipId ),
		    Form( "Vcal map ROC%d, CtrlReg %d, Vcal %d;col;row;<PH> [large Vcal DAC]",
			  chipId, creg, vcal ),
		    ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
		    ROCNUMROWS, -0.5, ROCNUMROWS-0.5,
		    0, 256 );

  map4->SetMinimum(0);
  map4->SetMaximum(256);

  TH1D * his4 =
    new TH1D( Form( "Vcal_CR%i_Vcal%i_C%d", creg, vcal, chipId ),
	      Form( "Vcal ROC%d, CtrlReg %d, Vcal %d;PH [large Vcal DAC];pixels",
		    chipId, creg, vcal ),
	      256, -0.5, 255.5 );

  //----------------------------------------------------------------------------

  // Send rctk:

  tbInterface->DataCtrl( false, true ); // no clear, trigger, no_continue
  tbInterface->Single(RES|CAL|TRG|TOK);
  tbInterface->CDelay(500);

  tbInterface->Flush();
  tbInterface->CDelay(40000); // 40'000 BC = 1 ms

  // Read data from USB:
  // works, but get only 4096 words = FIFOSIZE

  short * buffer = new short[32768]; // 32768 = 2^15
  unsigned short nwords = 0;

  tbInterface->getCTestboard()->DataReadRaw( tbInterface->GetTBMChannel(), buffer, 32768, nwords );

  cout << "[ADC1] DataReadRaw: " << nwords << " words" << endl;

  // gain file:

  double amax[52][80];
  double Gain[52][80];
  double horz[52][80];
  double vert[52][80];
  double expo[52][80];

  string gainFileName;

  gainFileName = "/home/pitzl/psi/digi/chip301/gaindigi.dat";

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

  // decode digital ROC:

  DecodedReadoutModule * drm = new DecodedReadoutModule;

  int retval = 
    decode_digital_readout( drm, buffer, nwords, module->NRocs(), 0 );

  if( retval >= 0 ) { // Successful decoding:

    int hits = drm->roc[roc->GetChipId()].numPixelHits;

    cout << "[ADC1] decode: " << hits << " hits" << endl;

    for( int ipx = 0; ipx < hits; ipx++ ) {

      int col = drm->roc[roc->GetChipId()].pixelHit[ipx].columnROC;
      int row = drm->roc[roc->GetChipId()].pixelHit[ipx].rowROC;
      int adc = drm->roc[roc->GetChipId()].pixelHit[ipx].analogPulseHeight;

      hitmap->Fill( col, row );
      adcmap->Fill( col, row, adc );
      adchst->Fill( adc );

      // Gain:

      if( haveGain ) {

	double Aout = adc;

	double Ared = Aout - vert[col][row];//sub vert offset

	double ma9 = amax[col][row];

	if( Ared >  ma9-1 ) ma9 = 1.000001* Ared;
	if( Ared <  -ma9+1 ) ma9 = Ared - 1;

	// inverse Weibull gives large Vcal:

	double vcal4 = pow( -log( 1 - Ared / ma9 ), 1/expo[col][row] )
	  * Gain[col][row] + horz[col][row];

	map4->Fill( col, row, vcal4 );
	his4->Fill( vcal4 );

	//cout << ", Vcal " << vcal4;

      } // gain
    }
  }
  else {
    // Decoding failed. Try next block of data:
    cout << "[ADC]: digital decoder error" << endl;
  }

  // Cleanup:

  delete drm;
  delete buffer;

  if( haveGain ) {
    histograms->Add( his4 );
    histograms->Add( map4 );
  }
  histograms->Add( hitmap );
  histograms->Add( adcmap );
  histograms->Add( adchst );

  RestoreDacParameters();
}
