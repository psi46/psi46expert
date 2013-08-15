#include <TMath.h>


void phCal(const char *dir1, const char *dir2) {

  TString directories[] = {TString(dir1),TString(dir2)};

  phCal_DrawDiff(dir1, directories, 2);
  
}


void phCal_DrawDiff(char *module, TString dirNames[], int ndir) {
  
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

  TH1D *pedD1  = new TH1D("Pedestal Difference","", 1200, -600., 600.);
  TH1D *gainD1 = new TH1D("Gain Difference","", 60, -3., 3.);
  
  
  TH2D *mPedD1  = new TH2D("Pedestal Difference Map", "", 416, 0., 416., 160, 0., 160.);
  TH2D *mGainD1 = new TH2D("Gain Difference Map", "", 416, 0., 416., 160, 0., 160.);

  for (Int_t i = 0; i < 16; ++i) {

    pedD1->Reset();
    gainD1->Reset();

    phCal_getDiff(i, dirNames[1], dirNames[0], pedD1, gainD1, mPedD1, mGainD1);

    Int_t pad   = floor(i/4) + 2;
    Int_t pad_i = (i+1)-(4*(pad-2));
    c1->cd(pad)->cd(pad_i);
    gPad->SetTopMargin(0.15);
    pedD1->DrawCopy();
    tc->DrawLatex(0.1, 0.90, Form("Chip %i", i));
    formatHisto(pedD1);
    if (i == 0) tc->DrawLatex(0.5, 0.90, "Pedestal Difference");
    if (i == 3) tc->DrawLatex(0.6, 0.90, Form("Module: %s", module));

    c2->cd(pad)->cd(pad_i);
    gPad->SetTopMargin(0.15);
    gainD1->DrawCopy();
    tc->DrawLatex(0.1, 0.90, Form("Chip %i", i));
    formatHisto(gainD1);
    if (i == 0) tc->DrawLatex(0.5, 0.90, "Gain Difference");
    if (i == 3) tc->DrawLatex(0.6, 0.90, Form("Module: %s", module));
    
    c1->cd(1);
    gPad->SetTopMargin(0.15);
    mPedD1->SetMinimum(-600);
    mPedD1->SetMaximum(600);
    mPedD1->Draw("colz");
    tc->DrawLatex(0.1, 0.90, "Pedestal Difference");
    tc->DrawLatex(0.8, 0.90, Form("Module: %s", module));

    c2->cd(1);
    gPad->SetTopMargin(0.15);
    mGainD1->SetMinimum(-3);
    mGainD1->SetMaximum(3);
    mGainD1->Draw("colz");
    tc->DrawLatex(0.1, 0.90, "Gain Difference");

  }

  c1->SaveAs(Form("%s/diffPed.ps", module));
  c2->SaveAs(Form("%s/diffGain.ps", module));
}

 //-----------------------------------------------------------------
void phCal_getDiff(Int_t chipId, const char *dirN, const char *refN, TH1D *h1, TH1D *h2, TH2D *h3, TH2D *h4) {

  float par0, par1, par2, par3, par4, par5; //Parameters of Vcal vs. Pulse Height Fit
  float rped, rgain, cped, cgain;
  int a,b, div0(0);
  FILE *refFile, *compFile;

  char string[200];
  sprintf(string, "%s/phCalibrationFit_C%i.dat", refN, chipId);
  refFile = fopen(string, "r");

  sprintf(string, "%s/phCalibrationFit_C%i.dat", dirN, chipId);
  compFile = fopen(string, "r");

  printf("refFile: %s/phCalibrationFit_C%i.dat\n", refN, chipId);
  printf("compFile: %s/phCalibrationFit_C%i.dat\n", dirN, chipId);

  // -- get gains & pedestal of old measurements
  if (!refFile || !compFile)
  {
    printf("!!!!!!!!!  ----> PH Calibration: Could not open file(s) to read fit results\n");
  }
  else
  {
    for (int j = 0; j < 2; j++) {
      fgets(string, 200, refFile);
      fgets(string, 200, compFile);
    }

    for (int icol = 0; icol < 52; ++icol)  {
      for (int irow = 0; irow < 80; ++irow)  {

        div0 = 0;

        fscanf(refFile, "%e %e %e %e %e %e %s %2i %2i", &par0, &par1, &par2, &par3, &par4, &par5, string, &a, &b);
        if (par2 != 0) { rped = -par3/par2;  rgain = 1/par2; }
        else { rgain = 0.; rped = 0.; div0 = 1; }
        //printf("rped %e cped %e rgain %e cgain %e %2i %2i\n", rped, cped, rgain, cgain, icol, irow);

        fscanf(compFile, "%e %e %e %e %e %e %s %2i %2i", &par0, &par1, &par2, &par3, &par4, &par5, string, &a, &b);
        if (par2 != 0) { cped = -par3/par2;  cgain = 1/par2; }
        else { cgain = 0.; cped = 0.; div0 = 1; }


        if( !div0 ) )  {

          h1->Fill(rped-cped);
          h2->Fill(rgain-cgain);
    
          if (chipId < 8) { h3->SetBinContent(415-(chipId*52+icol)+1, 159-irow+1, rped-cped);
                            h4->SetBinContent(415-(chipId*52+icol)+1, 159-irow+1, rgain-cgain);}
          if (chipId > 7) { h3->SetBinContent((chipId-8)*52+icol+1, irow+1, rped-cped);
                            h4->SetBinContent((chipId-8)*52+icol+1, irow+1, rgain-cgain);}

        }
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
