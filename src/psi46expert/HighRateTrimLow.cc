#include <iostream>
using namespace std;

#include "HighRateTrimLow.h"

#include "TestModule.h"
#include "TestRoc.h"
#include "TestPixel.h"
#include "interface/Delay.h"
#include "BasePixel/RawPacketDecoder.h"
#include "DataFilter.h"
#include "interface/Log.h"

#include <TMath.h>

HRTrimLow::HRTrimLow(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
}

HRTrimLow::~HRTrimLow()
{

}

void HRTrimLow::RocAction(void)
{

    /* Set trim bits all to 1111=0xf */
    roc->SetTrim(0xf);
    roc->SetDAC("Vtrim", 130);

    /* Unmask the ROC */
    roc->EnableAllPixels();
    tbInterface->Flush();

    /* Find lowest working threshold (highest value of VthrComp) */
    const int start = 110;
    int threshold;
    int increment = 5;
    int breakdown = 130;
    float previous_rate = 0.0;
    TH1F * threshold_scan = new TH1F("trim_low_threshold_scan", "Threshold scan", breakdown - start, start, breakdown);
    for (threshold = start; threshold < breakdown; threshold += increment) {
        cout << endl;
        psi::LogInfo() << "Setting threshold (VcThr) to " << threshold << psi::endl;
        roc->SetDAC("VthrComp", threshold);

        /* Measure the map */
        MakeMap();

        /* Add histogram */
        map->SetNameTitle(Form("hitmap-vcthr-%i", threshold), Form("Hit map VcThr=%i", threshold));
        map->SetMinimum(0);
        map->SetMaximum(100);
        histograms->Add(map);

        /* Calculate rate */
        float rate = (map->GetEntries() / triggers) * 40e6 / 1e6 / (0.79 * 0.77);
        float rate_error = (TMath::Sqrt(map->GetEntries()) / triggers) * 40e6 / 1e6 / (0.79 * 0.77);
        threshold_scan->SetBinContent(threshold - start + 1, rate);
        threshold_scan->SetBinError(threshold - start + 1, rate_error);
        psi::LogInfo() << "Rate: " << rate;
        psi::LogInfo() << " megahits / s / cm2" << psi::endl;

        /* Breakdown reached? */
        if (rate < previous_rate) {
            if (increment == 5) {
                breakdown = threshold;
                threshold -= increment;
                increment = 2;
            } else if (increment == 2) {
                breakdown = threshold;
                threshold -= increment;
                increment = 1;
            } else {
                breakdown = threshold;
                break;
            }
        } else {
            previous_rate = rate;
        }
    }
    threshold_scan->GetXaxis()->SetTitle("VcThr");
    threshold_scan->GetYaxis()->SetTitle("Rate [MHz/cm2]");
    histograms->Add(threshold_scan);

    /* Set threshold to a safe distance from breakdown */
    if (threshold >= breakdown)
        threshold = breakdown - 1;
    threshold -= 2;
    roc->SetDAC("VthrComp", threshold);

    cout << endl;
    psi::LogInfo() << "Lowest value of threshold (VthrComp/VcThr): " << threshold << psi::endl;

    TH2I * trim_bits = new TH2I("trimbits", "Trim bits", 52, 0, 52, 80, 0, 80);
    for (int col = 0; col < 52; col++) {
        for (int row = 0; row < 80; row++) {
            trim_bits->SetBinContent(col + 1, row + 1, 15);
        }
    }

    Test::RocAction();

    /* Unmask the ROC (again) */
    roc->EnableAllPixels();
    tbInterface->Flush();

    /* Make final map of trimmed chip */
    MakeMap();
    map->SetNameTitle(Form("trim_low_pixelmap_final", row), Form("Pixelmap trimmed", row));
    map->SetMinimum(0);
    map->SetMaximum(100);
    histograms->Add(map->Clone());
    histograms->Add(roc->TrimMap());
}

#define COLA 0
#define COLB 52

void HRTrimLow::PixelAction(void)
{
    /* Ignore all pixels except some */
    if (column != 26 || row < 40 || row > 45)
        return;
    if (column != 26 || row != 41)
        return;

    roc->EnableAllPixels();

    cout << endl;
    cout << "Testing pixel " << column << " " << row << endl;

    TH1F * trim_scan [52];
    int trim [52];
    int increment[52];
    int nlast [52];
    int breakdown [52];
    float last [52];

    for (int col = 0; col < 52; col++) {
        trim[col] = 15;
        increment[col] = 4;
        last[col] = 0;
        nlast[col] = 0;
        breakdown[col] = -1;
        trim_scan[col] = new TH1F(Form("trim_scan_col%i_row%i", col, row), Form("Trim bit scan col=%i row=%i", col, row), 16, 0, 16);
    }

    while (1) {
        bool done = true;
        cout << endl;
        for (int col = COLA; col < COLB; col += 1) {
            if (trim[col] > breakdown[col]) {
                cout << "Setting trim bit of pixel " << col << ":" << row << " to " << trim[col] << endl;
                tbInterface->RocPixTrim(col, row, trim[col]);
            }
            done = done && (trim[col] <= breakdown[col]);
        }

        if (done)
            break;

        MakeMap();

        map->SetNameTitle(Form("trim_low_pixelmap_row%i", row), Form("Pixelmap Row=%i", row));
        histograms->Add(map->Clone());

        for (int col = COLA; col < COLB; col += 1) {

            if (trim[col] <= breakdown[col])
                continue;

            trim_scan[col]->SetBinContent(trim[col] + 1, map->GetBinContent(col + 1, row + 1));
            trim_scan[col]->SetBinError(trim[col] + 1, map->GetBinError(col + 1, row + 1));

            int current = map->GetBinContent(col + 1, row + 1);
            cout << "Hits pixel " << col << ":" << row << " : " << current << " (last=" << last[col] << ")" << endl;
            if (trim[col] == 15) {
                last[col] = current;
                nlast[col]++;
            } else {
                if (current > 3 * last[col]) {
                    if (increment[col] == 4) {
                        breakdown[col] = trim[col];
                        trim[col] += increment[col];
                        increment[col] = 2;
                    } else if (increment[col] == 2) {
                        breakdown[col] = trim[col];
                        trim[col] += increment[col];
                        increment[col] = 1;
                    } else {
                        breakdown[col] = trim[col];
                    }
                } else {
                    if (increment[col] == 4 && trim[col] - increment[col] <= breakdown[col])
                        increment[col] = 2;
                    if (increment[col] == 2 && trim[col] - increment[col] <= breakdown[col])
                        increment[col] = 1;
                    last[col] = current + nlast[col] * last[col];
                    last[col] /= ++nlast[col];
                }
            }

            trim[col] -= increment[col];

        }
    }

    cout << endl;

    for (int col = COLA; col < COLB; col += 1) {
        cout << "Found trim bit of pixel " << col << ":" << row << " : " << breakdown[col] + 1 << endl;
        histograms->Add(trim_scan[col]);
        tbInterface->RocPixTrim(col, row, breakdown[col] + 1);
        roc->SetTrim(col, row, breakdown[col] + 1);
    }
}

/* Takes x-ray data */
void HRTrimLow::MakeMap(void)
{
    tbInterface->Flush();

    /* ??? */
    tbInterface->getCTestboard()->DataBlockSize(100);
    tbInterface->Flush();

    /* Set local trigger and tbm present */
    tbInterface->SetReg(41, 0x20 | 0x02);
    tbInterface->Flush();

    /* Send a reset to the chip */
    tbInterface->Single(RES);

    /* Prepare the data aquisition (store to testboard RAM) */
    unsigned int data_pointer = tbInterface->getCTestboard()->Daq_Init(30000000);

    /* Enable DMA (direct memory access) controller */
    tbInterface->getCTestboard()->Daq_Enable();

    /* Set data aquisition to no clear buffer, multi trigger, continuous. */
    tbInterface->DataCtrl(false, false, true);

    /* Reset the clock counter on the testboard */
    tbInterface->SetReg(43, (1 << 1));

    /* Set the trigger frequency (f = 40000000 / (256 * n)) */
    //tbInterface->getCTestboard()->Set(T_Periode, 5);
    tbInterface->getCTestboard()->Set(21, 5); // T_Periode has the wrong value. Should be fixed.

    /* Issue continuous Reset-(Calibrate-)Trigger-Token pattern */
    tbInterface->Intern(TRG | TOK);

    /* Set local trigger, tbm present, and run data aquisition */
    tbInterface->SetReg(41, 0x20 | 0x02 | 0x08);
    tbInterface->Flush();

    /* Reset the aquisition on the testboard */
    tbInterface->SetReg(43, (1 << 0));

    float seconds = 1;
    for (float t = seconds; t > 0; t--) {
        cout << "\rTaking data (" << t << " seconds) ... ";
        cout.flush();
        gDelay->Mdelay(1000);
    }
    cout << "\r";
    cout.flush();
    gDelay->Mdelay((int)((seconds - (int)(seconds)) * 1000));

    /* Stop triggering */
    tbInterface->Single(RES);
    tbInterface->Flush();

    /* Wait for data aquisition to finish */
    gDelay->Mdelay(100);

    /* Get pointer to the end of the data block */
    int data_end = tbInterface->getCTestboard()->Daq_GetPointer();
    tbInterface->Flush();

    /* Disable data aquisition */
    tbInterface->SetReg(41, 0x20 | 0x02);
    tbInterface->getCTestboard()->Daq_Disable();
    tbInterface->DataCtrl(false, false, false);
    tbInterface->Flush();

    /* Number of words stored in memory */
    int nwords = (data_end - data_pointer) / 2;
    psi::LogInfo() << "Megabytes in RAM: " << nwords * 2. / 1024. / 1024. << psi::endl;

    /* Prepare data decoding */
    RAMRawDataReader rd(tbInterface->getCTestboard(), (unsigned int) data_pointer, (unsigned int) data_pointer + 30000000, nwords * 2);
    RawData2RawEvent rs;
    RawEventDecoder ed(1, roc->has_analog_readout(), roc->has_row_address_inverted());
    HitMapper hm(1, seconds);
    EventCounter count;

    /* Decoding chain */
    rd >> rs >> ed >> hm >> count >> pipe_end;

    /* Store histogram and number of triggers */
    map = (TH2I *) hm.getHitMap(0)->Clone();
    triggers = count.TriggerCounter;

    /* Free the memory in the RAM */
    tbInterface->getCTestboard()->Daq_Done();

    /* Reset the chip */
    tbInterface->Single(RES);
    tbInterface->Flush();
}
