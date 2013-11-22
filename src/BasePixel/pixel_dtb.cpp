// psi46_tb.cpp
#include "pixel_dtb.h"
#include <stdio.h>
#include "interface/analyzer.h"
#ifndef _WIN32
#include <unistd.h>
#include <iostream>
#include <iomanip>
#endif

// missing defs
#define VCAL_TEST          20
int tct_wbc = 0;

bool CTestboard::EnumNext(string &name)
{
	char s[64];
	if (!usb.EnumNext(s)) return false;
	name = s;
	return true;
}

bool CTestboard::Enum(unsigned int pos, string &name)
{
	char s[64];
	if (!usb.Enum(s, pos)) return false;
	name = s;
	return true;
}


bool CTestboard::FindDTB(string &usbId)
{
	string name;
	vector<string> devList;
	unsigned int nDev;
	unsigned int nr;

	try
	{
		if (!EnumFirst(nDev)) throw int(1);
		for (nr=0; nr<nDev; nr++)
		{
			if (!EnumNext(name)) continue;
			if (name.size() < 4) continue;
			if (name.compare(0, 4, "DTB_") == 0) devList.push_back(name);
		}
	}
	catch (int e)
	{
		switch (e)
		{
		case 1: printf("Cannot access the USB driver\n"); return false;
		default: return false;
		}
	}

	if (devList.size() == 0)
	{
		printf("No DTB connected.\n");
		return false;
	}

	if (devList.size() == 1)
	{
		usbId = devList[0];
		return true;
	}

	// If more than 1 connected device list them
	printf("\nConnected DTBs:\n");
	for (nr=0; nr<devList.size(); nr++)
	{
		printf("%2u: %s", nr, devList[nr].c_str());
		if (Open(devList[nr], false))
		{
			try
			{
				unsigned int bid = GetBoardId();
				printf("  BID=%2u\n", bid);
			}
			catch (...)
			{
				printf("  Not identifiable\n");
			}
			Close();
		}
		else printf(" - in use\n");
	}

	printf("Please choose DTB (0-%u): ", (nDev-1));
	char choice[8];
	fgets(choice, 8, stdin);
	sscanf (choice, "%d", &nr);
	if (nr >= devList.size())
	{
		nr = 0;
		printf("No DTB opened\n");
		return false;
	}

	usbId = devList[nr];
	return true;
}


bool CTestboard::Open(string &usbId, bool init)
{
	rpc_Clear();
	if (!usb.Open(&(usbId[0]))) return false;

	if (init) Init();
	return true;
}


void CTestboard::Close()
{
//	if (usb.Connected()) Daq_Close();
	usb.Close();
	rpc_Clear();
}


void CTestboard::mDelay(uint16_t ms)
{
	Flush();
#ifdef _WIN32
	Sleep(ms);			// Windows
#else
	usleep(ms*1000);	// Linux
#endif
}

void CTestboard::SetMHz(int MHz = 0){
    Sig_SetDelay(SIG_CLK,  delayAdjust);
    Sig_SetDelay(SIG_SDA,  delayAdjust+15);
    Sig_SetDelay(SIG_CTR,  delayAdjust);
    Sig_SetDelay(SIG_TIN,  delayAdjust+5);
    Flush();
    tct_wbc = 5;
}

void CTestboard::prep_dig_test(){
    SetMHz();
    roc_I2cAddr(0);
    SetRocAddress(0);
}

void CTestboard::InitDAC()
{ 
    roc_SetDAC(  1,  4); // Vdig
    roc_SetDAC(  2, 100);
    roc_SetDAC(  3,  40);    // Vsf
    roc_SetDAC(  4,  12);    // Vcomp
    roc_SetDAC(  7,  60);    // VwllPr
    roc_SetDAC(  9,  60);    // VwllSh
    roc_SetDAC( 10, 117);    // VhldDel
    roc_SetDAC( 11,  40);    // Vtrim
    roc_SetDAC( 12,  20);    // VthrComp
    roc_SetDAC( 13,  30);    // VIBias_Bus
    roc_SetDAC( 14,   6);    // Vbias_sf
    roc_SetDAC( 22,  99);    // VIColOr
    roc_SetDAC( 15,  40);    // VoffsetOp
    roc_SetDAC( 17,  80);    // VoffsetRO
    roc_SetDAC( 18, 115);    // VIon
    roc_SetDAC( 19, 100);    // Vcomp_ADC
    roc_SetDAC( 20,  90);    // VIref_ADC
    roc_SetDAC( 25,   200);    // Vcal
    roc_SetDAC( 26,  68);  // CalDel
    roc_SetDAC( 0xfe,15);   // WBC
    roc_SetDAC( 0xfd, 0);   // CtrlReg

    Flush();
}


// it's also sending a Cal signal right now...readout both
int32_t CTestboard::MaskTest(int16_t nTriggers, int16_t res[])
{ 
    roc_Chip_Mask();
    Daq_Open(50000);
    Daq_Select_Deser160(deserAdjust);
    Daq_Start();

    // --- scan all pixel ------------------------------------------------------
    unsigned char col, row;
    for (col=0; col<ROC_NUMCOLS; col++)
    {
        roc_Col_Enable(col, true);
        uDelay(10);
        for (row=0; row<ROC_NUMROWS; row++)
        {
            roc_Pix_Cal(col, row, false);
            uDelay(20);
            //Pg_Single();
            //uDelay(10);
            roc_Pix_Trim(col, row, 15);
            uDelay(5);
            roc_Pix_Mask(col, row);
            uDelay(5);
            Pg_Single();
            uDelay(10);

            //roc_Pix_Mask(col, row);
            roc_ClrCal();
        }
        roc_Col_Enable(col, false);
        uDelay(10);
    }
    Daq_Stop();

    vector<uint16_t> data;
    Daq_Read(data, 50000);
    Daq_Close();

// --- analyze data --------------------------------------------------------
    // for each col, for each row, (masked pixel, unmasked pixel)
    PixelReadoutData pix;
    int pos = 0;
    cout << "analyze mask test" << endl;
    try
    {
        for (col=0; col<ROC_NUMCOLS; col++)
        {
            for (row=0; row<ROC_NUMROWS; row++)
            {
                // must be empty readout
                DecodePixel(data, pos, pix);
                res[(int)row+((int)col*(int)ROC_NUMROWS)]=pix.n;
                //g_chipdata.pixmap.SetMaskedCount(col, row, pix.n);

                // must be single pixel hit
                //DecodePixel(data, pos, pix);
                //g_chipdata.pixmap.SetUnmaskedCount(col, row, pix.n);
                //if (pix.n > 0)
                //{
                //    g_chipdata.pixmap.SetDefectColCode(col, row, pix.x != col);
                //    g_chipdata.pixmap.SetDefectrowCode(col, row, pix.y != row);
                //    g_chipdata.pixmap.SetPulseHeight(col, row, pix.p);
                //}
            }
        }
    } catch (int) {}
    return 1;
}

int32_t CTestboard::ChipEfficiency(int16_t nTriggers, int32_t trim[], double res[])
{ 
    // --- scan all pixel ------------------------------------------------------
    int col, row;
    PixelReadoutData pix;
    int nHits = 0;
    cout << "analyze chip efficiency" << endl;
    for (col=0; col<ROC_NUMCOLS; col++)
    {
        int pos = 0;
        Daq_Open(500000);
        Daq_Select_Deser160(deserAdjust);
        Daq_Start();
        roc_Col_Enable(col, true);
        for (row=0; row<ROC_NUMROWS; row++)
        {   
            //roc_Pix_Trim(col, row, 15);
            roc_Pix_Trim(col, row, trim[(int)row+((int)col*(int)ROC_NUMROWS)]);
            roc_Pix_Cal(col, row, false);
			uDelay(20);
            for (int16_t i=0; i < nTriggers; i++)
            {
			    Pg_Single();
            }
            roc_Pix_Mask(col, row);
			roc_ClrCal();
        }
        roc_Col_Enable(col, false);
        Daq_Stop();

        vector<uint16_t> data;
        Daq_Read(data,500000);
        Daq_Close();
        try
        {
            for (row=0; row<ROC_NUMROWS; row++)
            {
                // must be nTriggers
                nHits = 0;
                for (int16_t i=0; i < nTriggers; i++)
                {
                    DecodePixel(data, pos, pix);
                    if (pix.n > 0) nHits++;
                }
                // for each col, for each row, count number of hits and divide by triggers
                res[(int)row+((int)col*(int)ROC_NUMROWS)]=(double)nHits/nTriggers;
                //cout << "Pix (" << col << "," << row << "): " << res[(int)row+((int)col*(int)ROC_NUMROWS)] << endl;
            }
        } catch (int) {}
    }

    //roc_SetDAC(CtrlReg,0);
    return 1;
}

int32_t CTestboard::CountReadouts(int32_t nTrig, int32_t chipId)
{
    roc_I2cAddr(chipId);
	return CountReadouts(nTrig);
}

int32_t CTestboard::CountReadouts(int32_t nTrig, int32_t dacReg, int32_t dacValue)
{
    //cout << "Reg " << dacReg << " value " << dacValue << endl; 
	roc_SetDAC(dacReg, dacValue);
	return CountReadouts(nTrig);
}

int32_t CTestboard::CountReadouts(int32_t nTriggers)
{ 
    int32_t nHits = 0;
    Daq_Open(5000);
    Daq_Select_Deser160(deserAdjust);
    Daq_Start();
    for (int16_t i=0; i < nTriggers; i++)
    {
	    Pg_Single();
    }
    Daq_Stop();

    vector<uint16_t> data;
    Daq_Read(data,5000);
    Daq_Close();
    PixelReadoutData pix;
    int pos = 0;
    try
    {
        for (int16_t i=0; i < nTriggers; i++)
        {
            DecodePixel(data, pos, pix);
            if (pix.n > 0) nHits++;
        }
    } catch (int) {}
    return nHits;
}


void CTestboard::DacDac(int32_t dac1, int32_t dacRange1, int32_t dac2, int32_t dacRange2, int32_t nTrig, int32_t res[])
{
	for (int i = 0; i < dacRange1; i++)
	{
		roc_SetDAC(dac1, i);
		for (int k = 0; k < dacRange2; k++)
		{
			roc_SetDAC(dac2, k);
			res[i*dacRange1 + k] = CountReadouts(nTrig);
            //cout << "hits:" << res[i*dacRange1 + k] << endl;
		}
	}
    return;
}

void CTestboard::ArmPixel(int col, int row)
{
	ArmPixel(col, row, 15);
}


void CTestboard::ArmPixel(int col, int row, int trim)
{
	roc_Pix_Trim(col, row, trim);
	roc_Pix_Cal(col, row, false);
}


void CTestboard::EnableColumn(int col)
{
		roc_Col_Enable(col, 1);
		cDelay(20);
}


void CTestboard::EnableAllPixels(int32_t trim[])
{
	for (int col = 0; col < ROC_NUMCOLS; col++)
	{
		EnableColumn(col);
		for (int row = 0; row < ROC_NUMROWS; row++)
		{
			roc_Pix_Trim(col, row, trim[col*ROC_NUMROWS + row]);
		}
	}
}
	
	
void CTestboard::DisableAllPixels()
{
    roc_Chip_Mask();
}


void CTestboard::DisarmPixel(int col, int row)
{
	roc_ClrCal();
	roc_Pix_Mask(col,row);
}

void CTestboard::SetChip(int iChip)
{
	int portId = iChip/4;
	tbm_Addr(hubId, portId);
	roc_I2cAddr(iChip);
}

// == Thresholds ===================================================


int32_t CTestboard::Threshold(int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg)
{
	int32_t threshold = start, newValue, oldValue, result;
	int stepAbs;
	if (step < 0) stepAbs = -step; else stepAbs = step;
			
	newValue = CountReadouts(nTrig, dacReg, threshold);
	if (newValue > thrLevel)
	{
		do
		{
			threshold-=step;
			oldValue = newValue;
			newValue = CountReadouts(nTrig, dacReg, threshold);
		}
		while ((newValue > thrLevel) && (threshold > (stepAbs - 1)) && (threshold < (256 - stepAbs)));

		if (oldValue - thrLevel > thrLevel - newValue) result = threshold;
		else result = threshold+step;
	}
	else
	{
		do
		{
			threshold+=step;
			oldValue = newValue;
			newValue = CountReadouts(nTrig, dacReg, threshold);
		}
		while ((newValue <= thrLevel) && (threshold > (stepAbs - 1)) && (threshold < (256 - stepAbs)));

		if (thrLevel - oldValue > newValue - thrLevel) result = threshold;
		else result = threshold-step;
	}

	if (result > 255) result = 255;
	if (result < 0) result = 0;

	return result;
}

int32_t CTestboard::PixelThreshold(int32_t col, int32_t row, int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim)
{
	int calRow = row;
	roc_Pix_Trim(col, row, trim);

	if (xtalk)
	{
		if (row == ROC_NUMROWS - 1) calRow = row - 1;
		else calRow = row + 1;
	}
	if (cals) roc_Pix_Cal(col, calRow, true);
	else roc_Pix_Cal(col, calRow, false);

	int32_t res = Threshold(start, step, thrLevel, nTrig, dacReg);
    roc_ClrCal();
    roc_Pix_Mask(col, row);
	return res;
}

void CTestboard::ChipThresholdIntern(int32_t start[], int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim[], int32_t res[])
{
  int32_t thr, startValue;
	for (int col = 0; col < ROC_NUMCOLS; col++)
	{
		EnableColumn(col);
		for (int row = 0; row < ROC_NUMROWS; row++)
		{
			if (step < 0) startValue = start[col*ROC_NUMROWS + row] + 10;
			else startValue = start[col*ROC_NUMROWS + row];
			if (startValue < 0) startValue = 0;
			else if (startValue > 255) startValue = 255;
			
			thr = PixelThreshold(col, row, startValue, step, thrLevel, nTrig, dacReg, xtalk, cals, trim[col*ROC_NUMROWS + row]);
			res[col*ROC_NUMROWS + row] = thr;
		}
		roc_Col_Enable(col, 0);
	}
}

void CTestboard::Init_Reset()
{
    prep_dig_test();
    //InitDAC();
    //roc_Chip_Mask();

    Pg_SetCmd(0, PG_RESR + 25);
    Pg_SetCmd(1, PG_CAL  + 101 + tct_wbc);
    Pg_SetCmd(2, PG_TRG  + 16);
    Pg_SetCmd(3, PG_TOK);
    uDelay(100);
    Flush();
}

int32_t CTestboard::ChipThreshold(int32_t start, int32_t step, int32_t thrLevel, int32_t nTrig, int32_t dacReg, int32_t xtalk, int32_t cals, int32_t trim[], int32_t res[])
{
  int startValue;
  int32_t roughThr[ROC_NUMROWS * ROC_NUMCOLS], roughStep;
  if (step < 0) 
  {
  	startValue = 255;
  	roughStep = -4;
  }
  else 
  {
  	startValue = 0;
  	roughStep = 4;
  }
  
  for (int i = 0; i < ROC_NUMROWS * ROC_NUMCOLS; i++) roughThr[i] = startValue;
  ChipThresholdIntern(roughThr, roughStep, 0, 1, dacReg, xtalk, cals, trim, roughThr);
  ChipThresholdIntern(roughThr, step, thrLevel, nTrig, dacReg, xtalk, cals, trim, res);  
  return 1;
}

int32_t CTestboard::SCurve(int32_t nTrig, int32_t dacReg, int32_t threshold, int32_t sCurve[])
{
	for (int i = 0; i < 256; i++) sCurve[i] = 0;
	
	int start = threshold - 16;
	if (start < 0) start = 0;
	int stop = threshold + 16;
	if (stop > 256) stop = 256;
	for (int i = start; i < stop; i++)
	{
		sCurve[i] = CountReadouts(nTrig, dacReg, i);
	}
    return 1;
}

int32_t CTestboard::SCurve(int32_t nTrig, int32_t dacReg, int32_t thr[], int32_t chipId[], int32_t sCurve[])
{
	int dac;
	for (int i = 0; i < 32; i++)
	{
		for (int iChip = 0; iChip < nRocs; iChip++) 
		{
			SetChip(chipId[iChip]);
			sCurve[i*nRocs + iChip] = 0;
			
			dac = thr[iChip] - 16 + i;
			if (dac < 0) dac = 0;
			else if (dac > 255) dac = 255;
			roc_SetDAC(dacReg, dac);
		}
		cDelay(1200);
		if (i == 0) cDelay(1200);		
		
        sCurve[i] = CountReadouts(nTrig);
	}
    return 1;
}


int32_t CTestboard::SCurveColumn(int32_t iColumn, int32_t nTrig, int32_t dacReg, int32_t thr[], int32_t trim[], int32_t chipId[], int32_t sCurve[])
{	
	int32_t buffer[nRocs*32], thresholds[nRocs];
	long position = 0;
		
	for (int iChip = 0; iChip < nRocs; iChip++)
	{
		SetChip(chipId[iChip]);
  	    EnableColumn(iColumn);
	}
	
	for (int iRow = 0; iRow < ROC_NUMROWS; iRow++)
	{
		for (int iChip = 0; iChip < nRocs; iChip++)
		{
			thresholds[iChip] = thr[nRocs * iRow + iChip];
			SetChip(chipId[iChip]);
            ArmPixel(iColumn,iRow,trim[nRocs * iRow + iChip]);
		}
		
		SCurve(nTrig, dacReg, thresholds, chipId, buffer);

		for (int iChip = 0; iChip < nRocs; iChip++)
		{
			SetChip(chipId[iChip]);
 		 	DisarmPixel(iColumn, iRow);
		}

		for (int i = 0; i < 32*nRocs; i++) sCurve[position + i] = buffer[i];
		position+=32*nRocs;
	}
	
	for (int iChip = 0; iChip < nRocs; iChip++)
	{
		SetChip(chipId[iChip]);
  	    roc_Col_Enable(iColumn, 0);
	}
    return 1;
}

int32_t CTestboard::PH(int32_t col, int32_t row)
{
    Daq_Open(50000);
    Daq_Select_Deser160(deserAdjust);
    uDelay(100);
    Daq_Start();
    uDelay(100);

    roc_Col_Enable(col, true);
    roc_Pix_Trim(col, row, 15);
    roc_Pix_Cal (col, row, false);

    vector<uint16_t> data;
    unsigned int nTrig = 10;

    //roc_SetDAC(Vcal, vcal);
    uDelay(100);
    for (int k=0; k<nTrig; k++)
    {
        Pg_Single();
        uDelay(20);
    }

    roc_Pix_Mask(col, row);
    roc_Col_Enable(col, false);
    roc_ClrCal();

    Daq_Stop();
    Daq_Read(data, 4000);
    Daq_Close();

    DumpData(data,3*nTrig);
    // --- analyze data
    PixelReadoutData pix;

    int pos = 0;
    try
    {
        int cnt = 0;
        double yi = 0.0;
        for (int k=0; k<nTrig; k++)
        {
            DecodePixel(data, pos, pix);
            if (pix.n > 0) { 
	      yi += pix.p; 
	      cnt++; 
	      cout << setw(3) << cnt
		   << ". " << pix.n
		   << " " << setw(2) << pix.x
		   << " " << setw(2) << pix.y
		   << " " << setw(3) << pix.p
		   << endl;
	    }
        }
        if (cnt > 0)
            return yi/cnt;
        else
            return -1;
    } catch (int) {}

  
  
    }

bool CTestboard::test_pixel_address(int32_t col, int32_t row)
{
    Daq_Open(5000);
    Daq_Select_Deser160(deserAdjust);
    uDelay(100);
    Daq_Start();
    uDelay(100);

    roc_Col_Enable(col, true);
    
    ArmPixel(col,row);
    Pg_Single();
    DisarmPixel(col,row);
    
    roc_Col_Enable(col, false);
    Daq_Stop();

    vector<uint16_t> data;
    Daq_Read(data, 5000);
    Daq_Close();
    // --- analyze data
    PixelReadoutData pix;

    int pos = 0;
    try
    {
        DecodePixel(data, pos, pix);
        if (pix.n > 0){
            //cout << "("<< pix.x  <<',' << pix.y << ')' << "("<< row  <<',' << col << ')' <<endl; 
            return (pix.x == col && pix.y == row);
        }
        else return false;
    } catch (int) {}

    return false;
  
}
