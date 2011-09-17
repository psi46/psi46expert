#include "TrimBits.h"
#include "ThresholdMap.h"
#include "Analysis.h"
#include "interface/Log.h"

TrimBits::TrimBits(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
  psi::LogDebug() << "[TrimBits] Initialization." << psi::endl;

	testRange = aTestRange;
	tbInterface = aTBInterface;
	ReadTestParameters(testParameters);
}


void TrimBits::ReadTestParameters(TestParameters *testParameters)
{
	vtrim14 = (*testParameters).TrimBitsVtrim14;
	vtrim13 = (*testParameters).TrimBitsVtrim13;
	vtrim11 = (*testParameters).TrimBitsVtrim11;
	vtrim7 = (*testParameters).TrimBitsVtrim7;
	nTrig = (*testParameters).TrimBitsNTrig;
}


void TrimBits::RocAction()
{
	ThresholdMap *thresholdMap = new ThresholdMap();
	TH2D *map, *thrMap;
	int trim, vtrim;
	
	SaveDacParameters();	
	
	SetDAC("Vtrim", 0);
	
	thrMap = thresholdMap->GetMap("CalThresholdMap", roc, testRange, nTrig);
	thrMap->Write();
	histograms->Add(thrMap);
	
	for (int i = 0; i < 4; i++)
	{
		if (i == 0)
		{
			trim = 14;	
			vtrim = vtrim14;
		}
		else if (i == 1)
		{
			trim = 13;	
			vtrim = vtrim13;
		}
		else if (i == 2)
		{
			trim = 11;	
			vtrim = vtrim11;
		}
		else if (i == 3)
		{
			trim = 7;	
			vtrim = vtrim7;
		}

		SetDAC("Vtrim", vtrim);
		roc->SetTrim(trim);
		
		map = thresholdMap->GetMap("CalThresholdMap", roc, testRange, nTrig);
		map->Write();
		histograms->Add(map);
		histograms->Add(gAnalysis->TrimBitTest(thrMap, map, Form("TrimBit%i_C%i", trim, roc->GetChipId())));

	}	
	
	RestoreDacParameters();
}


