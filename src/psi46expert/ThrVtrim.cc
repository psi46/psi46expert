
// Daniel Pitzl, DESY, May 2013
// pixel threshold vs Vtrim and trim bits

#include "ThrVtrim.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"

//------------------------------------------------------------------------------
ThrVtrim::ThrVtrim( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void ThrVtrim::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).ThresholdNTrig;
  cout << "ThrVtrim nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void ThrVtrim::RocAction()
{
  SaveDacParameters();
  Test::RocAction();
  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void ThrVtrim::PixelAction()
{
  cout << "chipId = " << chipId
       << ", col = " << column
       << ", row = " << row
       << endl;

  int strt = 40;
  int step = 1;
  int tdac = 25; // 25 = Vcal
  int xtlk = 0;
  int cals = 0;
  int trim = pixel->GetTrim();

  cout << "trimmed to " << trim << endl;

  // Set local trigger and channel:
  /*
  if( ! tbInterface->IsAnalog() ) {
    tbInterface->SetReg(41, 0x20 | 0x01);
    tbInterface->DataCtrl( false, false, true );
  }
  */
  // loop over Vtrim

  int originalVtrim = GetDAC( "Vtrim" );


  TProfile * profile15 =
    new TProfile( Form( "Thr_vs_Vtrim_bit15_c%02i_r%02i", column, row ),
		  Form( "Thr vs Vtrim bit 15 col %i row %i", column, row ),
		  256, -0.5, 255.5 );

  profile15->GetXaxis()->SetTitle( "Vtrim [DAC units]" );
  profile15->GetYaxis()->SetTitle( "threshold [Vcal DAC units]" );
  profile15->SetMinimum(0);

  TProfile * profile7 =
    new TProfile( Form( "Thr_vs_Vtrim_bit7_c%02i_r%02i", column, row ),
		  Form( "Thr vs Vtrim bit 7 col %i row %i", column, row ),
		  256, -0.5, 255.5 );

  profile7->GetXaxis()->SetTitle( "Vtrim [DAC units]" );
  profile7->GetYaxis()->SetTitle( "threshold [Vcal DAC units]" );
  profile7->SetMinimum(0);

  TProfile * profile0 =
    new TProfile( Form( "Thr_vs_Vtrim_bit0_c%02i_r%02i", column, row ),
		  Form( "Thr vs Vtrim bit 0 col %i row %i", column, row ),
		  256, -0.5, 255.5 );

  profile0->GetXaxis()->SetTitle( "Vtrim [DAC units]" );
  profile0->GetYaxis()->SetTitle( "threshold [Vcal DAC units]" );
  profile0->SetMinimum(0);

  //ArmPixel(); // arm = enable + cal = settrim + cal
  Cal();

  for( int idac = 0; idac < 256; idac++ ) {

    SetDAC( "Vtrim", idac );

    //Flush();
    // PixelThreshold does enable = trim and flush

    // trim bit 15 = off

    int thr15 = tbInterface->PixelThreshold( column, row, strt, step, nTrig, 2*nTrig,
				    tdac, xtlk, cals, 15 );
    // PixelThreshold ends with disable = mask

    profile15->Fill( idac, thr15 );

    // trim bit 7 = mid
    strt = thr15;
    int thr7 = tbInterface->PixelThreshold( column, row, strt, step, nTrig, 2*nTrig,
				   tdac, xtlk, cals, 7 );

    if( thr7 < 255 ) {
      profile7->Fill( idac, thr7 );
      strt = thr7;
    }

    // trim bit 0 = strongest

    int thr0 = tbInterface->PixelThreshold( column, row, strt, step, nTrig, 2*nTrig,
				   tdac, xtlk, cals, 0 );

    if( thr0 < 255 ) profile0->Fill( idac, thr0 );
    strt = thr15; // for next round

    cout << idac << ": " << thr15
	 << "  " << thr7
	 << "  " << thr0
	 << endl;

  } // idac loop

  histograms->Add(profile0);
  histograms->Add(profile7);
  histograms->Add(profile15);

  SetDAC( "vtrim", originalVtrim );

  DisarmPixel(); // Disarm = Disable + ClrCal
  Flush();

}
