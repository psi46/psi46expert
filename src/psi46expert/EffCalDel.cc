//
// Daniel Pitzl, DESY, April 2013
// Chip efficiency vs CalDel
//
#include "interface/Log.h"
#include <iomanip>

#include "EffCalDel.h"
#include "TestRoc.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBInterface.h"

//------------------------------------------------------------------------------
EffCalDel::EffCalDel( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters( testParameters );
}

//------------------------------------------------------------------------------
void EffCalDel::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).PixelMapReadouts;
  //efficiency = (double)(*testParameters).PixelMapEfficiency / 100. ;
}

//------------------------------------------------------------------------------
void EffCalDel::RocAction()
{
  TH1D *halv = new TH1D( "ali", "chip efficiency vs CalDel;CalDel [DAC];pixels alive", 128, -0.5, 255.5 );

  TH1D *hper = new TH1D( "per", "perfect pixels vs CalDel;CalDel [DAC];perfect pixels", 128, -0.5, 255.5 );

  SaveDacParameters();  

  EnableAllPixels();
  Flush();
  gDelay->Mdelay(20);

  cout << "CtrlReg " << GetDAC( "CtrlReg" ) << endl;
  cout << "Vcal    " << GetDAC( "Vcal" ) << endl;

  int nmax = 0;
  int imax = 256;
  int n0 = 0;

  double data[ROC_NUMROWS*ROC_NUMCOLS]; // 4160

  for( int idel = 0; idel < 256; idel += 2 ) {

    SetDAC( "CalDel", idel );
    gDelay->Mdelay(20);

    roc->ChipEfficiency( nTrig, data );

    int nAlive = 0;
    int nPerfect = 0;

    for( int l = 0; l < ROC_NUMROWS*ROC_NUMCOLS; l++) {

      if( data[l] > 0.500 ) nAlive++;
      if( data[l] > 0.999 ) nPerfect++;
    }

    halv->Fill( idel, nAlive );
    hper->Fill( idel, nPerfect );

    if( nAlive > nmax ) {
      nmax = nAlive;
      imax = idel;
    }

    if( nAlive > 0 ) n0 = 0;
    else n0++;

    cout << "CalDel " << setw(3) << GetDAC( "CalDel" );
    cout << ", alive " << setw(4) << nAlive << " pixels";
    cout << ", perfect " << setw(4) << nPerfect << " pixels";
    cout << endl;

    if( nmax > 999 && n0 > 4 ) break; // save time

  } // loop CalDel

  histograms->Add(halv);
  histograms->Add(hper);

  cout << "best: " << nmax << " pixel alive" << endl;
  cout << "from CalDel " << imax << endl;

  tbInterface->SetEnableAll( 0 );

  RestoreDacParameters();
  cout << "done" << endl;
}
