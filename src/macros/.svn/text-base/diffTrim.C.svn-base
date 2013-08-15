#include <TMath.h>


void trimming(const char *dir1, const char *dir2) {

  TString directories[] = {TString(dir1),TString(dir2)};

  trimming_DrawDiff(dir1, directories, 2);
  
}


void trimming_DrawDiff(char *module, TString dirNames[], int ndir) {
  
  setStyle();

  TCanvas *c1 = new TCanvas("c1", "", 700, 800);
  c1->Clear();
  c1->Divide(1,5);
  
  c1_2->Divide(4,1);
  c1_3->Divide(4,1);
  c1_4->Divide(4,1);
  c1_5->Divide(4,1);

  TLatex *tc = new TLatex;
  tc->SetNDC(kTRUE);
  tc->SetTextSize(0.07);

  TLatex *tl = new TLatex;
  tl->SetNDC(kTRUE);
  tl->SetTextSize(0.06);

  TH1D *trimD1  = new TH1D("Trim Parameters Difference","", 20, -10., 10.);
  TH2D *mTrimD1  = new TH2D("Trim Parameters Difference","", 416, 0., 416., 160, 0., 160.);
  
  for (Int_t i = 0; i < 16; ++i) {

    trimD1->Reset();

    trimming_GetDiff(i, dirNames[1], dirNames[0], trimD1, mTrimD1);
    
    
    Int_t pad;
    pad  = floor(i/4) + 2;
    Int_t pad_i;
    pad_i  = (i+1)-(4*(pad-2));
    c1->cd(pad)->cd(pad_i);
    gPad->SetTopMargin(0.15);
    trimD1->DrawCopy();
    tc->DrawLatex(0.1, 0.90, Form("Chip %i", i));
    formatHisto(trimD1);
    if (i == 0) tc->DrawLatex(0.5, 0.90, "Trim Parameters Difference");
    if (i == 3) tc->DrawLatex(0.6, 0.90, Form("Module: %s", module));    
    
    c1->cd(1);
    gPad->SetTopMargin(0.15);
    mTrimD1->SetMinimum(-10);
    mTrimD1->SetMaximum(10);
    mTrimD1->Draw("colz");
    tc->DrawLatex(0.1, 0.90, "Trim Parameters Difference");
    tc->DrawLatex(0.8, 0.90, Form("Module: %s", module));

  }

  c1->SaveAs(Form("%s/diffTrim.ps", module));

}

 //-----------------------------------------------------------------
void trimming_GetDiff(Int_t chipId, const char *dirN, const char *refN, TH1D *h1, TH2D *h2) {

  int rtrim, ctrim;
  int a,b;
  FILE *refFile, *compFile;

  char string[200];
  sprintf(string, "%s/trimParameters60_C%i.dat", refN, chipId);
  refFile = fopen(string, "r");

  sprintf(string, "%s/trimParameters60_C%i.dat", dirN, chipId);
  compFile = fopen(string, "r");

  printf("refFile: %s/trimParameters60_C%i.dat\n", refN, chipId);
  printf("compFile: %s/trimParameters60_C%i.dat\n", dirN, chipId);

  // -- get gains & pedestal of old measurements
  if (!refFile || !compFile)
  {
    printf("!!!!!!!!!  ----> Trim Parameters: Could not open file(s) to read fit results\n");
  }
  else
  {

    for (int j = 0; j < 2; j++) {
      fgets(string, 200, refFile);
      fgets(string, 200, compFile);
    }

    for (int icol = 0; icol < 52; ++icol)  {
      for (int irow = 0; irow < 80; ++irow)  {

        fscanf(refFile, "%2i %s %2i %2i", &rtrim, string, &a, &b);
        fscanf(compFile, "%2i %s %2i %2i", &ctrim, string, &a, &b);
        //printf("rtrim %d ctrim %d\n", rtrim, ctrim);

        h1->Fill(rtrim-ctrim);
        
        if (chipId < 8) { h2->SetBinContent(415-(chipId*52+icol)+1, 159-irow+1, rtrim-ctrim);}
        if (chipId > 7) { h2->SetBinContent((chipId-8)*52+icol+1, irow+1, rtrim-ctrim);}

      }
    }
  }

 fclose(refFile);
 fclose(compFile);


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
