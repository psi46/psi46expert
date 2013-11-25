
// SCurve plot, one pixel

#include "SCplot.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>

//------------------------------------------------------------------------------
SCplot::SCplot( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void SCplot::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).SCurveNTrig;
  cout << "SCplot nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void SCplot::RocAction()
{
  SaveDacParameters();	
  Test::RocAction();	 
  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void SCplot::PixelAction()
{
  bool dbg = 0;

  int col = pixel->GetColumn();
  int row = pixel->GetRow();
  int chp = roc->GetChipId();
  int itrm = pixel->GetTrim();

  int idac = 25; // 25 = Vcal

  TH1D *histo = 
    new TH1D(
	     Form( "SCc%02dr%02d_C%d", col, row, chp ),
	     Form( "S-Curve col%02d row %02d ROC %d;Vcal [DAC];valid readouts",
		   col, row, chp ),
	     256, -0.5, 255.5 );

  // analog ROC:

  if( tbInterface->IsAnalog() ) return;

  cout << "  col " << pixel->GetColumn()
       << ", row " << pixel->GetRow();
  if( dbg ) cout << endl;

  //ArmPixel(); // arm = enable + cal
  Cal();
  Flush();

  // coarse threshold scan += 16*step around start:

  //int strt = 128; // mid
  //int step = 8; // full range

  int strt = 60; // good guess
  int step = 1; // fine

  //int ithr = roc->PixelThreshold( col, row, strt, step, nTrig, 2*nTrig, idac, false, false, itrm ); // contains SetChip

  int ithr = tbInterface->PixelThreshold( col, row, strt, step, nTrig, 2*nTrig, idac, false, false, itrm );

  if( dbg ) cout << ": PixelThreshold coarse " << ithr << endl;

  // PixelThreshold ends with disable:

  ArmPixel(); // arm = enable + cal
  Flush();

  // SCurve:

  int sCurve[256];

  tbInterface->SCurve( nTrig, idac, ithr, sCurve );

  for( int ii = 0; ii < 256; ++ii ){
    histo->Fill( ii, sCurve[ii] );
    if( dbg ) cout << " " << sCurve[ii];
  }
  if( dbg ) cout << endl;

  double thr = roc->Threshold( sCurve, 0, 1, 1, nTrig/2 ); // start, sign, step

  cout << ", thr " << thr << endl;

  histograms->Add(histo);

  // Cleanup:

  DisarmPixel(); // Disarm = Disable + ClrCal
  Flush();

}
