#include "ChipVariation.h"
#include "TestRoc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TCanvas.h"
#include "PhDacScan.h"

ChipVariation::ChipVariation(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
  : PhDacScan(aTestRange, testParameters, aTBInterface)
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
  debug = true;
}


void ChipVariation::ReadTestParameters(TestParameters *testParameters)
{
  PhDacScan::ReadTestParameters(testParameters);
  NumberOfSteps = (*testParameters).PHNumberOfSteps;
}


void ChipVariation::ModuleAction()
{
  linRange = new TH1D("Linear_Range","Linear Range",2000,0,2000);
  Test::ModuleAction();
  histograms->Add(linRange);
}

void ChipVariation::RocAction()
{
  SaveDacParameters();
  Test::RocAction();
  RestoreDacParameters();
}


void ChipVariation::PixelAction()
{
  ArmPixel();
  Flush();
  Scan();
  DisarmPixel();
}

void ChipVariation::Scan()
{

  int linearRange;

  SetDAC(253,4);

  int offset;
  if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
  else offset = 9;
  
  TH1D *histo = new TH1D(Form("ROC%i_Col%i_Row%i",chipId,column,row), Form("ROC%i_Col%i_Row%i",chipId,column,row), 256, 0, 256);
  cout << "Chip position " << aoutChipPosition << endl;
  short result[256];
  ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition*3, result);
  for (int dac = 0; dac < 256; dac++)
    {
      if (result[dac] == 7777) histo->SetBinContent(dac+1, 0);
      else histo->SetBinContent(dac+1, result[dac]);
    }
  linearRange = static_cast<int>( FindLinearRange(histo) );
  cout << "LINEAR RANGE = " << linearRange << endl;
  linRange->Fill(linearRange);
  histograms->Add(histo);
  
}
