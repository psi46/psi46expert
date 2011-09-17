int cd(0), ct(0), index(0);
double dtl[10], cst[10], ft10a[2], ft10b[2], ft17a[2], ftc[2], iv10b[2], iv17a[2];
int dtlIndex[10], cstIndex[10], ft10aIndex[2], ft10bIndex[2], ft17aIndex[2], ftcIndex[2], iv10bIndex[2], iv17aIndex[2];
double ymin(-15.), ymax(30.);
float conversion = 1./60.;
TLine   *line  = new TLine;
TArrow  *arrow = new TArrow;
TBox    *box   = new TBox;

void readProfile(const char *dirName="") {
 
  printf("\ntempProfile> Starting ...\n");
  for (int i = 0; i < 2; i++ ) { 
    ft10a[0] = -1.;
    ft10a[1] = -1.;
    ft10b[0] = -1.;
    ft10b[1] = -1.;
    ft17a[0] = -1.;
    ft17a[1] = -1.;
    ftc[0] = -1.;
    ftc[1] = -1.;

    iv10b[0] = -1.;
    iv10b[1] = -1.;
    iv17a[0] = -1.;
    iv17a[1] = -1.;
  }

  setStyle();
  FILE *tFile, *psFile;
  
  TLatex *tl = new TLatex;
  tl->SetNDC(kTRUE);
  tl->SetTextSize(0.08);
  
  TString noslash(dirName);
  noslash.ReplaceAll("/", "");
  noslash.ReplaceAll("..", "");
  noslash.ReplaceAll("T-10a", "");
  noslash.ReplaceAll("T-10b", "");
  noslash.ReplaceAll("T+17a", "");
  
  char  buffer[200];
  char  hash[20];
  char  fname[1000];
  char  pname[1000];
  
  sprintf(fname, "%s/../temperature.log", dirName);
  printf("tempProfile> Looking for temperature logfile: %s\n", fname);
  tFile = fopen(fname, "r");
  
  if (!tFile)
    {
      printf("tempProfile> !!!!!!!!!  ----> Could not open file %s to read data\n", fname);
    }
  
  else {

    sprintf(pname, "%s/../tProfile.gif", dirName);
    printf("tempProfile> Looking for temp. profile plot: %s\n", pname);
    psFile = fopen(pname, "r");
    
    if (psFile) {  // < ===========================================================================
    // if (0) {
          
      printf("tempProfile> ----> Temperature profile plot %s already exists\n", pname);
      fclose(psFile);

      printf("\ntempProfile> ................................................ finished\n\n");
      return;
    }
    
    fclose(tFile);
    
    ifstream is(fname);
  
    float hour, min, sec; 
    float tValue, hValue;
    int ok(0);
    
    char string[200], flag[20], test[20];
    
    char string[200], flag[20], test[20], nr[20];
    
    
    TCanvas *c1 = new TCanvas("c1", "", 1000, 400);
    c1->Clear();
    
    TH1D *h1 = new TH1D("h1", "", 1500, -0.5, 12.); h1->Sumw2();
    h1->SetMaximum(ymax); h1->SetMinimum(ymin);
    h1->SetXTitle("Time / h");
    h1->SetYTitle("Temperature / ^{o}C");
    setHist(h1,24,0);

    c1->cd();
    gPad->SetBottomMargin(0.65);
    gPad->SetTopMargin(0.15);
    gPad->SetBottomMargin(0.2);
    gPad->SetLeftMargin(0.2);
    
    
    TGraph* g1 = new TGraph();
    TGraph* f1 = new TGraph();
    TGraph* e1 = new TGraph();
    
    int m(0), cnt(0); 
    int lastMin(-1), lastHour(-1);
    while (is.getline(buffer, 200, '\n')) {
    
      if (buffer[0] == '#') {
	
	sscanf(buffer, "%s %s %s %s %s", hash, flag, test, nr, string);
	mark(m, flag, test, nr);
// printf( "%s %s %s %s %s\n", hash, flag, test, nr, string);
	continue;
	
      }
      
      if (buffer[0] == ',') {continue;}
      if (buffer[0] == 'e') {continue;}
      if (buffer[0] == ' ') {continue;}

      sscanf(buffer, "%2f:%2f:%2f, %f, %f", &hour, &min, &sec, &tValue, &hValue);
// if ( cnt < 200 ) { printf("%.2i:%.2i:%.2i, %f, %f\n", hour, min, sec, tValue, hValue); cnt++;}

      if ( lastMin  < 0 ) { 
	
	g1->SetPoint(index, conversion*m, tValue);
	lastMin  = min; 
	lastHour = hour;
	index++;
	
	
      }
      
      // -- same hour
      if( hour == lastHour && min > lastMin ) {

	m += (min - lastMin);
	
	g1->SetPoint(index, conversion*m, tValue);
	lastMin = min;
	index++;
      }

      // -- hour change
      if( hour > lastHour ) {

	m += (60 - lastMin) + min;

	g1->SetPoint(index, conversion*m, tValue);
	lastMin  = min;
	lastHour = hour;
	index++;
      }

      // -- hour change mid-night
      if( hour == 0 && lastHour == 23 ) {

	m += (60 - lastMin) + min;

	g1->SetPoint(index, conversion*m, tValue);
	lastMin  = min;
	lastHour = hour;
	index++;
      }
    }
    
    
    h1->GetXaxis()->SetLabelSize(0.06); 
    h1->GetXaxis()->SetTitleSize(0.06); 
    h1->GetYaxis()->SetLabelSize(0.06); 
    h1->GetYaxis()->SetTitleSize(0.06);
    h1->Draw("e");
    
    f1->SetMarkerColor(93);
    f1->SetMarkerSize(2);
    f1->SetMarkerStyle(29);
    
    e1->SetMarkerColor(107);
    e1->SetMarkerSize(2);
    e1->SetMarkerStyle(29);
    
    g1->SetLineColor(108);
    g1->SetLineWidth(2);
    g1->Draw();
    
    int color(1);
    tl->SetTextSize(0.04);
    
    color = 1;
    tl->SetTextColor(color);
    drawShadesPlot(ft10a[0], ymin, ft10a[1], ymax, color, 3017);
    tl->DrawLatex(0.2+ft10a[0]/18., 0.91, "FullTest -10^{o} C");
    
    color=31;
    drawShadesPlot(ftc[0], ymin, ftc[1], ymax, color, 3003);
    color=13;
    tl->SetTextColor(color);
    tl->DrawLatex(0.24+ftc[0]/18., 0.87, "Thermal cycling");
    tl->DrawLatex(0.26+ftc[0]/18., 0.80, "(10 cycles)");
    
    color=4;
    drawShadesPlot(ft10b[0], ymin, ft10b[1], ymax, color, 3017);
    tl->SetTextColor(color);
    tl->DrawLatex(0.21+ft10b[0]/18., 0.91, "FullTest &");
    drawShadesPlot(iv10b[0], ymin, iv10b[1], ymax, color, 3003);
    tl->DrawLatex(0.21+iv10b[0]/18., 0.87, "IV -10^{o} C");

    color=2;
    drawShadesPlot(ft17a[0], ymin, ft17a[1], ymax, color, 3017);
    tl->SetTextColor(color);
    tl->DrawLatex(0.21+ft17a[0]/18., 0.91, "FullTest &");
    drawShadesPlot(iv17a[0], ymin, iv17a[1], ymax, 2, 3003);
    tl->SetTextColor(color);
    tl->DrawLatex(0.21+iv17a[0]/18., 0.87, "IV 17^{o} C");

    color = 107;
    for( int i = 0; i < cd; i++ )  {
      e1->SetPoint(i, dtl[i], 27);
    }
    tl->SetTextColor(color);
    tl->SetTextSize(0.04);
    tl->DrawLatex(0.80, 0.80, "dtlScan");

    color = 93;
    tl->SetTextColor(color);
    tl->DrawLatex(0.80, 0.76, "currentTest");
    for( int i = 0; i < ct; i++ )  {
	f1->SetPoint(i, cst[i], 24);
    }

    g1->Draw();
    e1->Draw("PSAME");
    f1->Draw("PSAME");

    tl->SetTextColor(1);
    tl->SetTextSize(0.03);
    tl->DrawLatex(0.90, 0.92, "Module-Test");
    tl->DrawLatex(0.90, 0.88, Form("%s", noslash.Data()));

    c1->SaveAs(Form("%s/../tProfile.gif", dirName));
    c1->SaveAs(Form("%s/../tProfile.ps", dirName));

    FILE *sumFile; 
    sprintf(fname, "%s/../summaryTemp.txt", dirName);
    sumFile = fopen(fname, "r");

    double meanT(0.), sigmaT(0.);

    //    if (sumFile)  <========================================================================
    if (0)
    {
	printf("tempProfile> Temperature summary %s file already exist, skipping ...\n", fname);
	fclose(sumFile);
    }
    
    else {
      
      ofstream tsum(fname);

      if( ft10a[0] != -1 && ft10a[1] != -1 ) {

	meanT = calcMean(g1, ft10aIndex[0], ft10aIndex[1]);
	sigmaT = calcRMS(g1, ft10aIndex[0], ft10aIndex[1], meanT);
	tsum << "T-10a " << meanT << " " << sigmaT << endl;
      }
      else {
	tsum << "T-10a -100 -100" << endl;
      }


      if( ft10b[0] != -1 && iv10b[1] != -1 ) {

	meanT = calcMean(g1, ft10bIndex[0], iv10bIndex[1]);
	sigmaT = calcRMS(g1, ft10bIndex[0], iv10bIndex[1], meanT);
	tsum << "T-10b " << meanT << " " << sigmaT << endl;
      }
      else {
	tsum << "T-10b -100 -100" << endl;
      }

      if( ft17a[0] != -1 && iv17a[1] != -1 ) {

	meanT = calcMean(g1, ft17aIndex[0], iv17aIndex[1]);
	sigmaT = calcRMS(g1, ft17aIndex[0], iv17aIndex[1], meanT);
	tsum << "T+17a " << meanT << " " << sigmaT << endl;
      }
      else {
	tsum << "T+17a -100 -100" << endl;
      }
      
      if( ftc[0] != -1 && ftc[0] != -1 ) {

	meanT = calcMean(g1, ftcIndex[0], ftcIndex[1]);
	sigmaT = calcRMS(g1, ftcIndex[0], ftcIndex[1], meanT);
	tsum << "T-cycl " << meanT << " " << sigmaT << endl;
      }
      else {
	tsum << "T-cycl -100 -100" << endl;
      }

      printf("tempProfile> Temperature summary file created: %s ...\n", fname);
    }
  }

  printf("\ntempProfile> ................................................ finished\n\n");
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

void mark(int point, const char *t2, const char *t1, const char *t3) {  

    if (!strcmp(t1,"dtlScanTest:")) {
	if (!strcmp(t2,"Start")) { 
	    dtl[cd] = conversion*point;
	    dtlIndex[cd] = index;
	    cd++;
	}
    }
    
    if (!strcmp(t1,"currentTest:")) {
	if (!strcmp(t2,"Start")) { 
	    cst[ct] = conversion*point;
	    dtlIndex[cd] = index;
	    ct++;
	}
    }
    
    if (!strcmp(t1,"FullTest")) {
	if (!strcmp(t2,"Start")) { 
	    if (!strcmp(t3,"T-10a:")) { 
		ft10a[0] = conversion*point;
		ft10aIndex[0] = index;
	    }
	}
    }
    
    if (!strcmp(t1,"FullTest")) {
	if (!strcmp(t2,"End")) { 
	    if (!strcmp(t3,"T-10a:")) { 
		ft10a[1] = conversion*point;
		ft10aIndex[1] = index;
	    }
	}
    }
    
    if (!strcmp(t1,"FullTest")) {
	if (!strcmp(t2,"Start")) { 
	    if (!strcmp(t3,"T-10b:")) { 
	      printf("Point = %i %i\n", point, index);
		ft10b[0] = conversion*point;
		ft10bIndex[0] = index;
	    }
	}
    }
    
    if (!strcmp(t1,"FullTest")) {
	if (!strcmp(t2,"End")) { 
	    if (!strcmp(t3,"T-10b:")) { 
		ft10b[1] = conversion*point;
		ft10bIndex[1] = index;
	    }
	}
    }
    
    if (!strcmp(t1,"FullTest")) {
	if (!strcmp(t2,"Start")) { 
	    if (!strcmp(t3,"T+17a:")) { 
		ft17a[0] = conversion*point;
		ft17aIndex[0] = index;
	    }
	}
    }
    
    if (!strcmp(t1,"FullTest")) {
	if (!strcmp(t2,"End")) { 
	    if (!strcmp(t3,"T+17a:")) { 
		ft17a[1] = conversion*point;
		ft17aIndex[0] = index;
	    }
	}
    }
    
    if (!strcmp(t1,"T-cycling:")) {
	if (!strcmp(t2,"Start")) { 
	    ftc[0] = conversion*point;
	    ftcIndex[0] = index;
	}
    }
    
    if (!strcmp(t1,"T-cycling:")) {
	if (!strcmp(t2,"End")) { 
	    ftc[1] = conversion*point;
	    ftcIndex[1] = index;
	}
    }
    
    if (!strcmp(t1,"ivTests:")) {
	if (!strcmp(t2,"Start")) { 
	    if(ft17a[0] == -1) { 
		iv10b[0] = conversion*point;
		iv10bIndex[0] = index;
	    }
	    else {
		iv17a[0] = conversion*point;
		iv17aIndex[0] = index;
	    }
	}
    }
    
    if (!strcmp(t1,"ivTests:")) {
	if (!strcmp(t2,"End")) {  
	    if(ft17a[0] == -1) { 
	      printf("Point = %i %i\n", point, index);
		iv10b[1] = conversion*point;
		iv10bIndex[1] = index;
	    }
	    else {
		iv17a[1] = conversion*point;
		iv17aIndex[1] = index;
	    }
	}

    }
}
 //----------------------------------------------------------------------------------------- 
void drawShadesPlot( double x_1, double y_1, double x_2, double y_2, double color, double style) {

    box->SetFillColor(color);
    box->SetFillStyle(style);
    box->DrawBox(x_1, y_1, x_2, y_2);
 //    line->SetLineColor(color);
//     line->DrawLine(x_1, y_1, x_1, y_2);
//     line->DrawLine(x_2, y_1, x_2, y_2);
} 
//----------------------------------------------------------------------------------------- 
void drawArrowPlot( double x_1, double y_1, double x_2, double y_2, double color) {

    arrow->SetLineColor(color);
    arrow->SetLineWidth(2);
    line->SetLineStyle(kSolid);
    arrow->DrawArrow(x_1, y_1, x_2, y_2, 0.008);
    arrow->DrawArrow(x_1, y_1, x_2, y_2, 0.008);
}
//-----------------------------------------------------------------------------------------
double calcMean(TGraph *g, int start, int stop) {

  double mean(0.); 
  double x1(0.), x2(0.);

  for (int m = start; m < stop; m++) {

    g->GetPoint(m, x1, x2);
    mean += x2;
     
  }

  if(start != stop) { mean = mean/ (stop - start);}
 
  return mean;


}
//-----------------------------------------------------------------------------------------
double calcRMS(TGraph *g, int start, int stop, double mean) {

  double rms(0.);
  double x1(0.), x2(0.);
 
  for (int m = start; m < stop; m++) {

    g->GetPoint(m, x1, x2);
    rms = rms + (mean - x2)*(mean - x2);
  }

  //  rms = TMath::Sqrt(rms);
  if (start != stop) { rms = TMath::Sqrt((rms)/(stop - start)); }

  return rms;
}
