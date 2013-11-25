
// renamed from PHTest
// PH vs DAC

#include "PHscan.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/pixel_dtb.h"
#include "BasePixel/CMSPixelDecoder.h"
//#include "BasePixel/DigitalReadoutDecoder.h"
//#include "BasePixel/analyzer.h"
#include <iomanip>

using namespace CMSPixel;

//------------------------------------------------------------------------------
PHscan::PHscan( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void PHscan::ReadTestParameters( TestParameters *testParameters )
{
  mode  = (*testParameters).PHMode;
  nTrig = (*testParameters).PHNTrig;
  cout << "PHscan mode  " << mode << endl;
  cout << "PHscan nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void PHscan::RocAction()
{
  SaveDacParameters();	

  Test::RocAction();

  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void PHscan::PixelAction()
{
  DACParameters* parameters = new DACParameters();
  char *dacName = parameters->GetName(mode);
  delete parameters;

  PhDac(dacName);

}

//------------------------------------------------------------------------------
void PHscan::PhDac( char *dacName )
{
  bool ldb = 1;

  bool atb4 = 1; // atb4: compact data = 12 valid bits per word

  TH1D *histo = 
    new TH1D(
	     Form( "PHvs%s_c%02dr%02d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId() ),
	     Form( "PHvs%s_c%02dr%02d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId() ),
	     256, 0, 256 );

  //histo->GetXaxis()->SetTitle( "Vcal (DAC units)" );
  histo->GetXaxis()->SetTitle( dacName );
  histo->GetYaxis()->SetTitle( "Pulse height [ADC]" );

  //cout << "EmptyReadoutLengthADC = " << tbInterface->GetEmptyReadoutLengthADC() << endl;

  // analog ROC:

  if( tbInterface->IsAnalog() ) {

    TH1D * ubHist = new TH1D( "ubHist", "ubHist", 256, 0, 256 );
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

    tbInterface->PHDac( mode, 256, nTrig, offset + aoutChipPosition * 3, result );
    tbInterface->ADCData( data, count );

    int ubLevel = data[ubPosition];

    for( int dac = 0; dac < 256; dac++) {
      if( result[dac] == 7777)
	histo->SetBinContent(dac + 1, 0);
      else
	histo->SetBinContent(dac + 1, result[dac]);

      ubHist->SetBinContent(dac + 1, ubLevel);
    }

    roc->ClrCal();
    //DP DisablePixel();
    DisarmPixel(); // Disarm = Disable + ClrCal
    histograms->Add(histo);
    histograms->Add(ubHist);

  } // analog

  //- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
  // digital ROC:

  else {

    // Show the whole range of the ADC data in the histogram
    histo->SetMinimum(0);
    histo->SetMaximum(255);

    // Structure that holds the decoded readout

    //int flaggen = FLAG_12BITS_PER_WORD | FLAG_ALLOW_CORRUPT_ROC_HEADERS;
    int flaggen = FLAG_12BITS_PER_WORD;
    CMSPixelEventDecoderDigital * singledec = new CMSPixelEventDecoderDigital( (unsigned int)nTrig, flaggen, ROC_PSI46DIG ); // config roctype
    std::vector<cmsPixel> * singleevt = new std::vector<cmsPixel>;

    const int deserAdjust = 4;

    tbInterface->getCTestboard()->Daq_Open(50000);
    tbInterface->getCTestboard()->Daq_Select_Deser160(deserAdjust);
    tbInterface->getCTestboard()->uDelay(100);

    EnablePixel();
    ArmPixel();

    cout << "  col " << setw(2) << pixel->GetColumn()
	 << ", row " << setw(2) << pixel->GetRow()
      ;

    int n0 = 0;
    int n255 = 0;
    int nhit = 0;
    int amin = 255;
    int vmin = 0;

    // Loop through the whole dac range:

    int scanMax;
    if( mode == 1 ||
	mode == 4 ||
	mode == 6 ||
	mode == 8 ||
	mode == 14 )
      scanMax = 16; // 4-bit
    else
      scanMax = 256; // 8-bit

    for( int vdac = 0; vdac < scanMax; vdac++ ) {

      /* Set Vdac */
      //SetDAC( dacName, vdac );
      SetDAC( mode, vdac );
      tbInterface->CDelay(400); // 400 clocks = 10 us
      //DP tbInterface->CDelay(500);

      tbInterface->getCTestboard()->Daq_Start();
      tbInterface->getCTestboard()->uDelay(100);

      tbInterface->Flush();

      // Send nTrig calibrates to the chip:

      for( int i = 0; i < nTrig; i++ ) {
	tbInterface->getCTestboard()->Pg_Single();
	tbInterface->getCTestboard()->uDelay(20);
	//tbInterface->Single(RES|CAL|TRG|TOK);
	//tbInterface->CDelay(400); // 400 BC = 10 us
	//tbInterface->CDelay(4000); // 4000 BC = 100 us for digV2 306 no help
      }
      tbInterface->CDelay(500);
      tbInterface->Flush();

      // Read the data from the FIFO on the testboard:

      vector<uint16_t> dvec;

      tbInterface->getCTestboard()->Daq_Stop();
      tbInterface->getCTestboard()->Daq_Read( dvec, 4000 );

      if( ldb ) cout << dacName << setw(3) << vdac
		     << ":  DataRead " << dvec.size() << " words"
		     << endl;

      // decode:

      float ph_mean = 0.0;
      int measurement_num = 0;

      int decodeEventReturnCode = singledec->get_event( dvec, singleevt );

      if( ldb ) cout << "decoder: " << decodeEventReturnCode
		     << ":  size " << singleevt->size() << " pixels"
		     << endl;

      for( int ii = 0; ii < singleevt->size(); ii++ ) {

	if( ldb ) cout << "  ii " << ii;

	nhit++;

	if( ldb ) cout << " at col " << singleevt->at(ii).col;
	if( ldb ) cout << ", row "   << singleevt->at(ii).row;

	int ph = singleevt->at(ii).raw;

	measurement_num++;

	if( ldb ) cout << ", PH "   << ph;

	ph_mean += ph;

	if( ph < amin ) {
	  amin = ph;
	  vmin = vdac;
	}

	if( ldb ) cout << endl;

      } // triggers

      // average:
      if( measurement_num > 0 )
	ph_mean /= measurement_num;

      if( ph_mean < 0.5 ) n0++;
      if( ph_mean > 254.5 ) n255++;

      histo->SetBinContent( vdac + 1, ph_mean );

    } // Vdac loop

    tbInterface->getCTestboard()->Daq_Close();

    cout << ": zeroes " << n0;
    cout << ", overflows " << n255;
    cout << ", min " << amin;
    cout << " at " << vmin;
    cout << endl;

    // Cleanup
    //DP DisablePixel();
    DisarmPixel(); // Disarm = Disable + ClrCal

    delete singleevt;
    delete singledec;

    histograms->Add(histo);

  } // digital
}
