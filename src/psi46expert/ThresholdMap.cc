#include "ThresholdMap.h"
#include "TestRoc.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBAnalogInterface.h"


ThresholdMap::ThresholdMap()
{
	doubleWbc = false;
}


void ThresholdMap::SetParameters(const char* mapName)
{
	cals = false;
	xtalk = false;
	reverseMode = false;
	dacReg = 12; //VthrComp
	
	if (strcmp(mapName, "VcalThresholdMap") == 0)
	{
		dacReg = 25; //Vcal
	}
	else if (strcmp(mapName, "VcalsThresholdMap") == 0)
	{
		dacReg = 25; //Vcal
		cals = true;
	}
	else if (strcmp(mapName, "XTalkMap") == 0)
	{
		dacReg = 25; //Vcal
		xtalk = true;
	}
	else if (strcmp(mapName, "NoiseMap") == 0)
	{
		reverseMode = true;
	}
	else if (strcmp(mapName, "CalXTalkMap") == 0)
	{
		xtalk = true;
	}
}


TH2D* ThresholdMap::GetMap(const char * mapName, TestRoc *roc, TestRange *testRange, int nTrig)
{
	SetParameters(mapName);
	MeasureMap(mapName, roc, testRange, nTrig);
	return histo;
}


void ThresholdMap::MeasureMap(const char * mapName, TestRoc *roc, TestRange *testRange, int nTrig)
{
	char totalMapName[100];
	sprintf(totalMapName, "%s_C%i", mapName, roc->GetChipId());
	histo = new TH2D(totalMapName, totalMapName, ROCNUMCOLS, 0., ROCNUMCOLS, ROCNUMROWS, 0., ROCNUMROWS);
	
	int wbc = roc->GetDAC("WBC");
	if (doubleWbc)
	{
		roc->SetDAC("WBC", wbc-1);
		roc->Flush();		
	}
	
	int sign = 1;
	if (reverseMode) sign = -1;

	int data[4160];
	roc->ChipThreshold(100, sign, nTrig/2, nTrig, dacReg, xtalk, cals, data);
	
	for (int iCol = 0; iCol < ROCNUMCOLS ; iCol++)  
	{
		for (int iRow = 0; iRow < ROCNUMROWS ; iRow++)
		{
			if (testRange->IncludesPixel(roc->GetChipId(), iCol, iRow))
			{
				histo->SetBinContent(iCol+1, iRow+1, data[iCol*ROCNUMROWS + iRow]);
			}
		}
	}

	if (doubleWbc) 
	{
		roc->SetDAC("WBC", wbc);
		roc->Flush();
		
		if (histo->GetMaximum() == 255)  // if there are pixels where no threshold could be found, test other wbc
		{
			int data2[4160];
			roc->ChipThreshold(100, sign, nTrig/2, nTrig, dacReg, xtalk, cals, data2);
			
			for (int iCol = 0; iCol < ROCNUMCOLS ; iCol++)  
			{
				for (int iRow = 0; iRow < ROCNUMROWS ; iRow++)
				{
					if (testRange->IncludesPixel(roc->GetChipId(), iCol, iRow))
					{
						int index = iCol*ROCNUMROWS + iRow;
						if (data2[index] < data[index]) histo->SetBinContent(iCol+1, iRow+1, data2[index]);
					}
				}
			}
		}
	}
		
	roc->SetDAC("WBC", wbc); // restore original wbc
}


void ThresholdMap::SetCals()
{
	cals = true;
}


void ThresholdMap::SetXTalk()
{
	xtalk = true;
}


void ThresholdMap::SetDoubleWbc()
{
	doubleWbc = true;
}


void ThresholdMap::SetSingleWbc()
{
	doubleWbc = false;
}

void ThresholdMap::SetReverseMode()
{
	reverseMode = true;
}

