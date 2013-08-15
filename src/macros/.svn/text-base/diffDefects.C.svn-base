#include <TMath.h>


void directories(const char *baseDir) {

  TString directories[] = {TString(Form("%s",baseDir)),TString(Form("%sN10",baseDir))};
  
  TString problems[] = {TString("pixel"), TString("mask"), TString("bump"), TString("trim"), TString("address")};
  
  drawDiff(baseDir, directories, 2, problems, 5);
  
}


void drawDiff(char *module, TString dirNames[], int ndir, TString problemType[], int nprob) {
  
  setStyle();

  TCanvas *c1 = new TCanvas("c1", "", 900, 500);
  c1->Clear();
  c1->Divide(3,2);
  
  TLatex *tc = new TLatex;
  tc->SetNDC(kTRUE);
  tc->SetTextSize(0.07);

  TLatex *tl = new TLatex;
  tl->SetNDC(kTRUE);
  tl->SetTextSize(0.06);

  TH1D *probD1 = new TH1D("Defects difference","", 20, -10., 10.);

  char *prob[50];

  for(Int_t iprob = 0; iprob < nprob; ++iprob) {

    prob = problemType[iprob];
    probD1->Reset();

    for (Int_t i = 0; i < 16; ++i)  getDiff(problemType[iprob], i, dirNames[1], dirNames[0], probD1);

    c1->cd(iprob+1);
    gPad->SetTopMargin(0.15);
    probD1->DrawCopy();
    tc->DrawLatex(0.1, 0.95, Form("%s defects", prob));
    formatHisto(probD1);

  }

   c1->SaveAs(Form("%s/diffDefects.ps", module));

}

 //-----------------------------------------------------------------
void getDiff(const char *probType, int chipId, const char *dirN, const char *refN, TH1D *h) {

   Int_t *vref, *vcomp;
   Int_t defectR[5];
   Int_t defectC[5];
   Int_t totalR, totalC;
   
   vref  = findProblems(probType, refN, chipId);
   vcomp = findProblems(probType, dirN, chipId);
   
   if ( !strcmp(probType,"pixel") )    { defectR[0] = vref;  defectC[0] = vcomp; }
   if ( !strcmp(probType,"mask") )     { defectR[1] = vref;  defectC[1] = vcomp; }
   if ( !strcmp(probType,"bump") )     { defectR[2] = vref;  defectC[2] = vcomp; }
   if ( !strcmp(probType,"trim") )     { defectR[3] = vref;  defectC[3] = vcomp; }
   if ( !strcmp(probType,"address") )  { defectR[4] = vref;  defectC[4] = vcomp; }
   
   for(int l = 0; l < 5; l++)          { totalR += defectR[l]; totalC += defectC[l]; 

   h->Fill(vref - vcomp);

}
 //-----------------------------------------------------------------
int findProblems(const char *type, const char *dir, int chip) {

  char fil[200] = "FullTest.root";
  TH1D *h1;
  TH2D *h2;
  int nProblems = 0;
  
  printf("%s/%s\n",dir,fil);
  
//   int ch = 0;
//   ch = fileCheck(dir, fil);
//   printf("ch %i\n", ch);
  
  if ( fileCheck(dir, fil) != 0 )
  {
    printf("!!!!!!!!!  ----> Search for Defects: No file %s in directory %s\n", fil, dir);
  }
  else
  {

     TFile *f = new TFile(Form("%s/%s", dir, fil));
     printf(" Type %s\n", type);
    
    //-- Dead Pixels
    if ( !strcmp(type,"pixel") )  {
      h2 = (TH2D*)f->Get(Form("PixelMap_C%i", chip));
      printf("PixelMap_C%i\n", chip);
      for (int icol = 0; icol < 52; ++icol)  {
        for (int irow = 0; irow < 80; ++irow)  {
          if (h2->GetBinContent(icol+1, irow+1)  == 0)  {
              nProblems++;
          }
        }
      }
      printf("dead pixels: %i\n", nProblems);
    }
  
    //-- Mask defects
    if ( !strcmp(type,"mask") )  {
      h2 = (TH2D*)f->Get(Form("PixelMap_C%i", chip));
      printf("PixelMap_C%i\n", chip);
      for (int icol = 0; icol < 52; ++icol)  {
        for (int irow = 0; irow < 80; ++irow)  {
          if (h2->GetBinContent(icol+1, irow+1)  < 0)   {
             ++nProblems;
            }  
          }
      }
      printf("mask defects: %i\n", nProblems);
    }
   
    // -- Bump defect
    if ( !strcmp(type,"bump") )  {
      double minThrDiff(-2.);
      double maxThrDiff(2.);
      h2 = (TH2D*)f->Get(Form("vcals_xtalk_C%i", chip));
      printf("vcals_xtalk_C%i\n", chip);  
      for (int icol = 0; icol < 52; ++icol)  {
        for (int irow = 0; irow < 80; ++irow)  {
          if ( h2->GetBinContent(icol+1, irow+1)  > minThrDiff && h2->GetBinContent(icol+1, irow+1)  < maxThrDiff )  {
            ++nProblems;
          }
        }
      }
      printf("dead bumps: %i\n", nProblems);
    }
  
    // -- Dead trimbit
    if ( !strcmp(type,"trim") )  {
      int trimbitbins(3);
      h1 = (TH1D*)f->Get(Form("TrimBit7_C%i", chip));
      for (int i = 1; i <= trimbitbins; ++i) { nProblems += h1->GetBinContent(i); }
      h1 = (TH1D*)f->Get(Form("TrimBit11_C%i", chip));
      for (int i = 1; i <= trimbitbins; ++i) { nProblems += h1->GetBinContent(i); }
      h1 = (TH1D*)f->Get(Form("TrimBit13_C%i", chip));
      for (int i = 1; i <= trimbitbins; ++i) { nProblems += h1->GetBinContent(i); }
      h1 = (TH1D*)f->Get(Form("TrimBit14_C%i", chip));
      for (int i = 1; i <= trimbitbins; ++i) { nProblems += h1->GetBinContent(i); }
      printf("trimbit problems: %i\n", nProblems);
    }

    // -- Address problem
    if ( !strcmp(type,"address") )  {
      h2 = (TH2D*)f->Get(Form("AddressDecoding_C%i", chip));
      printf("AddressDecoding_C%i\n", chip);
      for (int icol = 0; icol < 52; ++icol) {
        for (int irow = 0; irow < 80; ++irow) {
          if (h2->GetBinContent(icol+1, irow+1) < 1) {
             ++nProblems;
          }
        }
      }
      printf("address problems: %i\n", nProblems);
    }
  }
  
  return nProblems;
}
     

 //-----------------------------------------------------------------

void setStyle() {

  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(1);
  gStyle->SetTitle(0);

  gStyle->SetStatFont(132);
  gStyle->SetTextFont(132);
  gStyle->SetLabelFont(132, "X");
  gStyle->SetLabelFont(132, "Y");
  gStyle->SetTitleFont(132);

  gROOT->ForceStyle();

  }
 
//-----------------------------------------------------------------

void formatHisto(TH1D *h) {
  
  TLatex *ts = new TLatex;
  ts->SetNDC(kTRUE);
  ts->SetTextSize(0.06);

  if ( h->GetMean() < (h->GetXaxis()->GetXmin()+((h->GetXaxis()->GetXmax() - h->GetXaxis()->GetXmin())/2)) ) {

    ts->DrawLatex(0.60, 0.78, Form("Entries:%4.0f", h->GetEntries()));
    ts->DrawLatex(0.60, 0.72, Form("Mean:%4.2f", h->GetMean()));
    ts->DrawLatex(0.60, 0.66, Form("RMS: %4.2f", h->GetRMS()));
  }
  
  else {

    ts->DrawLatex(0.15, 0.78, Form("Entries:%4.0f", h->GetEntries()));
    ts->DrawLatex(0.15, 0.72, Form("Mean:%4.2f", h->GetMean()));
    ts->DrawLatex(0.15, 0.66, Form("RMS: %4.2f", h->GetRMS()));
  }  
  
}
//-----------------------------------------------------------------
int fileCheck(const char *dName, const char *fName) {

  FILE *file;

  char string[200];
  sprintf(string, "%s/%s", dName, fName);
  file = fopen(string, "r");

  if ( !file )
  {
    return 1;
  }
  else
  {
    return 0;
  }

  fclose(file);


}
