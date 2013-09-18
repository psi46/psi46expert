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
    TBAnalogInterface * ai = (TBAnalogInterface *) tbInterface;
    ai->Flush();

    /* ??? */
    ai->getCTestboard()->DataBlockSize(100);
    ai->Flush();

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
    ai->Flush();

    /* Set local trigger and tbm present */
    if (module->GetRoc(0)->has_analog_readout())
        ai->SetReg(41, 0x20 | 0x02);
    else
        ai->SetReg(41, 0x20 | 0x01);
    ai->Flush();

    /* Send a reset to the chip */
    ai->Single(RES);

    /* Set clock stretch */
    if (testParameters->HRPixelMapClockStretch > 1)
        ai->SetClockStretch(STRETCH_AFTER_CAL, testParameters->HRPixelMapStretchDelay, testParameters->HRPixelMapClockStretch);

    /* Get the digital and analog voltages / currents */
    psi::LogInfo() << "[HRPixelMap] Measuring chip voltages and currents ..." << psi::endl;
    TParameter<float> vd("hr_pixelmap_digital_voltage", ai->GetVD());
    TParameter<float> id("hr_pixelmap_digital_current", ai->GetID());
    TParameter<float> va("hr_pixelmap_analog_voltage", ai->GetVA());
    TParameter<float> ia("hr_pixelmap_analog_current", ai->GetIA());
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
        unsigned int data_pointer = ai->getCTestboard()->Daq_Init(30000000);

        /* Enable DMA (direct memory access) controller */
        ai->getCTestboard()->Daq_Enable();

        /* Set data aquisition to no clear buffer, multi trigger, continuous. */
        ai->DataCtrl(false, false, true);

        /* Reset the clock counter on the testboard */
        ai->SetReg(43, (1 << 1));

        /* Set the trigger frequency (f = 40000000 / (256 * n)) */
        //ai->getCTestboard()->Set(T_Periode, 5);
        ai->getCTestboard()->Set(21, testParameters->HRPixelMapTriggerRate); // T_Periode has the wrong value. Should be fixed.

        /* Issue continuous Reset-(Calibrate-)Trigger-Token pattern */
        ai->Single(RES);
        ai->Intern(CAL | TRG | TOK);

        /* Set local trigger, tbm present, and run data aquisition */
        if (module->GetRoc(0)->has_analog_readout())
            ai->SetReg(41, 0x20 | 0x02 | 0x08);
        else
            ai->SetReg(41, 0x20 | 0x01 | 0x08);

        ai->Flush();

        /* Reset the aquisition on the testboard */
        ai->SetReg(43, (1 << 0));

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
        ai->Single(RES);
        ai->Flush();

        /* Wait for data aquisition to finish */
        gDelay->Mdelay(100);

        /* Get pointer to the end of the data block */
        int data_end = ai->getCTestboard()->Daq_GetPointer();
        ai->Flush();

        /* Disable data aquisition */
        ai->SetReg(41, 0x20 | 0x02);
        ai->getCTestboard()->Daq_Disable();
        ai->DataCtrl(false, false, false);
        ai->Flush();

        /* Number of words stored in memory */
        int nwords = (data_end - data_pointer) / 2;
        psi::LogInfo() << "[HRPixelMap] Megabytes in RAM: " << nwords * 2. / 1024. / 1024. << psi::endl;

        /* Prepare data decoding */
        RAMRawDataReader rd(ai->getCTestboard(), (unsigned int) data_pointer, (unsigned int) data_pointer + 30000000, nwords * 2);

        /* Decoding chain */
        rd >> rs >> ed >> hm >> count >> mh >> phh >> end;

        /* Free the memory in the RAM */
        ai->getCTestboard()->Daq_Done();
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
        ai->SetClockStretch(0, 0, 0);

    /* Reset the chip */
    ai->Single(RES);
    ai->Flush();
}
