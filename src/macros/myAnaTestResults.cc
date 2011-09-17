#include <iostream.h>
#include <math.h>

#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TProfile.h"
#include "TProfile2D.h"
#include "TGraphErrors.h"
#include "TMath.h"

#include "myAnaTestResults.hh"
#include "myAnaTestResults.icc"


void MyAnaTestResults::Init(const char *dir, const char *temp, int grades)
{
  gradeOption = grades;
  if (gradeOption == 5) ReadModuleList();

  // -- Take out the \\ for T+17a
  tempNoSlash = TString(temp);
  tempNoSlash.ReplaceAll("\\", "");

  resultsFile = fopen(Form("Results-%s.tex", tempNoSlash.Data()),"w");
  moduleTableFile = fopen(Form("ModuleTable-%s.txt", tempNoSlash.Data()),"w");
  fFile = new TFile(Form("AnaTestresults-%s.root", tempNoSlash.Data()), "RECREATE");

  for (int i = 0; i < 2000; i++)
  {
    allGrades[i] = 0;
    allOverallGrades[i] = 0;
    allFinalGrades[i] = 0;
    allShortGrades[i] = 0;
    allDates[i] = 0;
    allAllTestsDone[i] = 0;
  }

  gainRMSDC = 0.;
  pedRMSDC = 0.;
  nPHDC = 0;

  nModules = 0;
  nHalfModulesA = 0;
  nHalfModulesB = 0;
  nA = 0;
  nB = 0;
  nC = 0;

  nAllPixels = 0;
  nDeadPixels = 0;
  nGoodBumps = 0;
  nBadBumps = 0;
  nAddressProblems = 0;

  nTrim1Defects = 0;
  nTrim2Defects = 0;
  nTrim3Defects = 0;
  nTrim4Defects = 0;

  hCaldel = new TH1F("CalDel", "CalDel", 256, 0., 256.);
  hBumpbonding = new TH1F("Bumpbonding", "Bumpbonding", 512, -256., 256.);
  hBumpbondingM0086 = new TH2F("BumpbondingM0086", "BumpbondingM0086", 416, 0., 416., 160, 0., 160.);
  hTrimBitDefects = new TH1F("TrimBitDefects", "TrimBitDefects", 4160, 0., 4160.);
  
  hTrimMean = new TH1F("TrimMean", "TrimMean", 100, 3500., 4200.);
  hTrimRMS = new TH1F("TrimRMS", "TrimRMS", 100, 0., 600.);
  hTrimmedThreshold = new TH1F("TrimmedThreshold", "TrimmedThreshold", 256, 0., 256.*vcalConversion);
  hUntrimmedThreshold = new TH1F("UntrimmedThreshold", "UntrimmedThreshold", 256, 0., 256.*vcalConversion);
  hTrimBits = new TH1F("TrimBits", "TrimBits", 16, 0., 16.);

  hIV150 = new TH1F("IV150", "IV150", 100, 0., 5.);
  hIVSlope = new TH1F("IVSlope", "IVSlope", 100, 0., 10.);
  
  hIA = new TH1F("IA", "IA", 100, 0., 1.);
  hID = new TH1F("ID", "ID", 100, 0., 1.);
  hVA = new TH1F("VA", "VA", 100, 0., 5.);
  hVD = new TH1F("VD", "VD", 100, 0., 5.);
  
  hNoise = new TH1F("Noise", "Noise", 1000, 0., 1000.);
  hNoiseEdge = new TH1F("NoiseEdge", "NoiseEdge", 1000, 0., 1000.);
  hNoiseCorner = new TH1F("NoiseCorner", "NoiseCorner", 1000, 0., 1000.);
  
  hDefects = new TH1F("defects", "defects", 21, -1., 20.);
  hDefects->GetXaxis()->SetBinLabel(1, "norm");
  hDefects->GetXaxis()->SetBinLabel(2, "dead");
  hDefects->GetXaxis()->SetBinLabel(3, "mask");
  hDefects->GetXaxis()->SetBinLabel(4, "bumps");
  hDefects->GetXaxis()->SetBinLabel(5, "address");
  hDefects->GetXaxis()->SetBinLabel(6, "trim bits");
  
  hGain = new TH1F("Gain", "Gain", 100, 0., 10.);
  hGainMean = new TH1F("GainMean", "GainMean", 100, 0., 10.);
  hGainRMS = new TH1F("GainRMS", "GainRMS", 100, 0., 1.);
  hRelGainSpread = new TH1F("RelGainSpread", "RelGainSpread", 60, 0., 0.3);
      
  hPedestal = new TH1F("Pedestal", "Pedestal", 100, -20000., 50000.);
  hPedestalMean = new TH1F("PedestalMean", "PedestalMean", 70, -20000., 50000.);
  hPedestalRMS = new TH1F("PedestalRMS", "PedestalRMS", 100, 0., 10000.);

  hPar1 = new TH1F("Par1", "Par1", 100, -1., 4.);
  
  hTempVoltage = new TH1F("TempVoltage", "TempVoltage", 200, 400., 600.);

  hCFailures= new TH1F("CFailures", "CFailures", 21, -1., 20.);
  hCFailures->GetXaxis()->SetBinLabel(1, "norm");
  hCFailures->GetXaxis()->SetBinLabel(2, "pix defects");
  hCFailures->GetXaxis()->SetBinLabel(3, "dead");
  hCFailures->GetXaxis()->SetBinLabel(4, "mask");
  hCFailures->GetXaxis()->SetBinLabel(5, "bump");
  hCFailures->GetXaxis()->SetBinLabel(6, "trim bits");
  hCFailures->GetXaxis()->SetBinLabel(7, "address");
  hCFailures->GetXaxis()->SetBinLabel(8, "noise");
  hCFailures->GetXaxis()->SetBinLabel(9, "thr width");
  hCFailures->GetXaxis()->SetBinLabel(10, "rel gain width");
  hCFailures->GetXaxis()->SetBinLabel(11, "ped spread");
  hCFailures->GetXaxis()->SetBinLabel(12, "i150");
  hCFailures->GetXaxis()->SetBinLabel(13, "iv slope");
  hCFailures->GetXaxis()->SetBinLabel(14, "par1");

  hBFailures= new TH1F("BFailures", "BFailures", 21, -1., 20.);
  hBFailures->GetXaxis()->SetBinLabel(1, "norm");
  hBFailures->GetXaxis()->SetBinLabel(2, "pix defects");
  hBFailures->GetXaxis()->SetBinLabel(3, "dead");
  hBFailures->GetXaxis()->SetBinLabel(4, "mask");
  hBFailures->GetXaxis()->SetBinLabel(5, "bump");
  hBFailures->GetXaxis()->SetBinLabel(6, "trim bits");
  hBFailures->GetXaxis()->SetBinLabel(7, "address");
  hBFailures->GetXaxis()->SetBinLabel(8, "noise");
  hBFailures->GetXaxis()->SetBinLabel(9, "thr width");
  hBFailures->GetXaxis()->SetBinLabel(10, "rel gain width");
  hBFailures->GetXaxis()->SetBinLabel(11, "ped spread");
  hBFailures->GetXaxis()->SetBinLabel(12, "i150");
  hBFailures->GetXaxis()->SetBinLabel(13, "iv slope");
  hBFailures->GetXaxis()->SetBinLabel(14, "par1");
}


void MyAnaTestResults::Summary() 
{
  printf("--------------------------------\n");
  printf("Modules %i     A: %i  B: %i  C: %i\n", nModules, nA, nB, nC);
  printf("Modules %i     Full: %i  HalfA: %i  HalfB: %i\n", nModules, nModules - nHalfModulesA - nHalfModulesB, nHalfModulesA, nHalfModulesB);

  ModuleTable();

  fprintf(resultsFile, "\\vdef{BadBumpsFraction} {\\ensuremath {%s}}\n", PrintNumber((double)nBadBumps/nGoodBumps, 1));
  fprintf(resultsFile, "\\vdef{DeadPixelsFraction} {\\ensuremath {%s}}\n", PrintNumber((double)nDeadPixels/nAllPixels, 1));
  fprintf(resultsFile, "\\vdef{NTrim1Defects} {\\ensuremath {%i}}\n", nTrim1Defects);
  fprintf(resultsFile, "\\vdef{NTrim2Defects} {\\ensuremath {%i}}\n", nTrim2Defects);
  fprintf(resultsFile, "\\vdef{NTrim3Defects} {\\ensuremath {%i}}\n", nTrim3Defects);
  fprintf(resultsFile, "\\vdef{NTrim4Defects} {\\ensuremath {%i}}\n", nTrim4Defects);
  fprintf(resultsFile, "\\vdef{NAllPixels} {\\ensuremath {%i}}\n", nAllPixels);
  fprintf(resultsFile, "\\vdef{NAddressProblems} {\\ensuremath {%i}}\n", nAddressProblems);
  fprintf(resultsFile, "\\vdef{GainDCRMS} {\\ensuremath {%s}}\n", PrintNumber(gainRMSDC/nPHDC, 2));
  fprintf(resultsFile, "\\vdef{PedDCRMS} {\\ensuremath {%s}}\n", PrintNumber(pedRMSDC/nPHDC, 2));
  fprintf(resultsFile, "\\vdef{GainROCRMS} {\\ensuremath {%s}}\n", PrintNumber(hGainRMS->GetMean(), 2));
  fprintf(resultsFile, "\\vdef{PedROCRMS} {\\ensuremath {%s}}\n", PrintNumber(hPedestalRMS->GetMean(), 2));



  fclose(resultsFile);
  fclose(moduleTableFile);
}


// ----------------------------------------------------------------------
void MyAnaTestResults::Analyze()
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
  if (gradeOption == 5 && moduleList[moduleNr] == 0) return; // module not in moduleList

  nModules++;
  if (halfModule == 1) nHalfModulesA++;
  else if (halfModule == 2) nHalfModulesB++;
  if (finalGrade == 1) nA++;
  else if (finalGrade == 2) nB++;
  else nC++;

  Bumpbonding();
  TrimBitsTest();
  Trimming();
  TBMErrors();
  MaskDefects();
  DACHistos();
  IV();
  Noise();
  Defects();
  PHCalibration();
  TempVoltage();
  Grading();
  CurrentsVoltages();
}


// ----------------------------------------------------------------------
void MyAnaTestResults::setHalfModulesA()
{
  FCHP = 0;
  LCHP = 8;
  NCHP = LCHP - FCHP;
}


// ----------------------------------------------------------------------
void MyAnaTestResults::setHalfModulesB()
{
  FCHP = 8;
  LCHP = 16;
  NCHP = LCHP - FCHP;
}


// ----------------------------------------------------------------------
void MyAnaTestResults::setFullModules()
{
  FCHP = 0;
  LCHP = 16;
  NCHP = LCHP - FCHP;
}


int MyAnaTestResults::GetHalfModule()
{
  return halfModule;
}

double MyAnaTestResults::GetTempVoltage(int i)
{
  return temperatureVoltage[i];
}


int MyAnaTestResults::GetModuleNr()
{
  return moduleNr;
}


void MyAnaTestResults::DACHistos()
{  
  for (int i = FCHP; i < LCHP; i++)
  {
    hCaldel->Fill(caldel[i]);
  }
}


void MyAnaTestResults::TBMErrors()
{
  if (tbmError1 != 0 && tbmError1 != 4 && tbmError1 != 8) printf("Mod %d tbmError1 %i\n", moduleNr, tbmError1);
  if (tbmError2 != 0 && tbmError2 != 4 && tbmError2 != 8) printf("Mod %d tbmError2 %i\n", moduleNr, tbmError2);
}


void MyAnaTestResults::MaskDefects()
{
  for (int i = FCHP; i < LCHP; i++)
  {
    for (int k = 0; k < 4160; k++)
    {
      //if (pixmap[i][k] < 0.) printf("Mod %d Chip %i pix map defect %f\n", moduleNr, i, pixmap[i][k]);
    }
  }
}


void MyAnaTestResults::Bumpbonding()
{
  for (int i = FCHP; i < LCHP; i++)
  {
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        hBumpbonding->Fill(bumpThrDiff[i][k]);
        if (bumpThrDiff[i][k] >= -5.) nBadBumps++;
        else nGoodBumps++;

        if (moduleNr == 86) 
        {
          int mx, my;
          if (i < 8) {
            mx = 415-(i*52+k/80);
            my = 159-k%80;
          }
          if (i > 7) {
            mx = (i-8)*52+k/80;
            my = k%80;
          }
          hBumpbondingM0086->SetBinContent(mx+1, my+1, bumpThrDiff[i][k]);
        }
      }
    }
  }
}


void MyAnaTestResults::TrimBitsTest()
{
  for (int i = FCHP; i < LCHP; i++)
  {
    int nTrimBitDefects = 0;
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        if (defects[i][k] & 32 || defects[i][k] & 64 || defects[i][k] & 128 || defects[i][k] & 256) nTrimBitDefects++;
        if (defects[i][k] & 32) nTrim1Defects++; //lowest trim bit (trimvalue 14)
        if (defects[i][k] & 64) nTrim2Defects++;
        if (defects[i][k] & 128) nTrim3Defects++;
        if (defects[i][k] & 256) nTrim4Defects++;
        //if (defects[i][k] & 32 || defects[i][k] & 64 || defects[i][k] & 128 || defects[i][k] & 256) printf("defect %i chip %i module %i\n",  defects[i][k], i, moduleNr);
      }
    }
//     if (nTrimBitDefects > 0) printf("Trimbit defect in module %i\n", moduleNr);
    hTrimBitDefects->Fill(nTrimBitDefects);
  }
}

void MyAnaTestResults::CurrentsVoltages()
{
  hIA->Fill(ia);
  hID->Fill(id);
  hVA->Fill(va);
  hVD->Fill(vd);
}


void MyAnaTestResults::Trimming()
{
  for (int i = FCHP; i < LCHP; i++)
  {
    double n = 0.;
    double mean = 0.;
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        hTrimBits->Fill(trims[i][k]);
        hTrimmedThreshold->Fill(vcalConversion*tthr[i][k]);
        hUntrimmedThreshold->Fill(vcalConversion*uthr[i][k]);

        mean+=tthr[i][k];
        n++;
      }
    }
    mean/=n;

    double rms = 0.;
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10) rms+=(tthr[i][k]-mean)*(tthr[i][k]-mean);
    }

    hTrimRMS->Fill(vcalConversion*TMath::Sqrt(rms/n));
    hTrimMean->Fill(vcalConversion*mean);
    
  }
}


void MyAnaTestResults::IV()
{
  hIV150->Fill(iv150);
  hIVSlope->Fill(ivSlope);
}


void MyAnaTestResults::Noise()
{
  int pix;
  for (int i = FCHP; i < LCHP; i++)
  {
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        hNoise->Fill(noise[i][k]);
      }
    }
    
    for (int l = 0; l < 79; l++) //without upper corners
    {
      pix = l;
      if (pixmap[i][pix] == 10) hNoiseEdge->Fill(noise[i][pix]);  //left edge of ROC
      pix = 51*80+l;
      if (pixmap[i][pix] == 10) hNoiseEdge->Fill(noise[i][pix]);  //right edge of ROC
    }
    for (int l = 1; l < 51; l++) //without upper corners
    {
      pix = l*80+79;
      if (pixmap[i][pix] == 10) hNoiseEdge->Fill(noise[i][pix]);  //upper edge of ROC
    }

    if (pixmap[i][79] == 10) hNoiseCorner->Fill(noise[i][79]);  // upper left corner
    if (pixmap[i][51*80+79] == 10) hNoiseCorner->Fill(noise[i][51*80+79]); // upper right corner
  }
}


void MyAnaTestResults::Defects()
{
  bool pixelOk;
  for (int i = FCHP; i < LCHP; i++)
  {
    int nDeadPixelsPerROC = 0;
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] != 10) nDeadPixelsPerROC++;
    }

    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] >= 0 && pixmap[i][k] < 10)
      {
          hDefects->Fill(0.1);  // DEFECT: dead
          nDeadPixels++;
          //printf("dead problems module %i\n", moduleNr);
      }
      nAllPixels++;
      hDefects->Fill(-0.1); // normalization

      pixelOk = true;
      if (pixmap[i][k] != 10) pixelOk = false;
      if (defects[i][k] &   4)
      {
        hDefects->Fill(1.1);  // DEFECT: mask
        //printf("mask problems module %i\n", moduleNr);
      }
      if (pixelOk && defects[i][k] &   8) 
      {
          //printf("bump bonding problems module %i\n", moduleNr);
          hDefects->Fill(2.1);  // DEFECT: bump bonding
      }
      if (pixelOk && defects[i][k] &  16)
      {
        //check M1075 address decoding, M1069 high black level
        if (nDeadPixelsPerROC < 40. && moduleNr != 1075 && moduleNr != 1069 && moduleNr != 1037 && moduleNr != 37 && moduleNr != 596 && moduleNr != 285) //bad ROCs or modules with fake address decoding problems
//           if (moduleNr != 37 && moduleNr != 380 && moduleNr != 540 && moduleNr != 1037) //modules with strange problems which cause fake address level errors
          {
            printf("address problems module %i\n", moduleNr);
            hDefects->Fill(3.1);  // DEFECT: address level
            nAddressProblems++;
          }
      }
      if (pixelOk && defects[i][k] & 480) 
      {
          //printf("trimbit problems module %i\n", moduleNr);
          hDefects->Fill(4.1);  // DEFECT: any TB
      }
    }
  }
}


void MyAnaTestResults::PHCalibration()
{
  TH1F *hGainDC = new TH1F("gainDC", "gainDC", 100, 0., 10.);
  TH1F *hPedDC = new TH1F("pedDC", "pedDC", 140, -20000., 50000.);
  
  for (int i = FCHP; i < LCHP; i++)
  {
    //per double column
    for (int k = 0; k < 26; k++)
    {
      hGainDC->Reset();
      hPedDC->Reset();
      for (int l = 0; l < 160; l++)
      {
        if (pixmap[i][k*160 + l] == 10)
        {
          hGainDC->Fill(gain[i][k*160 + l]);
          hPedDC->Fill(vcalConversion*ped[i][k*160 + l]);
        }
      }
      if (hGainDC->GetEntries() > 0)
      {
        nPHDC++;
        gainRMSDC+=hGainDC->GetRMS();
        pedRMSDC+=hPedDC->GetRMS();
      }
    }

    //per roc
    int n = 0;
    double meanG = 0., meanP = 0.;
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        hPedestal->Fill(vcalConversion*ped[i][k]);
        hGain->Fill(gain[i][k]);
        hPar1->Fill(tanhFit[i][k][1]);

        meanG+=gain[i][k];
        meanP+=ped[i][k];
        n++;
      }
    }
    meanG/=n;
    meanP/=n;
  
    double rmsG = 0., rmsP = 0.;
    for (int k = 0; k < 4160; k++)
    {
      if (pixmap[i][k] == 10)
      {
        rmsG+=(gain[i][k]-meanG)*(gain[i][k]-meanG);
        rmsP+=(ped[i][k]-meanP)*(ped[i][k]-meanP);
      }
    }
    
    rmsG = TMath::Sqrt(rmsG/n);
    rmsP = TMath::Sqrt(rmsP/n);
    
    hGainRMS->Fill(rmsG);
    hGainMean->Fill(meanG);
    hRelGainSpread->Fill(rmsG/meanG);
    
    hPedestalRMS->Fill(vcalConversion*rmsP);
    hPedestalMean->Fill(vcalConversion*meanP);
  }
  
  delete hGainDC;
  delete hPedDC;
}

void MyAnaTestResults::TempVoltage()
{
  for (int i = FCHP; i < LCHP; i++)
  {
    hTempVoltage->Fill(temperatureVoltage[i]);
  }
}


int MyAnaTestResults::BReason()
{
  int reason = 0;

  for (int i = 0; i < 20; i++)
  {
    if (nRocsB[i] > 0)
    {
      reason+=(int)TMath::Power(2, i);
    }
  }
  return reason;
}



void MyAnaTestResults::Grading()
{
  hBFailures->Fill(-0.1); //normalization
  hCFailures->Fill(-0.1);

  //printf("Module %i %s\n", moduleNr, gradeLetter(grade));
  int myGrade = 1;

  for (int i = 0; i < 20; i++)
  {
    if (nRocsB[i] > 0)
    {
      if (finalGrade == 2) hBFailures->Fill(i);
      if (myGrade == 1) myGrade = 2;
      //printf("B %i\n", i);
    }
    if (nRocsC[i] > 0)
    {
      if (finalGrade == 3) hCFailures->Fill(i);
      myGrade = 3;
      //printf("C %i\n", i);
    }
  }

  //if (grade != myGrade) printf(">>>>>>>>> Module %i: Error in grading \n", moduleNr);
}


char *MyAnaTestResults::gradeLetter(int aGrade)
{
  if (aGrade == 1) return "A";
  else if (aGrade == 2) return "B";
  else return "C";
}


void MyAnaTestResults::ModuleTable()
{
  for (int i = 0; i < 1300; i++)
  {
    fprintf(moduleTableFile, "%4i\t%6i\t%i\t%s\t%s\t%s\t%s\n" , i, allDates[i], allAllTestsDone[i], gradeLetter(allGrades[i]), gradeLetter(allOverallGrades[i]), gradeLetter(allFinalGrades[i]), gradeLetter(allShortGrades[i]));
  }
}


const char* MyAnaTestResults::PrintNumber(Double_t number, Int_t nDigis, bool noExp)
{
  TString *result = new TString("");
  char string[1000];
  Int_t power = (Int_t)TMath::Floor(TMath::Log10(TMath::Abs(number)));
  if (number == 0.) power = 1;
  //printf("power %i %i\n", power, nDigis);
  if ((power >= -1 && power < nDigis) || noExp)
  {
    if (nDigis - power == 1) sprintf(string, "%.0f", number);
    else if (nDigis - power == 2) sprintf(string, "%.1f", number);
    else if (nDigis - power == 3) sprintf(string, "%.2f", number);
    else if (nDigis - power == 4) sprintf(string, "%.3f", number);
    else if (nDigis - power == 5) sprintf(string, "%.4f", number);
  }
  else if (nDigis == 1) sprintf(string, "%.0f \\cdot 10^{%i}", number/pow(10, power), power);
  else if (nDigis == 2) sprintf(string, "%.1f \\cdot 10^{%i}", number/pow(10, power), power);
  else if (nDigis == 3) sprintf(string, "%.2f \\cdot 10^{%i}", number/pow(10, power), power);
  else if (nDigis == 4) sprintf(string, "%.3f \\cdot 10^{%i}", number/pow(10, power), power);
  else if (nDigis == 5) sprintf(string, "%.4f \\cdot 10^{%i}", number/pow(10, power), power);
  result->Append(string);
  return result->Data();
}


// -- reads an integer from a file
void MyAnaTestResults::ReadModuleList()
{
  for (int i = 0; i < 2000; i++) moduleList[i] = 0;
	FILE *file = fopen("goodModules.txt", "r");
	if (!file)
	{
		printf("!!!!!!!!!  ----> Could not open ModuleList");
		return;
	}

	char s[200];

	do
	{
    fgets(s, 200, file);
    if (s == NULL) continue;
    int v = -1, i = -1;
    sscanf(s,"%i\t%i ",&i, &v);
    if (i > -1 && i < 2000) moduleList[i] = v;
    //printf("%s mod: %i value: %i\n", s, i, v);
  }
  while (!feof(file));
	fclose(file);
}
