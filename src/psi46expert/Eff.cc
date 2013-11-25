
// Daniel Pitzl, DESY, Sep 2012
// Vcal efficiency, all pix enabled

#include "Eff.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>

//------------------------------------------------------------------------------
Eff::Eff( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void Eff::ReadTestParameters(TestParameters *testParameters)
{
  nTrig = (*testParameters).PHNTrig;
  cout << "Eff nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void Eff::RocAction()
{
  SaveDacParameters();	

  roc->EnableAllPixels();

  Test::RocAction();	 

  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void Eff::PixelAction()
{
  bool ldb = 1;
  bool ldb2 = 0;

  int Tcol = pixel->GetColumn();
  int Trow = pixel->GetRow();

  cout << "Target col " << setw(2) << Tcol;
  cout << ", row " << setw(2) << Trow;
  cout << endl;

  TH1D *hnh =
    new TH1D( Form( "hitmult_c%02dr%02d_C%d", Tcol, Trow, roc->GetChipId() ),
	      Form( "hitmult_c%02dr%02d_C%d", Tcol, Trow, roc->GetChipId() ),
	      101, -0.5, 100.5 );
  hnh->GetXaxis()->SetTitle( "pixels fired" );
  hnh->GetYaxis()->SetTitle( "events" );

  TProfile *pnh =
    new TProfile( Form( "multvsCal_c%02dr%02d_C%d", Tcol, Trow, roc->GetChipId() ),
		  Form( "multvsCal_c%02dr%02d_C%d", Tcol, Trow, roc->GetChipId() ),
		  256, -0.5, 255.5 );
  pnh->GetXaxis()->SetTitle( "Vcal (DAC units)" );
  pnh->GetYaxis()->SetTitle( "<hits/trigger>" );

  TProfile *pph =
    new TProfile( Form( "PHvsCal_c%02dr%02d_C%d", Tcol, Trow, roc->GetChipId() ),
		  Form( "PHvsCal_c%02dr%02d_C%d", Tcol, Trow, roc->GetChipId() ),
		  256, -0.5, 255.5 );
  pph->GetXaxis()->SetTitle( "Vcal (DAC units)" );
  pph->GetYaxis()->SetTitle( "<PH> [ADC counts]" );

  TProfile *peff =
    new TProfile( Form( "EffVcal_c%02dr%02d_C%d", Tcol, Trow, roc->GetChipId() ),
		  Form( "EffVcal_c%02dr%02d_C%d", Tcol, Trow, roc->GetChipId() ),
		  256, -0.5, 255.5 );
  peff->GetXaxis()->SetTitle( "Vcal (DAC units)" );
  peff->GetYaxis()->SetTitle( "test pixel efficiency" );

  // analog ROC:

  if( tbInterface->IsAnalog() ) {

    TH1D * ubHist = new TH1D("ubHist", "ubHist", 256, 0, 256);
    ubHist->SetLineColor(kRed);

    EnablePixel();
    Cal();
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

    tbInterface->PHDac( 25, 256, nTrig, offset + aoutChipPosition * 3, result );
    tbInterface->ADCData( data, count );

    int ubLevel = data[ubPosition];

    for( int dac = 0; dac < 256; dac++) {
      if( result[dac] == 7777)
	peff->SetBinContent(dac + 1, 0);
      else
	peff->SetBinContent(dac + 1, result[dac]);

      ubHist->SetBinContent(dac + 1, ubLevel);
    }

    roc->ClrCal();
    //DP DisablePixel();
    DisarmPixel(); // Disarm = Disable + ClrCal
    histograms->Add(peff);
    histograms->Add(ubHist);

  } // analog

  else { // digital ROC:

    /* Buffer for the ADC data */
    short * buffer = new short [4096];
    unsigned short nwords;

    /* Structure that holds the decoded readout */
    DecodedReadoutModule * drm = new DecodedReadoutModule;

    /* Set local trigger and channel */
    tbInterface->SetReg( 41, 0x20 | 0x01 );
    tbInterface->DataCtrl( false, false, true );

    EnablePixel();
    ArmPixel();

    // Loop through the whole Vcal range:

    for( int vcal = 1; vcal < 256; vcal += 1 ) {

      SetDAC( "Vcal", vcal );
      tbInterface->Flush();
      tbInterface->CDelay(400);
      tbInterface->CDelay(400);

      if( ldb ) cout << "Vcal" << setw(3) << vcal;

      // Send nTrig calibrates to the chip:

      for( int i = 0; i < nTrig; i++ ) {
	tbInterface->Single(RES|CAL|TRG|TOK);
	tbInterface->CDelay(400);
      }
      tbInterface->CDelay(400);
      tbInterface->Flush();

      // Read the data from the FIFO on the testboard:

      tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, 4096, nwords );

      if( ldb ) cout << ":  DataRead " << nwords << " words"
		     << endl;

      // decode:

      int data_pos = 0;
      int nfound = 0;

      for( int trig = 0; trig < nTrig; trig++ ) {

	if( ldb2 ) cout << "  trig " << trig;

	int retval = decode_digital_readout( drm, buffer + data_pos,
					     nwords, module->NRocs(), 0 );

	if( retval >= 0 ) {

	  /* Successful decoding */

	  int nhits = drm->roc[roc->GetChipId()].numPixelHits;

	  hnh->Fill( nhits );
	  pnh->Fill( vcal, nhits );

	  if( ldb2 ) cout << ": hits " << nhits << endl;

	  //data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 6;
	  data_pos += tbInterface->GetEmptyReadoutLengthADC() + nhits * 2; // atb4

	  for( int ii = 0; ii < nhits; ++ii ) {

	    int ph = drm->roc[0].pixelHit[ii].analogPulseHeight;
	    int col = drm->roc[0].pixelHit[ii].columnROC;
	    int row = drm->roc[0].pixelHit[ii].rowROC;

	    bool lfound = 0;
	    if( col == Tcol && row == Trow ) lfound = 1;

	    if( lfound ) nfound++;
	    if( lfound ) pph->Fill( vcal, ph );
	    peff->Fill( vcal, lfound );

	    if( ldb2 ) cout << "                     ";
	    if( ldb2 ) cout << "hit " << setw(4) << ii+1 << ":";
	    if( ldb2 && lfound ) cout << "*";
	    if( ldb2 ) cout << " col " << setw(2) << col;
	    if( ldb2 ) cout << ", row " << setw(2) << row;
	    if( ldb2 ) cout << ", PH " << setw(3) << ph;
	    if( ldb2 ) cout << endl;

	  }
	}
	else {
	  /* Decoding failed. Try next block of data. */
	  cout << ": error" << endl;
	  data_pos += tbInterface->GetEmptyReadoutLengthADC();
	}

      } // triggers

    } // Vcal loop

    /* Cleanup */

    DisarmPixel(); // Disarm = Disable + ClrCal

    EnablePixel(); // like qll others

    delete drm;
    delete buffer;

    histograms->Add(hnh);
    histograms->Add(pnh);
    histograms->Add(pph);
    histograms->Add(peff);

  } // digital
}
