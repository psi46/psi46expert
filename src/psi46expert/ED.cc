
// Daniel Pitzl, DESY, Sep 2012
// ROC noise map

#include "ED.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/DigitalReadoutDecoder.h"

#include <iomanip>

using namespace RawPacketDecoderConstants;
using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;

//------------------------------------------------------------------------------
ED::ED( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void ED::ReadTestParameters( TestParameters *testParameters )
{
}

//------------------------------------------------------------------------------
void ED::RocAction()
{
  SaveDacParameters();	

  int vthr = GetDAC( "VthrComp" );
  int vtrm = GetDAC( "Vtrim" );

  cout << "VthrComp " << vthr << ", Vtrim " << vtrm << endl;

  map = new TH2D( Form( "ED_C%d", chipId ),
		  Form( "ED_C%d", chipId ),
		  ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
		  ROCNUMROWS, -0.5, ROCNUMROWS-0.5 );

  map->GetXaxis()->SetTitle( "col" );
  map->GetYaxis()->SetTitle( "row" );
  map->GetZaxis()->SetTitle( "pulse height [ADC]" );

  // Show the whole range of the ADC data in the histogram:

  if( ! tbInterface->IsAnalog() ) {
    map->SetMinimum(0);
    map->SetMaximum(256);
  }

  // Buffer for the ADC data:
  short * buffer = new short[16000]; // 26 DC * 80 buf * 6 words = 12480 words
  unsigned short nwords;

  // Set local trigger and channel:
  //tbInterface->SetReg( 41, 0x20 | 0x01 );
  //tbInterface->DataCtrl( false, false, true );

  tbInterface->DataCtrl( false, true ); // no clear, trigger

  //roc->EnableAllPixels();

  // one event:

  tbInterface->Single(RES|CAL|TRG|TOK);
  tbInterface->CDelay(500);
  tbInterface->Flush();

  // Read the data from the FIFO on the testboard:

  tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, 16000, nwords );

  // decode:

  if( tbInterface->IsAnalog() ) {

    RawPacketDecoder *gDecoder = RawPacketDecoder::Singleton();

    int count = nwords;

    int empty = 10;
    if( tbInterface->TBMPresent() ) empty = 19;

    cout << "count " << setw(4) << count << endl;
    for( unsigned int n = 0; n < count; n++ ) {
      cout << setw(6) << buffer[n];
      if( n ==  7 ) cout << " : "; // adc header
      if( n == 10 ) cout << " : "; // UB, B, lastDAC
      if( n > 15 && n < count - 7 && (n-11)%6 == 5 ) cout << " : "; // after each pixel
    }
    if( count > 0 ) cout << endl;

    int npix = (count - empty)/6;

    cout << npix << " pixels" << endl;

    // adca : UB B lD : C1 C0 A2 A1 A0 outt : adcx adcy -750 -750  0  0
    //    0   1  2  3    4  5  6  7  8  9 

    int iLD = 3;//last DAC
    if( tbInterface->TBMPresent() ) iLD = 10;

    for( int ipix = 0; ipix < npix; ++ipix ){

      int kk = iLD+1 + 6*ipix;

      //--- decode row and column addresses:
    
      ADCword rawADC[6];
      rawADC[0] = buffer[kk+0]; // high valued part of column address
      rawADC[1] = buffer[kk+1]; // low -""-
      rawADC[2] = buffer[kk+2]; // high valued part of row address
      rawADC[3] = buffer[kk+3]; // medium -""-
      rawADC[4] = buffer[kk+4]; // low -""-
      rawADC[5] = buffer[kk+5]; // analog pulse height
    
      int columnROC, rowROC, rawColumn, rawPixel;
      int rocId = 0;
      // col and row start at 1
      int errorFlag = gDecoder->decodeROCaddress( rocId, rawADC, columnROC, rowROC, rawColumn, rawPixel );

      if( errorFlag ) {
	cout << "hit " << setw(4) << ipix+1 << endl;
	cout << ": errorFlag " << errorFlag;
	cout << ", C1 " << rawADC[0];
	cout << ", C0 " << rawADC[1];
	cout << ", rawCol " << rawColumn;
	cout << ", col " << columnROC;
	cout << ", A2 " << rawADC[2];
	cout << ", A1 " << rawADC[3];
	cout << ", A0 " << rawADC[4];
	cout << ", rawRow " << rawPixel;
	cout << ", row " << rowROC;
	cout << endl;
      }
      else { // OK
	cout << "hit " << setw(4) << ipix+1;
	cout << ": col " << setw(2) << columnROC-1;
	cout << ", row " << setw(2) << rowROC-1;
	cout << ", PH " << setw(4) << rawADC[5];
	cout << endl;
	map->Fill( columnROC-1, rowROC-1, rawADC[5] );
      }

    }//ipix

      /*
	cout << "col " << pixel->GetColumn();
	cout << ", row " << pixel->GetRow();
	cout << ", N " << ph_mean;
	cout << endl;
      */

  }//analog

  else { // digital ROC:

    cout << nwords << " data words" << endl;

    DecodedReadoutModule * drm = new DecodedReadoutModule;

    int data_pos = 0;

    int retval = decode_digital_readout( drm, buffer + data_pos,
					 nwords, 1, 0 );

    if( retval >= 0 ) { // Successful decoding:

      int nhits = drm->roc[0].numPixelHits;
      cout << nhits << " pixel hits" << endl;

      for( int ii = 0; ii < nhits; ++ii ) {
	/* Record the pulse height and move to the next block of data */
	int ph = drm->roc[0].pixelHit[ii].analogPulseHeight;
	int col = drm->roc[0].pixelHit[ii].columnROC;
	int row = drm->roc[0].pixelHit[ii].rowROC;
	cout << "hit " << setw(4) << ii+1;
	cout << ": col " << setw(2) << col;
	cout << ", row " << setw(2) << row;
	cout << ", PH " << setw(3) << ph;
	cout << endl;
	map->Fill( col, row, ph );
      }
    }
    else {
      cout << "digital decoder error" << endl;
    }

    delete drm;

  } // digital

  //roc->Mask();

  delete buffer;

  histograms->Add( map );

  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void ED::PixelAction()
{

}//pixel
