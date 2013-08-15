#include <TMath.h>
#include <iostream>
#include <fstream>

void overview(int firstMod, int lastMod, int allT=1)  {
 
 setStyle();

 TH1D *mSCurve     = new TH1D("SCurve Mean","", 100, 100., 250.);
 TH1D *mThreshold  = new TH1D("Threshold Mean","", 500, 35., 65.);
 TH1D *mGain       = new TH1D("Gain Mean","", 100, 0., 5.);
 TH1D *mPedestal   = new TH1D("Pedestal Mean","", 100, -1000., 0.);
 
 TH1D *sSCurve     = new TH1D("SCurve Sigma","", 100, 0., 50.);
 TH1D *sThreshold  = new TH1D("Threshold Sigma","", 100, 0., 5.);
 TH1D *sGain       = new TH1D("Gain Sigma","", 100, 0., 0.5);
 TH1D *sPedestal   = new TH1D("Pedestal Sigma","", 100, 0., 100.);

 TLatex *tc = new TLatex;
 tc->SetNDC(kTRUE);
 tc->SetTextSize(0.07);

 TLatex *tl = new TLatex;
 tl->SetNDC(kTRUE);
 tl->SetTextSize(0.06);
 
 char *string[80];
 char *file[80];
 char *dir[80];
 char *test[80];
 float mean[500][4], sigma[500][4];
 float m, s;
 float sigmaEl;
 
 for (int t = 0; t < 3; t++) {
 
   int k = 0;
   
   if ( !allT ) {
     mSCurve->Reset();
     mThreshold->Reset();
     mGain->Reset();
     mPedestal->Reset();
     sSCurve->Reset();
     sThreshold->Reset();
     sGain->Reset();
     sPedestal->Reset();
   }
 
   for (int i = firstMod; i < lastMod; i++ ) {
 
     if (t == 0) test = "T-10a";
     if (t == 1) test = "T-10b";
     if (t == 2) test = "T+17a";
     
     sprintf(dir, "M00%.2i/%s", i, test); 
     
     for (int j = 0; j < 16; j++ ) {
   
       sprintf(file, "%s/summary_C%i.txt", dir, j);
       FILE *inFile = fopen(file,"r");    
       
       if (!inFile) 
       {
       printf("!!!!!!!!!  ----> Will not include %s in plots, as %s could not be found!\n", dir, file);
       break;
       }
       else {
       
         if (j == 0) { printf("----> Scanning summary-files in %s!\n", dir, file);}
        
	 fclose(inFile);
	 ifstream is(file);
	 
	 char  buffer[200];
	 int l = 0;
	 
	 while (is.getline(buffer, 200, '\n') && l < 4) {
	   
	   if (buffer[0] != 'n' )  {continue;}
	   
	   sscanf(buffer,"%s %f %f",string, &m, &s);  
	   
	   mean[k][l] = m;
	   sigma[k][l] = s;
	   
	   l++;
	   
	 }

         k++;
       
        }
      }
    }
  
   
    TCanvas *c1 = new TCanvas("c1", "", 600, 900);
    c1->Clear();
    c1->Divide(2,4);
   
    c1->cd(1);
    for (int n = 0; n < k; n++) {
   
      c1->cd(1);
      gPad->SetTopMargin(0.15);
      mSCurve->Fill(mean[n][0]); 
      mSCurve->DrawCopy();
      formatHisto(mSCurve);
      tc->DrawLatex(0.1, 0.90, "SCurve Width: #mu");
      
      c1->cd(2);
      gPad->SetTopMargin(0.15);
      sSCurve->Fill(sigma[n][0]); 
      sSCurve->DrawCopy();
      formatHisto(sSCurve);
      tc->DrawLatex(0.1, 0.90, "SCurve Width: #sigma");
      if (!allT ) { tc->DrawLatex(0.6, 0.90, Form("Summary of %s", test));  }
      else        { tc->DrawLatex(0.6, 0.90, Form("Summary of all T", test));  }
     
     
      c1->cd(3);
      gPad->SetTopMargin(0.15);
      mThreshold->Fill(mean[n][1]);//printf("%f\n",sigma[n][1]);
      mThreshold->DrawCopy();
      formatHisto(mThreshold);
      tc->DrawLatex(0.1, 0.90, "Vcal Threshold Trimmed: #mu");
     
      c1->cd(4);
      gPad->SetTopMargin(0.15);
      sThreshold->Fill(sigma[n][1]);//printf("%f\n",sigma[n][1]);
      sThreshold->DrawCopy();
      formatHisto(sThreshold);
      tc->DrawLatex(0.1, 0.90, "Vcal Threshold Trimmed: #sigma");
      
      c1->cd(5);
      gPad->SetTopMargin(0.15);
      mGain->Fill(mean[n][2]); //printf("%f\n",sigma[n][2]/mean[n][2]);
      mGain->DrawCopy();
      formatHisto(mGain);
      tc->DrawLatex(0.1, 0.90, "Gain: #mu");
      
      c1->cd(6);
      gPad->SetTopMargin(0.15);
      sGain->Fill(sigma[n][2]); //printf("%f\n",sigma[n][2]/mean[n][2]);
      sGain->DrawCopy();
      formatHisto(sGain);
      tc->DrawLatex(0.1, 0.90, "Gain: #sigma");
     
      c1->cd(7);
      gPad->SetTopMargin(0.15);
      mPedestal->Fill(mean[n][3]); 
      mPedestal->DrawCopy();
      formatHisto(mPedestal);
      tc->DrawLatex(0.1, 0.90, "Pedestal: #mu");
      
      c1->cd(8);
      gPad->SetTopMargin(0.15);
      sPedestal->Fill(sigma[n][3]);  
      sPedestal->DrawCopy();
      formatHisto(sPedestal);
      tc->DrawLatex(0.1, 0.90, "Pedestal: #sigma");
      
    }
   
    if (!allT )  { c1->SaveAs(Form("testOverview_M%.2iM%.2i-%s.ps", firstMod, lastMod, test)); }
    else         { c1->SaveAs(Form("testSummary_M%.2iM%.2i.ps", firstMod, lastMod, test)); }
   
  }
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
