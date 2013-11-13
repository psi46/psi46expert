#include <iostream>
using namespace std;

#include "HighRatePixelMap.h"

#include "TestModule.h"
#include "TestRoc.h"
#include "TestPixel.h"
#include "interface/Delay.h"
#include "BasePixel/RawPacketDecoder.h"
#include "DataFilter.h"
#include "interface/Log.h"

#include <TMath.h>
#include <TParameter.h>

HRPixelMap::HRPixelMap(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    this->testParameters = testParameters;
}

HRPixelMap::~HRPixelMap()
{

}

void HRPixelMap::ModuleAction(void)
{
    tbInterface->Flush();

    /* ??? */
    tbInterface->getCTestboard()->DataBlockSize(100);
    tbInterface->Flush();

    /* Unmask the ROC */
    int nroc = module->NRocs();
    psi::LogInfo() << "[HRPixelMap] Excluding masked pixels ... " << psi::endl;
    for (int i = 0; i < nroc; i++) {
        if (!testRange->IncludesRoc(i))
            continue;
        for (int col = 0; col < ROC_NUMCOLS; col++) {
            if (!testRange->IncludesColumn(i, col))
                continue;
            for (int row = 0; row < ROC_NUMROWS; row++) {
                if (testRange->IncludesPixel(i, col, row))
                    module->GetRoc(i)->EnablePixel(col, row);
            }
        }
    }
    tbInterface->Flush();

    /* Set local trigger and tbm present */
    if (module->GetRoc(0)->has_analog_readout())
        tbInterface->SetReg(41, 0x20 | 0x02);
    else
        tbInterface->SetReg(41, 0x20 | 0x01);
    tbInterface->Flush();

    /* Send a reset to the chip */
    tbInterface->Single(RES);

    /* Set clock stretch */
    if (testParameters->HRPixelMapClockStretch > 1)
        tbInterface->SetClockStretch(STRETCH_AFTER_CAL, testParameters->HRPixelMapStretchDelay, testParameters->HRPixelMapClockStretch);

    /* Get the digital and analog voltages / currents */
    psi::LogInfo() << "[HRPixelMap] Measuring chip voltages and currents ..." << psi::endl;
    TParameter<float> vd("hr_pixelmap_digital_voltage", tbInterface->GetVD());
    TParameter<float> id("hr_pixelmap_digital_current", tbInterface->GetID());
    TParameter<float> va("hr_pixelmap_analog_voltage", tbInterface->GetVA());
    TParameter<float> ia("hr_pixelmap_analog_current", tbInterface->GetIA());
    vd.Write();
    id.Write();
    va.Write();
    ia.Write();

    float seconds = testParameters->HRPixelMapAquisitionTime;
    int repetitions = testParameters->HRPixelMapRepetitions;

    /* Data filters */
    RawData2RawEvent rs;
    RawEventDecoder ed(nroc, module->GetRoc(0)->has_analog_readout(), module->GetRoc(0)->has_row_address_inverted());
    HitMapper hm(nroc, seconds * repetitions);
    EventCounter count;
    MultiplicityHistogrammer mh;
    PulseHeightHistogrammer phh;
    ConfigParameters * configParameters = ConfigParameters::Singleton();
    phh.LoadCalibration(nroc, configParameters->directory);

    /* Repeat measurements multiple times to collect statistics */
    for (int rep = 0; rep < repetitions; rep++) {
        if (testParameters->HRPixelMapRepetitions > 1)
            psi::LogInfo() << "[HRPixelMap] Masuring iteration " << rep + 1 << "/" << testParameters->HRPixelMapRepetitions << " ..." << psi::endl;
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
        tbInterface->getCTestboard()->Set(21, testParameters->HRPixelMapTriggerRate); // T_Periode has the wrong value. Should be fixed.

        /* Issue continuous Reset-(Calibrate-)Trigger-Token pattern */
        tbInterface->Single(RES);
        tbInterface->Intern(CAL | TRG | TOK);

        /* Set local trigger, tbm present, and run data aquisition */
        if (module->GetRoc(0)->has_analog_readout())
            tbInterface->SetReg(41, 0x20 | 0x02 | 0x08);
        else
            tbInterface->SetReg(41, 0x20 | 0x01 | 0x08);

        tbInterface->Flush();

        /* Reset the aquisition on the testboard */
        tbInterface->SetReg(43, (1 << 0));

        for (float t = seconds; t >= 1; t--) {
            cout << "\r[HRPixelMap] Taking data (" << t << " seconds) ... ";
            cout.flush();
            gDelay->Mdelay(1000);
        }
        cout << "\r[HRPixelMap] Taking data (" << (seconds - (int)(seconds)) << " seconds) ... ";
        cout.flush();
        gDelay->Mdelay((int)((seconds - (int)(seconds)) * 1000));
        cout << "\r[HRPixelMap] Taking data (" << seconds << " seconds) ... done" << endl;

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
        psi::LogInfo() << "[HRPixelMap] Megabytes in RAM: " << nwords * 2. / 1024. / 1024. << psi::endl;

        /* Prepare data decoding */
        RAMRawDataReader rd(tbInterface->getCTestboard(), (unsigned int) data_pointer, (unsigned int) data_pointer + 30000000, nwords * 2);

        /* Decoding chain */
        rd >> rs >> ed >> hm >> count >> mh >> phh >> end;

        /* Free the memory in the RAM */
        tbInterface->getCTestboard()->Daq_Done();
    }

    /* Store histograms */
    int core_hits = 0;
    for (int i = -2; i < nroc; i++) {
        /* -2: Module with double sized edges, -1: Module */
        TH2I * map = (TH2I *) hm.getHitMap(i)->Clone();
        histograms->Add(map);

        /* Make dcol map and hit distribution */
        if (i >= 0) {
            for (int c = 3; c <= 50; c++) {
                for (int r = 1; r <= 79; r++) {
                    core_hits += map->GetBinContent(c, r);
                }
            }

            TH1I * dcol_map = new TH1I(Form("dcol_map_C%i", i), Form("DCol hit map ROC %i", i), 26, 0, 26);
            int x, y, z;
            map->GetMaximumBin(x, y, z);
            z = map->GetBinContent(x, y);
            TH1I * hit_dist = new TH1I(Form("hit_dist_C%i", i), Form("Hit distribution ROC %i", i), z > 100 ? 100 : z, 0, z);
            for (int dcol = 0; dcol < 26; dcol++) {
                int sum = 0;
                for (int row = 0; row < 80; row++) {
                    sum += map->GetBinContent(2 * dcol + 1, row + 1);
                    sum += map->GetBinContent(2 * dcol + 2, row + 1);
                    hit_dist->Fill(map->GetBinContent(2 * dcol + 1, row + 1));
                    hit_dist->Fill(map->GetBinContent(2 * dcol + 2, row + 1));
                }
                dcol_map->SetBinContent(dcol + 1, sum);
            }
            dcol_map->Sumw2();
            dcol_map->SetMinimum(0);
            histograms->Add(dcol_map);
            histograms->Add(hit_dist);

            TH1I * multi = (TH1I *) mh.getRocMultiplicity(i)->Clone();
            histograms->Add(multi);
        }
    }
    histograms->Add((TH1I *) hm.getHitsVsTimeDcol()->Clone());
    histograms->Add((TH1I *) hm.getHitsVsTimeRoc()->Clone());
    histograms->Add((TH1I *) phh.getPulseHeightDistribution()->Clone());
    histograms->Add((TH2F *) phh.getPulseHeightMap()->Clone());
    histograms->Add((TH2F *) phh.getPulseHeightWidthMap()->Clone());
    histograms->Add((TH1I *) phh.getCalPulseHeightDistribution()->Clone());
    histograms->Add((TH2F *) phh.getCalPulseHeightMap()->Clone());
    histograms->Add((TH2F *) phh.getCalPulseHeightWidthMap()->Clone());

    float active_area = nroc * (52 - 2 * 2) * (80 - 1) * 0.01 * 0.015; /* cm2 */
    float active_time = count.TriggerCounter * 25e-9; /* s */
    if (testParameters->HRPixelMapClockStretch > 1)
        active_time *= testParameters->HRPixelMapClockStretch;
    TH2I * map = (TH2I *) hm.getHitMap(-1);
    psi::LogInfo() << "[HRPixelMap] Number of triggers: " << count.TriggerCounter << psi::endl;
    psi::LogInfo() << "[HRPixelMap] Number of hits: " << map->GetEntries() << psi::endl;
    psi::LogInfo() << "[HRPixelMap] Rate: " << (core_hits / active_time / active_area / 1e6);
    psi::LogInfo() << " +/- " << (TMath::Sqrt(core_hits) / active_time / active_area / 1e6);
    psi::LogInfo() << " megahits / s / cm2" << psi::endl;
    psi::LogInfo() << "[HRPixelMap] Number of ROC sequence problems: " << count.RocSequenceErrorCounter << psi::endl;
    psi::LogInfo() << "[HRPixelMap] Number of decoding problems: " << ed.GetDecodingErrors() << psi::endl;

    TParameter<float> triggers("pixelmap_triggers", count.TriggerCounter);
    triggers.Write();

    /* Disable clock stretch */
    if (testParameters->HRPixelMapClockStretch > 1)
        tbInterface->SetClockStretch(0, 0, 0);

    /* Reset the chip */
    tbInterface->Single(RES);
    tbInterface->Flush();
}
