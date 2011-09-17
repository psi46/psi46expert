#include <TMath.h>


void addLevels(const char *dir1, const char *dir2) {

  TString directories[] = {TString(dir1),TString(dir2)};
 
  TString histogram = "AddressLevels";

  addLevels_DrawDiff(dir1, directories, 4, histogram, 1);

}


void addLevels_DrawDiff(char *module, TString dirNames[], int ndir, TString histName, int nhist) {

  setStyle();

  TCanvas *c0 = new TCanvas("c0", "", 1000, 700);   // For the fits
  TCanvas *c1 = new TCanvas("c1", "", 700, 1500);    // UB&AL comparison

  c0->Clear();
  c1->Clear();

  c0->Divide(4,4);
  c1->Divide(3,7);

  TH1D *htmpR  = new TH1D("temp Reference","", 7, 0., 7.);
  TH1D *htmpC  = new TH1D("temp Compared","", 7, 0., 7.);

  TH1D *href  = new TH1D("Peak Reference","", 16, 0., 16.);
  TH1D *hcomp = new TH1D("Peak Compared","", 16, 0., 16.);
    
  TH1D *addD1 = new TH1D("UB/AL Difference","", 125, -150., 100.);  
  TH1D *sigD1 = new TH1D("UB/AL Width Difference","", 40, -20., 20.);


  TLatex *tc = new TLatex;
  tc->SetNDC(kTRUE);
  tc->SetTextSize(0.07);

  TLatex *tl = new TLatex;
  tl->SetNDC(kTRUE);
  tl->SetTextSize(0.06);

  TLatex *tg = new TLatex;
  tg->SetNDC(kTRUE);
  tg->SetTextColor(kGreen);
  tg->SetTextSize(0.06);

  TLatex *tb = new TLatex;
  tb->SetNDC(kTRUE);
  tb->SetTextColor(kBlue);
  tb->SetTextSize(0.06);

  TH1D* data;
  Double_t addDataR[16][7];
  Double_t addDataC[16][7];
  Double_t sigDataR[16][7];
  Double_t sigDataC[16][7];

  for (Int_t i = 0; i < 16 ; ++i)  {

     c0->cd(i+1);  gPad->SetTopMargin(0.15);
     addLevels_AddChip(histName, i, dirNames[1], dirNames[0], htmpR, htmpC);  // Find peaks & fit gaussian
     
     //for (Int_t j = 0; j < 7; j++) printf("%d %d %d %d\n",htmpR->GetBinContent(j),htmpC->GetBinContent(j),htmpR->GetBinError(j),htmpC->GetBinError(j));

    for (Int_t j = 0; j < 7; j++)  {

       addDataR[i][j] = htmpR->GetBinContent(j);
       addDataC[i][j] = htmpC->GetBinContent(j);
       sigDataR[i][j] = htmpR->GetBinError(j);
       sigDataC[i][j] = htmpC->GetBinError(j);

    }
  }


  for (Int_t j = 0; j < 7; j++) {

    href->Reset();
    hcomp->Reset();
    addD1->Reset();
    sigD1->Reset();
    

    for (Int_t i = 0; i < 16 ; ++i) {

      href->SetBinContent(i+1, addDataR[i][j]);
      hcomp->SetBinContent(i+1, addDataC[i][j]);
      href->SetBinError(i+1, sigDataR[i][j]);
      hcomp->SetBinError(i+1, sigDataC[i][j]);
      
      if ( addDataR[i][j] > -10000. && addDataC[i][j] > -10000. ) {

        addD1->Fill(addDataR[i][j]-addDataC[i][j]);
        sigD1->Fill(sigDataR[i][j]-sigDataC[i][j]);
      }

    }

    c1->cd(3*j+1);
    gPad->SetTopMargin(0.15);
    hcomp->SetXTitle("Chip Nr.");
    scaleHisto(hcomp, href);

    setHist(hcomp,kBlue,2);
    hcomp->DrawCopy("LP");

    setHist(href,kGreen,26);
    href->DrawCopy("LPSAME");
    
    tb->DrawLatex(0.55, 0.78, Form("%s", dirNames[1].Data()));
    tg->DrawLatex(0.55, 0.71, Form("Ref.: %s", dirNames[0].Data()));

    tl->DrawLatex(0.1, 0.87, Form("Add. Level %i", j));
    if(j==0)  { tc->DrawLatex(0.01, 0.95, "Peak Positions"); 
                tl->DrawLatex(0.1, 0.87, "Ultrablack"); 
                tc->DrawLatex(0.5, 0.95, Form("Module: %s", module)); }
    else      { tl->DrawLatex(0.1, 0.87, Form("Add. Level %i", j)); }
    
    c1->cd(3*j+2); 
    gPad->SetTopMargin(0.15);
    addD1->DrawCopy();
    if(j==0)  { tc->DrawLatex(0.01, 0.95, "Difference of Peak Position"); 
                tl->DrawLatex(0.1, 0.87, "Ultrablack"); 
                tc->DrawLatex(0.5, 0.95, Form("Module: %s", module)); }
    else      { tl->DrawLatex(0.1, 0.87, Form("Add. Level %i", j)); }
    formatHisto(addD1);  
  
    c1->cd(3*j+3); 
    gPad->SetTopMargin(0.15);
    sigD1->DrawCopy();
    if(j==0)  { tc->DrawLatex(0.01, 0.95,"Difference of #sigma"); 
                tl->DrawLatex(0.1, 0.87, "Ultrablack"); 
                tc->DrawLatex(0.5, 0.95, Form("Module: %s", module)); }
    else      { tl->DrawLatex(0.1, 0.87, Form("Add. Level %i", j)); }
    formatHisto(sigD1);
  
  }

  // -- Save canvases ----------------------------------
  c1->SaveAs(Form("%s/diffAddLevels.ps", module));

}
 //-----------------------------------------------------------------
void addLevels_AddChip(const char *parHistogram, int chipId, const char *dirN, const char *refN, TH1D *h1, TH1D *h2) {

   TH1D *ha, *hb;
   ha  = fitPeaks(refN, parHistogram, chipId);
   hb  = fitPeaks(dirN, parHistogram, chipId);

   for (int k = 0; k < 7; k++) { h1->SetBinContent(k,ha->GetBinContent(k)); h1->SetBinError(k,ha->GetBinError(k));}
   for (int k = 0; k < 7; k++) { h2->SetBinContent(k,hb->GetBinContent(k)); h2->SetBinError(k,hb->GetBinError(k));}
}

//-----------------------------------------------------------------
TH1D* fitPeaks(const char *dir, const char *parHist, int chipId) {  

  TFile *f = new TFile(Form("%s/FullTest.root", dir));
  h = (TH1D*)f->Get(Form("%s_C%i", parHist, chipId));

  // -- find peaks in ref. histogram

  //printf("--->%s: Searching for peaks.\n",dir);
  TSpectrum *s    = new TSpectrum(14); // 2*NrOfPeaks = 14
  Int_t nfound    = s->Search(h,10,"new",10);
  Float_t *peaks  = s->GetPositionX();
  Float_t *peaksY = s->GetPositionY();

  //printf("Found %d candidate peaks to fit\n",nfound);

  Double_t ranges[14];

  for (int k = 0; k < 7; k++) {
     ranges[k]   = peaks[k] - 25. ;
     ranges[k+1] = peaks[k] + 25. ;
     //printf("%i. peak at %d (fit range [%d, %d])\n",k, peaks[k], ranges[k], ranges[k+1]);

     }

  Double_t par[21];
  Double_t fitpar [21];  // mean, sigma, const

  for (int k = 0; k < 7; k++) {

    h->Fit("gaus","Q","same", ranges[k], ranges[k+1]);
    h->GetFunction("gaus")->GetParameters(fitpar);
    h->GetFunction("gaus")->SetLineColor(kBlue);
    h->GetFunction("gaus")->SetLineWidth(1);

    for (int m = 0; m < 3; m++) par[3*k + m] = fitpar[m];

  }
  
  for (int k = 0; k < 7; k++) {   
   drawFit(par[3*k], par[3*k + 1], par[3*k + 2])->Draw();     
  }
  
  //printf("Fit parameter\t peak\t mean\t sigma\t const\n");
  for (int k = 0; k < 7; k++) //printf("\t%i.\t %d\t %d\t %4.0d\t %d\n",k, peaks[k], par[3*k + 1], par[3*k + 2], par[3*k]);
  
  TH1D *h2 = new TH1D("peaks","", 10, 0., 10.);

  // -- Take mean of gauss fits
  for (int k = 0; k < 7; k++) {

    if ( par[3*k] > (peaksY[k] - (peaksY[k]/2)) )  {

      h2->SetBinContent(k, par[3*k + 1]);
      h2->SetBinError(k, par[3*k + 2]);

    }
    else {

      h2->SetBinContent(k, -10000);
      h2->SetBinError(k, -10000);

    }
  }

// -- Take peaks from peak search
//  for (int k = 0; k < 7; k++) {
//     h2->SetBinContent(k+1, peaks[k]);
//  }

  return h2;

}
 

//-----------------------------------------------------------------

void setStyle() {

  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
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
//------------------------------------------------------------------------
double findMax(TH1D *h) {

    double max = -2000.;
    double tmp = 0.;

    for (int i = 0; i < 16; ++i) {
      
      tmp = h->GetBinContent(i);
      if ( tmp > max && tmp != 0 ) max = tmp;

    }

    return max;
}
//------------------------------------------------------------------------
double findMin(TH1D *h) {

    double min = 2000.;
    double tmp = 0.;

    for (int i = 0; i < 16; ++i) {
      
      tmp = h->GetBinContent(i);
      if ( tmp < min  && tmp != 0 ) min = tmp;

    }

    return min;
}
//------------------------------------------------------------------------
void scaleHisto(TH1D *ha, TH1D *hb) {

  Double_t extra(0.); Double_t scale(0.6);
  Double_t amax, bmax, amin, bmin;
  Double_t max, min;

  amax = findMax(ha);
  bmax = findMax(hb);
  if ( amax > bmax ) { max = amax; }
  else { max = bmax; }

  amin = findMin(ha);
  bmin = findMin(hb);
  if ( amin < bmin ) { min = amin; }
  else { min = bmin; }
  
  extra = scale*(max - min);

  ha->GetYaxis()->SetRangeUser(min-extra, max+extra);
  hb->GetYaxis()->SetRangeUser(min-extra, max+extra);
  gPad->Update();

}

//------------------------------------------------------------------------
void setHist(TH1 *h, Int_t color = 1, Int_t symbol = 20, Double_t size = 0.6, Double_t width = 1.) {

  h->SetLineColor(color);
  h->SetFillColor(color);
  h->SetMarkerColor(color);
  h->SetMarkerStyle(symbol);
  h->SetMarkerSize(size);
  h->SetLineWidth(width);
  h->SetStats(kFALSE);
  h->SetFillStyle(0);
}

//-----------------------------------------------------------------

TGraph* drawFit(Double_t p0, Double_t p1, Double_t p2) {

   const Int_t n = 50;
   Double_t x[n], y[n];
   for (Int_t i=0;i<n;i++) {
     x[i] = (p1 - 25) + i;
     y[i] = p0*exp(-0.5*pow((x[i]-p1)/p2,2));
   }
   gr = new TGraph(n,x,y);
   gr->SetLineColor(kBlue);
   gr->SetLineWidth(1);

   return gr;

}
