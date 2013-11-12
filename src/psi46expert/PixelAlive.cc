#include "interface/Log.h"

#include "PixelAlive.h"
#include "TestRoc.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBInterface.h"

PixelAlive::PixelAlive(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    psi::LogDebug() << "[PixelAlive] Initialization." << psi::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
}


void PixelAlive::ReadTestParameters(TestParameters * testParameters)
{
    nTrig = (*testParameters).PixelMapReadouts;
    efficiency = (double)(*testParameters).PixelMapEfficiency / 100. ;
}


void PixelAlive::RocAction()
{
    psi::LogInfo() << "[PixelAlive] Measuring calibration efficiency for Roc " << chipId << " ..." << psi::endl;

    TH2D * histo = GetMap("PixelMap");
    histo->SetMaximum(nTrig);
    histo->SetMinimum(0);

    short mask[ROC_NUMROWS * ROC_NUMCOLS];
    roc->MaskTest(1, mask);

    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            int n = mask[i * ROCNUMROWS + k];
            if (n != 0)
            {
                GetPixel(i, k)->SetAlive(false);

                psi::LogInfo() << "[PixelAlive] Warning: Mask Defect. n = " << n
                               << " for Pixel( " << i << ", " << k << ")." << psi::endl;

                histo->SetBinContent(i + 1, k + 1, -1);
            }
            else histo->SetBinContent(i + 1, k + 1, 0);
        }
    }

    double data[ROC_NUMROWS * ROC_NUMCOLS];
    roc->ChipEfficiency(nTrig, data);

    for (int i = 0; i < ROC_NUMROWS * ROC_NUMCOLS; i++)
    {
        double value = data[i] * nTrig;
        if (value == 0)
            psi::LogInfo() << "[PixelAlive] Warning: Dead Pixel( "
                           << (i / ROCNUMROWS) << ", " << (i % ROCNUMROWS)
                           << ") with n = " << static_cast<int>(value) << psi::endl;
        if (value < 0) value = -2;  // to distinguish this problem from mask defects
        if (histo->GetBinContent(i / ROC_NUMROWS + 1, i % ROC_NUMROWS + 1) == 0) histo->SetBinContent(i / ROC_NUMROWS + 1, i % ROC_NUMROWS + 1, value);
    }
    histo->GetXaxis()->SetTitle("Column");
    histo->GetYaxis()->SetTitle("Row");
    histo->GetZaxis()->SetTitle("Readouts");

    histograms->Add(histo);
}
