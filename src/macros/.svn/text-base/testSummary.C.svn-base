#include <TMath.h>
#include <iostream>
#include <fstream>

void summary(int firstMod, int lastMod, int allT=1)  {
 
 setStyle();

 TH1D *SCurve     = new TH1D("SCurve Mean","", 100, 100., 250.);
 TH1D *Threshold  = new TH1D("Threshold","", 100, 1., 2.);
 TH1D *Gain       = new TH1D("Gain","", 100, 0., 0.1);
 TH1D *Pedestal   = new TH1D("Pedestal","", 100, 0., 5000.);

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
     SCurve->Reset();
     Threshold->Reset();
     Gain->Reset();
     Pedestal->Reset();
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
  
   
    TCanvas *c1 = new TCanvas("c1", "", 900, 600);
    c1->Clear();
    c1->Divide(2,2);
   
    c1->cd(1);
    for (int n = 0; n < k; n++) {
   
      c1->cd(1);
      gPad->SetTopMargin(0.15);
      SCurve->Fill(mean[n][0]); 
      SCurve->DrawCopy();
      formatHisto(SCurve);
      tc->DrawLatex(0.1, 0.90, "SCurve Width: #mu");
     
      c1->cd(2);
      gPad->SetTopMargin(0.15);
      Threshold->Fill(sigma[n][1]);//printf("%f\n",sigma[n][1]);
      Threshold->DrawCopy();
      formatHisto(Threshold);
      tc->DrawLatex(0.1, 0.90, "Vcal Threshold Trimmed: #sigma");
      if (!allT ) { tc->DrawLatex(0.6, 0.90, Form("Summary of %s", test));  }
      else        { tc->DrawLatex(0.6, 0.90, Form("Summary of all T", test));  }
     
      c1->cd(3);
      gPad->SetTopMargin(0.15);
      if (mean[n][2] != 0 ) Gain->Fill(sigma[n][2]/mean[n][2]); //printf("%f\n",sigma[n][2]/mean[n][2]);
      Gain->DrawCopy();
      formatHisto(Gain);
      tc->DrawLatex(0.1, 0.90, "Gain: #sigma/#mu");
     
      c1->cd(4);
      gPad->SetTopMargin(0.15);
      if (mean[n][2] != 0 ) sigmaEl = (sigma[n][3]/mean[n][2])*65;
      Pedestal->Fill(sigmaEl);  //printf("%f\n",sigmaEl);
      
      Pedestal->DrawCopy();
      formatHisto(Pedestal);
      tc->DrawLatex(0.1, 0.90, "Pedestal: #sigma in units of e^{-}");
    }
   
    if (!allT )  { c1->SaveAs(Form("testOverview_M%.2iM%.2i-%s.ps", firstMod, lastMod, test)); }
    else         { c1->SaveAs(Form("testOverview_M%.2iM%.2i.ps", firstMod, lastMod, test)); }
   
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
