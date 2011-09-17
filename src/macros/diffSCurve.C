#include <TMath.h>


void sCurve(const char *dir1, const char *dir2) {

  TString directories[] = {TString(dir1),TString(dir2)};

  sCurve_DrawDiff(dir1, directories, 2);
  
}


void sCurve_DrawDiff(char *module, TString dirNames[], int ndir) {

  setStyle();
  TCanvas *c1 = new TCanvas("c1", "", 700, 800);
  c1->Clear();
  c1->Divide(1,5);
  
  c1_2->Divide(4,1);
  c1_3->Divide(4,1);
  c1_4->Divide(4,1);
  c1_5->Divide(4,1);

  TCanvas *c2 = new TCanvas("c2", "", 700, 800);  
  c2->Clear();
  c2->Divide(1,5);
  
  c2_2->Divide(4,1);
  c2_3->Divide(4,1);
  c2_4->Divide(4,1);
  c2_5->Divide(4,1);
  
  TLatex *tc = new TLatex;
  tc->SetNDC(kTRUE);
  tc->SetTextSize(0.07);
  
  TLatex *tl = new TLatex;
  tl->SetNDC(kTRUE);
  tl->SetTextSize(0.06);

  TH1D *thrD1   = new TH1D("Threshold Difference","", 40, -20., 20.);
  TH1D *noiseD1 = new TH1D("Noise Difference","", 400, -200., 200.);  
  
  TH2D *mThrD1    = new TH2D("Threshold Difference Map", "", 416, 0., 416., 160, 0., 160.);
  TH2D *mNoiseD1  = new TH2D("Noise Difference Map", "", 416, 0., 416., 160, 0., 160.);

  for (Int_t i = 0; i < 16; ++i) {

    thrD1->Reset();
    noiseD1->Reset();

    sCurve_GetDiff(i, dirNames[1], dirNames[0], thrD1, noiseD1, mThrD1, mNoiseD1);

    Int_t pad   = floor(i/4) + 2;
    Int_t pad_i = (i+1)-(4*(pad-2));
    c1->cd(pad)->cd(pad_i);
    gPad->SetTopMargin(0.15);
    thrD1->DrawCopy();
    tc->DrawLatex(0.1, 0.90, Form("Chip %i", i));
    formatHisto(thrD1);
    if (i == 0) tc->DrawLatex(0.3, 0.90, "Vcal Threshold Difference");
    if (i == 3) tc->DrawLatex(0.6, 0.90, Form("Module: %s", module));

    c2->cd(pad)->cd(pad_i);;
    gPad->SetTopMargin(0.15);
    noiseD1->DrawCopy();
    tc->DrawLatex(0.1, 0.90, Form("Chip %i", i));
    formatHisto(noiseD1);
    if (i == 0) tc->DrawLatex(0.3, 0.90, "SCurve Width Difference");
    if (i == 3) tc->DrawLatex(0.6, 0.90, Form("Module: %s", module));
    
    c1->cd(1);
    gPad->SetTopMargin(0.15);
    mNoiseD1->SetMinimum(-20);
    mNoiseD1->SetMaximum(20);
    mNoiseD1->Draw("colz");
    tc->DrawLatex(0.1, 0.90, "Vcal Threshold Difference");
    tc->DrawLatex(0.8, 0.90, Form("Module: %s", module));

    c2->cd(1);
    gPad->SetTopMargin(0.15);
    mThrD1->SetMinimum(-200);
    mThrD1->SetMaximum(200);
    mThrD1->Draw("colz");
    tc->DrawLatex(0.1, 0.90, "SCurve Width Difference (Noise)");

  }

  c1->SaveAs(Form("%s/diffThreshold.ps", module));
  c2->SaveAs(Form("%s/diffNoise.ps", module));
}

 //-----------------------------------------------------------------
void sCurve_GetDiff(Int_t chipId, const char *dirN, const char *refN, TH1D *h1, TH1D *h2, TH2D *h3, TH2D *h4) {

  float rthr, rsig, cthr, csig;
  int a,b;
  FILE *refFile, *compFile;

  char string[200];
  sprintf(string, "%s/SCurve_C%i.dat", refN, chipId);
  refFile = fopen(string, "r");

  sprintf(string, "%s/SCurve_C%i.dat", dirN, chipId);
  compFile = fopen(string, "r");

  printf("refFile: %s/SCurve_C%i.dat\n", refN, chipId);
  printf("compFile: %s/SCurve_C%i.dat\n", dirN, chipId);

  // -- get gains & pedestal of old measurements
  if (!refFile || !compFile)
  {
    printf("!!!!!!!!!  ----> SCurve: Could not open file(s) to read fit results\n");
  }
  else
  {
    for (int j = 0; j < 2; j++) {
      fgets(string, 200, refFile);
      fgets(string, 200, compFile);
    }

    for (int icol = 0; icol < 52; ++icol)  {
      for (int irow = 0; irow < 80; ++irow)  {

        fscanf(refFile, "%e %e %s %2i %2i", &rthr, &rsig, string, &a, &b);
        rthr = rthr / 65;

        fscanf(compFile, "%e %e %s %2i %2i", &cthr, &csig, string, &a, &b);
        cthr = cthr / 65;
        //printf("rthr %e cthr %e rsig %e csig %e\n", rthr, cthr, rsig, csig);

        h1->Fill(rthr-cthr);
        h2->Fill(rsig-csig);

        if (chipId < 8) { h3->SetBinContent(415-(chipId*52+icol)+1, 159-irow+1, rsig-csig);
                          h4->SetBinContent(415-(chipId*52+icol)+1, 159-irow+1, rthr-cthr);}
        if (chipId > 7) { h3->SetBinContent((chipId-8)*52+icol+1, irow+1, rsig-csig);
                          h4->SetBinContent((chipId-8)*52+icol+1, irow+1, rthr-cthr);}

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
