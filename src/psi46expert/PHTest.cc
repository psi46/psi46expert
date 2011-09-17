#include "PHTest.h"
#include "TestRoc.h"
#include "BasePixel/TBAnalogInterface.h"


PHTest::PHTest(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
}


void PHTest::ReadTestParameters(TestParameters *testParameters)
{
	mode = (*testParameters).PHMode;
	nTrig = (*testParameters).PHNTrig;
}


void PHTest::RocAction()
{
	SaveDacParameters();	
	if (mode == 0)
	{
		map = new TH2D(Form("PH_C%d", chipId),Form("PH_C%d", chipId), ROCNUMCOLS, 0, ROCNUMCOLS, ROCNUMROWS, 0, ROCNUMROWS);
		int data[ROCNUMROWS*ROCNUMCOLS], offset;
		if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
		else offset = 9;
		roc->AoutLevelChip(offset + aoutChipPosition*3, nTrig, data);
		for (int col = 0; col < ROCNUMCOLS; col++)
		{	
	        	for (int row = 0; row < ROCNUMROWS; row++) map->SetBinContent(col+1, row+1, data[col*ROCNUMROWS+row]);
		}
                histograms->Add(map); 
		 
	}
	Test::RocAction();	 
	RestoreDacParameters();
}


void PHTest::PixelAction()
{
	if (mode == 0) {}
	else
	{
		DACParameters* parameters = new DACParameters();
		char *dacName = parameters->GetName(mode);
		delete parameters;
		PhDac(dacName);
	}
}


void PHTest::PhDac(char *dacName)
{
	TH1D *histo = new TH1D(Form("Ph%s_c%dr%d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId()),Form("Ph%s_c%dr%d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId()), 256, 0, 256);
	TH1D *ubHist = new TH1D("ubHist","ubHist",256,0,256);
	ubHist->SetLineColor(kRed);
	
	EnablePixel();
	Cal();
	Flush();
	
	short result[256], data[10000];
	int offset;
	int ubPosition = 8+aoutChipPosition*3;
	unsigned short count;

	if (((TBAnalogInterface*)tbInterface)->TBMPresent()) offset = 16;
	else offset = 9;
	((TBAnalogInterface*)tbInterface)->PHDac(mode, 256, nTrig, offset + aoutChipPosition*3, result);
	
	((TBAnalogInterface*)tbInterface)->ADCData(data, count);
		
	int ubLevel = data[ubPosition];	

	for (int dac = 0; dac < 256; dac++)
	  {
	    if (result[dac] == 7777) histo->SetBinContent(dac+1, 0);
	    else histo->SetBinContent(dac+1, result[dac]);
	    ubHist->SetBinContent(dac+1, ubLevel);
	  }

	roc->ClrCal();
	DisablePixel();
	histograms->Add(histo);
	histograms->Add(ubHist);
}

