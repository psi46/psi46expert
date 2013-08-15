#include <iostream.h>
#include <math.h>

#include <TH1.h>
#include <TH2.h>
#include <TF1.h>
#include <TProfile.h>
#include <TProfile2D.h>
#include <TGraphErrors.h>

#include "anaTestResults.hh"
#include "anaTestResults.icc"



// ----------------------------------------------------------------------
Double_t Fitfcn( Double_t *x, Double_t *par)
{
  //	if (par[0]*x[0] - par[4] > xCut) return tanXCut + (x[0] - (xCut + par[4])/par[0])* 1e8;
	return TMath::Tan(par[0]*x[0] - par[4]) + par[1]*x[0]*x[0]*x[0] + par[5]*x[0]*x[0] + par[2]*x[0] + par[3];
}


Double_t FitfcnTan( Double_t *x, Double_t *par)
{
	return par[3] + par[2] * TMath::TanH(par[0]*x[0] - par[1]);
}


void anaTestResults::SetAbTest(bool abTest) {
  abCheck = abTest;
}

// ----------------------------------------------------------------------
void anaTestResults::openResults() {
  resultsFile = fopen("results.txt","w");
}

void anaTestResults::closeResults() {
  fclose(resultsFile);
}

// ----------------------------------------------------------------------
void anaTestResults::setHalfModulesA()
{
  cout << "Using half modules typ A " << endl;
  FCHP = 0;
  LCHP = 8;
  NCHP = LCHP - FCHP;
}


// ----------------------------------------------------------------------
void anaTestResults::setHalfModulesB()
{
  cout << "Using bottom half modules " << endl;
  FCHP = 8;
  LCHP = 16;
  NCHP = LCHP - FCHP;
}


// ----------------------------------------------------------------------
void anaTestResults::setFullModules()
{
  cout << "Using full modules " << endl;

  FCHP = 0;
  LCHP = 16;
  NCHP = LCHP - FCHP;
}

void anaTestResults::bookHistograms(const char *dir, const char *temp) {

  fNmod = 0;
  fNchip = 0;
  fMeanTrim = 0.;

  // -- Take out the \\ for T+17a
  TString tempNoSlash(temp);
  tempNoSlash.ReplaceAll("\\", "");

  fFile = new TFile(Form("anaTestresults-%s.root", tempNoSlash.Data()), "RECREATE");

  TH1D *h;
  h = new TH1D("e100", "errors", 21, -1., 20.);
  h->GetXaxis()->SetBinLabel(1, "norm");
  h->GetXaxis()->SetBinLabel(2, "dead");
  h->GetXaxis()->SetBinLabel(3, "noise 1");
  h->GetXaxis()->SetBinLabel(4, "mask");
  h->GetXaxis()->SetBinLabel(5, "bumps");
  h->GetXaxis()->SetBinLabel(6, "add lvl");
  h->GetXaxis()->SetBinLabel(7, "TB 1");
  h->GetXaxis()->SetBinLabel(8, "TB 2");
  h->GetXaxis()->SetBinLabel(9, "TB 3");
  h->GetXaxis()->SetBinLabel(10,"TB 4");
  h->GetXaxis()->SetBinLabel(11,"hi noise");
  h->GetXaxis()->SetBinLabel(12,"lo noise");
  h->GetXaxis()->SetBinLabel(13,"trimmed");
  h->GetXaxis()->SetBinLabel(14,"gain/PH");

  h = new TH1D("h1", "noise", 1100, -100., 1000.);
  h = new TH1D("h2", "noise (edges)", 1100, -100., 1000.);
  h = new TH1D("h100", "trims", 16, 0., 16.);
  h = new TH1D("h200", "addresslevels", 4000, -2000., 2000.);
  h = new TH1D("h201", "modules with noise>400", maxModNr, 0., maxModNr);
  h = new TH1D("h202", "modules with noise<50", maxModNr, 0., maxModNr);
  h = new TH1D("h203", "noise for modules with noise>400", 1100, -100., 1100.);
  h = new TH1D("h204", "noise for modules with noise<50",  1100, -100., 1100.);
  h = new TH1D("h205", "ped for edge pixels",   1000, 0., 1000.);
  h = new TH1D("h206", "ped for inner pixels",  1000, 0., 1000.);

  h = new TH1D("h300", "pixelmap entries", 200, -100., 100.);
  h = new TH1D("h301", "bumpThrDiff", 100, -80., 20.);
  h = new TH1D("h302", "par1", 1000, -5., 15.);
  h = new TH1D("h303", "par2", 2000, 0., 2000.);
  h = new TH1D("h304", "par3", 2000, 0., 2000.);
  h = new TH1D("h305", "par0", 1000, 0., 0.01);


  // -- TH2D covering the entire module
  TH2D *h2;
  h2 = new TH2D("H1", "s-curve width vs. Delta(tthr)", 500, 0., 500., 400, -10., 10.);

  h2 = new TH2D("H100", "trims", 416, 0., 416., 160, 0., 160.);

  h2 = new TH2D("H101", "defects", 416, 0., 416., 160, 0., 160.);
  h2 = new TH2D("H102", "par1 vs. vana", 256, 0., 255.,  100, 0., 10.);
  h2 = new TH2D("H103", "par1 vs. fNchip", 6000, 0., 6000.,  100, 0., 10.);

  h2 = new TH2D("H200", "noise > 400", 416, 0., 416., 160, 0., 160.);
  h2 = new TH2D("H201", "noise < 50",  416, 0., 416., 160, 0., 160.);
  h2 = new TH2D("H202", "ped vs. chip",  16, 0., 16., 80, -200., 600.);
  h2 = new TH2D("H203", "gain vs. chip",  16, 0., 16., 50, 0., 5.);
  h2 = new TH2D("H204", "noise vs. chip",  16, 0., 16., 50, 0., 400.);
  h2 = new TH2D("H205", "tthr vs. chip",  16, 0., 16., 60, 40., 100.);
  h2 = new TH2D("H206", "uthr vs. chip",  16, 0., 16., 60, 40., 100.);
  h2 = new TH2D("H207", "gainR/gainM vs. chip",  16, 0., 16., 50, 0., 0.1);
  h2 = new TH2D("H208", "pedR vs. chip",  16, 0., 16., 50, 0., 50);
  h2 = new TH2D("H209", "tthrR vs. chip",  16, 0., 16., 50, 0., 2.);
  h2 = new TH2D("H210", "tthrM vs. chip",  16, 0., 16., 50, 55., 65.);
  h2 = new TH2D("H211", "Vtrim vs. UthrWidthOld",  256, 0., 256., 60, 0., 60.);
  h2 = new TH2D("H212", "Vtrim vs. UthrWidthNew",  256, 0., 256., 60, 0., 60.);


  h2 = new TH2D("H400", "A-D vs. chip",  maxModNr*16, 0., maxModNr*16, 20, -5., 15.);

  // -- Profiles covering the entire module
  TProfile2D *P2;
  P2 = new TProfile2D("P100", "trimbits", 416, 0., 416., 160, 0., 160., 0., 16.);
  P2 = new TProfile2D("P200", "untrimmed thresholds", 416, 0., 416., 160, 0., 160., 0., 255.);
  P2 = new TProfile2D("P210", "trimmed thresholds", 416, 0., 416., 160, 0., 160., 0., 255.);
  P2 = new TProfile2D("P300", "noise", 416, 0., 416., 160, 0., 160., 0., 2000.);
  P2 = new TProfile2D("P400", "ped", 416, 0., 416., 160, 0., 160., -100., 1000.);
  P2 = new TProfile2D("P500", "gain", 416, 0., 416., 160, 0., 160., 0., 5.);
  P2 = new TProfile2D("P600", "noise", 416, 0., 416., 160, 0., 160., 0., 1000.);
  P2 = new TProfile2D("P700", "tthr", 416, 0., 416., 160, 0., 160., 0., 100.);

  // -- 1d rollouts of all pixels
  TProfile *P1;
  P1 = new TProfile("p100", "trimbits", 16*4160, 0., 16*4160., 0., 20.);
  P1 = new TProfile("p200", "untrimmed thresholds", 16*4160, 0., 16*4160., 0., 255.);
  P1 = new TProfile("p210", "trimmed thresholds", 16*4160, 0., 16*4160., 0., 255.);
  P1 = new TProfile("p300", "noise", 16*4160, 0., 16*4160., 0., 1000.);



  P1 = new TProfile("p333", "par1 vs. fNchip", 6000, 0., 6000.,0,10,"s");


  for (int i = FCHP; i < LCHP; ++i) {
    P1 = new TProfile(Form("p%d", 400+i), Form("C%i:ped vs nmod", i), maxModNr, 0., maxModNr, -200., 600.);
    P1 = new TProfile(Form("p%d", 500+i), Form("C%i:gain vs nmod", i), maxModNr, 0., maxModNr, 0., 6.);
    P1 = new TProfile(Form("p%d", 600+i), Form("C%i:noise vs nmod", i), maxModNr, 0., maxModNr, 0., 400.);
    P1 = new TProfile(Form("p%d", 700+i), Form("C%i:ped/gain vs nmod", i), maxModNr, 0., maxModNr, 0., 200.);
    h  = new TH1D(Form("h%d", 1000+i), Form("C%i:ped", i), 1000, -1000., 1000.);
    h  = new TH1D(Form("h%d", 1100+i), Form("C%i:gain", i), 1000, 0., 10.);
    h  = new TH1D(Form("h%d", 1200+i), Form("C%i:noise", i), 1100, -100., 1000.);
    h  = new TH1D(Form("h%d", 1300+i), Form("C%i:tthr", i), 100, 0., 100.);

    h  = new TH1D(Form("p%d", 4000+i), Form("C%i:voffset_OP vs nmod", i), maxModNr, 0., maxModNr);
    h  = new TH1D(Form("p%d", 4100+i), Form("C%i:vana vs nmod", i), maxModNr, 0., maxModNr);
    h  = new TH1D(Form("p%d", 4200+i), Form("C%i:ibias_DAC vs nmod", i), maxModNr, 0., maxModNr);
    h  = new TH1D(Form("p%d", 4300+i), Form("C%i:vdig vs nmod", i), maxModNr, 0., maxModNr);

    //    h  = new TH1D(Form("ab%d", 100+i), Form("aovergain C%i", i),1000,-0.05,0.5); xxx
    h  = new TH1D(Form("ab%d", 100+i), Form("aovergain C%i", i),1000,0.,5.);
    h  = new TH1D(Form("ab%d", 200+i), Form("C%i:aovergain width vs nmod", i), maxModNr, 0., maxModNr);
    h  = new TH1D(Form("ab%d", 400+i), Form("C%i:aovergain mean vs nmod", i), maxModNr, 0., maxModNr);
    h2  = new TH2D(Form("ab%d", 300+i), Form("nmod vs aovergain C%i", i),100,-0.05,0.5,maxModNr,0.,maxModNr);

  }

  // -- Outliers
  h  = new TH1D("o100", "gain-gainM", 100, -1., 1.);
  h  = new TH1D("o101", "gain outliers", 500, 0., 500.);
  h  = new TH1D("o102", "gain outliers 10s", 500, 0., 500.);
  h  = new TH1D("o110", "tthr - tthrM", 100, -10., 10.);
  h  = new TH1D("o111", "tthr outliers: abs(tthr - tthrM) > 2", 500, 0., 500.);

  h  = new TH1D("o120", "tthr", 100, 50., 70.);
  h  = new TH1D("o121", "tthrM", 100, 55., 65.);




}


// ----------------------------------------------------------------------
void anaTestResults::fillHistograms() {

  // -- Look at grade A or B only
  if ((grade == 1) || (grade == 2)) {
    // A or B
  } else {
    cout << "Module " << nmod << " is grade " << grade << ", returning" << endl;
    return;
  }

  if (nmod == 160) {
    cout << "Module " << nmod << " is ignored, returning" << endl;
    return;
  }

  if (vdig[0] > 6) {
    cout << "Old test, skipping module " << nmod << endl;
    return;
  }

  TH1D *e100 = (TH1D*)fFile->Get("e100");

  TH1D *h1   = (TH1D*)fFile->Get("h1");
  TH1D *h2   = (TH1D*)fFile->Get("h2");
  TH1D *h100 = (TH1D*)fFile->Get("h100");
  TH1D *h200 = (TH1D*)fFile->Get("h200");
  TH1D *h201 = (TH1D*)fFile->Get("h201");
  TH1D *h202 = (TH1D*)fFile->Get("h202");
  TH1D *h203 = (TH1D*)fFile->Get("h203");
  TH1D *h204 = (TH1D*)fFile->Get("h204");
  TH1D *h205 = (TH1D*)fFile->Get("h205");
  TH1D *h206 = (TH1D*)fFile->Get("h206");

  TH1D *h300 = (TH1D*)fFile->Get("h300");
  TH1D *h301 = (TH1D*)fFile->Get("h301");
  TH1D *h302 = (TH1D*)fFile->Get("h302");
  TH1D *h303 = (TH1D*)fFile->Get("h303");
  TH1D *h304 = (TH1D*)fFile->Get("h304");
  TH1D *h305 = (TH1D*)fFile->Get("h305");


  TH2D *H1   = (TH2D*)fFile->Get("H1");
  TH2D *H100 = (TH2D*)fFile->Get("H100");
  TH2D *H101 = (TH2D*)fFile->Get("H101");
  TH2D *H102 = (TH2D*)fFile->Get("H102");
  TH2D *H103 = (TH2D*)fFile->Get("H103");

  TH2D *H200 = (TH2D*)fFile->Get("H200");
  TH2D *H201 = (TH2D*)fFile->Get("H201");
  TH2D *H202 = (TH2D*)fFile->Get("H202");
  TH2D *H203 = (TH2D*)fFile->Get("H203");
  TH2D *H204 = (TH2D*)fFile->Get("H204");
  TH2D *H205 = (TH2D*)fFile->Get("H205");
  TH2D *H206 = (TH2D*)fFile->Get("H206");
  TH2D *H207 = (TH2D*)fFile->Get("H207");
  TH2D *H208 = (TH2D*)fFile->Get("H208");
  TH2D *H209 = (TH2D*)fFile->Get("H209");
  TH2D *H210 = (TH2D*)fFile->Get("H210");
  TH2D *H400 = (TH2D*)fFile->Get("H400");

  TProfile2D *P100 = (TProfile2D*)fFile->Get("P100");
  TProfile2D *P200 = (TProfile2D*)fFile->Get("P200");
  TProfile2D *P210 = (TProfile2D*)fFile->Get("P210");
  TProfile2D *P300 = (TProfile2D*)fFile->Get("P300");
  TProfile2D *P400 = (TProfile2D*)fFile->Get("P400");
  TProfile2D *P500 = (TProfile2D*)fFile->Get("P500");
  TProfile2D *P600 = (TProfile2D*)fFile->Get("P600");
  TProfile2D *P700 = (TProfile2D*)fFile->Get("P700");

  TProfile *p100 = (TProfile*)fFile->Get("p100");
  TProfile *p200 = (TProfile*)fFile->Get("p200");
  TProfile *p210 = (TProfile*)fFile->Get("p210");
  TProfile *p300 = (TProfile*)fFile->Get("p300");
  TProfile *p333 = (TProfile*)fFile->Get("p333");

  TProfile *p400, *p500, *p600, *p700;

  TH1D *h1000, *h1100, *h1200, *h1300;

  TH1D *o100 = (TH1D*)fFile->Get("o100");
  TH1D *o101 = (TH1D*)fFile->Get("o101");
  TH1D *o102 = (TH1D*)fFile->Get("o102");
  TH1D *o110 = (TH1D*)fFile->Get("o110");
  TH1D *o111 = (TH1D*)fFile->Get("o111");

  TH1D *o120 = (TH1D*)fFile->Get("o120");
  TH1D *o121 = (TH1D*)fFile->Get("o121");


  TH2D *modAve = new TH2D(Form("modAve%d", nmod), Form("modAve%d", nmod), 52, 0., 52., 80, 0., 80.);

  TH1D *abh[16];
  for (int i = FCHP; i < LCHP; ++i) {
    //    abh[i]  = new TH1D(Form("singleAb%i", 100+100*fNmod+i), Form("aovergain C%i, mod %i", i, nmod),1000,-0.05,0.5); xxx
    abh[i]  = new TH1D(Form("singleAb%i", 100+100*fNmod+i), Form("aovergain C%i, mod %i", i, nmod),1000,0.,5.);
  }


  cout << Form("Module %4d, fNmod = %d", nmod, fNmod) << endl;
  ++fNmod;

  dead.clear();
  noisy.clear();
  mask.clear();
  bb.clear();
  al.clear();
  tb1.clear();
  tb2.clear();
  tb3.clear();
  tb4.clear();

  int icol(0), irow(0);
  int mx(0), my(0);
  int n = 0;
  int trimZero(0), trimOutlier(0);
  double testMean(0.);

  int cGain(0), c2Gain(0);
  int cTthr(0);

  double aovergain;
  double abWidth[16], abMean[16];

  fprintf(resultsFile, "module %d \n", nmod);

  for (int iChip = FCHP; iChip < LCHP; ++iChip) {

    fNchip = (fNmod-1)*16 + iChip;

    printf("Module number = %i, chip number = %i, fNmod = %i, fNchip = %i\n", nmod, iChip, fNmod, fNchip);

    //    printf("chip number = %d\n", fNchip);

    fprintf(resultsFile, "Chip %d \n", iChip);

    trimZero = 0;
    trimOutlier = 0;

    cGain  = 0;
    c2Gain = 0;

    cTthr  = 0;

    ((TH1D*)fFile->Get(Form("p%d", 4000+iChip)))->SetBinContent(nmod, voffsetop[iChip]);
    ((TH1D*)fFile->Get(Form("p%d", 4100+iChip)))->SetBinContent(nmod, vana[iChip]);
    ((TH1D*)fFile->Get(Form("p%d", 4200+iChip)))->SetBinContent(nmod, ibias_dac[iChip]);
    ((TH1D*)fFile->Get(Form("p%d", 4300+iChip)))->SetBinContent(nmod, vdig[iChip]);

    H207->Fill(iChip, gainR[iChip]/gainM[iChip]);
    H208->Fill(iChip, pedR[iChip]);
    H209->Fill(iChip, tthrR[iChip]);
    H210->Fill(iChip, tthrM[iChip]);

    h1000 = ((TH1D*)fFile->Get(Form("h%d", 1000+iChip)));
    h1100 = ((TH1D*)fFile->Get(Form("h%d", 1100+iChip)));
    h1200 = ((TH1D*)fFile->Get(Form("h%d", 1200+iChip)));
    h1300 = ((TH1D*)fFile->Get(Form("h%d", 1300+iChip)));

    p400 = ((TProfile*)fFile->Get(Form("p%d", 400+iChip)));
    p500 = ((TProfile*)fFile->Get(Form("p%d", 500+iChip)));
    p600 = ((TProfile*)fFile->Get(Form("p%d", 600+iChip)));
    p700 = ((TProfile*)fFile->Get(Form("p%d", 700+iChip)));

    o121->Fill(tthrM[iChip]);
    FillVtrimUthrWidths(iChip);

    //    for (int i = 0; i < 10; ++i) {
    for (int i = 0; i < 4160; ++i) {
      icol = i/80;
      irow = i%80;

      if (iChip < 8) {
	mx = 415-(iChip*52+icol);
	my = 159-irow;
      }
      if (iChip > 7) {
	mx = (iChip-8)*52+icol;
	my = irow;
      }

      H102->Fill(vana[iChip],tanhFit[iChip][i][1],1);
      H103->Fill(fNchip,tanhFit[iChip][i][1],1);
      p333->Fill(fNchip,tanhFit[iChip][i][1],1);
      h302->Fill(tanhFit[iChip][i][1]);
      h303->Fill(tanhFit[iChip][i][2]);
      h304->Fill(tanhFit[iChip][i][3]);
      h305->Fill(tanhFit[iChip][i][0]);

      // -- module plots
      fMeanTrim += trims[iChip][i];
      h100->Fill(trims[iChip][i]);
      H100->Fill(mx, my, trims[iChip][i]);
      P100->Fill(mx, my, trims[iChip][i]);

      e100->Fill(-0.1); // normalization
      if (defects[iChip][i] &   1)  e100->Fill(0.1);  // DEFECT: dead
      if (defects[iChip][i] &   2)  e100->Fill(1.1);  // DEFECT: beat's noise
      if (defects[iChip][i] &   4)  e100->Fill(2.1);  // DEFECT: mask
      if (defects[iChip][i] &   8)  e100->Fill(3.1);  // DEFECT: bump bonding
      if (defects[iChip][i] &  16)  e100->Fill(4.1);  // DEFECT: address level
      if (defects[iChip][i] &  32)  e100->Fill(5.1);  // DEFECT: TB 1
      if (defects[iChip][i] &  64)  e100->Fill(6.1);  // DEFECT: TB 2
      if (defects[iChip][i] & 128)  e100->Fill(7.1);  // DEFECT: TB 3
      if (defects[iChip][i] & 256)  e100->Fill(8.1);  // DEFECT: TB 4

      if (defects[iChip][i] == 0) {
	if (noise[iChip][i] > 400) {
	  cout << Form(" noise:400 Mod %i Chip %i %i/%i: %g", nmod, iChip, icol, irow, noise[iChip][i]) << endl;
	  h201->Fill(nmod);
	  h203->Fill(noise[iChip][i]);
	  H200->Fill(mx, my);

	  e100->Fill(9.1);  // DEFECT: high noise
	}

	if (noise[iChip][i] < 50 && noise[iChip][i] > 0) {
	  cout << Form(" noise:50 Mod %i Chip %i %i/%i: %f", nmod, iChip, icol, irow, noise[iChip][i]) << endl;
	  h202->Fill(nmod);
	  h204->Fill(noise[iChip][i]);
	  H201->Fill(mx, my);

	  e100->Fill(10.1);  // DEFECT: low noise
	}

	if (gain[iChip][i] < 1.) {
	  cout << Form(" gain:1 Mod %i Chip %i %i/%i: %f", nmod, iChip, icol, irow, gain[iChip][i]) << endl;
	  e100->Fill(12.1);  // DEFECT: gain / PH
	}

	p400->Fill(nmod, ped[iChip][i]);
	p500->Fill(nmod, gain[iChip][i]);
	p600->Fill(nmod, noise[iChip][i]);
	p700->Fill(nmod, ped[iChip][i]/gain[iChip][i]);

	P400->Fill(mx, my, ped[iChip][i]);
	P500->Fill(mx, my, gain[iChip][i]);
	P600->Fill(mx, my, noise[iChip][i]);
	P700->Fill(mx, my, tthr[iChip][i]);

	h1000->Fill(ped[iChip][i]);
	h1100->Fill(gain[iChip][i]);
	h1200->Fill(noise[iChip][i]);
	h1300->Fill(tthr[iChip][i]);

	H202->Fill(iChip, ped[iChip][i]);
	H203->Fill(iChip, gain[iChip][i]);
	H204->Fill(iChip, noise[iChip][i]);
	H205->Fill(iChip, tthr[iChip][i]);
	H206->Fill(iChip, uthr[iChip][i]);

	if (ped[iChip][i] < 0) {
	  //	  cout << Form(" ped:0 Mod %i Chip %i %i/%i: %f", nmod, iChip, icol, irow, ped[iChip][i]) << endl;
	}

 	if (tthr[iChip][i] >68) {
 	  cout << Form(" tthr:68 Mod %i Chip %i %i/%i: %i", nmod, iChip, icol, irow, tthr[iChip][i]) << endl;
 	}


	// -- noise distribution
	if (icol == 0 || icol == 51 || irow == 0 || irow == 79) {
	  h2->Fill(noise[iChip][i]);
	  h205->Fill(ped[iChip][i]);
	} else {
	  h1->Fill(noise[iChip][i]);
	  h206->Fill(ped[iChip][i]);
	}

	o100->Fill(gain[iChip][i] - gainM[iChip]);
	if (TMath::Abs(gain[iChip][i] - gainM[iChip]) > 0.12) {
	  cGain++;
	}
	if (TMath::Abs(gain[iChip][i] - gainM[iChip]) > 0.24) {
	  c2Gain++;
	}

	o110->Fill(double(tthr[iChip][i]) - tthrM[iChip]);
	o120->Fill(tthr[iChip][i]);
	if (TMath::Abs(tthr[iChip][i] - tthrM[iChip]) > 10) {
	  e100->Fill(11.1);  // DEFECT: tthr outlier
	}


      }


      H101->Fill(mx, my, defects[iChip][i]);
      if (defects[iChip][i] > 0) {
	cout << Form(" Chip %i %i/%i: %i", iChip, icol, irow, defects[iChip][i]) << endl;

	triplet a = {iChip, icol, irow};

	if (defects[iChip][i] &   1) dead.push_back(a);
	if (defects[iChip][i] &   2) noisy.push_back(a);
	if (defects[iChip][i] &   4) mask.push_back(a);
	if (defects[iChip][i] &   8) bb.push_back(a);

      }

      modAve->Fill(icol, irow, uthr[iChip][i]);

      h300->Fill(pixmap[iChip][i]);
      if ((!(defects[iChip][i] & 7))) h301->Fill(bumpThrDiff[iChip][i]);

      P200->Fill(mx, my, uthr[iChip][i]);
      P210->Fill(mx, my, tthr[iChip][i]);
      if (noise[iChip][i] < 2.e3) {
	P300->Fill(mx, my, noise[iChip][i]);
      }

      if (abCheck)  // not needed anymore, replaced by tanh fit in webupd
	{
	  if (defects[iChip][i] == 0)
	    {

	      phFit = new TF1("phFit", Fitfcn, -400., 1000., nFitParams);
	      phFit->SetNpx(1000);
	      tanFit = new TF1("tanFit", FitfcnTan, 50., 1500., 4);
	      tanFit->SetNpx(1000);

	      n = 0;

	      for (int point = 0; point < 10; point++)
		{
		  if (point == 0) x[n] = 50;
		  else if (point == 1) x[n] = 100;
		  else if (point == 2) x[n] = 150;
		  else if (point == 3) x[n] = 200;
		  else if (point == 4) x[n] = 250;
		  else if (point == 5) x[n] = 210;
		  else if (point == 6) x[n] = 350;
		  else if (point == 7) x[n] = 490;
		  else if (point == 8) x[n] = 560;
		  else if (point == 9) x[n] = 1400;

		  if (ph[iChip][i][point] != 7777)
		    {
		      y[n] = ph[iChip][i][point];
		      n++;
		    }
		}

	      for (int points = 0; points < 10; points++)
		{
		  yErr[points] = 10.;
		  xErr[points] = 2.;
		}

	      graph = new TGraphErrors(n, x, y, xErr, yErr);

	      //	      aovergain = FitPol2()/gain[iChip][i]; xxx
	      aovergain = FitTanH();
	      fprintf(resultsFile, "%f \n", gain[iChip][i]);
	      ((TH1D*)fFile->Get(Form("ab%d", 100+iChip)))->Fill(aovergain);
	      abh[iChip]->Fill(aovergain);

	      ((TH2D*)fFile->Get(Form("ab%d", 300+iChip)))->Fill(aovergain,nmod);

 	      if (i == 1) // condition for writing output can be changed
 		{
//		  fprintf(resultsFile, "aovergain =  %f \n", aovergain);
// 		  graph->Write(Form("graphM%dC%dP%d",nmod,iChip,i));
// 		  fprintf(resultsFile, "module %d, chip %d, pixel %d, aovergain =  %f\n", nmod, iChip, i, aovergain);
 		}
	      
	      delete graph;
	      //	      delete abh[iChip];	
	      
	    }
	} // end abCheck

      // -- roll-out
      p100->Fill(iChip*4160 + i, trims[iChip][i]);
      p200->Fill(iChip*4160 + i, uthr[iChip][i]);
      p210->Fill(iChip*4160 + i, tthr[iChip][i]);
      p300->Fill(iChip*4160 + i, noise[iChip][i]);
      

      // -- suspicious trimbit maps
      if (trims[iChip][i] < 1) {
	trimZero++;
      }

      // -- suspicious trimmed thresholds
      if (tthr[iChip][i] > 200 || tthr[iChip][i] < 20) {
	trimOutlier++;
      }


      testMean += tthr[iChip][i];

      // -- s-curve width vs. trim
      H1->Fill(noise[iChip][i], tthr[iChip][i] - tthrM[iChip]); 

    } // end pixel loop


    o101->Fill(cGain);
    o102->Fill(c2Gain);

    o111->Fill(cTthr);


    for (int i = 0; i < 4000; ++i) {
      h200->Fill(i-2000., addlvl[iChip][i]);
    }

    if ((trimZero > 4) || (trimOutlier > 4)) {
      TH2D *ha = new TH2D(Form("trims_M%d_C%d", nmod, iChip), 
			  Form("trims_M%d_C%d: %s", nmod, iChip, (trimZero > 4? "trimZero": "trimOutlier")), 
			  52, 0., 52., 80, 0., 80.);
      TH2D *hb = new TH2D(Form("uthr_M%d_C%d", nmod, iChip), 
			  Form("uthr_M%d_C%d: %s", nmod, iChip, (trimZero > 4? "trimZero": "trimOutlier")), 
			  52, 0., 52., 80, 0., 80.);
      TH2D *hc = new TH2D(Form("tthr_M%d_C%d", nmod, iChip),
			  Form("tthr_M%d_C%d: %s", nmod, iChip, (trimZero > 4? "trimZero": "trimOutlier")), 
			  52, 0., 52., 80, 0., 80.);
      
      for (int i = 0; i < 4160; ++i) {
	icol = i/80;
	irow = i%80;
	ha->SetBinContent(icol+1, irow+1, trims[iChip][i]);
	hb->SetBinContent(icol+1, irow+1, uthr[iChip][i]);
	hc->SetBinContent(icol+1, irow+1, tthr[iChip][i]);
      }
    }

    if (abCheck)
      {
	abWidth[iChip] = abh[iChip]->GetRMS();
	abMean[iChip] = abh[iChip]->GetMean();
	cout << "Chip " << iChip << " aovergain width = " << abWidth[iChip] << endl;
	((TH1D*)fFile->Get(Form("ab%d", 200+iChip)))->SetBinContent(nmod+1,abWidth[iChip]);
	((TH1D*)fFile->Get(Form("ab%d", 400+iChip)))->SetBinContent(nmod+1,abMean[iChip]);
      }
    
  } // end chip loop

} // end fillHistograms



// ----------------------------------------------------------------------
void anaTestResults::summary() {

  TH1D *h1 = (TH1D*)fFile->Get("h1");
  TH1D *h2 = (TH1D*)fFile->Get("h2");

  double mean(0.);
  int cnt(0);
  TProfile2D *P100 = (TProfile2D*)fFile->Get("P100");
  for (int ix = 0; ix < P100->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < P100->GetNbinsY(); ++iy) {
      ++cnt; 
      mean += P100->GetBinContent(ix, iy); 
    }
  }

  mean = mean / cnt;
  fMeanTrim = fMeanTrim / (fNmod*416*160);

  cout << "trims mean: " << mean << " " << cnt << endl;
  cout << "fMeanTrim:  " << fMeanTrim << " " << endl;
  cout << "center:     " << h1->GetMean() << endl;
  cout << "edges:      " << h2->GetMean() << endl;

}



// ----------------------------------------------------------------------
double anaTestResults::FitPol2()
{
	bool verbose = false;
 	double slope = (x[5]-x[0])/(y[5]-y[0]);

	if (verbose) printf("slope = %e\n",slope);

	phFit->SetParameter(2, slope);
	phFit->SetParameter(3, x[5] - slope * y[5]);
	phFit->SetParameter(5, 0);
	phFit->FixParameter(0, 0.);
	phFit->FixParameter(1, 0.);
	phFit->FixParameter(4, 0.);
	phFit->SetRange(50,500);


 	if (verbose) graph->Fit("phFit", "R", "");
 	else graph->Fit("phFit", "RQ");

	graph->SetTitle("");
	graph->GetYaxis()->SetTitle("Pulse height (ADC units)");
	graph->GetYaxis()->SetRangeUser(-600., 2000.);
	graph->GetXaxis()->SetTitle("Vcal (DAC units)");
	graph->GetXaxis()->SetRangeUser(0., 500.);
	graph->GetYaxis()->SetTitleOffset(1.2);

  	double a = phFit->GetParameter(5);
	//	cout << "a = " <<  phFit->GetParameter(5) << " b = " << phFit->GetParameter(2) << endl;

	if (verbose) printf("a = %e\n",a);

	fprintf(resultsFile, "%f    %f    %f    ", phFit->GetParameter(5), phFit->GetParameter(2), phFit->GetParameter(3));

	return a;

}



// ----------------------------------------------------------------------
double anaTestResults::FitTanH()
{
	bool verbose = false;

	tanFit->SetParameter(0,0.00382);
	tanFit->SetParameter(1,0.886);
	tanFit->SetParameter(2,112.7);
	tanFit->SetParameter(3,113.0);
	tanFit->SetRange(50,1500);


 	if (verbose) graph->Fit("tanFit", "R", "");
 	else graph->Fit("tanFit", "RQ");

	graph->SetTitle("");
	graph->GetYaxis()->SetTitle("Pulse height (ADC units)");
	graph->GetYaxis()->SetRangeUser(-600., 2000.);
	graph->GetXaxis()->SetTitle("Vcal (DAC units)");
	graph->GetXaxis()->SetRangeUser(0., 500.);
	graph->GetYaxis()->SetTitleOffset(1.2);

  	double par1 = tanFit->GetParameter(1);

	if (verbose) printf("par1 = %e\n",par1);

	fprintf(resultsFile, "%f    %f    %f    %f    ", tanFit->GetParameter(0), tanFit->GetParameter(1), tanFit->GetParameter(2), tanFit->GetParameter(3));

	return par1;

}


void anaTestResults::FillVtrimUthrWidths(int iChip)
{
  TH2D *H211 = (TH2D*)fFile->Get("H211");
  TH2D *H212 = (TH2D*)fFile->Get("H212");
  float wert, mean, rms;
  int breite_new, breite_old, meanbin;
  TH1D *histo = new TH1D("histo","",256,0.,256);

   for (int Pix = 0; Pix <4160; Pix = Pix + 1)
   {
      histo -> Fill(uthr[iChip][Pix]);
   }

   mean = (histo -> GetMean());
   rms = (histo -> GetRMS());
   meanbin = histo -> FindBin(mean);


   for (int k = meanbin; k <= meanbin + (int)(5 * rms); ++k)
   {
      wert = histo -> GetBinContent(k);
      breite_new = k - 62;
      if (wert == 0) break;
   }

   for (int k = meanbin + (int)(5 * rms); k >= meanbin; --k)
   {
      wert = histo -> GetBinContent(k);
      breite_old = k - 61;
      if (wert != 0) break;
   }

   H211 -> Fill(vtrim[iChip],breite_old);
   H212 -> Fill(vtrim[iChip],breite_new);

}