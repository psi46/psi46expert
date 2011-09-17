#include <TMath.h>
#include <TParameter.h>

const char* fileName = "ShortTest.root";
const char* adFileName = "adDec.root"; 

char fname[200];
FILE *inputFile, *critFile;

TCanvas *c1 = NULL;
TLatex *tl;     
TLatex *ts;
TLine *line;
TBox *box;

float defectsB, defectsC, maskdefB,  maskdefC;
float currentB, currentC, slopeivB,  slopeivC;
float noiseB,   noiseC,   trimmingB, trimmingC;
float gainB,    gainC,    pedestalB, pedestalC, parB, parC;
float gainMin, parMin, parMax;
float pedDistr, gainDistr, trmDistr;

int nChips(16);
int startChip(0);
int readVerbose(1);

int fitsProblemB[] = {0, 0, 0};
int fitsProblemC[] = {0, 0, 0};
int currentProblemB(0), currentProblemC(0);
int slopeProblemB(0),   slopeProblemC(0);

int nMaskProblem(0);
 


// ----------------------------------------------------------------------
void all() {

  // -- find all files matching pattern in the current directory
  void *dirp = gSystem->OpenDirectory(".");
  const char *afile;
  while(afile = gSystem->GetDirEntry(dirp)) {
    if (strstr(afile, "M")) {
      cout << afile << endl;
      moduleSummary(afile);
    }
  }

}


// ----------------------------------------------------------------------
void moduleSummary(const char *dirName = "", const char *module_type) 
{

  printf("\nmoduleSummary> Starting ...\n");

  nChips = 16;
  startChip = 0;
	
  if ( !strcmp(module_type,"a") ) {
    
    nChips = 8; 
    startChip = 0; 
  }
  
  if ( !strcmp(module_type,"b") ) {
    
    nChips = 8; 
    startChip = 8; 
  }

  sprintf(fname, "%s/%s", dirName, fileName);
  inputFile = fopen(fname, "r");
  if (!inputFile) { 

    printf("\nmoduleSummary> ----> COULD NOT FIND %s IN DIRECTORY %s\n", fileName, dirName);
    printf("moduleSummary> ----> Aborting execution of moduleSummaryPage.C ... \n\n", fileName, dirName);   
    break; 
  }


  sprintf(fname, "%s/%s", dirName, adFileName);
  inputFile = fopen(fname, "r");
  if (!inputFile) {
    
    sprintf(adFileName,"%s", fileName); 
  }
  else {
    
    printf("moduleSummary> ----> found separate address decoding file: %s\n", adFileName); 
    fclose (inputFile); 
  }
  
  sprintf(fname, "%s/../../../macros/criteria-short.dat", dirName);
  if ( !readCriteria(fname) ) {  
    
    printf("\nmoduleSummary> ----> COULD NOT READ GRADING CRITERIA !!!\n");
    printf("moduleSummary> ----> Aborting execution of moduleSummaryPage.C ... \n\n", fileName, dirName);  
    break;
  }

  
  TFile *f = new TFile(Form("%s/%s", dirName, fileName));

  gROOT->SetStyle("Plain");
  
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetTitle(0);

  gStyle->SetStatFont(132);
  gStyle->SetTextFont(132);
  gStyle->SetLabelFont(132, "X");
  gStyle->SetLabelFont(132, "Y");
  gStyle->SetLabelSize(0.08, "X");
  gStyle->SetLabelSize(0.08, "Y");
  gStyle->SetTitleSize(0.08, "X");
  gStyle->SetTitleSize(0.08, "Y");
  gStyle->SetNdivisions(10, "X");
  gStyle->SetNdivisions(8, "Y");
  gStyle->SetTitleFont(132);
    
  gROOT->ForceStyle();
  
  tl = new TLatex;
  tl->SetNDC(kTRUE);
  tl->SetTextSize(0.1);
  
  ts = new TLatex;
  ts->SetNDC(kTRUE);
  ts->SetTextSize(0.09);

  line = new TLine;
  line->SetLineColor(kRed);
  line->SetLineStyle(kDashed);

  box = new TBox;
  box->SetFillColor(3);
  box->SetFillStyle(3004);

  c1 = new TCanvas("c1", "", 900, 700);
  c1->Clear();
  c1->Divide(1,3);
    
  int EColor[6]        = { 4, 8, 6, 1 };  
  int EMarkerStyle[10] = { 4, 25, 26, 23, 21, 27, 28, 20, 30, 29 };

  TH2D *mAlive     = new TH2D("mAlive",     "", 416, 0., 416., 160, 0., 160.);

  const int nfit = 3;  
  TString fitNames[] = {TString("Parameter 1"), TString("Rel. Gain Width"), TString("Pedestal Spread")};
  float limitB[] = { parB, gainB, pedestalB };   // limit for grading
  float limitC[] = { parC, gainC, pedestalC };   // limit for grading
  float max[]    = { parB + 0.5, gainB + 0.05, pedestalB + 1000. };    // scaling of histogram


  TH1D *fit[nfit];
  TH1D *fitEntries[nfit];

  for(int ifit = 0; ifit < nfit; ++ifit) {

    fit[ifit] = new TH1D(Form("%s", fitNames[ifit].Data()),"", nChips, float(startChip), float(startChip+nChips));
    fitEntries[ifit] = new TH1D(Form("n%s", fitNames[ifit].Data()),"", nChips, float(startChip), float(startChip+nChips));

    fit[ifit]->SetLineColor(EColor[ifit]);
    fit[ifit]->SetMarkerColor(EColor[ifit]);
    fit[ifit]->SetMarkerStyle(EMarkerStyle[ifit]);
    fit[ifit]->SetMarkerSize(0.5);
  }
 
  for (int i = startChip; i < startChip+nChips; i++) { addChip("PixelMap", i, mAlive); }

  if ( nChips < 16 && startChip == 0 ) { 
    for (int i = 8; i < nChips+8; i++) { removeChip(i, mAlive, -99); }
  }

  if ( nChips < 16 && startChip == 8 ) { 
    for (int i = 0; i < nChips; i++) { removeChip(i, mAlive, -99); }
  }

  TString noslash(dirName);
  noslash.ReplaceAll("/", "");
  noslash.ReplaceAll("..", "");
  
  if ( mAlive ) {
   
    c1->cd(1);
    mAlive->SetMaximum(10.);
    mAlive->SetMinimum(0.);  
    mAlive->DrawCopy("colz");
  }

  tl->DrawLatex(0.1, 0.92, "Pixel map");

  //   c1->cd(2);
//   c1_2->Divide(4,1);
//   c1_2->cd(1);
//   TH1 *hTW1 = (TH1*)(f->Get("twBefore"));
//   if ( hTW1 ) {
//     hTW1->SetTitle("");
//     hTW1->Draw();
//   }
//   tl->DrawLatex(0.2, 0.93, "TimeWalk");
  
//   c1_2->cd(2);
//   TH1 *hTW2 = (TH1*)(f->Get("twAfter"));
//   if (  hTW2 ) {
//     hTW2->SetTitle("");
//     hTW2->Draw();
//   }
//   tl->DrawLatex(0.2, 0.93, "TimeWalk optimized");

//   c1_2->cd(3);
//   TH1F *iana = new TH1F("iana", "", 16, 16., 32.);
//   TParameter<double>* pard;
//   for (int i = 0; i < nChips; i++)
//   {
//     pard = (TParameter<double>*)f->Get(Form("IA_C%i", i + startChip));
//     if ( pard ) iana->Fill(pard->GetVal()*1000.);  }

//   iana->Draw();
//   tl->DrawLatex(0.2, 0.93, "Iana");
  
//   if ( nChips < 16 && startChip == 0 ) { 
   
//     box->SetFillColor(29);
//     box->DrawBox( 0, 0,  416,  80);
//   }

//   if ( nChips < 16 && startChip == 8 ) { 
   
//     box->SetFillColor(29);
//     box->DrawBox( 0, 80,  416,  160);
//   }
  
  c1->cd(2);
  c1_2->Divide(3,1);
  c1_2->cd(1);
  gPad->SetBottomMargin(0.2);
  gPad->SetLogy(1);
  gPad->SetLeftMargin(0.20);
  gPad->SetRightMargin(0.01);
  
  float V, A;
  float x_V[250], y_A[250];
  int i(0); 
  float iv100(0.);
  float iv150(0.);
  float iv150_17(0.);
  float iv100_17(0.);
  float variation(0.);
  float variation_17(0.);
  
  FILE *sumWrite, *sumRead, *gradWrite; 
    
    
  char mod[20] = noslash.Data(), waf[20] = "",  test[20] = "",tmon[20], trim[20], ph[20], cycl[20];
  int tday;
  int dp(0), dm(0), db(0), dt(0), da(0);
  int root(0), a(0), b(0), c(0); 
  int badRocs[3] = {0, 0, 0};
  float voltage, current; 
  float temp, tempSigma, sollTemp;
  float cyclMean, cyclSigma;
  char  string[1000];
  
  c1_2->cd(2);
  
  sprintf(fname, "%s/summaryTest.txt", dirName);
  sumRead = fopen(fname, "r");

  if (!sumRead)
  {
    printf("\nmoduleSummary> !!!!!!!!!  ----> File %s does not exist yet...\n", fname);
    printf("moduleSummary> !!!!!!!!!  ----> Module summary not complete!\n\n");
  }
  else
  {
    fgets(string, 200, sumRead);
    //   fscanf(sumRead, "%s %s", string, mod);
    fscanf(sumRead, "%s %s %s %s", string, string, waf, test);
    fscanf(sumRead, "%s %i %i %i %i %i", string, &dp, &dm, &db, &dt, &da);
    nMaskProblem = dm;
    fscanf(sumRead, "%s %s %s %s %s %i %i %i", string, string, string, string, string, &a, &b, &c);
    badRocs[0]=a;  badRocs[1]=b;  badRocs[2]=c;
    fscanf(sumRead, "%s %s %i", string, string, &root);
    fscanf(sumRead, "%s %s %s %s %s %i %s %s", string, string, string, string, tmon, &tday, string, string);

    fgets(string, 200, sumRead);
    fscanf(sumRead, "%s %s", string, trim);
    fgets(string, 200, sumRead); 
    fscanf(sumRead, "%s %s", string, ph);
    fgets(string, 200, sumRead);
    
    fscanf(sumRead, "%s %f %f %s %f", string, &temp, &tempSigma, string, &sollTemp); 

    fscanf(sumRead, "%s %s %s %f %f", string, string, cycl, &cyclMean, &cyclSigma);

    fscanf(sumRead, "%s %s %f", string, string, &iv150);

    fclose(sumRead);
    
    tl->SetTextSize(0.09);
    tl->SetTextFont(22);
    double y = 0.92;
    tl->DrawLatex(0.01, y, Form("Test Summary of %s     %s", waf, test));
    tl->SetTextFont(132); 
    tl->SetTextSize(0.09);

    y -= 0.16;
    tl->DrawLatex(0.01, y, "ROCs > 1% defects: ");
    tl->DrawLatex(0.5, y, Form("%i", badRocs[0]));
    
    y -= 0.12;
    tl->DrawLatex(0.01, y, Form("Dead Pixel: "));
    tl->DrawLatex(0.5, y, Form("%i", dp));

    y = 0.76;
    tl->DrawLatex(0.72, y, Form("Tested on:"));
    y -= 0.11;
    tl->DrawLatex(0.72, y, "PhCal: ");

    c1_2->cd(3);
    y = 0.76;
    tl->DrawLatex(0.01, y, Form("%s %i", tmon, tday));
    
    y -= 0.11;
    tl->DrawLatex(0.01, y, Form("%s", ph));

  }

  int result;
  int tbm1(1), tbm2(1);
  TParameter<int>* par;
  

  // Convert current to currents at room temperature
  double Tk = 273.15;
  double egap = 1.12;
  double kB = 8.617343E-5;
  double tTest;
  // tTest = temp;  // --> averaged temperature
  tTest = sollTemp;
  
  double expnt  = egap*(1/(Tk+tTest) - 1/(Tk+17))/(2*kB);
  double fctr   = (Tk+17)*(Tk+17)/((Tk+tTest)*(Tk+tTest));
  
  iv150_17 = iv150*fctr*TMath::Exp(expnt);

  printf("\nmoduleSummary> converted I(150 V, %.0f C)    = %.4f       to  I(150 V, 17 C)    = %.4f \n", tTest, iv150, iv150_17);

  if ( iv150_17 != 0 ) {

    c1_2->cd(2);
    y = 0.52;
    tl->DrawLatex(0.01, y, "I(150 V) [T = 17 ^{o}C]");
    tl->DrawLatex(0.50, y, Form("%.2f #muA", iv150_17));
    c1_2->cd(3);
    
  }

  sprintf(fname, "%s/summaryTest.txt", dirName);
  sumWrite = fopen(fname, "a");

  c1_3->Divide(3,1);
  qualification(dirName, fit, fitEntries);
  

  for (int ifit = 0; ifit < nfit; ifit++) {

  // makePlot(TH1 *h, const char *title, int pad, double Ymin, double Ymax, double Ylimit)
    makePlot(fit[ifit], fitNames[ifit].Data(), ifit+1, 0, max[ifit], limitB[ifit], limitC[ifit]); 
  
  }

  FILE *missingData; 
  sprintf(fname, "%s/comment_3.txt", dirName);
  missingData = fopen(fname, "r");

  int grad(0);
  if ( missingData ) {

    printf("\nmoduleSummary> !!!!!!!!!  ----> Found file for missing data: comment_3.txt => GRADE C!\n\n");
    fclose(missingData);

  } else {
  
    grad = grading(badRocs, nMaskProblem, iv150_17, variation_17, fit, fitEntries, limitB, limitC, test);
  }
  
  c1_2->cd(3);    
  tl->SetTextSize(0.09);
  tl->SetTextFont(22);
 
  if (grad == 1) {  tl->DrawLatex(0.6, 0.92, "GRADE:  A");  fputs("Grade A\n", sumWrite); }
  if (grad == 2) {  tl->DrawLatex(0.6, 0.92, "GRADE:  B");  fputs("Grade B\n", sumWrite); }
  if (grad == 3) {  tl->DrawLatex(0.6, 0.92, "GRADE:  C");  fputs("Grade C\n", sumWrite); }  


  sprintf(fname, "%s/gradingTest.txt", dirName);
  gradWrite = fopen(fname, "a");
 
  if (!gradWrite)
  {
    printf("\nmoduleSummary> !!!!!!!!!  ----> File %s does not exist yet...\n", fname);
    printf("moduleSummary> !!!!!!!!!  ----> Grading data could not be written to file!\n\n");
  }
  else
  {

    fputs(Form("MeanParameter1 %i %i\n", fitsProblemB[0], fitsProblemC[0]), gradWrite);
    fputs(Form("RelGainWidth %i %i\n", fitsProblemB[1], fitsProblemC[1]), gradWrite);
    fputs(Form("PedSpread %i %i\n", fitsProblemB[2], fitsProblemC[2]), gradWrite);
    fputs(Form("I150V %i %i\n", currentProblemB, currentProblemC), gradWrite);
    // fputs(Form("Iratio %i 0\n",  slopeProblemB), gradWrite);
  }
 

  c1->SaveAs(Form("%s/moduleSummary_%s%s.ps", dirName, waf, test));
  c1->SaveAs(Form("%s/%s%s.gif", dirName, waf, test));

  printf("\nmoduleSummary> ................................................ finished\n");
}




// ----------------------------------------------------------------------
void addChip(const char *hist, int chip, TH2D *h3) {
 
  int icol, irow, value;

  TH2D *h2 = (TH2D*)gFile->Get(Form("%s_C%d", hist, chip));
  
  if ( h2 ) {
    for (icol = 0; icol < 52; icol++) {
      for (irow = 0; irow < 80; irow++)  {

	value = h2->GetBinContent(icol+1, irow+1);

	if (chip < 8) {h3->SetBinContent(415-(chip*52+icol)+1, 159-irow+1, value);}
	if (chip > 7) {h3->SetBinContent((chip-8)*52+icol+1, irow+1, value);}
      }
    }
  } 
}

// ----------------------------------------------------------------------
void removeChip(int chip, TH2D *h3, int value) {
 
  int icol, irow;
  
  for (icol = 0; icol < 52; icol++) {
    for (irow = 0; irow < 80; irow++)  {
      
      if (chip < 8) {h3->SetBinContent(415-(chip*52+icol)+1, 159-irow+1, value);}
      if (chip > 7) {h3->SetBinContent((chip-8)*52+icol+1, irow+1, value);}
    }
  }
}

// ----------------------------------------------------------------------
void addVcalThreshold(char *dirName, int chip, TH2D *h3) 
{
 
  int icol, irow, value;

  float thr, sig;
  int a,b;
  int VcalOut = 0;
  FILE *inputFile;
  char string[200];
  sprintf(string, "%s/SCurve_C%i.dat", dirName, chip);
  inputFile = fopen(string, "r");
  if (!inputFile)
  {
	 printf("moduleSummary> !!!!!!!!!  ----> SCurve: Could not open file %s to read fit results\n", string);
  }
  else {
    for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);
    
    for (icol = 0; icol < 52; icol++) {
      for (irow = 0; irow < 80; irow++)  {
	
	fscanf(inputFile, "%e %e %s %2i %2i\n", &thr, &sig, string, &a, &b); 
	value = thr / 65;
	
	if (chip < 8) {h3->SetBinContent(415-(chip*52+icol)+1, 159-irow+1, value);}
	if (chip > 7) {h3->SetBinContent((chip-8)*52+icol+1, irow+1, value);}

      }
    }
  }
}


//-----------------------------------------------------------------------------------------
void makePlot(TH1 *h, const char *title, int pad, float Ymin, float Ymax, float YlimitB, float YlimitC) {

  int rescale = 0;
  double mean, rms, y(0.76);
  c1_3->cd(pad);
  gPad->SetLeftMargin(0.15);

  for (int i = startChip; i < startChip+nChips; i++) {
    if ( h->GetBinContent(i+1) > Ymax ) {
      Ymax = h->GetBinContent(i+1)+0.2* h->GetBinContent(i+1);
    }
    if ( h->GetBinContent(i+1) > YlimitC ) {
      rescale = 1;
    }
    if ( h->GetBinContent(i+1) < Ymin ) {

      Ymin = h->GetBinContent(i+1)-0.2* h->GetBinContent(i+1);
    }
  }
 
  h->GetYaxis()->SetRangeUser(Ymin, Ymax);
  h->Draw("LP");
  
  line->DrawLine(startChip,YlimitB,startChip+nChips,YlimitB);
    
  if ( rescale ) {
    line->DrawLine(startChip,YlimitC,startChip+nChips,YlimitC);
  }
  
  mean = calcMean(h);
  rms = calcRMS(h, mean);
  
  tl->DrawLatex(0.2, 0.93, Form("%s", title));
  char title2[200];
  char number2[200];
 
  if (pad == 3) { 
    sprintf(title2,  "%s [e]", title); 
    sprintf(number2, "%.0f", mean); 
  } else if (pad == 2) {
    sprintf(title2, "%s [\%]", title); mean = 100*mean; rms = 100*rms;
    sprintf(number2, "%.2f", mean); 
  } else { 
    sprintf(title2, "%s", title);
    sprintf(number2, "%.2f", mean); 
  }
  y -= 0.11*(pad-1); 
  c1_2->cd(3);
  tl->DrawLatex(0.25, y, Form("%s",title2));
  tl->DrawLatex(0.72, y, Form("%s", number2));
  //  tl->DrawLatex(0.72, y, Form("%.1f +- %.1f", mean, rms));
  
}


//-----------------------------------------------------------------------------------------
void qualification(const char *dir, TH1D *h1[], TH1D *h2[]) {
  
  float nN(0.), mN(0.), sN(0.), nV(0.), mV(0.), sV(0.), 
        nG(0.), mG(0.), sG(0.), nP(0.), mP(0.), sP(0.), nP1(0.), mP1(0.), sP1(0.);
  float relGainWidth(0.), PedSpread(0.), Par1Mean(0.); 
  char string[2000];
  int ibin(0);
  
  for ( int i = startChip; i < startChip+nChips; i++ ) {
    
    nN = 0.; mN = 0.; sN = 0.; 
    nV = 0.; mV = 0.; sV = 0.; 
    nG = 0.; mG = 0.; sG = 0.; 
    nP = 0.; mP = 0.; sP = 0.;

    FILE *fitFile;
    sprintf(fname, "%s/summary_C%i.txt", dir, i);    
    fitFile = fopen(fname, "r");
    
    if ( !fitFile ) {
      
      printf("moduleSummary> !!!!!!!!!  ----> FITS: Could not find %s in %s\n", fname, dir);
      continue;
    }
    else {
      
      relGainWidth = 0;
      PedSpread    = 0;
              
      fclose(fitFile);
      ifstream is(fname);
	 
      char  buffer[200];
      float m(0.), s(0.);
      int n(0);
     	 
      while (is.getline(buffer, 200, '\n')) {
	   
	if (buffer[0] == 'n' )  {continue;}
	   
	sscanf(buffer,"%s %i %f %f",string, &n, &m, &s);  

	if ( !strcmp(string, "SCurve") )    { nN = n; mN = m; sN = s; }
	if ( !strcmp(string, "Threshold") ) { nV = n; mV = m; sV = s; }
	if ( !strcmp(string, "Gain") )      { nG = n; mG = m; sG = s; }
	if ( !strcmp(string, "Pedestal") )  { nP = n; mP = m; sP = s; }
	if ( !strcmp(string, "Parameter1") ) { nP1 = n; mP1 = m; sP1 = s; }

      }
      
      ibin = i - startChip;

      // Parameter1
      Par1Mean = mP1;
      h1[0]->SetBinContent(ibin+1, Par1Mean);
      h2[0]->SetBinContent(ibin+1, nP1);
  
      // Gain
      if ( mG!=0 ) { if ( sG/mG < 100 ) { relGainWidth = sG/mG; } }
      h1[1]->SetBinContent(ibin+1, relGainWidth);
      h2[1]->SetBinContent(ibin+1, nG);
      
      // Pedestal
      //if ( mG!=0 ) { if ( sG/mG < 4 ) { PedSpread = 65*sP; } }
      if ( mG!=0 ) { if ( sG/mG <100 ) { PedSpread = 65*sP; } }
      h1[2]->SetBinContent(ibin+1, PedSpread);
      h2[2]->SetBinContent(ibin+1, nP);
      
    }

  }

}
//-----------------------------------------------------------------------------------------
double calcMean(TH1 *h) {

  double mean(0.); 

  for (int m = startChip; m < startChip+nChips; m++) {
    
    mean += h->GetBinContent(m+1);
    
  }

  mean = mean/ nChips;

  return mean;
}
//-----------------------------------------------------------------------------------------
double calcRMS(TH1 *h, double mean) {

  double rms(0.);
 
  for (int m = startChip; m < startChip+nChips; m++) {

    rms = rms + (mean - h->GetBinContent(m+1))*(mean - h->GetBinContent(m+1));
  }

  rms = TMath::Sqrt(rms) / 15;

  return rms;
}

 
//-----------------------------------------------------------------------------------------
int readCriteria(const char *fcriteria) {


 defectsB = -99; defectsC = -99; maskdefB = -99;  maskdefC = -99;
 currentB = -99; currentC = -99; slopeivB = -99;  slopeivC = -99;
 noiseB = -99;   noiseC = -99;   trimmingB = -99; trimmingC = -99;
 gainB = -99;    gainC = -99;    pedestalB = -99; pedestalC = -99;
 pedDistr = -99; gainDistr = -99; trmDistr = -99;
  
   
  sprintf(fname, "%s", fcriteria);
  critFile = fopen(fname, "r");   
  
  if ( !critFile ) {
    
    printf("moduleSummary> !!!!!!!!!  ----> GRADING: Could not find %s\n", fname);
    return 0;
  }
  else {

    printf(Form("Reading grading criteria from %s ...\n\n",fname));      
    
    fclose(critFile);
    ifstream is(fname);
    
    char  buffer[200];    
    char  CritName[200];
    float CritValue(0.);
    int ok(0);

    while (is.getline(buffer, 200, '\n')) {
      
      ok =  = 00;
      
      if (buffer[0] == '#' )  {continue;}
      
      sscanf(buffer,"%s %f",CritName, &CritValue); 
      
      if (!strcmp(CritName, "defectsB")) {
	defectsB = int(CritValue); ok = 1;
	if ( readVerbose ) printf("DEFECTS B:           %4.0f\n", defectsB);
      }

      if (!strcmp(CritName, "defectsC")) {
	defectsC = int(CritValue); ok = 1;
	if ( readVerbose ) printf("DEFECTS C:           %4.0f\n", defectsC);
      }

      if (!strcmp(CritName, "maskdefB")) {
	maskdefB = int(CritValue); ok = 1;
	if ( readVerbose ) printf("MASK DEF. B:         %4.0f\n", maskdefB);
      }

      if (!strcmp(CritName, "maskdefC")) {
	maskdefC = int(CritValue); ok = 1;
	if ( readVerbose ) printf("MASK DEF. C:         %4.0f\n", maskdefC);
      } 
  
      if (!strcmp(CritName, "currentB")) {
	currentB = int(CritValue); ok = 1;
	if ( readVerbose ) printf("CURRENT B:           %4.0f\n", currentB);
      }

      if (!strcmp(CritName, "currentC")) {
	currentC = int(CritValue); ok = 1;
	if ( readVerbose ) printf("CURRENT C:           %4.0f\n", currentC);
      }

      if (!strcmp(CritName, "slopeivB")) {
	slopeivB = int(CritValue); ok = 1;
	if ( readVerbose ) printf("SLOPEIV B:           %4.0f\n", slopeivB);
      }

      if (!strcmp(CritName, "slopeivC")) {
	slopeivC = int(CritValue); ok = 1;
	if ( readVerbose ) printf("SLOPEIV C:           %4.0f\n", slopeivC);
      }

      if (!strcmp(CritName, "noiseB")) {
	noiseB = int(CritValue); ok = 1;
	if ( readVerbose ) printf("NOISE B:             %4.0f\n", noiseB);
      }

      if (!strcmp(CritName, "noiseC")) {
	noiseC = int(CritValue); ok = 1;
	if ( readVerbose ) printf("NOISE C:             %4.0f\n", noiseC);
      }
     if (!strcmp(CritName, "trimmingB")) {
	trimmingB = int(CritValue); ok = 1;
	if ( readVerbose ) printf("TRIMMING B:          %4.0f\n", trimmingB);
      }

      if (!strcmp(CritName, "trimmingC")) {
	trimmingC = int(CritValue); ok = 1;
	if ( readVerbose ) printf("TRIMMING C:          %4.0f\n", trimmingC);
      }

      if (!strcmp(CritName, "trmDistribution")) {
	trmDistr = CritValue; ok = 1;
	if ( readVerbose ) printf("TRIM DISTR.:         %4.2f\n", trmDistr);
      }

      if (!strcmp(CritName, "gainMin")) {
	gainMin = CritValue; ok = 1;
	if ( readVerbose ) printf("GAIN MIN:              %4.2f\n", gainMin);
      }

      if (!strcmp(CritName, "gainB")) {
	gainB = CritValue; ok = 1;
	if ( readVerbose ) printf("GAIN B:              %4.2f\n", gainB);
      }

      if (!strcmp(CritName, "gainC")) {
	gainC = CritValue; ok = 1;
	if ( readVerbose ) printf("GAIN C:              %4.2f\n", gainC);
      }

      if (!strcmp(CritName, "gainDistribution")) {
	gainDistr = CritValue; ok = 1;
	if ( readVerbose ) printf("GAIN DISTR.:         %4.2f\n", gainDistr);
      }

      if (!strcmp(CritName, "pedestalB")) {
	pedestalB = int(CritValue); ok = 1;
	if ( readVerbose ) printf("PEDESTAL B:          %4.0f\n", pedestalB);
      }

      if (!strcmp(CritName, "pedestalC")) {
	pedestalC = int(CritValue); ok = 1;
	if ( readVerbose ) printf("PEDESTAL C:          %4.0f\n", pedestalC);
      }

      if (!strcmp(CritName, "pedDistribution")) {
	pedDistr = CritValue; ok = 1;
	if ( readVerbose ) printf("PED. DISTR.:         %4.2f\n", pedDistr);
      }

      if (!strcmp(CritName, "par1Min")) {
	parMin = CritValue; ok = 1;
	if ( readVerbose ) printf("PAR1 MIN:              %4.2f\n", parMin);
      }

      if (!strcmp(CritName, "par1Max")) {
	parMax = CritValue; ok = 1;
	if ( readVerbose ) printf("PAR1 MAX:              %4.2f\n", parMax);
      }

      if (!strcmp(CritName, "par1B")) {
	parB = CritValue; ok = 1;
	if ( readVerbose ) printf("PAR1 B:              %4.2f\n", parB);
      }

      if (!strcmp(CritName, "par1C")) {
	parC = CritValue; ok = 1;
	if ( readVerbose ) printf("PAR1 C:              %4.2f\n", parC);
      }


      if ( !ok ) { printf("*** ERROR: unknown criteria %s !!!\n", CritName); return 0;}
    
    }

    printf("\n");
    readVerbose = 0;

    return 1;
  }
}


//-----------------------------------------------------------------------------------------
int grading(int *rocs, int mask, double i150, double ratio, TH1D *h1[], TH1D *h2[],
	    float *criteriaB, float *criteriaC, const char *testNr) {

  double value(0.),  nValue(0.);
  int gr(1); 
  int jbin(0);
  

  // -- mask defects
  if ( mask >= maskdefB && maskdefB > 0 ) { gr = 2; }
  if ( mask >= maskdefC && maskdefC > 0 ) { gr = 3; }
   
  // -- chip performance
  for (int i = 0; i < 3; i++) {
    for (int j = startChip; j < startChip+nChips; j++) {
      
      jbin = j - startChip;
      value  = h1[i]->GetBinContent(jbin+1); 
      nValue = h2[i]->GetBinContent(jbin+1); 

      // Grading 
      if ( (gr == 1) && (value > criteriaB[i]) && (criteriaB[i] > 0) ) { gr = 2; }
      if ( (value > criteriaC[i]) && (criteriaC[i] > 0) )              { gr = 3; }
      if ( (gr == 1) && (nValue < (4160 - defectsB)) && (defectsB > 0) )    { gr = 2; }
      if ( (nValue < (4160 - defectsC)) && (defectsC > 0) )                 { gr = 3; }
      
      // Failures reasons...
      if ( (value > criteriaB[i]) && (value < criteriaC[i]) && 
	   (criteriaB[i] > 0) && (criteriaC[i] > 0) )            { fitsProblemB[i]++; }
      if ( (value > criteriaC[i]) && (criteriaC[i] > 0) )        { fitsProblemC[i]++; }

    }
  }

  // -- module current
  if ( !strcmp(testNr,"T+17a") ) { 

    // Grading
    if( (gr == 1) && (rocs[1] > 0       ) && (rocs[1] > 0) )  { gr = 2; }
    if( (gr == 1) && (i150    > currentB) && (currentB > 0) )  { gr = 2; }
    if( (gr == 1) && (ratio   > slopeivB) && (slopeivB > 0) )  { gr = 2; }
    
    if( (rocs[2] > 0       ) && (rocs[2] > 0)  )  { gr = 3; }
    if( (i150    > currentC) && (currentC > 0) )  { gr = 3; }
    
    // Failures reasons...
    if( (i150  > currentB) && (i150 < currentC) && (currentC > 0) && (currentB > 0) )  { currentProblemB++; }
    if( (i150  > currentC) && (currentC > 0) )  { currentProblemC++;}
    if( (ratio > slopeivB) && (slopeivB > 0) )  { slopeProblemB++;}
  }
  
  
  if ( !strcmp(testNr,"T-10b") ||  !strcmp(testNr,"T-10a") ) {
    
    // Grading
    if( (gr == 1) && (rocs[1] > 0           ) && (rocs[1] > 0)  )  { gr = 2; }
    if( (gr == 1) && (i150    > 1.5*currentB) && (currentB > 0) )  { gr = 2; }
    if( (gr == 1) && (ratio   > slopeivB    ) && (slopeivB > 0) )  { gr = 2; }
    
    if( (rocs[2] > 0           ) && (rocs[2] > 0)  )  { gr = 3; }
    if( (i150    > 1.5*currentC) && (currentC > 0) )  { gr = 3; }
    
    // Failures reasons...
    if( (i150  > 1.5*currentB) && (i150 < 1.5*currentC) && (currentC > 0) && (currentB > 0) )  { currentProblemB++; }
    if( (i150  > 1.5*currentC) && (currentC > 0) )    { currentProblemC++;}
    if( (ratio > slopeivB    ) && (slopeivB > 0) )    { slopeProblemB++;}
    
    
  }
  
  return gr;
  
  
}
