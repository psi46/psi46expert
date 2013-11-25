
// Daniel Pitzl, DESY, May 2013

// user: arm i:j k:l

#include "ADCmap.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
//#include "BasePixel/psi46_tb.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>
#include <cmath>

//------------------------------------------------------------------------------
ADCmap::ADCmap( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters( testParameters );
}

//------------------------------------------------------------------------------
void ADCmap::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).PHNTrig;
  cout << "ADCmap nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void ADCmap::RocAction()
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

  TH1D * hnpx =
    new TH1D( Form( "npix_CR%i_Vcal%i_C%d", creg, vcal, chipId ),
	      Form( "hits ROC%d, CtrlReg %d, Vcal %d;hits per event;triggers",
		    chipId, creg, vcal ),
	      2201, -0.5, 2200.5 );

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
	      Form( "ADC ROC%d, CtrlReg %d, Vcal %d;PH [ADC];hits",
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
  // like in takeData:

  bool latb4 = 1; // atb4

  tbInterface->Intern(15); // internal trigger: rctk

  const unsigned int MemSize = 4*1024*1024;

  unsigned int dataBuffer_fpga1 =
    tbInterface->getCTestboard()->Daq_Init( MemSize );

  cout << "[ADCmap] Daq_Size: " << tbInterface->getCTestboard()->Daq_GetSize() << endl;

  cout << "[ADCmap] Daq_Init at: " << dataBuffer_fpga1 << endl;

  tbInterface->getCTestboard()->Daq_Enable();  
  tbInterface->Flush();

  cout << "[ADCmap] Daq_Pointer: " << tbInterface->getCTestboard()->Daq_GetPointer() << endl;

  //tbInterface->DataCtrl( false, false, true ); // start
  //tbInterface->Flush();
  //gDelay->Mdelay(50); // milli sec

  tbInterface->SetReg( 43, 3 ); // clear event timer, reset aquire
  tbInterface->Flush();

  cout << "[ADCmap] Start  Daq_Pointer: " << tbInterface->getCTestboard()->Daq_GetPointer() << endl;

  //----------------------------------------------------------------------------
  // start:

  int fReg41 = tbInterface->getCTestboard()->GetReg41();

  cout << "[ADCmap] Reg41: " << fReg41
       << " = 0x"  << hex << fReg41 << dec <<endl;

  fReg41 |= 0x8; // run
  tbInterface->SetReg( 41, fReg41 );
  cout << "[ADCmap] Reg41: " << fReg41
       << " = 0x"  << hex << fReg41 << dec <<endl;
  tbInterface->Flush();
  gDelay->Mdelay(50); // milli sec

  cout << "[ADCmap] Reg41  Daq_Pointer: " << tbInterface->getCTestboard()->Daq_GetPointer() << endl;

  // Send a reset to the chip:
  tbInterface->Single(RES);
  tbInterface->Flush();
  gDelay->Mdelay(10);

  // send triggers:

  for( int j = 0; j < nTrig; j++ ) {
    tbInterface->DataCtrl( false, true ); // no clear, trigger, no_continue
    tbInterface->CDelay(4000); // 4'000 BC = 100 us
    tbInterface->Single(RES|CAL|TRG|TOK);
    tbInterface->CDelay(40000); // 40'000 BC = 1 ms, leave time for readout...
    tbInterface->CDelay(40000); // 40'000 BC = 1 ms, leave time for readout...
  }
  tbInterface->Flush(); // USB: transfer all commands to FPGA
  tbInterface->CDelay(40000); // 40'000 BC = 1 ms, leave time for readout...
  tbInterface->CDelay(40000); // 40'000 BC = 1 ms
  tbInterface->Flush();

  cout << "[ADCmap] Single Daq_Pointer: " << tbInterface->getCTestboard()->Daq_GetPointer() << endl;

  //----------------------------------------------------------------------------
  // stop:

  gDelay->Mdelay(50); // wait for end of readout

  tbInterface->Single(RES);
  tbInterface->Flush();

  gDelay->Mdelay(80); // wait for end of readout

  fReg41 &= ~0x8; // stop
  tbInterface->SetReg( 41, fReg41 );
  tbInterface->Flush();
  gDelay->Mdelay(50); // milli sec

  tbInterface->getCTestboard()->Daq_Disable();
  //tbInterface->DataCtrl( false, false, false ); // stop
  tbInterface->Flush();
  gDelay->Mdelay(50); // milli sec

  cout << "[ADCmap] Stop   Daq_Pointer: " << tbInterface->getCTestboard()->Daq_GetPointer() << endl;

  //----------------------------------------------------------------------------
  // read:

  unsigned int filledMem1 =
    tbInterface->getCTestboard()->Daq_GetPointer() - dataBuffer_fpga1; // bytes

  cout << "[ADCmap] filledMem1 " << filledMem1 << endl;

  //cout << "DataClear " << tbInterface->DataClear() << endl; // FIFO clear
  //tbInterface->DataClear(); // FIFO clear
  tbInterface->Clear(); // usb.Clear()
  tbInterface->Flush();
  gDelay->Mdelay(50); // ms

  cout << "[ADCmap] Mem_ReadOut for " << filledMem1 << " bytes" << endl;

  unsigned char dbuffer[MemSize];

  unsigned int rest = filledMem1;
  int jj = 0;

  while( rest > 0 ) {

    unsigned short BLOCKSIZE = 50000;
    unsigned char cbuffer[BLOCKSIZE];
    unsigned short readBytes = BLOCKSIZE;
    if( rest < BLOCKSIZE ) readBytes = rest;

    tbInterface->getCTestboard()->MemRead( dataBuffer_fpga1, readBytes, cbuffer );

    for( int ii = 0; ii < readBytes; ++ii )
      dbuffer[jj++] = cbuffer[ii];

    rest -= readBytes;
  }

  tbInterface->getCTestboard()->Daq_Done();
  tbInterface->Flush();

  /* Reset the chip */
  tbInterface->Single(RES);
  tbInterface->Flush();

  //----------------------------------------------------------------------------
  // binary data, decode like in d2h.cc

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

  DecodedReadoutModule * drm = new DecodedReadoutModule;

  unsigned int ndata = 0;
  unsigned int ndataetrig = 0;
  unsigned int netrig = 0;
  unsigned int nitrig = 0;
  unsigned int nres = 0;
  unsigned int ncal = 0;
  unsigned int nwrong = 0;
  unsigned int ntbmres = 0;
  unsigned int ninfro = 0;
  unsigned int nover = 0;
  unsigned int ncor = 0;

  unsigned long timebefore = 0; // 64 bit

  char buf[600];
  short * rdd = new short[64003];

  int ii = 0;

  unsigned char a = dbuffer[ii++];
  unsigned char b = dbuffer[ii++];
  unsigned short word = (b << 8) | a;// word = 2 bytes

  while( ii < jj ) {

    int head[4] = {0};
    head[0] = word;

    a = dbuffer[ii++];
    b = dbuffer[ii++];
    word = (b << 8) | a; // read ahead

    // decode header:

    bool validHeader = 0;

    switch( head[0] ) {

    case 0x8001://data 8001
      ndata++;
      validHeader = 1;
      break;
    case 0x8002://ext. trig 8002
      netrig++;
      validHeader = 1;
      break;
    case 0x8004://int. trig 8004
      nitrig++;
      validHeader = 1;
      break;
    case 0x8008://reset 8008 
      nres++;
      validHeader = 1;
      break;
    case 0x8010://cal 8010 
      ncal++;
      validHeader = 1;
      break;
    case 0x8020://TBMreset 8020 
      ntbmres++;
      validHeader = 1;
      break;
    case 0x8040://infreadout 8040
      ninfro++;
      break;
    case 0x8080://data overflow 8080
      nover++;
      break;
    case 0x8081://data and what 8081 ?
      validHeader = 1;
      head[0] = 0x8001; // dirty hack
      break;
    case 0x8003://data and ext trig 8003
      ndataetrig++;
      validHeader = 1;
      break;
    default:
      ncor++;
    }//case head[0]

    if( validHeader ) {

      // read 3 more words after header (time stamp):

      int h = 1;

      for( ; h < 4 && ii < jj-2; ++h ){
	head[h] = word;
	a = dbuffer[ii++];
	b = dbuffer[ii++];
	word = (b << 8) | a; // word = 2 bytes
      }

      if( h != 4 ) cout << "ERROR: header length not 4: " << h << endl;

      sprintf( buf, "%4x %4x %4x %4x", head[0], head[1], head[2], head[3] );
      cout << buf;

      unsigned short t0 = head[1];
      unsigned short t1 = head[2];
      unsigned short t2 = head[3];
      unsigned int UpperTime = t0;
      unsigned int LowerTime = (t1 << 16) | t2;
      unsigned long time =
	( (unsigned long) UpperTime << 32 ) + LowerTime; // 64 bit

      cout << " (" << time << " clocks = "
	   << time/40E6 << " s)"
	   << "(+" << time-timebefore << " BC)";
      timebefore = time;

      if( head[0] != 0x8001 ) cout << endl;

    } // validHeader

    else {
      sprintf( buf, "? %4x", head[0] );
      cout << buf << endl;
    }

    // fill ROC data vector:

    if( head[0] == 0x8001 ) { // 8501 = data

      int k = 0;

      // read ROC data until next reset or next trig

      while( word != 0x8008 && word != 0x8004 ) { // 8008 = reset, 8004=trig

	if( k < 64000 ) {

	  if( latb4 ) { // Feb 2013 atb4

	    // data: 1000|XXXX|YYYY|ZZZZ (12 bits)

	    rdd[k] = word & 0xfff; // lowest 12 bits
	  }
	  else {

	    // data: 1000|0000|0000|XXXX (only XXXX is significant data)

	    rdd[k] = word & 0xf; // lowest 4 bits
	  }

	k++;

	//cout << "ii " << ii << ", k " << k << endl;

	} // k in 8001 data

	if( ii <= jj-2 ) {
	  a = dbuffer[ii++];
	  b = dbuffer[ii++];
	  word = (b << 8) | a; // word = 2 bytes
	}
	else break;

      } // while loop over ADC data

      cout << " (" << k << " words)";
      /*
      cout << hex;
      for( int j = 0; j < k; ++j ) cout << " " << rdd[j];
      cout << dec;
      */
      // decode digital ROC:

      int retval = 
	decode_digital_readout( drm, rdd, k, 1, 0 ); // 1 ROC, no TBM

      if( retval >= 0 ) { // Successful decoding:

	int hits = drm->roc[roc->GetChipId()].numPixelHits;

	hnpx->Fill( hits );

	cout << "(" << hits << " pix)"; 

	for( int ipx = 0; ipx < hits; ipx++ ) {

	  int col = drm->roc[roc->GetChipId()].pixelHit[ipx].columnROC;
	  int row = drm->roc[roc->GetChipId()].pixelHit[ipx].rowROC;
	  int adc = drm->roc[roc->GetChipId()].pixelHit[ipx].analogPulseHeight;

	  hitmap->Fill( col, row );
	  adcmap->Fill( col, row, adc );
	  adchst->Fill( adc );
	  /*
	  cout << endl
	       << "col " << col
	       << ", row " << row
	       << ", PH " << adc;
	  */
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

	} // ipx hits

      } // valid decode

      else {
	// Decoding failed
	cout << " (digital decoder error " << retval << ")" << endl;
      }

      cout << endl;

    } // 8001: ADC data

  } // ii bytes loop

  cout << "[ADCmap] read up to byte " << ii
      << " of " << filledMem1
      << endl;

  // Cleanup:

  //cout << "DataClear " << tbInterface->DataClear() << endl; // FIFO clear
  //tbInterface->DataClear(); // FIFO clear
  tbInterface->Clear(); // usb.Clear()
  gDelay->Mdelay(50); // ms

  delete drm;
  delete rdd;

  histograms->Add( hitmap );
  histograms->Add( hnpx );
  histograms->Add( adcmap );
  histograms->Add( adchst );

  if( haveGain ) {
    histograms->Add( map4 );
    histograms->Add( his4 );
  }

  RestoreDacParameters();
}
