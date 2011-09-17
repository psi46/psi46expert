#include <fstream.h>
#include <iostream.h>
#include <string.h>
#include <iomanip.h>
#include <stdlib.h>

#include "TLatex.h"
#include "TLegend.h"
#include "TLegendEntry.h"
#include "TLine.h"
#include "TArrow.h"
#include "TBox.h"
#include "TString.h"
#include "TObject.h"
#include "TFile.h"
#include "TDirectory.h"
#include "TTree.h"
#include "TH1.h"
#include "TH2.h"
#include "TF1.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TROOT.h"
#include "TVirtualPad.h"  // access to gPad

#include "TUnixSystem.h"
#include "TSystem.h"
#include "TRegexp.h"
#include "TProfile.h"


#include "anaQA.h"


ClassImp(anaQA)


// ----------------------------------------------------------------------
anaQA::anaQA(const char *rootfile, int verbose) {
  cout << "Ctor of anaQA" << endl;

  ts = new TLatex;
  ts->SetNDC(kTRUE);
}


// ----------------------------------------------------------------------
anaQA::~anaQA() {
  cout << "Dtor of anaQA" << endl;
}


// ----------------------------------------------------------------------
void anaQA::makeAll() {
  cout << "anaQA> makeAll()" << endl;

}



// ----------------------------------------------------------------------
void anaQA::diffDAC(const char *baseDir) {
  TString directories[] = {TString(Form("%s",baseDir)),TString(Form("%sN10",baseDir))};
  TString dacParameters[] = {TString("Vana"), TString("VthrComp"),TString("CalDel"),TString("Ibias_DAC")};
  drawDiff(baseDir, directories, 2, dacParameters, 4);
}



// ----------------------------------------------------------------------
void anaQA::drawDiff(const char *module, TString dirNames[], int ndir, TString parNames[], int npar) {
  
  setStyle();
  
  TCanvas *c1 = new TCanvas("c1", "", 900, 500);
  c1->Clear();
  c1->Divide(2,2);  // FIX: dependent on npar
  
  TLatex *tc = new TLatex;
  tc->SetNDC(kTRUE);
  tc->SetTextSize(0.07);
  
  TLatex *tl = new TLatex;
  tl->SetNDC(kTRUE);
  tl->SetTextSize(0.06);

  TH1D *dacD1 = new TH1D("DAC Difference","", 80, -40., 40.);
  
  char dac[50];  // Form("%s") does not work with parNames[ipar]
 
  for(int ipar = 0; ipar < npar; ++ipar) {
  
    dacD1->Reset();
    sprintf(dac, (parNames[ipar]).Data());
    
    for (int i = 0; i < 16; ++i)  getDiff(parNames[ipar], i, dirNames[1], dirNames[0], dacD1); 
    
    if (0 == ipar) c1->cd(1);
    if (1 == ipar) c1->cd(2);
    if (2 == ipar) c1->cd(3);
    if (3 == ipar) c1->cd(4);
//     c1->cd(iprob+1);                  FIX
    gPad->SetTopMargin(0.15);   
    dacD1->Draw();
    tc->DrawLatex(0.1, 0.95, Form("%s", dac));
    formatHisto(dacD1);
        
  }
  
  c1->SaveAs(Form("%s/diffDACs.ps", module));
}


// -----------------------------------------------------------------
void anaQA::getDiff(const char *dacParameter, int chipId, const char *dirN, const char *refN, TH1D *h) { 
    
   Double_t vref, vcomp;
   vref  = findParameter(refN, dacParameter, chipId);
   vcomp = findParameter(dirN, dacParameter, chipId);
    
   h->Fill(vref - vcomp); 

}


// -----------------------------------------------------------------
int anaQA::findParameter(const char *dir, const char *dacPar, int chipId) { 

  FILE *File;
  char fname[1000];
  char string[2000]; int a;
  int prm(-1);
   
  sprintf(fname, "%s/dacParameters_C%i.dat", dir, chipId);
  File = fopen(fname, "r");

  if (!File)
  {
    printf("!!!!!!!!!  ----> DAC Parameters: Could not open file %s to read DAC parameter\n", fname);
  }
  else
  {
    for (int i = 0; i < 29; i++) {
  
      fscanf(File, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  return prm;

    }
  }
  
  printf("!!!!!!!!!  ----> DAC Parameters: Parameter %s not found\n", string);
  return -99.;
}


//-----------------------------------------------------------------
void anaQA::setStyle() {

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

 
// -----------------------------------------------------------------
void anaQA::formatHisto(TH1D *h) {
 
  ts->SetTextSize(0.06);
  
  if ( h->GetMean() < 0 ) {
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
