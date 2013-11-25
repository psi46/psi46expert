
// Daniel Pitzl, DESY, May 2013
// Thr vs all DACs, one pixel

#include "ThrvsDACs.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"

//------------------------------------------------------------------------------
ThrvsDACs::ThrvsDACs( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void ThrvsDACs::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).ThresholdNTrig;
  cout << "ThrvsDACs nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void ThrvsDACs::RocAction()
{
  SaveDacParameters();
  Test::RocAction();
  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void ThrvsDACs::PixelAction()
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

  // Set local trigger and channel:

  if( ! tbInterface->IsAnalog() ) {
    tbInterface->SetReg(41, 0x20 | 0x01);
    tbInterface->DataCtrl( false, false, true );
  }

  // loop over DACs:

  for( int DacRegister = 1; DacRegister < 28; DacRegister++ ) {
  //for( int DacRegister = 12; DacRegister <= 12; DacRegister++ ) { // test

    // Exclude DACs that don't exist on the digital ROC psi46dig:

    if( ! tbInterface->IsAnalog() ) {
      switch( DacRegister ) {
      case 5: case 6: case 8: case 16: case 21: case 23: case 24: case 27: case 28:
	continue;
      }
    }

    // Get the name of the DAC:

    DACParameters * parameters = new DACParameters();
    char * dacName = parameters->GetName(DacRegister);
    delete parameters;
    cout << "scanning DAC #" << DacRegister
	 << " (" << dacName << ") ..."
	 << endl;

    // Set scan maximum:

    int scanMax;
    if( DacRegister == 1 ||
	DacRegister == 4 ||
	DacRegister == 6 ||
	DacRegister == 8 ||
	DacRegister == 14 )
      scanMax = 16; // 4-bit
    else
      scanMax = 256; // 8-bit

    // Scan the DAC:

    int defaultValue = GetDAC(DacRegister);

    TProfile * histo = 
      new TProfile( Form( "Thr_vs_DAC%02i_c%02i_r%02i",
			  DacRegister, column, row ),
		    Form( "Thr vs %s col %i row %i",
			  dacName, column, row ),
		    scanMax, -0.5, scanMax-0.5 );

    histo->GetXaxis()->SetTitle( Form( "%s [DAC units]", dacName ) );
    histo->GetYaxis()->SetTitle( "threshold [Vcal DAC units]" );
    histo->SetMinimum(0);
    histo->SetMaximum(256);

    //ArmPixel(); // arm = enable + cal = settrim + cal
    Cal();

    for( int idac = 0; idac < scanMax; idac++ ) {

      SetDAC( DacRegister, idac );
      //Flush();
      // PixelThreshold does enable = trim and flush
      int thr = tbInterface->PixelThreshold( column, row, strt, step, nTrig, 2*nTrig,
				    tdac, xtlk, cals, trim );
      // PixelThreshold ends with disable = mask

      histo->Fill( idac, thr );

      cout << idac << ": " << thr << endl;

    } // idac loop

    SetDAC( DacRegister, defaultValue );

    histograms->Add(histo);

  } // DACs

  DisarmPixel(); // Disarm = Disable + ClrCal
  Flush();

}
