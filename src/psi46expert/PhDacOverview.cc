// Modified: Samvel Khalatyan (samvel at cern dot ch)

#include "PhDacOverview.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TCanvas.h"
#include "PhDacScan.h"

PhDacOverview::PhDacOverview(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
  : PhDacScan(aTestRange, testParameters, aTBInterface)
{
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
  debug = true;
}


void PhDacOverview::ReadTestParameters(TestParameters *testParameters)
{
  PhDacScan::ReadTestParameters(testParameters);
  NumberOfSteps = (*testParameters).PHNumberOfSteps;
}

void PhDacOverview::RocAction()
{
  SaveDacParameters();
  Test::RocAction();
  RestoreDacParameters();
}


void PhDacOverview::PixelAction()
{
  ArmPixel();
  Flush();
  DoDacScan();
  //  DoVsfScan(); // xxx test me!!!
  DisarmPixel();
}

void PhDacOverview::DoDacScan()
{
   cout << " ************************* DAC SCAN **************************" << endl;
   
   int offset;
   if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
   else offset = 9;

   cout << "chipId = " << chipId << ", col = " << column << ", row = " << row << endl;

   for (int DacRegister = 1; DacRegister < 28; DacRegister++)
     {
       cout << "DAC set to " << DacRegister << endl;
       int scanMax;
       if ((DacRegister == 1)||(DacRegister == 4)||(DacRegister == 6)||(DacRegister == 8)||(DacRegister == 14)) scanMax = 16;
       else scanMax = 256;
       int defaultValue = GetDAC(DacRegister);
       // int defaultValue2 = GetDAC(DacRegister+2);
       int loopNumber = 0;
       for (int scanValue = 0; scanValue < scanMax; scanValue+=((int)scanMax/NumberOfSteps))
 	{
 	  loopNumber++;
 	  DACParameters* parameters = new DACParameters();
 	  char *dacName = parameters->GetName(DacRegister);
 	  delete parameters;
	  
 	  TH1D *histo = new TH1D(Form("DAC%i_Value%i",DacRegister,loopNumber), Form("%s=%d",dacName,scanValue), 256, 0, 256);
 	  cout << "default value = " << defaultValue << endl;
	  //cout << "default value2 = " << defaultValue2 << endl;
 	  SetDAC(DacRegister, scanValue);
 	  //SetDAC(DacRegister+2, scanValue);
 	  short result[256];
 	  ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition*3, result);
 	  for (int dac = 0; dac < 256; dac++)
 	    {
 	      if (result[dac] == 7777) histo->SetBinContent(dac+1, 0);
 	      else histo->SetBinContent(dac+1, result[dac]);
 	    }
 	  histograms->Add(histo);
 	}
       SetDAC(DacRegister, defaultValue);
       //SetDAC(DacRegister+2, defaultValue2);
     }

   
   for (int DacRegister = 2; DacRegister < 5; DacRegister++)
     {
       cout << "DAC set to " << DacRegister << endl;
       int scanMax = 256;
       int defaultValue = module->GetTBM(DacRegister);
       int loopNumber = 0;
       char *dacName;
       for (int scanValue = 0; scanValue < scanMax; scanValue+=((int)scanMax/NumberOfSteps))
	 {
	   loopNumber++;
	   
	   if (DacRegister == 2) dacName = "Inputbias";
	   else if (DacRegister == 3) dacName = "Outputbias";
	   else if (DacRegister == 4) dacName = "Dacgain";   
	   
	   TH1D *histo = new TH1D(Form("TBM_DAC%i_Value%i",DacRegister,loopNumber), Form("%s=%d",dacName,scanValue), 256, 0, 256);
	   cout << "default value = " << defaultValue << endl;
	   module->SetTBM(chipId,DacRegister,scanValue);
	   short result[256];
	   ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition*3, result);///!!!
	   for (int dac = 0; dac < 256; dac++)
	     {
	       if (result[dac] == 7777) histo->SetBinContent(dac+1, 0);
	       else histo->SetBinContent(dac+1, result[dac]);
	     }
	   histograms->Add(histo);
	 }
       module->SetTBM(chipId,DacRegister,defaultValue);
     }
   
}


void PhDacOverview::DoVsfScan()
{


  int offset;
  if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
  else offset = 9;
  int nTrig = 10;

  SetDAC("CtrlReg",4);

  for (int col = 0; col < 2; col++)
    {
      printf("col = %d\n",col);
      for (int row = 0; row < 2; row++)
	{  
	  //	  printf("row = %d\n",row);
	  for (int vsf = 150; vsf < 255; vsf+=20)
	    {
	      GetDAC("Vsf");
        SetDAC("Vsf", vsf);
	      Flush();
	      short result[256];
	      ((TBAnalogInterface*)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition*3, result);
	      TH1D *histo = new TH1D(Form("Vsf%d_Col%d_Row%d",vsf,col,row), Form("Vsf%d_Col%d_Row%d",vsf,col,row), 256, 0., 256.);
	      for (int dac = 0; dac < 256; dac++)
		{
		  if (result[dac] == 7777) histo->SetBinContent(dac+1, 555);
		  else histo->SetBinContent(dac+1, result[dac]);
		}
	    }
	}
    }





}
