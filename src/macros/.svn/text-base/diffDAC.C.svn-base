#include <TMath.h>
#include <stdio.h>
#include <stdlib.h>

FILE *File, *File50, *File60;
char fname[1000];

Int_t EColor[6]        = { kBlack, kMagenta, kBlue, kGreen, kCyan, kRed }; 
Int_t ELineStyle[4]    = { kSolid , kDashed, kDotted, kDashDotted };
Int_t EMarkerStyle[10] = { kDot, kPlus, kStar, kCircle, kMultiply, kFullDotMedium, 
                           kFullCircle, kFullSquare, kFullTriangleUp, kFullTriangleDown};
int max=-1000;
int min=1000;
//-----------------------------------------------------------------------------------------
void dac(const char *dir1, const char *dir2) {

  TString directories[] = {TString(dir1),TString(dir2)};
  
  TString dacParameters[] = {TString("Vana"), TString("VthrComp"),TString("CalDel"), 
			     TString("Ibias\_DAC"),TString("Vtrim"),TString("VoffsetOp")};
  
  dac_DrawDiff(dir1, directories, 2, dacParameters, 6);
  
}

//-------------------------after irradiation-------------------------------------------------
void dacRad(int modNr, int maxDose) {

  int m(0);
  int dose(0);
  char mod[10];
  m = floor(2+log(maxDose)/log(2));
  printf("%i measurements, right?\n", m);

  const int elements = m;
  TString directories[elements];
  directories[0] = TString(Form("Rad00M%.2i", modNr));
  printf("Dose: %i -> Ref.Directory: %s\n", dose, directories[0].Data());

  for (int i = 0; i < (m-1) ; i++) {

    dose = pow(2,i);
    directories[i+1] = TString(Form("Rad%.2iM%.2i/T-10a", dose, modNr)), TString("Rad00M%.2i", modNr);
    printf("Dose: %i -> Directory: %s\n", dose, directories[i+1].Data());
  
  }
  
  TString dacParameters[] = {TString("Vana"), TString("VthrComp"),TString("CalDel"), TString("Ibias\_DAC"),TString("Vtrim"),TString("VoffsetOp")};
  
  sprintf(mod,"M00%.2i",modNr);
  
  dac_DrawDiff(mod, directories, m, dacParameters, 6);

}
//-------------------------after annnealing-------------------------------------------------
void dacAnn(int modNr, int maxDose) {

  int m(0);
  int dose(0);
  char mod[10];
  m = floor(2+log(maxDose)/log(2));
  printf("%i measurements, right?\n", m);

  const int elements = m;
  TString directories[elements];
  directories[0] = TString(Form("Rad00M%.2i", modNr));
  printf("Dose: %i -> Ref.Directory: %s\n", dose, directories[0].Data());

  for (int i = 0; i < (m-1) ; i++) {

    dose = pow(2,i);
    directories[i+1] = TString(Form("Rad%.2iM%.2i/T-10b", dose, modNr)), TString("Rad00M%.2i", modNr);
    printf("Dose: %i -> Directory: %s\n", dose, directories[i+1].Data());
  
  }
  
  TString dacParameters[] = {TString("Vana"), TString("VthrComp"),TString("CalDel"), TString("Ibias\_DAC"),TString("Vtrim"),TString("VoffsetOp")};
  
  sprintf(mod,"M00%.2i",modNr);
  
  dac_DrawDiff(mod, directories, m, dacParameters, 6);

}

//==============================================================================================
void dac_DrawDiff(char *module, TString dirNames[], const int ndir, TString parNames[], const int npar) {
    
  setStyle();
  
  TCanvas *c1 = new TCanvas("c1", "", 800, 800);
  c1->Clear();
  c1->Divide(3,4);
  
  TCanvas *c2 = new TCanvas("c2", "", 800, 600);
  c2->Clear();
  c2->Divide(3,2);

  TLatex *tc = new TLatex;
  tc->SetNDC(kTRUE);
  tc->SetTextSize(0.07);
  
  TLatex *tl = new TLatex;
  tl->SetNDC(kTRUE);
  tl->SetTextSize(0.06);

  // per chip
  TH1D *dDAC  = new TH1D("DAC Difference","", 80, -40., 40.);
  TH1D *href  = new TH1D("DAC Reference","", 16, 0., 16.);
  TH1D *hcomp = new TH1D("DAC Compared","", 16, 0., 16.);

  // per module
  double dacModule(-1);
  double dacModErr(-1);
  TH1D *have[npar];
  for(int ipar = 0; ipar < npar; ++ipar) have[ipar] = new TH1D(Form("%s", parNames[ipar].Data()),"", pow(2,ndir-2)+3, -1., pow(2,ndir-2)+2);

  TLatex *tref = new TLatex;
  tref->SetNDC(kTRUE);
  tref->SetTextSize(0.06);

  TLatex *tcomp = new TLatex;
  tcomp->SetNDC(kTRUE);
  tcomp->SetTextSize(0.06);

  for(int idir = 0; idir < ndir-1; ++idir) {

    for(int ipar = 0; ipar < npar; ++ipar) {

      href->Reset();
      hcomp->Reset();
      dDAC->Reset();

      // diff Plots (histograms)
      if (ipar < 3 ) { c1->cd(ipar+1);  gPad->SetTopMargin(0.15); }
      if (ipar > 2 ) { c1->cd(ipar+4);  gPad->SetTopMargin(0.15); }

      for (int i = 0; i < 16; ++i)  dac_GetDiff(parNames[ipar], i, dirNames[idir+1].Data(), dirNames[0].Data(), dDAC);
      
      formatHisto(dDAC, idir+1);

      if (idir == 0 ){ 
        dDAC->GetYaxis()->SetRangeUser(0,15);
	dDAC->DrawCopy(); formatHisto(dDAC, idir+1);

      }
      else {  

	dDAC->DrawCopy("SAME");

      }

      tc->DrawLatex(0.1, 0.87, Form("%s", parNames[ipar].Data()));

      // values for each chip/test
      if (ipar < 3 )  { c1->cd((ipar+1)+3); gPad->SetTopMargin(0.15); } 
      if (ipar > 2 )  { c1->cd((ipar+1)+6); gPad->SetTopMargin(0.15); }

      for (int i = 0; i < 16; ++i) dac_AddChip(parNames[ipar], i, dirNames[idir+1].Data(), dirNames[0].Data(), href, hcomp );

      if ( idir == 0 ) {

	dacModule = calcMean(href);
	dacModErr = calcRMS(href, dacModule);
	//printf("mean dacs: %i %i %d +/- %d\n",ipar, idir+1, dacModule, dacModErr);
	have[ipar]->SetXTitle("Dose / Mrad");
      	have[ipar]->SetBinContent(2, dacModule);
	have[ipar]->SetBinError(2, dacModErr);

	href->SetXTitle("Chip Nr.");

        href->GetYaxis()->SetRangeUser(0,255);
        tref->SetTextColor(EColor[0]);
        formatPlot(href,idir);
	href->DrawCopy("LP");
        tref->DrawLatex(0.60, 0.78, Form("%s", dirNames[idir].Data()));
        formatPlot(href,idir);

      }
      


      dacModule = calcMean(hcomp);
      dacModErr = calcRMS(hcomp, dacModule);
      //printf("mean dacs: %i %i %d +/- %d\n", ipar, idir+2, dacModule, dacModErr);
      have[ipar]->SetXTitle("Dose / Mrad");
      have[ipar]->SetBinContent(floor(pow(2,idir))+2, dacModule);
      have[ipar]->SetBinError(floor(pow(2,idir))+2, dacModErr);

      tc->DrawLatex(0.1, 0.87, Form("%s", parNames[ipar].Data()));
      tcomp->SetTextColor(EColor[idir+1]);
      tcomp->DrawLatex(0.60, 0.72-idir*0.06, Form("%s", dirNames[idir+1].Data()));
      formatPlot(hcomp,idir+1);
      scaleHisto(href, hcomp); 
      hcomp->DrawCopy("LPSAME"); 
 

    }
  }
  
  c1->cd(1);
  tl->DrawLatex(0.1, 0.95, "DAC Difference (after trimming)");
  c1->cd(3);
  tc->DrawLatex(0.75, 0.90, Form("%s", module));
  // c1->Update();
  c1->SaveAs(Form("%s/diffDACs.ps", module));

  for(int pad = 0; pad < npar; pad++) {

    c2->cd(pad+1);
    gPad->SetTopMargin(0.15);  
    gPad->SetLogx(1);
    formatPlot(have[pad],pad);
    have[pad]->Draw("LP");
    tc->DrawLatex(0.1, 0.87, Form("%s", parNames[pad].Data()));
  
  }

  c2->cd(1);
  tl->DrawLatex(0.1, 0.95, "Averaged DAC value of module");
  c1->cd(3);
  tc->DrawLatex(0.75, 0.90, Form("%s", module));
  // c2->Update();
  c2->SaveAs(Form("%s/aveDACs.ps", module));

}


//==============================================================================================
void dac_AddChip(const char *dacParameter, Int_t chipId, const char *dirN, const char *refN, TH1D *h1, TH1D *h2) {

   int vref(0), vcomp(0);
   vref  = dac_findParameter(refN, dacParameter, chipId);
   if ( vref != 0  ) h1->SetBinContent(chipId+1, vref);

   vcomp = dac_findParameter(dirN, dacParameter, chipId);
   if ( vcomp != 0 ) h2->SetBinContent(chipId+1, vcomp);

}

 //-----------------------------------------------------------------
void dac_GetDiff(const char *dacParameter, int chipId, const char *dirN, const char *refN, TH1D *h) { 

   int vref(0), vcomp(0);
   vref  = dac_findParameter(refN, dacParameter, chipId);
   vcomp = dac_findParameter(dirN, dacParameter, chipId);

   if(vref > max )  max = vref; 
   if(vref < min )  min = vref; 

   if(vcomp > max )  max = vcomp; 
   if(vcomp < min )  min = vcomp; 

   if ( vref != 0 && vcomp != 0 ) h->Fill(vref - vcomp); 

}

//-------------------------------------------------------------------------
int dac_findParameter(const char *dir, const char *dacPar, int chipId) {

  char string[2000]; int a;
  int prm(-1);
   
  sprintf(fname, "%s/dacParameters_C%i.dat", dir, chipId);
  File = fopen(fname, "r");
  //printf("%s\n",fname);
  
  sprintf(fname, "%s/dacParameters50_C%i.dat", dir, chipId);
  File50 = fopen(fname, "r");
  //printf("%s\n",fname);
  
  sprintf(fname, "%s/dacParameters60_C%i.dat", dir, chipId);
  File60 = fopen(fname, "r");
  // printf("%s\n",fname);
  
  if ( File60 )
  {
    //printf("Reading %s from dac Parameters 60  (C%i) ...\n", dacPar, chipId);

    for (int i = 0; i < 29; i++) {
  
      fscanf(File60, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;
    }
  }
  
  if ( File50 && !File60 )
  {
    //printf("Reading %s from dac Parameters 50 (C%i) ...\n", dacPar, chipId);

    for (int i = 0; i < 29; i++) {

      fscanf(File50, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;
    }
  }
  
  if ( !File50 && !File60 )
  { 
    
    if ( !File )
    {
      printf("!!!!!!!!!  ----> DAC Parameters: Could not find a file to read DAC parameters in %s\n", dir);

      return 0;
    }
    else
    {
      printf("No DAC Parameters after trimming available. Reading %s  (C%i) ...\n", dacPar, chipId);
      for (int i = 0; i < 29; i++) {
  
	fscanf(File, "%i %s %i", &a, string, &prm);
	if ( strcmp(dacPar,string) == 0 )  break;
      }
    
    }

  }
   
  if (File)   fclose(File);
  if (File50) fclose(File50);
  if (File60) fclose(File60);

  return prm;

}

//-----------------------------------------------------------------
double calcMean(TH1D *h) {

  double mean(-1);

  for (int m = 0; m < 16; m++) {
    
    mean += h->GetBinContent(m+1);
  }

  mean = mean/16;

  return mean;
}
  
//-----------------------------------------------------------------
double calcRMS(TH1D *h, double mean) {

  double rms(-1);

  for (int m = 0; m < 16; m++) {

    rms += pow(mean - h->GetBinContent(m+1),2);
  }

  rms = sqrt(rms)/15;

  return rms;
}
//-----------------------------------------------------------------
 
void formatHisto(TH1D *h, int dir) {
  
  TLatex *ts = new TLatex;
  ts->SetNDC(kTRUE);
  ts->SetTextColor(EColor[dir]);
  ts->SetTextSize(0.06);
  h->SetLineColor(EColor[dir]);

  double offset;
  offset = 0.92 - dir*0.18; 

  if ( h->GetMean() < (h->GetXaxis()->GetXmin()+((h->GetXaxis()->GetXmax() - h->GetXaxis()->GetXmin())/2)) ) {

    //ts->DrawLatex(0.60, offset, Form("Entries:%4.0f", h->GetEntries()));
    ts->DrawLatex(0.65, offset, Form("Entries:%4.0f", h->GetEntries()));
    offset -= 0.06;
    ts->DrawLatex(0.65, offset, Form("Mean:%4.2f", h->GetMean()));
    offset -= 0.06;
    ts->DrawLatex(0.65, offset, Form("RMS: %4.2f", h->GetRMS()));
  }
  
  else {

    //ts->DrawLatex(0.15, offset, Form("Entries:%4.0f", h->GetEntries()));
    ts->DrawLatex(0.65, offset, Form("Entries:%4.0f", h->GetEntries()));
    offset -= 0.06;
    ts->DrawLatex(0.65, offset, Form("Mean:%4.2f", h->GetMean()));
    offset -= 0.06;
    ts->DrawLatex(0.65, offset, Form("RMS: %4.2f", h->GetRMS()));
  }  
  
}
//-----------------------------------------------------------------------
void scaleHisto(TH1D *ha, TH1D *hb) {
 
  Double_t extra(0.); Double_t scale(0.6);
  extra = scale*(max - min);

  ha->GetYaxis()->SetRangeUser(min-extra, max+extra);
  hb->GetYaxis()->SetRangeUser(min-extra, max+extra);
  gPad->Update();

}

//------------------------------------------------------------------------
void formatPlot(TH1 *h, int dir) {

  TLatex *ts = new TLatex;
  ts->SetNDC(kTRUE);
  ts->SetTextColor(EColor[dir]);
  ts->SetTextSize(0.06);

  h->SetLineColor(EColor[dir]);
  h->SetFillColor(EColor[dir]);
  h->SetMarkerColor(EColor[dir]);
  h->SetMarkerStyle(EMarkerStyle[dir]);
  h->SetMarkerSize(0.6);
  h->SetLineWidth(1.);
  h->SetStats(kFALSE);
  h->SetFillStyle(0);

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
