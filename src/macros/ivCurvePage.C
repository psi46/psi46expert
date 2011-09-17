#include <TMath.h>
#include <iostream>
#include <fstream>

using namespace std;

void ivCurve(const char *dirName) {

   Double_t V,A;
   Double_t x_V[250], y_A[250];
   
   gROOT->Reset();
   ifstream dataIn(Form("%s/iv.dat", dirName));
   printf("dirName = %s/iv.dat\n",dirName);
   
   const Int_t maxChar = 25, headerLines = 1;
   for(int i = 0; i < headerLines; i++)
     dataIn.ignore(maxChar);

   int nLines;
   
   for(nLines = 0; !dataIn.eof(); nLines++) {
    
      dataIn >> V >> A;
      x_V[nLines] = V;
      y_A[nLines] = 1e6*A;
      
      //printf("V=%e, A=%e\n",V,A);

   }

   const Int_t n = nLines;
   Double_t x[n], y[n];
   
   for(int i = 0; i < n; i++) {
   
     x[i] = x_V[i];
     y[i] = y_A[i];
     
   }
   
  
    TGraph *g1 = new TGraph(n,x,y);

    g1->Draw("aC");
    g1->SetTitle(Form("%s", dirName));
    g1->SetLineColor(4);
    g1->SetLineWidth(2);
  
    g1->GetXaxis()->SetTitle("Voltage [V]");
    g1->GetYaxis()->SetTitle("Current [#muA]");
    g1->GetYaxis()->SetDecimals();
    g1->GetYaxis()->SetTitleOffset(1.2);
    g1->GetYaxis()->CenterTitle();
  

  dataIn.close();
}

void ivCurves(const char *dirName0="",
              const char *dirName1="",int modNumber) {
  //	      const char *dirName2="",
  //	      const char *dirName3="",
  //	      const char *dirName4="",
  //	      const char *dirName5="",
  //	      const char *dirName6="",
  //           const char *dirName7="",
  //	      const char *dirName8="",
  //	      const char *dirName9="",
  //	      const char *dirName10="",
  //	      const char *dirName11="",
  //	      const char *dirName12="",
  //	      const char *dirName13="",
  //	      const char *dirName14="",
  //	      const char *dirName15="", int modNumber) {

	      
  gROOT->SetStyle("Plain");
  
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetTitle(0);
  
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetTitleFont(132);
  gStyle->SetTitleFontSize(0.06);
  gStyle->SetTitleBorderSize(0);
  gStyle->SetTickLength(0.02, "X");
  gStyle->SetTickLength(0.008, "Y");
  gStyle->SetStatFont(132);
  gStyle->SetTextFont(132);
  gStyle->SetLabelFont(132, "X");
  gStyle->SetLabelFont(132, "Y");
  gStyle->SetTitleFont(132, "X");
  gStyle->SetTitleFont(132, "Y");
  
  
  gROOT->ForceStyle();


  TCanvas *c1 = new TCanvas("c1", "", 1300, 1000);
  c1->Clear();
  c1->Divide(2,1);

  c1->cd(1); gPad->SetLogy(1); ivCurve(dirName0);
  c1->cd(2); gPad->SetLogy(1); ivCurve(dirName1);
  //c1->cd(3); gPad->SetLogy(1);
  //ivCurve(dirName2);c1->cd(4); gPad->SetLogy(1);
  //ivCurve(dirName3);c1->cd(5); gPad->SetLogy(1);
  //ivCurve(dirName4);c1->cd(6); gPad->SetLogy(1);
  //ivCurve(dirName5);c1->cd(7); gPad->SetLogy(1);
  //ivCurve(dirName6);c1->cd(8); gPad->SetLogy(1);
  //ivCurve(dirName7);c1->cd(9); gPad->SetLogy(1);
  //ivCurve(dirName8);c1->cd(10); gPad->SetLogy(1);
  //ivCurve(dirName9);c1->cd(11); gPad->SetLogy(1);
  //ivCurve(dirName10);c1->cd(12); gPad->SetLogy(1);
  //ivCurve(dirName11);c1->cd(13); gPad->SetLogy(1);
  //ivCurve(dirName12);c1->cd(14); gPad->SetLogy(1);
  //ivCurve(dirName13);c1->cd(15); gPad->SetLogy(1);
  //ivCurve(dirName14);c1->cd(16); gPad->SetLogy(1);
  //ivCurve(dirName14);c1->cd(16); gPad->SetLogy(1);
  // ivCurve(dirName15);c1->cd(17); gPad->SetLogy(1);
 // ivCurve(dirName16);c1->cd(18); gPad->SetLogy(1);
 // ivCurve(dirName17);c1->cd(19); gPad->SetLogy(1);
 // ivCurve(dirName18);c1->cd(20); gPad->SetLogy(1);
  //ivCurve(dirName19);
    
  c1->SaveAs("ivSummary.ps");


}
