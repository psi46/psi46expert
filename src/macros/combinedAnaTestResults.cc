#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include <math.h>
#include <stdlib.h>
#include <TH1F.h>
#include <TMath.h>

#include "myAnaTestResults.hh"


const char* PrintNumber(Double_t number, Int_t nDigis, bool noExp = false)
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


int moduleList[2000];

// -- reads an integer from a file
void ReadModuleList()
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


int main(int argc, char *argv[]) {

  // -- command line arguments
  char dir[200], dir2[200];
  double v1, v2;
  int bReason[5], nBModules = 0, nModules = 0, nFullModules = 0, nHalfModulesA = 0, nHalfModulesB = 0;

  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-d")) sprintf(dir,  argv[++i]); // directory with full ntuples
    if (!strcmp(argv[i],"-s")) sprintf(dir2,  argv[++i]); // directory with short ntupes
  }

  ReadModuleList();

  const char *temp1 = "T-10a";
  const char *temp2 = "T-10b";
  const char *temp3 = "T\\+17a";

  TChain *chain[5];
  MyAnaTestResults *result[5];

  chain[0] = new TChain("mod");
  chain[0]->Add(Form("%s/module-%s-*.root", dir, temp1));
  result[0] = new MyAnaTestResults(chain[0]);

  chain[1] = new TChain("mod");
  chain[1]->Add(Form("%s/module-%s-*.root", dir, temp2));
  result[1] = new MyAnaTestResults(chain[1]);

  chain[2] = new TChain("mod");
  chain[2]->Add(Form("%s/module-%s-*.root", dir, temp3));
  result[2] = new MyAnaTestResults(chain[2]);

  chain[3] = new TChain("mod");
  chain[3]->Add(Form("%s/module-%s-*.root", dir2, temp1));
  result[3] = new MyAnaTestResults(chain[3]);

  chain[4] = new TChain("mod");
  chain[4]->Add(Form("%s/module-%s-*.root", dir2, temp3));
  result[4] = new MyAnaTestResults(chain[4]);

  FILE *resultsTex = fopen("CombinedResults.tex","w");
  TFile *resultFile = new TFile("CombinedResults.root", "recreate");
  TH1F *hGain = new TH1F("tempGain", "tempGain", 250, -2., 3.);
  TH1F *hOffset = new TH1F("tempOffset", "tempOffset", 250, -1500., 1000.);

  TH1F *hBFailures= new TH1F("BFailures", "BFailures", 21, -1., 20.);
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

  int entry[5] = {0}, mod[5];
  bool atEnd = false;
  for (int i = 0; i < 5; i++)
  {
    if (chain[i]->GetEntriesFast() == entry[i]) atEnd = true;
  }

  while (!atEnd)
  {
    for (int i = 0; i < 5; i++)
    {
      result[i]->LoadTree(entry[i]);
      chain[i]->GetEntry(entry[i]);
      mod[i] = result[i]->GetModuleNr();
    }

//     printf("%i %i %i\n", mod1, mod2, mod3);
    int max = mod[0];
    for (int i = 0; i < 5; i++)
    {
      if (mod[i] > max) max = mod[i];
    }

    bool sync = true;
    for (int i = 0; i < 5; i++)
    {
      if (mod[i] != max) sync = false;
    }

    if (!sync)
    {
      for (int i = 0; i < 5; i++)
      {
        if (mod[i] < max) entry[i]++;
        if (chain[i]->GetEntriesFast() == entry[i]) atEnd = true;
      }
      continue;
    }

    if (moduleList[max] == 1)
    {
      for (int i = 0; i < 16; i++)
      {
        v1 = result[1]->GetTempVoltage(i);
        v2 = result[2]->GetTempVoltage(i);

        if (v1 < 1. || v2 < 1. || v1 > 10000. || v2 > 10000.) continue; //discard bad measurements

        double gain = (v2 - v1) / 27.;
        double offset = -10 - gain * v1;
        //T(v) = offset + gain * v

        hOffset->Fill(offset);
        hGain->Fill(gain);
      }

      bool isB = false;
      for (int i = 0; i < 5; i++)
      {
        bReason[i] = result[i]->BReason();
        if (bReason[i]) isB = true;
      }

      nModules++;
      if (isB) nBModules++;
      int halfModule = result[0]->GetHalfModule();
      if (halfModule == 0) nFullModules++;
      else if (halfModule == 1) nHalfModulesA++;
      else nHalfModulesB++;

      bool isReason;
      for (int k = 0; k < 20; k++)
      {
        isReason = false;
        for (int i = 0; i < 5; i++)
        {
          if (bReason[i] & (int)TMath::Power(2, k)) isReason = true;
          if (bReason[i]) isB = true;
        }
        if (isReason) hBFailures->Fill(k);
        if (isReason && k == 5)
        {
          printf("Module %i address problem %i\n", max, k);
        }
      }
    }

    for (int i = 0; i < 5; i++)
    {
      entry[i]++;
      if (chain[i]->GetEntriesFast() == entry[i]) atEnd = true;
    }
  }

  printf("Modules: %i\n", nModules);
  printf("B modules %i\n", nBModules);

  fprintf(resultsTex, "\\vdef{TempCalibGain} {\\ensuremath {%s}}\n", PrintNumber(hGain->GetMean(), 3));
  fprintf(resultsTex, "\\vdef{TempCalibOffset} {\\ensuremath {%s}}\n", PrintNumber(hOffset->GetMean(), 2));
  fprintf(resultsTex, "\\vdef{TempCalibGainRMS} {\\ensuremath {%s}}\n", PrintNumber(hGain->GetRMS(), 2));
  fprintf(resultsTex, "\\vdef{TempCalibOffsetRMS} {\\ensuremath {%s}}\n", PrintNumber(hOffset->GetRMS(), 3));

  fprintf(resultsTex, "\\vdef{BReasonDefPixels} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(2));
  fprintf(resultsTex, "\\vdef{BReasonDead} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(3));
  fprintf(resultsTex, "\\vdef{BReasonMask} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(4));
  fprintf(resultsTex, "\\vdef{BReasonBump} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(5));
  fprintf(resultsTex, "\\vdef{BReasonTrimBits} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(6));
  fprintf(resultsTex, "\\vdef{BReasonAddress} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(7));
  fprintf(resultsTex, "\\vdef{BReasonNoise} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(8));
  fprintf(resultsTex, "\\vdef{BReasonThrWidth} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(9));
  fprintf(resultsTex, "\\vdef{BReasonGain} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(10));
  fprintf(resultsTex, "\\vdef{BReasonPedestal} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(11));
  fprintf(resultsTex, "\\vdef{BReasonI150} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(12));
  fprintf(resultsTex, "\\vdef{BReasonIVSlope} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(13));
  fprintf(resultsTex, "\\vdef{BReasonPar1} {\\ensuremath {%i}}\n", (int)hBFailures->GetBinContent(14));

  fprintf(resultsTex, "\\vdef{NModules} {\\ensuremath {%i}}\n", nModules);
  fprintf(resultsTex, "\\vdef{NAModules} {\\ensuremath {%i}}\n", nModules-nBModules);
  fprintf(resultsTex, "\\vdef{NBModules} {\\ensuremath {%i}}\n", nBModules);
  fprintf(resultsTex, "\\vdef{NFullModules} {\\ensuremath {%i}}\n", nFullModules);
  fprintf(resultsTex, "\\vdef{NHalfModulesA} {\\ensuremath {%i}}\n", nHalfModulesA);
  fprintf(resultsTex, "\\vdef{NHalfModulesB} {\\ensuremath {%i}}\n", nHalfModulesB);

  resultFile->Write();
  resultFile->Close();
  fclose(resultsTex);
  return 0;

}
