
// Daniel Pitzl, DESY, May 2013
// Read Back scan for psi46digV2
// vs testboard voltages
// version for atb4: 12 valid bits per word

#include "RBscan.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include <iomanip>
#include <sys/time.h> // timeval

//------------------------------------------------------------------------------
RBscan::RBscan( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void RBscan::ReadTestParameters( TestParameters *testParameters )
{
}

//------------------------------------------------------------------------------
void RBscan::RocAction()
{
  if( tbInterface->IsAnalog() ) return;

  bool ldb = 0;

  SaveDacParameters();	

  int vv = 2; // 1 = board VD, 2 = board VA

  int vstrt;
  int vstop;
  char *cv;
  if( vv == 1 ) {
    vstrt = 1500; // VD [mV]
    vstop = 3000;
    cv = "VD";
  }
  else {
    vstrt = 1000; // VA [mV]
    vstop = 2000;
    cv = "VA";
    tbInterface->RocSetDAC( 2, 255 ); // set Vana to max
    tbInterface->Flush();
  }
  int vrnge = vstop - vstrt;

  TH1D *hnw = new TH1D( "words_read", "words read;words read;events",
			201, -0.5, 200.5 );

  TH1D *hstart = new TH1D( "start_bit", "start bit;trigger with start bit;events",
			42, -1.5, 40.5 );

  TH1D *hcount = new TH1D( "RB_count", "read back count;read back data length [bits = triggers];events",
			   21, -0.5, 20.5 );

  TProfile *h08a =
    new TProfile( Form( "RB_VDext_vs_C%s_C%d", cv, roc->GetChipId() ),
		  Form( "RB_VDext_vs_%s_C%d;%s [V];RB VD unreg [V]",
			cv, roc->GetChipId(), cv ),
		  vrnge/10, vstrt*1E-3, vstop*1E-3, 0, 5 );

  TProfile *h08b =
    new TProfile( Form( "tb_VD_vs_%s_C%d", cv, roc->GetChipId() ),
		  Form( "tb_VD_vs_%s_C%d;%s [V];tb VD [V]",
			cv, roc->GetChipId(), cv ),
		  vrnge/10, vstrt*1E-3, vstop*1E-3, 0, 5 );

  TProfile *h09a =
    new TProfile( Form( "RB_VAext_vs_%s_C%d", cv, roc->GetChipId() ),
		  Form( "RB_VAext_vs_%s_C%d;%s [V];RB VA unreg [V]",
			cv, roc->GetChipId(), cv ),
		  vrnge/10, vstrt*1E-3, vstop*1E-3, 0, 5 );

  TProfile *h09b =
    new TProfile( Form( "tb_VA_vs_%s_C%d", cv, roc->GetChipId() ),
		  Form( "tb_VA_vs_%s_C%d;%s [V];tb VA [V]",
			cv, roc->GetChipId(), cv ),
		  vrnge/10, vstrt*1E-3, vstop*1E-3, 0, 5 );

  TProfile *h10a =
    new TProfile( Form( "RB_Vana_vs_%s_C%d", cv, roc->GetChipId() ),
		  Form( "RB_Vana_vs_%s_C%d;%s [V];RB Vana reg [V]",
			cv, roc->GetChipId(), cv ),
		  vrnge/10, vstrt*1E-3, vstop*1E-3, 0, 5 );

  TProfile *h11a =
    new TProfile( Form( "RB_Vref_vs_%s_C%d", cv, roc->GetChipId() ),
		  Form( "RB_Vref_vs_%s_C%d;%s [V];RB Vref [V]",
			cv, roc->GetChipId(), cv ),
		  vrnge/10, vstrt*1E-3, vstop*1E-3, 0, 5 );

  TProfile *h12a =
    new TProfile( Form( "RB_Iana_vs_%s_C%d", cv, roc->GetChipId() ),
		  Form( "RB_Iana_vs_%s_C%d;%s [V];RB Iana [mA]",
			cv, roc->GetChipId(), cv ),
		  vrnge/10, vstrt*1E-3, vstop*1E-3, 0, 100 );

  TProfile *h12b =
    new TProfile( Form( "IAvs%s_C%d", cv, roc->GetChipId() ),
		  Form( "IAvs%s_C%d;%s [V];board IA [mA]",
			cv, roc->GetChipId(), cv ),
		  vrnge/10, vstrt*1E-3, vstop*1E-3, 0, 100 );

  TProfile *h12c =
    new TProfile( Form( "RB_Iana_vs_tb_IA_C%d", roc->GetChipId() ),
		  Form( "RB_Iana_vs_tb_IA_C%d;tb IA [mA];RB Iana [mA]",
			roc->GetChipId() ),
		  200, 0, 50, 0, 100 );

//------------------------------------------------------------------------------
  // Buffer for the ADC data:

  short * buffer = new short[FIFOSIZE]; // BasePixel/GlobalConstants.h 4096
  unsigned short nwords;

  int empty = tbInterface->GetEmptyReadoutLengthADC(); // 1 for atb4

  timeval tv;
  /*
  struct timeval { // in sys/time.h
    time_t      tv_sec;  // seconds since 1.1.1970
    suseconds_t tv_usec; // microseconds
  };
  */
  gettimeofday( &tv, NULL );
  long s00 = tv.tv_sec; // seconds since 1.1.1970
  long u00 = tv.tv_usec; // microseconds

  long s0, u0;
  long s9, u9;

  int vana = GetDAC( "Vana" );

//------------------------------------------------------------------------------
  // scan vv:

  for( int jv = vstrt; jv <= vstop; jv += 10 ) {
    //for( int idac = 11; idac < 22; idac++ ) { // quick test

    gettimeofday( &tv, NULL );
    s0 = tv.tv_sec; // seconds since 1.1.1970
    u0 = tv.tv_usec; // microseconds

    if( vv == 1 )
      tbInterface->SetVD( jv*1E-3 ); // test board suply
    else {
      tbInterface->SetVA( jv*1E-3 ); // test board suply
    }
    tbInterface->CDelay(65000); // uint16_t: 65'000 max
    tbInterface->Flush();

    if( vv == 2 ) { // Vana steers current - just to be sure
      SetDAC( "Vana", vana ); // DacParameters::SetDAC contains delay
      Flush();
    }

    gettimeofday( &tv, NULL );
    s9 = tv.tv_sec; // seconds since 1.1.1970
    u9 = tv.tv_usec; // microseconds

    cout << "set " << cv << " to " << jv << endl;
    //cout << "duration " << s9-s0 + (u9-u0)*1e-6 << " s";
    //cout << " = " << (s9-s0)*1E6 + u9-u0 << " us";
    //cout << endl;

    // DAC 255:
    // digital: (Update immer wenn DAC oder Pixel programmiert wird)
    // 0  (default) Zuletzt programmierter DAC Wert
    // 1  Zuletzt programmierte DAC Nummer
    // 2  Zuletzt programmierte Pixel Kolonne (Gray Code)
    // 3  Zuletzt programmierte Pixel Zeile (Gray Code)

    //analog: Eine Messung wird nach Programmierung von Register 255 ausgelöst
    //  8  VD unreg (2*5.75 mV / digit)
    //  9  VA unreg (2*5.75 mV / digit)
    // 10  Vana regulated (5.75 mV / digit)
    // 11  Vref (5.75 mV / digit)
    // 12  Iana (0.1425 mA / digit)

    //for( int irb = 8; irb <= 12; irb++ ) {
    for( int irb = 8; irb <= 8; irb++ ) { // quick test

      gettimeofday( &tv, NULL );
      s0 = tv.tv_sec; // seconds since 1.1.1970
      u0 = tv.tv_usec; // microseconds

      tbInterface->RocSetDAC( 255, irb );
      tbInterface->CDelay(4000); // 4000 clocks = 100 us
      tbInterface->Flush(); // USB: send all commands

      double vd = 0;
      double va = 0;
      double ia = 0;
      if     ( irb ==  8 ) vd = tbInterface->GetVD();
      else if( irb ==  9 ) va = tbInterface->GetVA();
      else if( irb == 10 ) va = tbInterface->GetVA();
      else if( irb == 11 ) va = tbInterface->GetVA();
      else if( irb == 12 ) ia = tbInterface->GetIA();

      gettimeofday( &tv, NULL );
      s9 = tv.tv_sec; // seconds since 1.1.1970
      u9 = tv.tv_usec; // microseconds
      /*
      cout << "set RBS to " << irb << endl;
      cout << "duration " << s9-s0 + (u9-u0)*1e-6 << " s";
      cout << " = " << (s9-s0)*1E6 + u9-u0 << " us";
      cout << endl;
      */

      // Send nTrig calibrates to the chip:

      gettimeofday( &tv, NULL );
      s0 = tv.tv_sec; // seconds since 1.1.1970
      u0 = tv.tv_usec; // microseconds

      int nTrig = 33; // need at most 16 + 1 + 16

      for( int itrig = 0; itrig < nTrig; itrig++ ) {

	tbInterface->DataCtrl( false, true, false ); // no_clear, trig, no_continue
	tbInterface->Single( RES|CAL|TRG|TOK );
	tbInterface->CDelay(4000); // 4000 clocks = 100 us

      } // itrig

      tbInterface->CDelay(40*1000); // 40'000 clocks = 1000 us
      tbInterface->Flush(); // USB: send all commands

      tbInterface->getCTestboard()->DataReadRaw( tbInterface->GetTBMChannel(), buffer, FIFOSIZE, nwords );

      hnw->Fill( nwords );

      if( nwords >= FIFOSIZE )
	cout << "[RBscan] FIFOSIZE limit reached! Please mask all pixels" << endl;

      //int wordsCleared = tbInterface->DataClear();
      //if( wordsCleared > 0 ) cout << "words cleared " << wordsCleared << endl;
      tbInterface->Clear(); // usb.Clear

      gettimeofday( &tv, NULL );
      s9 = tv.tv_sec; // seconds since 1.1.1970
      u9 = tv.tv_usec; // microseconds
      /*
      cout << "read " << nwords << endl;
      cout << "duration " << s9-s0 + (u9-u0)*1e-6 << " s";
      cout << " = " << (s9-s0)*1E6 + u9-u0 << " us";
      cout << endl;
      */
      if( ldb )
	cout << "nTrig " << setw(2) << nTrig << ": "
	     << nwords << " words: "
	  //<< hex
	     << " " << ( 0xf & buffer[0] )
	     << dec
	     << endl;

      // Decode readout:

      int start = -1; // -1 = no start found
      int count =  0; // want 16 bits
      int data  =  0; // for 16 bits
      int iw = 0;

      gettimeofday( &tv, NULL );
      s0 = tv.tv_sec; // seconds since 1.1.1970
      u0 = tv.tv_usec; // microseconds

      while( iw < nwords ) {

	// Das Startbit erscheint bereits eine Auslese bevor
	// das neue 16 bit Wort beginnt.
	// ROC header bits: 0111 | 1111 | 10SD
	// 7fa: S = start
	// 7f8: D = zero
	// 7f9: D = one

	if( ( buffer[iw] & 0xffc ) == 0x7f8 ) { // head

	  if( start < 0 ) {
	    if( buffer[iw] & 0x2 ) {
	      start = iw;
	      if( ldb ) cout << "start at " << iw/empty << endl;
	    }
	  }
	  else {
	    count++;
	    data <<= 1; // bit shift
	    if( buffer[iw] & 0x1 ) data++; // one
	    if( count >= 16 ) break;     // bit count
	    if( buffer[iw] & 0x2 ) break; // next start
	  }
	  iw += empty;
	}
	else {
	  cout  << "bad header at word " << iw << ":"
		<< " " << ( 0xf & buffer[iw] )
		<< endl;
	  iw++;
	}

      } // iw loop

      hstart->Fill( start/empty );
      hcount->Fill( count );

      // decode:

      // 16 bit Wort:
      // bit 15..12: ROC Addresse (0 ... 15)
      // bit 11.. 8: DAC 255 Wert
      // bit 7..0: Daten

      if( start < 0 )
	cout << "no start marker found" << endl;

      else if( count < 16 )
	cout << "read back data incomplete: "
	     << count << " bits"
	     << " starting from " << start/empty
	     << " up to " << nwords/empty << " triggers"
	     << endl;

      else {
	if( ldb )
	  cout << "read back data from " << start/empty
	       << " to " << iw/empty << " triggers"
	       << endl;

	int rocId = (data >> 12) & 0x0f;
	int mode  = (data >>  8) & 0x0f;
	int value = data & 0xff;

	//const double Vconv = 5.75E-3; // Beat VA ADC to V
	//const double Vconv2 = 2*Vconv; // Beat VD ADC to V

	const double Vconv = 8.4E-3; // Daniel VA ADC to V
	const double Vconv2 = 2*Vconv; // Daniel VD ADC to V

	const double Iconv = 0.125; // ADC to mA

	if( mode != irb )
	  cout << "error: mode " << mode
	       << ", irb " << irb
	       << endl;

	switch( mode )
	  {
	  case  0:
	    printf( "ROC %i: (%04X) I2C data = %i\n",
		    rocId, data, value );
	    break;
	  case  1:
	    printf( "ROC %i: (%04X) I2C addr = %i\n",
		    rocId, data, value );
	    break;
	  case  2:
	    printf( "ROC %i: (%04X) pixel col = %i\n",
		    rocId, data, value );
	    break;
	  case  3:
	    printf( "ROC %i: (%04X) pixel row = %i\n",
		    rocId, data, value );
	    break;

	  case  8:
	    printf( "ROC %i: (%04X) VD unreg = %5.3f V\n",
		    rocId, data, value*Vconv2 );
	    h08a->Fill( jv*1E-3, value*Vconv2 );
	    h08b->Fill( jv*1E-3, vd );
	    break;
	  case  9:
	    printf( "ROC %i: (%04X) VA unreg = %5.3f V\n", rocId, data, 
		    value*Vconv*2 );
	    h09a->Fill( jv*1E-3, value*Vconv2 );
	    h09b->Fill( jv*1E-3, va );
	    break;
	  case 10:
	    printf( "ROC %i: (%04X) VA reg = %5.3f V\n",
		    rocId, data, value*Vconv );
	    h10a->Fill( jv*1E-3, value*Vconv );
	    break;
	  case 11:
	    printf( "ROC %i: (%04X) Vref = %5.3f V\n",
		    rocId, data, value*Vconv );
	    h11a->Fill( jv*1E-3, value*Vconv );
	    break;
	  case 12:
	    printf( "ROC %i: (%04X) Iana = %4.2f mA\n",
		    rocId, data, value*Iconv );
	    h12a->Fill( jv*1E-3, value*Iconv );
	    h12b->Fill( jv*1E-3, ia*1E3 );
	    h12c->Fill( ia*1E3, value*Iconv );
	    break;
	  default:
	    printf( "ROC %i: (%04X) ?\n", rocId, data );

	  } //mode

	gettimeofday( &tv, NULL );
	s9 = tv.tv_sec; // seconds since 1.1.1970
	u9 = tv.tv_usec; // microseconds
	/*
	cout << "duration " << s9-s0 + (u9-u0)*1e-6 << " s";
	cout << " = " << (s9-s0)*1E6 + u9-u0 << " us";
	cout << endl;
	*/
      } // valid start

    } // irb loop

  } // jv loop

  // Cleanup:

  tbInterface->DataCtrl( false, false, false ); // no_clear. no_trig, no_continue

  delete buffer;

  histograms->Add(hnw);
  histograms->Add(hstart);
  histograms->Add(hcount);
  histograms->Add(h08a);
  histograms->Add(h08b);
  histograms->Add(h09a);
  histograms->Add(h09b);
  histograms->Add(h10a);
  histograms->Add(h11a);
  histograms->Add(h12a);
  histograms->Add(h12b);
  histograms->Add(h12c);

  RestoreDacParameters();

  gettimeofday( &tv, NULL );
  s9 = tv.tv_sec; // seconds since 1.1.1970
  u9 = tv.tv_usec; // microseconds
  cout << "total duration " << s9-s00 + (u9-u00)*1e-6 << " s";
  cout << endl;

  cout << "Read Back register 255 works only on psi46digV2" << endl;

}
