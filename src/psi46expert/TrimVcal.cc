#include "interface/Log.h"

#include "TrimVcal.h"
#include "TestRoc.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBInterface.h"
#include "Analysis.h"
#include "ThresholdMap.h"


TrimVcal::TrimVcal(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
    debug = true;
}


void TrimVcal::ReadTestParameters(TestParameters * testParameters)
{
    nTrig = (*testParameters).TrimNTrig;
}


void TrimVcal::AddMap(TH2D * calMap)
{
    TH1D * distr = gAnalysis->Distribution(calMap, 255, 0., 255.);
    calMap->Write();
    distr->Write();
    histograms->Add(calMap);
    histograms->Add(distr);
}


void TrimVcal::RocAction()
{
    psi::LogInfo() << "[TrimVcal] Roc #" << chipId << ": Start." << psi::endl;

    gDelay->Timestamp();
    SaveDacParameters();

    roc->SetTrim(15);
    SetDAC("Vtrim", 0);
    SetDAC("Vcal", 200);
    tbInterface->SetEnableAll(0);
    Flush();
    //
    //  //Find good VthrComp
    thresholdMap = new ThresholdMap();
    TH2D * calMap = thresholdMap->GetMap("NoiseMap", roc, testRange, nTrig);
    AddMap(calMap);
    TH1D * distr = gAnalysis->Distribution(calMap, 255, 1., 254.);
    double thrMinLimit = TMath::Max(1., distr->GetMean() - 5.*distr->GetRMS());

    double thrMin = 255., thr;
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            if (testRange->IncludesPixel(roc->GetChipId(), i, k))
            {
                thr = calMap->GetBinContent(i + 1, k + 1);
                if ((thr < thrMin) && (thr > thrMinLimit)) thrMin = thr;
            }
        }
    }
    SetDAC("VthrComp", (int)thrMin - 10);
    Flush();

    psi::LogDebug() << "[TrimVcal] Minimum Threshold is " << thrMin << psi::endl;
    psi::LogDebug() << "[TrimVcal] VtrhComp is set to "
                    << static_cast<int>(thrMin - 10) << psi::endl;

    //  SetDAC("VthrComp", 120);
    SetDAC("Vtrim", 120);
    Flush();

    short trim[ROCNUMCOLS * ROCNUMROWS];
    for (int i = 0; i < ROCNUMCOLS * ROCNUMROWS; i++) trim[i] = 15;

    /*  TH2D *effMap;
        bool noisy[ROCNUMCOLS], allNoisy;
        double data[ROC_NUMROWS*ROC_NUMCOLS];
        double sum;*/

    //  tbInterface->SetEnableAll(1);
    //  for (int iRow = 0; iRow < 5; iRow++)
    //  {
    //      roc->SetTrim(15);
    //      for (int iCol = 0; iCol < ROCNUMCOLS; iCol++) noisy[iCol] = false;
    //      for (int trimbit = 15; trimbit >= 1; trimbit--)
    //      {
    //          printf("%i %i\n", iRow, trimbit);
    //          effMap = GetMap(Form("EffMapRow%iTrim%i", iRow, trimbit));
    //          roc->ChipEfficiency(nTrig, data);
    //
    //          for (int iCol = 0; iCol < ROCNUMCOLS; iCol++)
    //          {
    //              sum = 0.;
    //              for (int i = 0; i < ROCNUMROWS; i++)
    //              {
    //                  effMap->SetBinContent(iCol+1, i+1, data[iCol*ROCNUMROWS + i]);
    //                  sum+=data[iCol*ROCNUMROWS + i];
    // //                   if (data[iCol*ROCNUMROWS + i] > 1.1) noisy[iCol] = true;
    //              }
    //              if (sum > 1.02 * ROCNUMROWS) noisy[iCol] = true;
    //              if (!noisy[iCol]) trim[iCol][iRow] = trimbit - 1;
    //              else if (trim[iCol][iRow] == trimbit) trim[iCol][iRow]++;
    //              roc->SetTrim(iCol, iRow, trim[iCol][iRow]);
    //          }
    //          AddMap(effMap);
    //          AddMap(roc->TrimMap());
    //
    //          allNoisy = true;
    //          for (int iCol = 0; iCol < ROCNUMCOLS; iCol++) if (!noisy[iCol]) allNoisy = false;
    //          if (allNoisy) break;
    //      }
    //  }

    //  short data[10000];
    //  unsigned short count;
    //         int noise = 0, length;
    //  int nReadouts, readoutStart[256];
    //  int calRow, calCol;
    //
    //  roc->SetTrim(15);
    //  roc->EnableAllPixels();
    //  for (int iCol = 0; iCol < ROCNUMCOLS; iCol++)
    //  {
    //      for (int iRow = 0; iRow < 2; iRow++)
    //      {
    //          for (int trimbit = 15; trimbit >= 0; trimbit--)
    //          {
    //              roc->SetTrim(iCol, iRow, trimbit);
    //              roc->EnablePixel(iCol, iRow);
    //
    //              if (iRow != ROCNUMROWS - 1) calRow = iRow + 1;
    //              else calRow = iRow - 1;
    //              calCol = iCol;
    //
    //
    // //               calRow = iRow;
    // //               if (iCol % 2 == 0) calCol = iCol + 1;
    // //               else calCol = iCol -1;
    //
    //              roc->Cal(calCol, calRow);
    //
    //              noise = 0;
    //              for (int i = 0; i < 4; i++)
    //              {
    //                  SendADCTrigs(10);
    //                  Flush();
    //                  GetADC(data, FIFOSIZE, count, 10, readoutStart, nReadouts);
    //                  for (int k = 0; k < nReadouts; k++)
    //                  {
    //                      if (k == nReadouts - 1) length = count - readoutStart[k];
    //                      else length = readoutStart[k+1] - readoutStart[k];
    //                      if (length > tbInterface->GetEmptyReadoutLengthADC() + 6) noise++;
    //                  }
    //              }
    // //               printf("%i %i trimbit %i noise %i\n", iCol, iRow, trimbit, noise);
    //              if (noise > 1.)
    //              {
    //                  trim[iCol*ROCNUMROWS + iRow] = trimbit + 2;
    //                  if (trim[iCol*ROCNUMROWS + iRow] > 15) trim[iCol*ROCNUMROWS + iRow] = 15;
    //                  break;
    //              }
    //              else if (trimbit == 0)
    //              {
    //                  trim[iCol*ROCNUMROWS + iRow] = 0;
    //              }
    //
    //          }
    //          roc->SetTrim(iCol, iRow, 15);
    //          roc->EnablePixel(iCol, iRow);
    //          roc->ClrCal();
    //          printf("%i %i trimbit %i noise %i\n", iCol, iRow, trim[iCol*ROCNUMROWS + iRow], noise);
    //
    //      }
    //  }

    int mode = 2; //mode 0: no cal 1: cal in same column 2: cal in same doublecolumn
    roc->TrimAboveNoise(10, 1, mode, trim);

    //Determine Vcal

    tbInterface->SetEnableAll(0);
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            //          printf("%i ",trim[i*ROCNUMROWS + k]);
            roc->SetTrim(i, k, trim[i * ROCNUMROWS + k]);
        }
    }

    thresholdMap->SetDoubleWbc();
    calMap = thresholdMap->GetMap("VcalThresholdMap", roc, testRange, nTrig);
    AddMap(calMap);
    distr = gAnalysis->Distribution(calMap, 255, 1., 254.);
    double vcalMaxLimit = TMath::Min(254., distr->GetMean() + 5.*distr->GetRMS());

    double vcalMin = 255., vcalMax = 0.;
    int thr255 = 0;
    for (int i = 0; i < ROCNUMCOLS; i++)
    {
        for (int k = 0; k < ROCNUMROWS; k++)
        {
            if (testRange->IncludesPixel(roc->GetChipId(), i, k))
            {
                thr = calMap->GetBinContent(i + 1, k + 1);
                if ((thr > vcalMax) && (thr < vcalMaxLimit)) vcalMax = thr;
                if ((thr < vcalMin) && (thr > 1.)) vcalMin = thr;
                if (thr == 255.) thr255++;
            }
        }
    }

    psi::LogDebug() << "[TrimVcal] There are " << thr255 << " pixels with "
                    << "Vcal 255." << psi::endl;
    psi::LogDebug() << "[TrimVcal] Vcal range is [ " << vcalMin << ", "
                    << vcalMax << "]." << psi::endl;

    RestoreDacParameters();
    gDelay->Timestamp();
}

