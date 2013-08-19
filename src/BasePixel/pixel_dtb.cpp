// psi46_tb.cpp
#include "pixel_dtb.h"
#include <stdio.h>
#include "analyzer.h"
#ifndef _WIN32
#include <unistd.h>
#include <iostream>
#endif

// missing defs
#define VCAL_TEST          20
int tct_wbc = 0;
const int delayAdjust = 4;
const int deserAdjust = 4;

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
    int bin = 1;
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
    roc_SetDAC( 25,   2);    // Vcal
    roc_SetDAC( 26,  68);  // CalDel
    roc_SetDAC( 0xfe,15);   // WBC
    roc_SetDAC( 0xfd, 4);   // CtrlReg

    Flush();
}


// it's also sending a Cal signal right now...readout both
int32_t CTestboard::MaskTest(int16_t nTriggers, int16_t res[])
{ 
    //TODO move init in common place
    // load settings
    prep_dig_test();
    InitDAC();
    roc_Chip_Mask();
    roc_SetDAC(Vcal, VCAL_TEST);
    roc_SetDAC(CtrlReg,0x04); // 0x04

    Pg_SetCmd(0, PG_RESR + 25);
    Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
    Pg_SetCmd(2, PG_TRG  + 16);
    Pg_SetCmd(3, PG_TOK);
    uDelay(100);
    Flush();

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
            roc_Pix_Cal (col, row, false);
            uDelay(20);
            Pg_Single();
            uDelay(10);
            roc_Pix_Trim(col, row, 15);
            uDelay(5);
            Pg_Single();
            uDelay(10);

            roc_Pix_Mask(col, row);
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
                DecodePixel(data, pos, pix);
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
    roc_SetDAC(CtrlReg,0);
    return 1;
}

int32_t CTestboard::ChipEfficiency(int16_t nTriggers, int32_t trim[], double res[])
{ 
    //TODO move init in common place
    // load settings
    prep_dig_test();
    InitDAC();
    roc_Chip_Mask();
    roc_SetDAC(Vcal, VCAL_TEST);
    roc_SetDAC(CtrlReg,0x04); // 0x04

    Pg_SetCmd(0, PG_RESR + 25);
    Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
    Pg_SetCmd(2, PG_TRG  + 16);
    Pg_SetCmd(3, PG_TOK);
    uDelay(100);
    Flush();


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
                cout << "Pix (" << col << "," << row << "): " << res[(int)row+((int)col*(int)ROC_NUMROWS)] << endl;
            }
        } catch (int) {}
    }

    roc_SetDAC(CtrlReg,0);
    return 1;
}

int32_t CTestboard::CountReadouts(int16_t nTriggers, int col = 5, int row = 5)
{ 
    int32_t nHits = 0;
    Daq_Open(5000);
    Daq_Select_Deser160(deserAdjust);
    Daq_Start();
    roc_Pix_Trim(col, row, 15);
    roc_Col_Enable(col, true);
    roc_Pix_Cal(col, row, false);
    for (int16_t i=0; i < nTriggers; i++)
    {
	    Pg_Single();
    }
    roc_Pix_Mask(col, row);
	roc_ClrCal();
    roc_Col_Enable(col, false);
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

    //TODO move init in common place
    prep_dig_test();
    InitDAC();
    roc_Chip_Mask();
    roc_SetDAC(Vcal, VCAL_TEST);
    roc_SetDAC(CtrlReg,0x04); // 0x04

    Pg_SetCmd(0, PG_RESR + 25);
    Pg_SetCmd(1, PG_CAL  + 15 + tct_wbc);
    Pg_SetCmd(2, PG_TRG  + 16);
    Pg_SetCmd(3, PG_TOK);
    uDelay(100);
    Flush();

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

