
// Daniel Pitzl, DESY, May 2013
// PH map cc 2 - cc 1

#include "PHmapcc.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>

//------------------------------------------------------------------------------
PHmapcc::PHmapcc( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void PHmapcc::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).PHNTrig;
  cout << "PHmapcc nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void PHmapcc::RocAction()
{

  if( tbInterface->IsAnalog() ) {
    cout << "not implemented for analog ROC" << endl;
    return;
  }

  bool atb4 = 1; // atb4: compact data = 12 valid bits per word

  SaveDacParameters();	

  int vcal = GetDAC( "Vcal" );
  int creg = GetDAC( "CtrlReg" );

  cout << "Vcal is " << vcal << " at CtrlReg " << creg << endl;

  cout << "tcc  " << tbInterface->GetParameter( "tcc" ) << " BC" << endl;
  cout << "tct  " << tbInterface->GetParameter( "tct" ) << " BC" << endl;

  int oldcc = tbInterface->GetParameter( "cc" );

  TProfile2D * map1 =
    new TProfile2D( Form( "PHmap_cc1_Vcal%i_CR%d", vcal, creg ),
		    Form( "PHmap cc1 for Vcal %d, CtrlReg %d;col;row;<PH> [ADC]",
			  vcal, creg ),
		    ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
		    ROCNUMROWS, -0.5, ROCNUMROWS-0.5,
		    -0.5, 255.5 );
  map1->SetMinimum(0);
  map1->SetMaximum(256);

  TProfile2D * map2 =
    new TProfile2D( Form( "PHmap_cc2_Vcal%i_CR%d", vcal, creg ),
		    Form( "PHmap cc2 for Vcal %d, CtrlReg %d;col;row;<PH> [ADC]",
			  vcal, creg ),
		    ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
		    ROCNUMROWS, -0.5, ROCNUMROWS-0.5,
		    -0.5, 255.5 );
  map2->SetMinimum(0);
  map2->SetMaximum(256);

  TH1D * his1 =
    new TH1D( Form( "PH_cc1_Vcal%i_CR%d", vcal, creg ),
	      Form( "PH cc 1 for Vcal %d, CtrlReg %d;PH [ADC];pixels",
		    vcal, creg ),
	      256, -0.5, 255.5 );

  TH1D * his2 =
    new TH1D( Form( "PH_cc2_Vcal%i_CR%d", vcal, creg ),
	      Form( "PH cc 2 for Vcal %d, CtrlReg %d;PH [ADC];pixels",
		    vcal, creg ),
	      256, -0.5, 255.5 );

  //----------------------------------------------------------------------------

  // Buffer for the ADC data:

  short * buffer = new short[FIFOSIZE];
  unsigned short nwords;

  // Structure that holds the decoded readout:

  DecodedReadoutModule * drm = new DecodedReadoutModule;

  // Set local trigger and channel:
  // tbInterface->SetReg( 41, 0x20 | 0x01 );
  tbInterface->DataCtrl( false, false, true );

  // loop over pixels

  for( int i = 0; i < ROCNUMCOLS; i++ ) {

    cout << "col " << setw(2) << i << endl;

    for( int k = 0; k < ROCNUMROWS; k++ ) {

      roc->EnablePixel( i, k );
      roc->ArmPixel( i, k );
      tbInterface->CDelay(400); // 400 BC = 10 us

      for( int icc = 1; icc <= 2; ++icc ) {

	tbInterface->SetTBParameter( "cc", icc );
	Flush();
	//cout << "cc: " << icc << " calibrates" << endl;

	// Send nTrig calibrates to the chip:

	for( int j = 0; j < nTrig; j++ ) {
	  tbInterface->Single(RES|CAL|TRG|TOK);
	  tbInterface->CDelay(400);
	}
	tbInterface->Flush(); // USB sends all commands to FPGA 

	// Read the data from the FIFO on the testboard:

	tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, FIFOSIZE, nwords );

      // decode digital ROC:

	int data_pos = 0;

	for( int trig = 0; trig < nTrig; trig++ ) {

	  int retval = 
	    decode_digital_readout( drm, buffer + data_pos,
				    nwords, module->NRocs(), 0 );

	  if( retval >= 0 ) { // Successful decoding:

	    int hits = drm->roc[roc->GetChipId()].numPixelHits;

	    if( hits == 1 ) {

	      int ph = drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;

	      if( icc == 1 ) {
		map1->Fill( i, k, ph );
		his1->Fill( ph );
	      }
	      else{
		map2->Fill( i, k, ph );
		his2->Fill( ph );
	      }

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
	    cout << "digital decoder error trig " << trig << endl;
	    data_pos += tbInterface->GetEmptyReadoutLengthADC();
	  }

	} // triggers

      } // icc

      // Cleanup:

      //DP roc->DisablePixel( i, k );
      roc->DisarmPixel( i, k ); // Disarm = Disable + ClrCal

    }
  }

  // analyze map:

  TProfile2D * mapd =
    new TProfile2D( Form( "dPHmap_Vcal%i_CR%d", vcal, creg ),
		    Form( "PH diff map for Vcal %d, CtrlReg %d;col;row;<PH> [ADC]",
			  vcal, creg ),
		    ROCNUMCOLS, -0.5, ROCNUMCOLS-0.5,
		    ROCNUMROWS, -0.5, ROCNUMROWS-0.5,
		    -1000, 1000 );

  TH1D * hisd =
    new TH1D( Form( "dPH_Vcal%i_CR%d", vcal, creg ),
	      Form( "PH diff for Vcal %d, CtrlReg %d;PH difference [ADC];pixels",
		    vcal, creg ),
	      40, -20, 20 );

  for( int col = 0; col < ROCNUMCOLS; col++ ) {	
    for( int row = 0; row < ROCNUMROWS; row++ ) {

      double ph1 = map1->GetBinContent( col+1, row+1 );
      double ph2 = map2->GetBinContent( col+1, row+1 );
      double dph = ph2-ph1;
      mapd->Fill( col, row, dph );
      hisd->Fill( dph );

    } // row
  } // col

  delete drm;
  delete buffer;

  histograms->Add( map1 );
  histograms->Add( map2 );
  histograms->Add( mapd );
  histograms->Add( his1 );
  histograms->Add( his2 );
  histograms->Add( hisd );

  tbInterface->SetTBParameter( "cc", oldcc );
  Flush();
  cout << "cc   " << oldcc << " calibrates" << endl;

  RestoreDacParameters();
}
