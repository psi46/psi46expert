
// Daniel Pitzl, DESY, Aug 2012

#include "PHmap.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>
#include <cmath>

//------------------------------------------------------------------------------
PHmap::PHmap( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void PHmap::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).PHNTrig;
  cout << "PHmap nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void PHmap::RocAction()
{
  if( tbInterface->IsAnalog() ) {
    cout << "not implemented for analog ROC" << endl;
    return;
  } // analog

  bool atb4 = 0; // atb4: compact data = 12 valid bits per word

  SaveDacParameters();	

  int vcal = GetDAC( "Vcal" );
  int creg = GetDAC( "CtrlReg" );

  cout << "Vcal is " << vcal << " at CtrlReg " << creg << endl;

  map = new TH2D( Form( "PHmap_CR%i_Vcal%i_C%d", creg, vcal, chipId ),
		  Form( "PHmap_C%d for Vcal %d, CtrlReg %d", chipId, vcal, creg ),
		  ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
		  ROCNUMROWS, -0.5, ROCNUMROWS-0.5 );

  map->GetXaxis()->SetTitle( "col" );
  map->GetYaxis()->SetTitle( "row" );
  map->GetZaxis()->SetTitle( "Pulse height [ADC]" );

  // Show the whole range of the ADC data in the histogram:

  map->SetMinimum(0);
  map->SetMaximum(256);

  his = new TH1D( Form( "PH_CR%i_Vcal%i_C%d", creg, vcal, chipId ),
		  Form( "PH_C%d for Vcal %d, CtrlReg %d;PH [ADC];pixels",
			chipId, vcal, creg ),
		  256, -0.5, 255.5 );

  // Buffer for the ADC data:
  short * buffer = new short [256];
  unsigned short nwords;

  // Structure that holds the decoded readout:
  DecodedReadoutModule * drm = new DecodedReadoutModule;

  // Set local trigger and channel:
  tbInterface->SetReg( 41, 0x20 | 0x01 );
  tbInterface->DataCtrl( false, false, true );

  // loop over pixels

  for( int i = 0; i < ROCNUMCOLS; i++ ) {

    cout << "col " << setw(2) << i << endl;

    for( int k = 0; k < ROCNUMROWS; k++ ) {

      roc->EnablePixel( i, k );
      roc->ArmPixel( i, k );

      // Send nTrig calibrates to the chip:

      for( int j = 0; j < nTrig; j++ ) {
	tbInterface->Single(RES|CAL|TRG|TOK);
	tbInterface->CDelay(400); // 400 BC = 10 us
      }
      tbInterface->Flush();

      // Read the data from the FIFO on the testboard:

      tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, 256, nwords );

      /*
      cout << "  col " << setw(2) << i
	   << "  row " << setw(2) << k
	   << ": DataRead " << setw(4) << nwords << " words"
	   << endl;
      */
      // decode:
      // digital ROC:

      float ph_mean = 0.0;
      int measurement_num = 0;
      int data_pos = 0;

      for( int trig = 0; trig < nTrig; trig++ ) {

	int retval = 
	  decode_digital_readout( drm, buffer + data_pos,
				  nwords, module->NRocs(), 0 );

	if( retval >= 0 ) { // Successful decoding:

	  int hits = drm->roc[roc->GetChipId()].numPixelHits;

	  if( hits == 1 ) {

	    int ph = drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;
	    ph_mean += ph;
	    measurement_num++;

	    if( atb4 )
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 2; // atb4
	    else
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
	  }
	  else if( hits > 1 ) {

	    // More hits than expected. Move to the next block of data:

	    if( atb4 )
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 2; // atb4
	    else
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
	    cout << "|" << hits << "|";
	  }
	  else {
	    // No hits, move to the next block of data:
	    data_pos += tbInterface->GetEmptyReadoutLengthADC();
	  }
	}
	else {
	  // Decoding failed. Try next block of data:
	  cout << "digital decoder error " << retval
	       << " at trig " << trig << endl;
	  data_pos += tbInterface->GetEmptyReadoutLengthADC();
	}

      } // triggers

	// average:
      if( measurement_num > 0 )
	ph_mean /= measurement_num;

      map->SetBinContent( i+1, k+1, ph_mean );
      his->Fill( ph_mean ); // PH distribution

      /*
	cout << "col " << pixel->GetColumn();
	cout << ", row " << pixel->GetRow();
	cout << ", PH " << ph_mean;
	cout << endl;
      */

      // Cleanup:

      //DP roc->DisablePixel( i, k );
      roc->DisarmPixel( i, k ); // Disarm = Disable + ClrCal

    }
  }

  delete drm;
  delete buffer;

  //Test::RocAction();

  histograms->Add( map );
  histograms->Add( his );

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

    map4 = new TH2D( Form( "VcalmapCR%iVcal%i_C%d", creg, vcal, chipId ),
		     Form( "Vcalmap_C%d for Vcal %d, CtrlReg %d", chipId, vcal, creg ),
		    ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
		    ROCNUMROWS, -0.5, ROCNUMROWS-0.5 );

    map4->GetXaxis()->SetTitle( "col" );
    map4->GetYaxis()->SetTitle( "row" );
    map4->GetZaxis()->SetTitle( "Pulse height [large Vcal DAC]" );

    // Show the whole Vcal range:

    map4->SetMinimum(0);
    map4->SetMaximum(256);

    his4 = new TH1D( Form( "Vcal_CR%i_Vcal%i_C%d", creg, vcal, chipId ),
		     Form( "Vcal_C%d for Vcal %d, CtrlReg %d;PH [large Vcal DAC];pixels",
			   chipId, vcal, creg ),
		     256, -0.5, 255.5 );

  } // gainFile

  // analyze map:

  int nover = 0;
  int n0 = 0;
  int phmax = 0;
  int colmax = -1;
  int rowmax = -1;
  int phmin = 256;
  int colmin = -1;
  int rowmin = -1;

  for( int col = 0; col < ROCNUMCOLS; col++ ) {	
    for( int row = 0; row < ROCNUMROWS; row++ ) {

      double ph = map->GetBinContent( col+1, row+1 );

      if( ph > phmax ) {
	phmax = ph;
	colmax = col;
	rowmax = row;
      }

      if( ph > 0.5 && ph < phmin ) {
	phmin = ph;
	colmin = col;
	rowmin = row;
      }

      if( ph > 254.5 ) {
	cout << "col " << col << ", row " << row << ", PH " << ph << endl;
	nover++;
      }
      else if( ph < 0.5 ) {
	cout << "col " << col << ", row " << row << ", PH " << ph << endl;
	n0++;
      }

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

	map4->SetBinContent( col+1, row+1, vcal4 );
	his4->Fill( vcal4 );

      } // gain

    } // row
  } // col

  if( haveGain ) {
    histograms->Add( map4 );
    histograms->Add( his4 );
  }

  cout << "total overlflows: " << nover << endl;
  cout << "total dead pixels " << n0 << endl;
  cout << "max pulse height " << phmax
       << " at col " << colmax
       << ", row " << rowmax
       << endl;
  cout << "min pulse height " << phmin
       << " at col " << colmin
       << ", row " << rowmin
       << endl;

  RestoreDacParameters();
}
