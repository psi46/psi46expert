#include "PhDacOverview.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/GlobalConstants.h"
#include <TParameter.h>
#include "TCanvas.h"
#include "PhDacScan.h"
#include "BasePixel/DecodedReadout.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include "interface/Log.h"

PhDacOverview::PhDacOverview(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
    : PhDacScan(aTestRange, testParameters, aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
    debug = true;
}


void PhDacOverview::ReadTestParameters(TestParameters * testParameters)
{
    PhDacScan::ReadTestParameters(testParameters);
    NumberOfSteps = (*testParameters).PHNumberOfSteps;
    ScanDac = (*testParameters).PHScanDac;
    VcalRange = (*testParameters).PHScanVcalRange;
}

void PhDacOverview::RocAction()
{
    SaveDacParameters();
    Test::RocAction();
    RestoreDacParameters();
}


void PhDacOverview::PixelAction()
{
    psi::LogInfo() << "[PhDacOverview] Testing ROC #" << chipId << ", pixel " << column << ":" << row << " ..." << psi::endl;
    ArmPixel();
    Flush();
    DoDacScan();
    //  DoVsfScan(); // xxx test me!!!
    DisarmPixel();
}

/* Test pulse height vs Vcal in the entire parameter space of the ROC and the TBM */
void PhDacOverview::DoDacScan()
{
    TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;

    int offset;
    if (ai->TBMPresent())
        offset = 16;
    else
        offset = 9;

    /* Iterate over ROC DACs */
    int DacMin, DacMax;
    if (ScanDac == 0) {
        DacMin = 1;
        DacMax = 27;
    } else {
        DacMin = ScanDac;
        DacMax = ScanDac;
    }
    for (int DacRegister = DacMin; DacRegister <= DacMax; DacRegister++) {
        /* Exclude DACs that don't exist on the digital ROC psi46dig */
        if (!(roc->has_analog_readout())) {
            switch (DacRegister) {
            case 5: case 6: case 8: case 16: case 23: case 24: case 27: case 28:
                continue;
            }
        }

        /* Get the name of the DAC */
        DACParameters * parameters = new DACParameters();
        char * dacName = parameters->GetName(DacRegister);
        delete parameters;

        /* Set scan maximum */
        int scanMax;
        if ((DacRegister == 1) || (DacRegister == 4) || (DacRegister == 6) || (DacRegister == 8) || (DacRegister == 14))
        {
            scanMax = 16;
        }
        else
        {
            scanMax = 256;
        }

        int steps = (NumberOfSteps > scanMax) ? scanMax : NumberOfSteps;

        psi::LogInfo() << "[PhDacOverview] Testing ROC DAC #" << DacRegister << " (" << dacName << ") in " << steps << " steps ..." << psi::endl;

        /* Scan the DAC */
        int defaultValue = GetDAC(DacRegister);
        int loopNumber = 0;

        /* Digital current histogram */
        TH1D * histo_id = new TH1D(Form("digital current of DAC %s", dacName), Form("digital current of DAC %s", dacName), scanMax, 0, scanMax);
        histo_id->GetXaxis()->SetTitle(Form("DAC %s (DAC units)", dacName));
        histo_id->GetYaxis()->SetTitle("Digital current (mA)");
        histo_id->SetMarkerStyle(20);
        histo_id->SetMarkerSize(2);
        histo_id->SetMinimum(0);

        cout << "[PhDacOverview] Scan in progress: ";
        for (int scanValue = 0; scanValue < scanMax; scanValue += ((int)scanMax / steps))
        {
            loopNumber++;

            DACParameters * parameters = new DACParameters();
            char * dacName = parameters->GetName(DacRegister);
            delete parameters;

            TH1D * histo_low = new TH1D(Form("DAC%i_Value%i", DacRegister, loopNumber), Form("%s=%d", dacName, scanValue), 256, 0, 256);
            TH1D * histo_high = new TH1D(Form("DAC%i_Value%i_h", DacRegister, loopNumber), Form("%s=%d", dacName, scanValue), 256, 0, 256);

            cout << "*"  << flush;

            histo_low->GetXaxis()->SetTitle("Vcal (low range) [DAC units]");
            histo_high->GetXaxis()->SetTitle("Vcal (high range) [DAC units]");
            histo_low->GetYaxis()->SetTitle("Pulse height [ADC units]");
            histo_high->GetYaxis()->SetTitle("Pulse height [ADC units]");
            SetDAC(DacRegister, scanValue);
            if (VcalRange == 0 || VcalRange == 2) {
                SetDAC("CtrlReg", 0);
                PHDac(histo_low);
            }
            if (VcalRange == 1 || VcalRange == 2) {
                SetDAC("CtrlReg", 4);
                PHDac(histo_high);
            }

            if (VcalRange == 0 || VcalRange == 2)
                histograms->Add(histo_low);
            if (VcalRange == 1 || VcalRange == 2)
                histograms->Add(histo_high);

            /* Measure digital current (2 times) */
            double id;
            id = ai->GetID();
            id = ai->GetID();
            TParameter<double> * parameter1 = new TParameter<double>(Form("ID_DAC%i_Value%i", DacRegister, loopNumber), id);
            parameter1->Write();

            histo_id->SetBinContent(scanValue + 1, id * 1000);
        }
        histograms->Add(histo_id);

        cout << endl;

        SetDAC(DacRegister, defaultValue);
    }

    /* Stop if there is no TBM */
    if (!(ai->TBMPresent()))
        return;

    /* Iterate over TBM DACs */
    for (int DacRegister = 2; DacRegister < 5; DacRegister++) {
        const char * dacName;
        if (DacRegister == 2)
            dacName = "Inputbias";
        else if (DacRegister == 3)
            dacName = "Outputbias";
        else if (DacRegister == 4)
            dacName = "Dacgain";

        psi::LogInfo() << "[PhDacOverview] Testing TBM DAC #" << DacRegister << "(" << dacName << ") ..." << psi::endl;

        int scanMax = 256;
        int steps = (NumberOfSteps > scanMax) ? scanMax : NumberOfSteps;
        int defaultValue = module->GetTBM(DacRegister);
        int loopNumber = 0;

        for (int scanValue = 0; scanValue < scanMax; scanValue += ((int) scanMax / steps)) {
            loopNumber++;

            TH1D * histo = new TH1D(Form("TBM_DAC%i_Value%i", DacRegister, loopNumber), Form("%s=%d", dacName, scanValue), 256, 0, 256);
            histo->GetXaxis()->SetTitle("Vcal [DAC units]");
            histo->GetYaxis()->SetTitle("Pulse height [ADC units]");
            module->SetTBM(chipId, DacRegister, scanValue);
            PHDac(histo);
            histograms->Add(histo);
        }
        module->SetTBM(chipId, DacRegister, defaultValue);
    }
}

void PhDacOverview::PHDac(TH1D * histo)
{
    TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;

    if (roc->has_analog_readout()) {
        int offset;
        if (ai->TBMPresent())
            offset = 16;
        else
            offset = 9;

        short result[256];
        ai->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result);
        for (int dac = 0; dac < 256; dac++) {
            if (result[dac] == 7777)
                histo->SetBinContent(dac + 1, 0);
            else
                histo->SetBinContent(dac + 1, result[dac]);
        }
    } else {
        /* Show the whole range of the ADC data in the histogram */
        histo->SetMinimum(0);
        histo->SetMaximum(255);

        /* Buffer for the ADC data */
        short * buffer = new short [256];
        unsigned short nwords;

        /* Structure that holds the decoded readout */
        DecodedReadoutModule * drm = new DecodedReadoutModule;

        /* Set local trigger and channel */
        ai->SetReg(41, 0x20 | 0x01);
        ai->DataCtrl(false, false, true);

        EnablePixel();
        ArmPixel();

        /* Decoding flags */
        int flags = module->GetRoc(0)->has_row_address_inverted() ? DRO_INVERT_ROW_ADDRESS : 0;

        /* Loop through the whole Vcal range */
        for (int vcal = 0; vcal < 256; vcal++) {
            /* Set Vcal */
            SetDAC("Vcal", vcal);
            ai->CDelay(500);
            ai->Flush();

            /* Send nTrig calibrates to the chip */
            for (int i = 0; i < nTrig; i++) {
                ai->Single(RES | CAL | TRG | TOK);
                ai->CDelay(500);
            }
            ai->Flush();

            /* Read the data from the FIFO on the testboard */
            ai->getCTestboard()->DataRead(ai->GetTBMChannel(), buffer, 256, nwords);

            /* Calculate the mean pulseheight from nTrig measurements by analysing the data */
            float ph_mean = 0.0;
            int measurement_num = 0;
            int data_pos = 0;
            for (int trig = 0; trig < nTrig; trig++) {
                int retval = decode_digital_readout(drm, buffer + data_pos, nwords, module->NRocs(), flags);
                if (retval >= 0) {
                    /* Successful decoding */
                    int hits = drm->roc[roc->GetChipId()].numPixelHits;
                    if (hits == 1) {
                        /* Record the pulse height and move to the next block of data */
                        ph_mean += drm->roc[roc->GetChipId()].pixelHit[0].analogPulseHeight;
                        data_pos += ai->GetEmptyReadoutLengthADC() + hits * 6;
                        measurement_num++;
                    } else if (hits > 1) {
                        /* More hits than expected. Move to the next block of data. */
                        data_pos += ai->GetEmptyReadoutLengthADC() + hits * 6;
                    } else {
                        /* No hits, move to the next block of data. */
                        data_pos += ai->GetEmptyReadoutLengthADC();
                    }
                } else {
                    /* Decoding failed. Try next block of data. */
                    data_pos += ai->GetEmptyReadoutLengthADC();
                }
            }

            /* Finalize the mean value of the pulseheight */
            if (measurement_num > 0)
                ph_mean /= measurement_num;

            histo->SetBinContent(vcal + 1, ph_mean);
        }

        /* Cleanup */
        DisablePixel();

        delete drm;
        delete buffer;
    }
}

void PhDacOverview::DoVsfScan()
{


    int offset;
    if (((TBAnalogInterface *)tbInterface)->TBMPresent()) offset = 16;
    else offset = 9;
    int nTrig = 10;

    SetDAC("CtrlReg", 4);

    for (int col = 0; col < 2; col++)
    {
        printf("col = %d\n", col);
        for (int row = 0; row < 2; row++)
        {
            //      printf("row = %d\n",row);
            for (int vsf = 150; vsf < 255; vsf += 20)
            {
                GetDAC("Vsf");
                SetDAC("Vsf", vsf);
                Flush();
                short result[256];
                ((TBAnalogInterface *)tbInterface)->PHDac(25, 256, nTrig, offset + aoutChipPosition * 3, result);
                TH1D * histo = new TH1D(Form("Vsf%d_Col%d_Row%d", vsf, col, row), Form("Vsf%d_Col%d_Row%d", vsf, col, row), 256, 0., 256.);
                for (int dac = 0; dac < 256; dac++)
                {
                    if (result[dac] == 7777) histo->SetBinContent(dac + 1, 555);
                    else histo->SetBinContent(dac + 1, result[dac]);
                }
            }
        }
    }





}
