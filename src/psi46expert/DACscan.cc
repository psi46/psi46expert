
// Daniel Pitzl. DESY, Aug 2012
// PH vs all DACs

#include "DACscan.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TCanvas.h"
#include "BasePixel/DecodedReadout.h"
#include "BasePixel/DigitalReadoutDecoder.h"

//------------------------------------------------------------------------------
DACscan::DACscan( TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface )
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}

//------------------------------------------------------------------------------
void DACscan::ReadTestParameters( TestParameters *testParameters )
{
  nTrig = (*testParameters).PHNTrig;
  cout << "PHmap nTrig " << nTrig << endl;
}

//------------------------------------------------------------------------------
void DACscan::RocAction()
{
  SaveDacParameters();
  Test::RocAction();
  RestoreDacParameters();
}

//------------------------------------------------------------------------------
void DACscan::PixelAction()
{
  ArmPixel();
  Flush();

  bool atb4 = 1; // atb4: compact data = 12 valid bits per word

  cout << "EmptyReadoutLengthADC = " << tbInterface->GetEmptyReadoutLengthADC() << endl;

  int vcal = GetDAC( "Vcal" );
  int creg = GetDAC( "CtrlReg" );
  cout << "Vcal is " << vcal
       << " at CtrlReg " << creg
       << endl;

  cout << "chipId = " << chipId
       << ", col = " << column
       << ", row = " << row
       << endl;

  /* Buffer for the ADC data */
  short * buffer = new short [256];
  unsigned short nwords;

  /* Structure that holds the decoded readout */
  DecodedReadoutModule * drm = new DecodedReadoutModule;

  /* Set local trigger and channel */
  if( ! tbInterface->IsAnalog() ) {
    tbInterface->SetReg(41, 0x20 | 0x01);
    tbInterface->DataCtrl( false, false, true );
  }

  // loop over DACs:

  for( int DacRegister = 1; DacRegister < 28; DacRegister++) {

    /* Exclude DACs that don't exist on the digital ROC psi46dig */
    // DP: 21 added to exclusion list

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

    TH1D * histo = new TH1D( Form( "PH_vs_DAC%02i", DacRegister ),
			     Form( "PH vs %s", dacName ),
			     scanMax, 0, scanMax );

    histo->GetXaxis()->SetTitle( Form( "%s [DAC units]", dacName ) );
    histo->GetYaxis()->SetTitle( "Pulse height [ADC units]" );
    histo->SetMinimum(0);
    histo->SetMaximum(256);

    for( int idac = 0; idac < scanMax; idac++ ) {

      SetDAC( DacRegister, idac );

      /* Send nTrig calibrates to the chip */
      for( int i = 0; i < nTrig; i++) {
	tbInterface->Single(RES|CAL|TRG|TOK);
	tbInterface->CDelay(500);
      }
      tbInterface->Flush();

      /* Read the data from the FIFO on the testboard */
      tbInterface->getCTestboard()->DataRead( tbInterface->GetTBMChannel(), buffer, 256, nwords );

      /* Calculate the mean pulseheight from nTrig measurements by analysing the data */
      float ph_mean = 0.0;
      int measurement_num = 0;
      int data_pos = 0;

      for( int trig = 0; trig < nTrig; trig++ ) {

	//DP int retval = decode_digital_readout( drm, buffer + trig * (tbInterface->GetEmptyReadoutLengthADC() + 6 ), nwords, module->NRocs(), 0 );
	int retval =
	  decode_digital_readout( drm, buffer + data_pos,
				  nwords, module->NRocs(), 0 );
	if( retval >= 0 ) {
	  /* Successful decoding */
	  int hits = drm->roc[roc->GetChipId()].numPixelHits;
	  if( hits == 1 ) {
	    /* Record the pulse height and move to the next block of data */
	    ph_mean += drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;
	    if( atb4 )
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 2; // atb 4
	    else	    
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
	    measurement_num++;
	  }
	  else if( hits > 1 ) {
	    /* More hits than expected. Move to the next block of data. */
	    if( atb4 )
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 2; // atb4
	    else
	      data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
	  }
	  else {
	    /* No hits, move to the next block of data. */
	    data_pos += tbInterface->GetEmptyReadoutLengthADC();
	  }
	}
	else {
	  /* Decoding failed. Try next block of data. */
	  data_pos += tbInterface->GetEmptyReadoutLengthADC();
	}

      } // triggers

      if( measurement_num < nTrig )
	cout << dacName << "  " << idac
	     << ": only " << measurement_num << " triggers with pulse" << endl;

      // average:

      if( measurement_num > 0 )
	ph_mean /= measurement_num;

      histo->SetBinContent( idac + 1, ph_mean );

    } // idac loop

    SetDAC( DacRegister, defaultValue );

    histograms->Add(histo);

  }//DACs

  DisarmPixel();

  delete drm;
  delete buffer;
}
