#include <iostream>
using namespace std;

#include "HighRateSCurve.h"

#include "TestModule.h"
#include "TestRoc.h"
#include "TestPixel.h"
#include "interface/Delay.h"
#include "BasePixel/RawPacketDecoder.h"
#include "DataFilter.h"
#include "interface/Log.h"

#include <TMath.h>
#include <TParameter.h>

HRSCurve::HRSCurve(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    this->testParameters = testParameters;

    rough_threshold = NULL;
    efficiency_map = NULL;
}

HRSCurve::~HRSCurve()
{
    if (rough_threshold)
        delete rough_threshold;
    if (efficiency_map)
        delete efficiency_map;
}

void HRSCurve::ModuleAction(void)
{
    rough_threshold = new TH2I * [module->NRocs()];
    efficiency_map = new TH2I * [module->NRocs()];

    for (int iroc = 0; iroc < module->NRocs(); iroc++) {
        rough_threshold[iroc] = new TH2I(Form("rough_threshold_C%i", iroc), Form("Rough VCal threshold ROC %i", iroc), 52, 0, 52, 80, 0, 80);
        for (int col = 0; col < 52; col++) {
            for (int row = 0; row < 80; row++) {
                rough_threshold[iroc]->SetBinContent(col + 1, row + 1, -1);
            }
        }
    }

    /* Get the digital and analog voltages / currents */
    psi::LogInfo() << "Measuring chip voltages and currents ..." << psi::endl;

    TParameter<float> vd("hr_scurve_digital_voltage", tbInterface->GetVD());
    TParameter<float> id("hr_scurve_digital_current", tbInterface->GetID());
    TParameter<float> va("hr_scurve_analog_voltage", tbInterface->GetVA());
    TParameter<float> ia("hr_scurve_analog_current", tbInterface->GetIA());
    vd.Write();
    id.Write();
    va.Write();
    ia.Write();

    psi::LogInfo() << "Determining rough threshold" << psi::endl;
    for (int vcal = testParameters->HRSCurveThrStart; vcal <= testParameters->HRSCurveThrEnd; vcal += 4) {
        psi::LogInfo() << "Testing vcal " << vcal << " ..." <<  psi::endl;
        for (int iroc = 0; iroc < module->NRocs(); iroc++)
            module->GetRoc(iroc)->SetDAC("Vcal", vcal);
        TakeEfficiencyMap(4, false, 0);
        for (int iroc = 0; iroc < module->NRocs(); iroc++) {
            efficiency_map[iroc]->SetNameTitle(Form("effmap_vcal%i_C%i", vcal, iroc), Form("Efficiency map VCal %i ROC %i", vcal, iroc));
            histograms->Add(efficiency_map[iroc]);
            for (int col = 0; col < 52; col++) {
                for (int row = 0; row < 80; row++) {
                    if (efficiency_map[iroc]->GetBinContent(col + 1, row + 1) >= 3 && rough_threshold[iroc]->GetBinContent(col + 1, row + 1) == -1)
                        rough_threshold[iroc]->SetBinContent(col + 1, row + 1, vcal);
                }
            }
        }
    }

    psi::LogInfo() << "Determining SCurve" << psi::endl;
    for (int offset = -16; offset <= 16; offset++) {
        psi::LogInfo() << "Testing offset " << offset << " ..." <<  psi::endl;
        TakeEfficiencyMap(testParameters->HRSCurveTriggers, true, offset);
        for (int iroc = 0; iroc < module->NRocs(); iroc++) {
            efficiency_map[iroc]->SetNameTitle(Form("effmap_offset%i_C%i", offset, iroc), Form("Efficiency map offset %i ROC %i", offset, iroc));
            histograms->Add(efficiency_map[iroc]);
        }
    }

    for (int iroc = 0; iroc < module->NRocs(); iroc++) {
        rough_threshold[iroc]->SetMinimum(testParameters->HRSCurveThrStart - 1);
        rough_threshold[iroc]->SetMaximum(testParameters->HRSCurveThrEnd);
        histograms->Add(rough_threshold[iroc]);
    }
}

void HRSCurve::TakeEfficiencyMap(int ntrig, bool set_vcal, int vcal_offset)
{
    tbInterface->Flush();

    /* ??? */
    tbInterface->getCTestboard()->DataBlockSize(100);
    tbInterface->Flush();

    /* Unmask ROC */
    for (int iroc = 0; iroc < module->NRocs(); iroc++)
        module->GetRoc(iroc)->EnableAllPixels();
    tbInterface->Flush();

    /* Set local trigger and tbm present */
    tbInterface->SetReg(41, 0x20 | 0x02);
    tbInterface->Flush();

    /* Send a reset to the chip */
    tbInterface->Single(RES);
    tbInterface->Flush();
    gDelay->Mdelay(10);

    /* Prepare the data aquisition (store to testboard RAM) */
    unsigned int data_pointer = tbInterface->getCTestboard()->Daq_Init(30000000);

    /* Enable DMA (direct memory access) controller */
    tbInterface->getCTestboard()->Daq_Enable();

    /* Set data aquisition to no clear buffer, multi trigger, continuous. */
    tbInterface->DataCtrl(false, false, true);

    /* Reset the clock counter on the testboard */
    tbInterface->SetReg(43, (1 << 1));

    /* Set local trigger, tbm present, and run data aquisition */
    if (module->GetRoc(0)->has_analog_readout())
        tbInterface->SetReg(41, 0x20 | 0x02 | 0x08);
    else
        tbInterface->SetReg(41, 0x20 | 0x01 | 0x08);
    tbInterface->Flush();

    /* Reset the aquisition on the testboard */
    tbInterface->SetReg(43, (1 << 0));

    /* iterate over columns and rows to get each pixel efficiency */
    for (int col = 0; col < 52; col++) {
        for (int row = 0; row < 80; row++) {
            /* Arm the pixel */
            for (int iroc = 0; iroc < module->NRocs(); iroc++) {
                module->GetRoc(iroc)->ArmPixel(col, row);
                if (set_vcal)
                    module->GetRoc(iroc)->SetDAC("Vcal", rough_threshold[iroc]->GetBinContent(col + 1, row + 1) + vcal_offset);
            }
            tbInterface->CDelay(5000);
            tbInterface->Flush();

            /* send ntrig triggers with calibrates */
            for (int t = 0; t < ntrig; t++) {
                tbInterface->Single(RES | CAL | TRG | TOK);
                tbInterface->CDelay(500);
            }
            tbInterface->Flush();

            /* Disarm the pixel, but leave it enabled */
            for (int iroc = 0; iroc < module->NRocs(); iroc++) {
                module->GetRoc(iroc)->DisarmPixel(col, row);
                module->GetRoc(iroc)->EnablePixel(col, row);
            }
            tbInterface->Flush();
        }
    }

    /* Stop triggering */
    tbInterface->Single(RES);
    tbInterface->Flush();

    /* Wait for data aquisition to finish */
    gDelay->Mdelay(100);

    /* Get pointer to the end of the data block */
    int data_end = tbInterface->getCTestboard()->Daq_GetPointer();
    tbInterface->Flush();

    /* Disable data aquisition */
    if (module->GetRoc(0)->has_analog_readout())
        tbInterface->SetReg(41, 0x20 | 0x02);
    else
        tbInterface->SetReg(41, 0x20 | 0x01);
    tbInterface->getCTestboard()->Daq_Disable();
    tbInterface->DataCtrl(false, false, false);
    tbInterface->Flush();

    /* Number of words in memory */
    int nwords = (data_end - data_pointer) / 2;

    /* Prepare data decoding */
    RAMRawDataReader rd(tbInterface->getCTestboard(), (unsigned int) data_pointer, (unsigned int) data_pointer + 30000000, nwords * 2);
    RawData2RawEvent rs;
    RawEventDecoder ed(module->NRocs(), module->GetRoc(0)->has_analog_readout(), module->GetRoc(0)->has_row_address_inverted());
    EfficiencyMapper em(module->NRocs(), ntrig);

    /* Decoding chain */
    rd >> rs >> ed >> em >> pipe_end;

    /* Store histograms */
    for (int iroc = 0; iroc < module->NRocs(); iroc++) {
        efficiency_map[iroc] = (TH2I *) em.getEfficiencyMap(iroc)->Clone();
        efficiency_map[iroc]->SetMinimum(0);
        efficiency_map[iroc]->SetMaximum(ntrig);
    }
    TH1I * effdist = (TH1I *) em.getEfficiencyDist(-1);
    TH2I * bkgmap = (TH2I *) em.getBackgroundMap(-1);
    psi::LogInfo() << "Rate: " << (bkgmap->GetEntries() / (ntrig * 4160)) * 40e6 / 1e6 / (0.79 * 0.77 * module->NRocs());
    psi::LogInfo() << " +/- " << (TMath::Sqrt(bkgmap->GetEntries()) / (ntrig * 4160)) * 40e6 / 1e6 / (0.79 * 0.77 * module->NRocs());
    psi::LogInfo() << " megahits / s / cm2" << psi::endl;
    psi::LogInfo() << "Overall efficiency: " << effdist->GetMean() << " %" << psi::endl;

    /* Free the memory in the RAM */
    tbInterface->getCTestboard()->Daq_Done();

    /* Reset the chip */
    tbInterface->Single(RES);
    tbInterface->Flush();
}
