// Modified: Samvel Khalatyan (samvel at cern dot ch)

#include "interface/Log.h"

#include "TestDoubleColumn.h"
#include "interface/Delay.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/Roc.h"


TestPixel *TestDoubleColumn::GetPixel(int column, int row)
{
	return (TestPixel*)DoubleColumn::GetPixel(column,row);
}


TestPixel *TestDoubleColumn::GetPixel(int iPixel)
{
	return (TestPixel*)pixel[iPixel];
}


TestDoubleColumn::TestDoubleColumn(Roc* aRoc, int dColumn, TestParameters *parameters)
{
	roc = aRoc;
	doubleColumn = dColumn;
	for (int i = 0; i<ROCNUMROWS; i++)
	{
		pixel[i] = new TestPixel(roc,doubleColumn*2,i, parameters);
		pixel[i+ROCNUMROWS] = new TestPixel(roc,doubleColumn*2+1,i, parameters);
	}

}




// Performs three double column tests, not debugged nor tested yet
void TestDoubleColumn::DoubleColumnTest()
{
	TestWBCSBC();
	TestTimeStampBuffer();
	TestDataBuffer();
}


// Find a good pixel in the double column
int TestDoubleColumn::FindGoodPixels(int count, TestPixel* pix[])
{
	TestPixel* pixel;
	int x, y;
	int x2 = 2*doubleColumn+1;
	int pos = 0;

	for (x=2*doubleColumn; x<x2; x++) for (y=0; y<ROCNUMROWS; y++)
	{
		if (!(pos < count)) return pos;
		pixel = GetPixel(x,y);
		if ( pixel->IsAlive() == 1) { pix[pos] = pixel; pos++; }
	}
	return pos;
}


void TestDoubleColumn::TestWBCSBC()
{
	TestPixel* pixel;
	
	if (FindGoodPixels(1, &pixel) != 1)
	{
    psi::LogInfo() << "[TestDoubleColumn] Error: can not find good pixel."
                   << psi::endl;

		return;
	}
	TBInterface* tbInterface = roc->GetTBInterface();

	roc->SetDAC("Vcal", 180);
	roc->ClrCal();
	pixel->EnablePixel();
	pixel->Cal();
	EnableDoubleColumn();
	
	tbInterface->SaveTBParameters();
	roc->SaveDacParameters();
	
	tbInterface->SetTBParameter("trc", 15);
	tbInterface->SetTBParameter("tct", 15);
	tbInterface->SetTBParameter("ttk", 10);
	tbInterface->SetTBParameter("cc", 1);
	roc->SetDAC("WBC",  15);

	
	const int wbcStep = 8;
	int wbc[wbcStep] = { 0x08, 0x09, 0x0A, 0x0C, 0x10, 0x20, 0x40, 0x80 };

	int n, td;
	bool err = false;
	bool res[wbcStep];
	for (n=0; n<wbcStep; n++)
	{
		res[n] = false;
		roc->SetDAC("WBC", wbc[n]);
		for (td=6; td<=255; td++)
		{
			tbInterface->SetTBParameter("tct", td);
			roc->SingleCal();
			gDelay->Udelay(100);
			int cnt = roc->GetRoCnt();
			if (wbc[n]==td && cnt==0 || wbc[n]!=td && cnt!=0)
			{
				err = true;
				res[n] = true;
			}
		}
	}

	pixel->DisablePixel();
	DisableDoubleColumn();
	roc->ClrCal();
	tbInterface->RestoreTBParameters();
	roc->RestoreDacParameters();

	if (err)
    psi::LogInfo() << "[TestDoubleColumn] Error." << psi::endl;
}


void TestDoubleColumn::TestTimeStampBuffer()
{
	TestPixel* pixel;
	
	if (FindGoodPixels(1, &pixel) != 1)
	{
    psi::LogInfo() << "[TestDoubleColumn] Error: can not find good pixel."
                   << psi::endl;
		return;
	}
	TBInterface* tbInterface = roc->GetTBInterface();

	roc->SetDAC("Vcal", 180);
	roc->ClrCal();
	pixel->EnablePixel();
	pixel->Cal();
	EnableDoubleColumn();

	tbInterface->SaveTBParameters();
	roc->SaveDacParameters();
	
	tbInterface->SetTBParameter("trc", 15);
	tbInterface->SetTBParameter("tcc",  6);
	tbInterface->SetTBParameter("tct", 120);
	tbInterface->SetTBParameter("ttk", 15);

	roc->SetDAC("WBC",  120);

	const int steps = 15;
	int res[steps];

	bool err = false;
	for (int n=1; n<steps; n++)
	{
		tbInterface->SetTBParameter("cc", n);
		roc->SingleCal();
		gDelay->Udelay(200);
		res[n] = roc->GetRoCnt();
		if (n<=12 && res[n]!= 1 || n>12 && res[n]!= 0) err = true;
	}
	pixel->DisablePixel();
	DisableDoubleColumn();
	roc->ClrCal();
	tbInterface->RestoreTBParameters();
	roc->RestoreDacParameters();

	if (err)
    psi::LogInfo() << "[TestDoubleColumn] Error." << psi::endl;
}


void TestDoubleColumn::TestDataBuffer()
{
	const int nPixels = 32;
	int res[nPixels];
	TestPixel* pixel[nPixels];

	if (FindGoodPixels(32, pixel) != 32)
	{
    psi::LogInfo() << "[TestDoubleColumn] Error: can not find good pixel."
                   << psi::endl;
		return;
	}

	TBInterface* tbInterface = roc->GetTBInterface();

	tbInterface->SaveTBParameters();
	roc->SaveDacParameters();

	tbInterface->SetTBParameter("tct", 80);
	tbInterface->SetTBParameter("ttk", 20);
	tbInterface->SetTBParameter("cc", 1);

	bool err = false;

	for (int n = 0; n < nPixels; n++)

	{
		pixel[n]->EnablePixel();
		pixel[n]->Cal();
		roc->SingleCal();
		gDelay->Udelay(200);
		res[n] = roc->GetRoCnt();
		if (n<31 && res[n]!= n+1 || n>=31 && res[n]!= 0) err = true;
	}
	for (int n = 0; n < nPixels; n++) pixel[n]->DisablePixel();

	DisableDoubleColumn();
	roc->ClrCal();
	tbInterface->RestoreTBParameters();
	roc->RestoreDacParameters();
	if (err)
    psi::LogInfo() << "[TestDoubleColumn] Error." << psi::endl;
}

