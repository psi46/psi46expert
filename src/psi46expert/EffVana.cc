//
// Daniel Pitzl, DESY, April 2013
// Chip efficiency vs Vana
//
#include "interface/Log.h"
#include <iomanip>

#include "EffVana.h"
#include "TestRoc.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBInterface.h"

//------------------------------------------------------------------------------
EffVana::EffVana( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters( testParameters );
}

//------------------------------------------------------------------------------
void EffVana::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).PixelMapReadouts;
  //efficiency = (double)(*testParameters).PixelMapEfficiency / 100. ;
}

//------------------------------------------------------------------------------
void EffVana::RocAction()
{
  TH1D *halv = new TH1D( "effvsvana", "chip efficiency vs Vana;Vana [DAC];pixels alive", 128, -0.5, 255.5 );

  TH1D *hper = new TH1D( "perfectvsvana", "perfect pixels vs Vana;Vana [DAC];perfect pixels", 128, -0.5, 255.5 );

  TH1D *hia = new TH1D( "iavsvana", "Ia vs Vana;Vana [DAC];Ia [mA]", 128, -0.5, 255.5 );

  SaveDacParameters();  

  EnableAllPixels();
  Flush();
  gDelay->Mdelay(20);

  cout << "CtrlReg " << GetDAC( "CtrlReg" ) << endl;
  cout << "CalDel  " << GetDAC( "CalDel" ) << endl;

  int nmax = 0;
  int imax = 256;
  int n0 = 0;

  double data[ROC_NUMROWS*ROC_NUMCOLS]; // 4160

  for( int iv = 0; iv < 256; iv += 2 ) {

    SetDAC( "Vana", iv );
    gDelay->Mdelay(20);

    roc->ChipEfficiency( nTrig, data );

    double ia = tbInterface->GetIA();

    int nAlive = 0;
    int nPerfect = 0;

    for( int l = 0; l < ROC_NUMROWS*ROC_NUMCOLS; l++) {

      if( data[l] > 0.500 ) nAlive++;
      if( data[l] > 0.999 ) nPerfect++;
    }

    halv->Fill( iv, nAlive );
    hper->Fill( iv, nPerfect );
    hia->Fill( iv, ia*1E3 );

    if( nAlive > nmax ) {
      nmax = nAlive;
      imax = iv;
    }

    if( nAlive > 0 ) n0 = 0;
    else n0++;

    cout << "Vana " << setw(3) << GetDAC( "Vana" );
    cout << ", Ia " << ia*1E3 << " mA";
    cout << ", alive " << setw(4) << nAlive << " pixels";
    cout << ", perfect " << setw(4) << nPerfect << " pixels";
    cout << endl;

    if( nmax > 999 && n0 > 4 ) break; // save time

  } // loop Vana

  histograms->Add(halv);
  histograms->Add(hper);
  histograms->Add(hia);

  cout << "best: " << nmax << " pixel alive" << endl;
  cout << "from Vana " << imax << endl;

  tbInterface->SetEnableAll( 0 );

  RestoreDacParameters();
  cout << "done" << endl;
}
