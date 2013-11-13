#include "interface/Log.h"
#include "PHTest.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBInterface.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include "Analysis.h"


PHTest::PHTest(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
}


void PHTest::ReadTestParameters(TestParameters * testParameters)
{
    mode = (*testParameters).PHMode;
    nTrig = (*testParameters).PHNTrig;
}

void PHTest::SetMode(int new_mode)
{
        mode = new_mode;
}

void PHTest::SetNTrig(int new_ntrig)
{
        nTrig = new_ntrig;
}

void PHTest::RocAction(TestRoc * testRoc)
{
        Test::RocAction(testRoc);
}

void PHTest::RocAction()
{
    SaveDacParameters();
    if (mode == 0)
    {
        psi::LogInfo() << "[PHTest] Measuring pulse height map ..." << psi::endl;
        map = new TH2D(Form("PH_C%d", chipId), Form("Pulse height map C%d", chipId), ROCNUMCOLS, 0, ROCNUMCOLS, ROCNUMROWS, 0, ROCNUMROWS);
        int data[ROCNUMROWS * ROCNUMCOLS], offset;
        if (tbInterface->TBMPresent()) offset = 16;
        else offset = 9;
        if (roc->has_analog_readout())
            roc->AoutLevelChip(offset + aoutChipPosition * 3, nTrig, data);
        else
            PulseHeightRocDigital(data);
        for (int col = 0; col < ROCNUMCOLS; col++)
        {
            for (int row = 0; row < ROCNUMROWS; row++) map->SetBinContent(col + 1, row + 1, data[col * ROCNUMROWS + row]);
        }

        map->GetXaxis()->SetTitle("Column");
        map->GetYaxis()->SetTitle("Row");
        histograms->Add(map);

        /* Distribution of pulse height */
        TH1D * distr;
        if (roc->has_analog_readout())
            distr = gAnalysis->Distribution(map, 4096, -2048, 2048);
        else
            distr = gAnalysis->Distribution(map, 256, 0, 256);
        distr->GetXaxis()->SetTitle("Pulse height [ADC units]");
        distr->GetYaxis()->SetTitle("Pixels / bin [1]");
        histograms->Add(distr);

        /* X-projection of the map */
        TH1D * projx = map->ProjectionX();
        projx->SetTitle(Form("%s (x-projection)", map->GetTitle()));
        projx->GetXaxis()->SetTitle("Column");
        projx->GetYaxis()->SetTitle("Pulse height [ADC units]");
        *projx = 1.0 / ROCNUMROWS * *projx;
        histograms->Add(projx);

        /* Y-projection of the map */
        TH1D * projy = map->ProjectionY();
        *projy = 1.0 / ROCNUMCOLS * *projy;
        projy->SetTitle(Form("%s (y-projection)", map->GetTitle()));
        projy->GetXaxis()->SetTitle("Row");
        projy->GetYaxis()->SetTitle("Pulse height [ADC units]");
        histograms->Add(projy);
    }
    Test::RocAction();
    RestoreDacParameters();
}


void PHTest::PixelAction()
{
    if (mode == 0) {}
    else
    {
        DACParameters * parameters = new DACParameters();
        char * dacName = parameters->GetName(mode);
        delete parameters;
        psi::LogInfo() << "[PHTest] Testing DAC " << dacName << " for pixel " << column << ":" << row << psi::endl;
        PhDac(dacName);
    }
}


void PHTest::PhDac(char * dacName)
{
    TH1D * histo = new TH1D(Form("Ph%s_c%dr%d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId()), Form("Ph%s_c%dr%d_C%d", dacName, pixel->GetColumn(), pixel->GetRow(), roc->GetChipId()), 256, 0, 256);
    histo->GetXaxis()->SetTitle(Form("%s (DAC units)", dacName));
    histo->GetYaxis()->SetTitle("Pulse height [ADC]");

    if (roc->has_analog_readout()) {
        TH1D * ubHist = new TH1D("ubHist", "ubHist", 256, 0, 256);
        ubHist->SetLineColor(kRed);

        EnablePixel();
        Cal();
        Flush();

        short result[256], data[10000];
        int offset;
        int ubPosition = 8 + aoutChipPosition * 3;
        unsigned short count;

        if (tbInterface->TBMPresent())
            offset = 16;
        else
            offset = 9;

        tbInterface->PHDac(mode, 256, nTrig, offset + aoutChipPosition * 3, result);
        tbInterface->ADCData(data, count);

        int ubLevel = data[ubPosition];

        for (int dac = 0; dac < 256; dac++) {
            if (result[dac] == 7777)
                histo->SetBinContent(dac + 1, 0);
            else
                histo->SetBinContent(dac + 1, result[dac]);

            ubHist->SetBinContent(dac + 1, ubLevel);
        }

        roc->ClrCal();
        DisablePixel();
        histograms->Add(histo);
        histograms->Add(ubHist);
    } else {

        int col = pixel->GetColumn();
        int row = pixel->GetRow();

        histo->SetMinimum(0);
        histo->SetMaximum(255);
        
        for (int dac = 0; dac < 256; dac++) {
            /* Set DAC */
            SetDAC(dacName, dac);
            tbInterface->CDelay(500);
            tbInterface->Flush();
            histo->SetBinContent(dac + 1, tbInterface->PH(col, row));
        }
    histograms->Add(histo);
    return;
        ///* Show the whole range of the ADC data in the histogram */
        //histo->SetMinimum(0);
        //histo->SetMaximum(255);

        ///* Buffer for the ADC data */
        //short * buffer = new short [256];
        //unsigned short nwords;

        ///* Structure that holds the decoded readout */
        //DecodedReadoutModule * drm = new DecodedReadoutModule;

        ///* Set local trigger and channel */
        //tbInterface->SetReg(41, 0x20 | 0x01);
        //tbInterface->DataCtrl(false, false, true);

        //EnablePixel();
        //ArmPixel();

        ///* Decoding flags */
        //int flags = module->GetRoc(0)->has_row_address_inverted() ? DRO_INVERT_ROW_ADDRESS : 0;

        ///* Loop through the whole DAC range */
        //for (int dac = 0; dac < 256; dac++) {
        //    /* Set DAC */
        //    SetDAC(dacName, dac);
        //    tbInterface->CDelay(500);
        //    tbInterface->Flush();

        //    /* Send nTrig calibrates to the chip */
        //    for (int i = 0; i < nTrig; i++) {
        //        tbInterface->Single(RES | CAL | TRG | TOK);
        //        tbInterface->CDelay(500);
        //    }
        //    tbInterface->Flush();

        //    /* Read the data from the FIFO on the testboard */
        //    tbInterface->getCTestboard()->DataRead(tbInterface->GetTBMChannel(), buffer, 256, nwords);

        //    /* Calculate the mean pulseheight from nTrig measurements by analysing the data */
        //    float ph_mean = 0.0;
        //    int measurement_num = 0;
        //    int data_pos = 0;
        //    for (int trig = 0; trig < nTrig; trig++) {
        //        int retval = decode_digital_readout(drm, buffer + trig * (tbInterface->GetEmptyReadoutLengthADC() + 6), nwords, module->NRocs(), flags);
        //        if (retval >= 0) {
        //            /* Successful decoding */
        //            int hits = drm->roc[roc->GetChipId()].numPixelHits;
        //            if (hits == 1) {
        //                /* Record the pulse height and move to the next block of data */
        //                ph_mean += drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;
        //                data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
        //                measurement_num++;
        //            } else if (hits > 1) {
        //                /* More hits than expected. Move to the next block of data. */
        //                data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
        //            } else {
        //                /* No hits, move to the next block of data. */
        //                data_pos += tbInterface->GetEmptyReadoutLengthADC();
        //            }
        //        } else {
        //            /* Decoding failed. Try next block of data. */
        //            data_pos += tbInterface->GetEmptyReadoutLengthADC();
        //        }
        //    }

        //    /* Finalize the mean value of the pulseheight */
        //    if (measurement_num > 0)
        //        ph_mean /= measurement_num;

        //    histo->SetBinContent(dac + 1, ph_mean);
        //}

        ///* Cleanup */
        //DisablePixel();

        //delete drm;
        //delete buffer;

        //histograms->Add(histo);
    }
}

void PHTest::PulseHeightRocDigital(int data [])
{

    tbInterface->Flush();

    for (int col = 0; col < 52; col++) {
        for (int row = 0; row < 80; row++) {
            data[80 * col + row] = tbInterface->PH(col, row);
        }
    }

    return;

    //
    //tbInterface->Flush();

    ///* Structure that holds the decoded readout */
    //DecodedReadoutModule * drm = new DecodedReadoutModule;

    ///* Set local trigger and channel */
    //tbInterface->SetReg(41, 0x20 | 0x01);

    ///* Enable the FIFO gate */
    //tbInterface->DataCtrl(false, false, true);

    ///* Buffer for the ADC data */
    //short * buffer = new short [256];
    //unsigned short nwords;

    ///* Decoding flags */
    //int flags = module->GetRoc(0)->has_row_address_inverted() ? DRO_INVERT_ROW_ADDRESS : 0;

    ///* iterate over columns and rows to get each pixel efficiency */
    //for (int col = 0; col < 52; col++) {
    //    for (int row = 0; row < 80; row++) {
    //        /* Arm the pixel */
    //        roc->ArmPixel(col, row);
    //        tbInterface->CDelay(500);
    //        tbInterface->Flush();

    //        /* send nTrig triggers with calibrates */
    //        for (int t = 0; t < nTrig; t++) {
    //            tbInterface->Single(RES | CAL | TRG | TOK);
    //            tbInterface->CDelay(500);
    //        }
    //        tbInterface->Flush();

    //        tbInterface->getCTestboard()->DataRead(tbInterface->GetTBMChannel(), buffer, 256, nwords);

    //        /* Calculate the mean pulseheight from nTrig measurements by analysing the data */
    //        float ph_mean = 0.0;
    //        int measurement_num = 0;
    //        int data_pos = 0;
    //        for (int trig = 0; trig < nTrig; trig++) {
    //            int retval = decode_digital_readout(drm, buffer + data_pos, nwords, module->NRocs(), flags);
    //            if (retval >= 0) {
    //                /* Successful decoding */
    //                int hits = drm->roc[roc->GetChipId()].numPixelHits;
    //                if (hits == 1) {
    //                    /* Record the pulse height and move to the next block of data */
    //                    ph_mean += drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;
    //                    data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
    //                    measurement_num++;
    //                } else if (hits > 1) {
    //                    /* More hits than expected. Move to the next block of data. */
    //                    data_pos += tbInterface->GetEmptyReadoutLengthADC() + hits * 6;
    //                } else {
    //                    /* No hits, move to the next block of data. */
    //                    data_pos += tbInterface->GetEmptyReadoutLengthADC();
    //                }
    //            } else {
    //                /* Decoding failed. Try next block of data. */
    //                data_pos += tbInterface->GetEmptyReadoutLengthADC();
    //            }
    //        }

    //        /* Finalize the mean value of the pulseheight */
    //        if (measurement_num > 0)
    //            ph_mean /= measurement_num;

    //        data[80 * col + row] = ph_mean;

    //        /* Disarm the pixel */
    //        roc->DisarmPixel(col, row);
    //        tbInterface->Flush();
    //    }
    //}

    //delete buffer;
    //delete drm;
    //return;
}
