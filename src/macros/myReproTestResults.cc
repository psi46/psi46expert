#include <iostream.h>
#include <math.h>

#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TGraphErrors.h"
#include "TMath.h"

#include "myReproTestResults.hh"
#include "myReproTestResults.icc"


void MyReproTestResults::Init(const char *dir, const char *temp, int grades)
{
  gradeOption = grades;

  // -- Take out the \\ for T+17a
  tempNoSlash = TString(temp);
  tempNoSlash.ReplaceAll("\\", "");

  resultsFile = fopen(Form("ReproResults-%s.tex", tempNoSlash.Data()),"w");
  fFile = new TFile(Form("ReproTestresults-%s.root", tempNoSlash.Data()), "RECREATE");

  nModules = 0;

  noiseSum = new double[16*4160];
  noise2Sum = new double[16*4160];

  uthrSum = new double[16*4160];
  uthr2Sum = new double[16*4160];
  tthrSum = new double[16*4160];
  tthr2Sum = new double[16*4160];

  gainSum = new double[16*4160];
  gain2Sum = new double[16*4160];
  pedSum = new double[16*4160];
  ped2Sum = new double[16*4160];
  par1Sum = new double[16*4160];
  par12Sum = new double[16*4160];

  trimBitSum = new int[16*4160];
  trimBit2Sum = new int[16*4160];

  for (int k = 0; k < 16; k++)
  {
    for (int i = 0; i < 4160; i++)
    {
      *(noiseSum + k*4160 + i) = 0.;
      *(noise2Sum + k*4160 + i) = 0.;

      *(uthrSum + k*4160 + i) = 0.;
      *(uthr2Sum + k*4160 + i) = 0.;
      *(tthrSum + k*4160 + i) = 0.;
      *(tthr2Sum + k*4160 + i) = 0.;

      *(gainSum + k*4160 + i) = 0.;
      *(gain2Sum + k*4160 + i) = 0.;
      *(pedSum + k*4160 + i) = 0.;
      *(ped2Sum + k*4160 + i) = 0.;
      *(par1Sum + k*4160 + i) = 0.;
      *(par12Sum + k*4160 + i) = 0.;

      *(trimBitSum + k*4160 + i) = 0;
      *(trimBit2Sum + k*4160 + i) = 0;
    }
  }

  hTrimmedThresholdRMS = new TH1F("TrimmedThresholdRMS", "TrimmedThresholdRMS", 100, 0., 10.*vcalConversion);
  hUntrimmedThresholdRMS = new TH1F("UntrimmedThresholdRMS", "UntrimmedThresholdRMS", 100, 0., 10.*vcalConversion);
  hTrimBitsRMS = new TH1F("TrimBitsRMS", "TrimBitsRMS", 100, 0., 2.);

  hIV150 = new TH1F("IV150", "IV150", 100, 0.04, .08);
  hIVSlope = new TH1F("IVSlope", "IVSlope", 100, 0.8, 1.2);

  hIA = new TH1F("IA", "IA", 100, 0.35, .45);
  hID = new TH1F("ID", "ID", 100, 0.4, .5);
  hVA = new TH1F("VA", "VA", 100, 0., 5.);
  hVD = new TH1F("VD", "VD", 100, 0., 5.);

  hNoiseRMS = new TH1F("NoiseRMS", "NoiseRMS", 100, 0., 100.);

  hDead = new TH1F("dead", "dead", 100, 0., 100.);
  hMask = new TH1F("mask", "mask", 100, 0., 100.);
  hBumps = new TH1F("bumps", "bumps", 100, 0., 100.);
  hAddress = new TH1F("address", "address", 100, 0., 100.);
  hTrimBitDefects = new TH1F("trimbits", "trimbits", 100, 0., 100.);

  hGainRMS = new TH1F("GainRMS", "GainRMS", 100, 0., 0.1);
  hPedestalRMS = new TH1F("PedestalRMS", "PedestalRMS", 100, 0., 30.);

  hPar1RMS = new TH1F("Par1RMS", "Par1RMS", 100, 0., 0.1);

  for (int i = 0; i < 16; i++)
  {
    hTempVoltage[i] = new TH1F(Form("TempVoltage%i", i), Form("TempVoltage%i", i), 400, 440., 520.);
    hvdig[i] = new TH1F(Form("vdig%i", i), Form("vdig%i", i), 256, 0., 256.);
    hvana[i] = new TH1F(Form("vana%i", i), Form( "vana%i", i), 256, 0., 256.);
    hvsf[i] = new TH1F(Form("vsf%i", i), Form( "vsf%i", i), 256, 0., 256.);
    hvhlddel[i] = new TH1F(Form("vhlddel%i", i), Form( "vhlddelg%i", i), 256, 0., 256.);
    hvtrim[i] = new TH1F(Form("vtrim%i",  i), Form("vtrim%i", i), 256, 0., 256.);
    hvthrcomp[i] = new TH1F(Form("vthrcomp%i", i), Form( "vthrcomp%i", i), 256, 0., 256.);
    hvoffsetop[i] = new TH1F(Form("voffsetop%i", i), Form( "voffsetop%i", i), 256, 0., 256.);
    hvibias_ph[i] = new TH1F(Form("vibiasph%i", i), Form( "vibiasph%i", i), 256, 0., 256.);
    hibias_dac[i] = new TH1F(Form("ibiasdac%i", i), Form( "ibiasdac%i", i), 256, 0., 256.);
    hvibias_roc[i] = new TH1F(Form("ibiasroc%i", i), Form( "ibiasroc%i", i), 256, 0., 256.);
    hcaldel[i] = new TH1F(Form("caldel%i", i), Form( "caldel%i", i), 256, 0., 256.);
  }

}


void MyReproTestResults::LoopEnd()
{
  for (int k = 0; k < 16; k++)
  {
    for (int i = 0; i < 4160; i++)
    {
      hNoiseRMS->Fill(TMath::Sqrt(1./(nModules-1.)*(*(noise2Sum + k*4160 + i) - (*(noiseSum + k*4160 + i))*(*(noiseSum + k*4160 + i))/nModules)));
      hUntrimmedThresholdRMS->Fill(TMath::Sqrt(1./(nModules-1.)*(*(uthr2Sum + k*4160 + i) - (*(uthrSum + k*4160 + i))*(*(uthrSum + k*4160 + i))/nModules)));
      hTrimmedThresholdRMS->Fill(TMath::Sqrt(1./(nModules-1.)*(*(tthr2Sum + k*4160 + i) - (*(tthrSum + k*4160 + i))*(*(tthrSum + k*4160 + i))/nModules)));
      hGainRMS->Fill(TMath::Sqrt(1./(nModules-1.)*(*(gain2Sum + k*4160 + i) - (*(gainSum + k*4160 + i))*(*(gainSum + k*4160 + i))/nModules)));
      hPedestalRMS->Fill(TMath::Sqrt(1./(nModules-1.)*(*(ped2Sum + k*4160 + i) - (*(pedSum + k*4160 + i))*(*(pedSum + k*4160 + i))/nModules)));
      hPar1RMS->Fill(TMath::Sqrt(1./(nModules-1.)*(*(par12Sum + k*4160 + i) - (*(par1Sum + k*4160 + i))*(*(par1Sum + k*4160 + i))/nModules)));
      hTrimBitsRMS->Fill(TMath::Sqrt(1./(nModules-1.)*(*(trimBit2Sum + k*4160 + i) - (*(trimBitSum + k*4160 + i))*(*(trimBitSum + k*4160 + i))/nModules)));
    }
  }

  double sumRMS = 0.;
  for (int i = 0; i < 16; i++) sumRMS+=hvdig[i]->GetRMS();
  sumRMS/=16.;
  printf("RMS vdig %e\n", sumRMS);

  sumRMS = 0.;
  for (int i = 0; i < 16; i++) sumRMS+=hvana[i]->GetRMS();
  sumRMS/=16.;
  printf("RMS vana %e\n", sumRMS);

  sumRMS = 0.;
  for (int i = 0; i < 16; i++) sumRMS+=hvtrim[i]->GetRMS();
  sumRMS/=16.;
  printf("RMS vtrim %e\n", sumRMS);

  sumRMS = 0.;
  for (int i = 0; i < 16; i++) sumRMS+=hvthrcomp[i]->GetRMS();
  sumRMS/=16.;
  printf("RMS vthrcomp %e\n", sumRMS);

  sumRMS = 0.;
  for (int i = 0; i < 16; i++) sumRMS+=hTempVoltage[i]->GetRMS();
  tempVoltageRMS = sumRMS/=16.;
  printf("RMS tempVoltage %e\n", tempVoltageRMS);

}


void MyReproTestResults::Summary()
{
  printf("--------------------------------\n");
  printf("Modules %i     A: %i  B: %i  C: %i\n", nModules, nA, nB, nC);

  fprintf(resultsFile, "\\vdef{NoiseRMS} {\\ensuremath {%s}}\n", PrintNumber(hNoiseRMS->GetMean(), 2));
  fprintf(resultsFile, "\\vdef{ThresholdRMS} {\\ensuremath {%s}}\n", PrintNumber(hTrimmedThresholdRMS->GetMean()/vcalConversion, 2));
  fprintf(resultsFile, "\\vdef{GainRMS} {\\ensuremath {%s}}\n", PrintNumber(hGainRMS->GetMean(), 2));
  fprintf(resultsFile, "\\vdef{PedestalRMS} {\\ensuremath {%s}}\n", PrintNumber(vcalConversion*hPedestalRMS->GetMean(), 2));

  fprintf(resultsFile, "\\vdef{TempVoltRMS} {\\ensuremath {%s}}\n", PrintNumber(tempVoltageRMS, 2));
  fprintf(resultsFile, "\\vdef{ivRMS} {\\ensuremath {%s}}\n", PrintNumber(hIV150->GetRMS(), 2));

  fclose(resultsFile);
}


// ----------------------------------------------------------------------
void MyReproTestResults::Analyze()
{
  if (halfModule == 0) setFullModules();
  else if (halfModule == 1) setHalfModulesA();
  else setHalfModulesB();
  allFinalGrades[moduleNr] = finalGrade;
  allShortGrades[moduleNr] = shortGrade;
  allOverallGrades[moduleNr] = overallGrade;
  allGrades[moduleNr] = grade;
  allDates[moduleNr] = date;
  allAllTestsDone[moduleNr] = allTestsDone;

  if (allTestsDone != 1)
  {
    //printf("Module %i: Test not complete\n", moduleNr);
    return;
  }

  if (gradeOption == 1 && finalGrade != 1) return; //only A
  if (gradeOption == 2 && finalGrade != 2) return; //only B
  if (gradeOption == 3 && finalGrade != 3) return; //only C
  if (gradeOption == 4 && finalGrade == 3) return; //A & B
  
  nModules++;
  if (finalGrade == 1) nA++;
  else if (finalGrade == 2) nB++;
  else nC++;

  Trimming();
  DACHistos();
  IV();
  Noise();
  Defects();
  PHCalibration();
  TempVoltage();
  CurrentsVoltages();
}


// ----------------------------------------------------------------------
void MyReproTestResults::setHalfModulesA()
{
  FCHP = 0;
  LCHP = 8;
  NCHP = LCHP - FCHP;
}


// ----------------------------------------------------------------------
void MyReproTestResults::setHalfModulesB()
{
  FCHP = 8;
  LCHP = 16;
  NCHP = LCHP - FCHP;
}


// ----------------------------------------------------------------------
void MyReproTestResults::setFullModules()
{
  FCHP = 0;
  LCHP = 16;
  NCHP = LCHP - FCHP;
}


double MyReproTestResults::GetTempVoltage(int i)
{
  return temperatureVoltage[i];
}


int MyReproTestResults::GetModuleNr()
{
  return moduleNr;
}


void MyReproTestResults::DACHistos()
{  
  for (int i = FCHP; i < LCHP; i++)
  {
    hvdig[i]->Fill(caldel[i]);
    hvana[i]->Fill(vana[i]);
    hvsf[i]->Fill(vsf[i]);
    hvhlddel[i]->Fill(vhlddel[i]);
    hvtrim[i]->Fill(vtrim[i]);
    hvthrcomp[i]->Fill(vthrcomp[i]);
    hvoffsetop[i]->Fill(voffsetop[i]);
    hvibias_ph[i]->Fill(vibias_ph[i]);
    hibias_dac[i]->Fill(ibias_dac[i]);
    hvibias_roc[i]->Fill(vibias_roc[i]);
    hcaldel[i]->Fill(caldel[i]);
  }
}



void MyReproTestResults::CurrentsVoltages()
{
  hIA->Fill(ia);
  hID->Fill(id);
  hVA->Fill(va);
  hVD->Fill(vd);
}


void MyReproTestResults::Trimming()
{

  for (int i = FCHP; i < LCHP; i++)
  {
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        (*(trimBitSum + i*4160 + k))+=trims[i][k];
        (*(trimBit2Sum + i*4160 + k))+=trims[i][k]*trims[i][k];

        (*(tthrSum + i*4160 + k))+=vcalConversion*tthr[i][k];
        (*(tthr2Sum + i*4160 + k))+=vcalConversion*tthr[i][k]*vcalConversion*tthr[i][k];

        (*(uthrSum + i*4160 + k))+=vcalConversion*uthr[i][k];
        (*(uthr2Sum + i*4160 + k))+=vcalConversion*uthr[i][k]*vcalConversion*uthr[i][k];
      }
    }
  }
}


void MyReproTestResults::IV()
{
  hIV150->Fill(iv150);
  hIVSlope->Fill(ivSlope);
}


void MyReproTestResults::Noise()
{
  for (int i = FCHP; i < LCHP; i++)
  {
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        (*(noiseSum + i*4160 + k))+=noise[i][k];
        (*(noise2Sum + i*4160 + k))+=noise[i][k]*noise[i][k];
      }
    }
  }
}


void MyReproTestResults::Defects()
{
  int nDead = 0, nMask = 0, nAddress = 0, nTrim = 0, nBumps = 0;
  bool pixelOk;
  for (int i = FCHP; i < LCHP; i++)
  {
    for (int k = 0; k < 4160; k++)
    {
      pixelOk = true;
      if (pixmap[i][k] != 10) pixelOk = false;

      if (pixmap[i][k] >= 0 && pixmap[i][k] < 10) nDead++;
      if (defects[i][k] &   4) nMask++;
      if (pixelOk && defects[i][k] &   8) nBumps++;
      if (pixelOk && defects[i][k] &  16) nAddress++;
      if (pixelOk && defects[i][k] & 480) nTrim++;
    }
  }

  hDead->Fill(nDead);
  hMask->Fill(nMask);
  hBumps->Fill(nBumps);
  hAddress->Fill(nAddress);
  hTrimBitDefects->Fill(nTrim);
}


void MyReproTestResults::PHCalibration()
{
  for (int i = FCHP; i < LCHP; i++)
  {
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        (*(gainSum + i*4160 + k))+=gain[i][k];
        (*(gain2Sum + i*4160 + k))+=gain[i][k]*gain[i][k];
        (*(pedSum + i*4160 + k))+=ped[i][k];
        (*(ped2Sum + i*4160 + k))+=ped[i][k]*ped[i][k];
        (*(par1Sum + i*4160 + k))+=tanhFit[i][k][1];
        (*(par12Sum + i*4160 + k))+=tanhFit[i][k][1]*tanhFit[i][k][1];
      }
    }
  }
}

void MyReproTestResults::TempVoltage()
{
  for (int i = FCHP; i < LCHP; i++)
  {
    hTempVoltage[i]->Fill(temperatureVoltage[i]);
  }
}


char *MyReproTestResults::gradeLetter(int aGrade)
{
  if (aGrade == 1) return "A";
  else if (aGrade == 2) return "B";
  else return "C";
}


const char *MyReproTestResults::PrintNumber(Double_t number, Int_t nDigis)
{
  TString *result = new TString("");
  char string[1000];
  Int_t power = (Int_t)TMath::Floor(TMath::Log10(TMath::Abs(number)));
  //printf("power %i %i\n", power, nDigis);
  if (power >= -1 && power < nDigis)
  {
    if (nDigis - power == 1) sprintf(string, "%.0f", number);
    else if (nDigis - power == 2) sprintf(string, "%.1f", number);
    else if (nDigis - power == 3) sprintf(string, "%.2f", number);
    else if (nDigis - power == 4) sprintf(string, "%.3f", number);
  }
  else if (nDigis == 1) sprintf(string, "%.0f \\cdot 10^{%i}", number/pow(10, power), power);
  else if (nDigis == 2) sprintf(string, "%.1f \\cdot 10^{%i}", number/pow(10, power), power);
  else if (nDigis == 3) sprintf(string, "%.2f \\cdot 10^{%i}", number/pow(10, power), power);
  else if (nDigis == 4) sprintf(string, "%.3f \\cdot 10^{%i}", number/pow(10, power), power);
  result->Append(string);
  return result->Data();
}

