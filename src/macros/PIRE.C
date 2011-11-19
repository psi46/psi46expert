#include <TMath.h>

// -- data files ----------------------------------------------------------------------------------
const char* fileName = "FullTest.root";
const char* adFileName = "adDec.root"; 
const char* trimFileName = "trim.root" ;

float defectsB, defectsC, maskdefB,  maskdefC;
float currentB, currentC, slopeivB,  slopeivC;
float noiseB,   noiseC,   trimmingB, trimmingC;
float gainB,    gainC,    pedestalB, pedestalC;
float pedDistr, gainDistr, trmDistr;

int nChips(16);
int startChip(0);
int readVerbose(1);

char fname[200];
FILE *inputFile, *critFile;

TFile *f, *f1, *g;

TCanvas* c1 = NULL; 
TLatex *tl;
TLatex *ts;
TLine *line;
TBox *box;

// -- temperature calibration ---------------------------------------------------------------------
static const Int_t numROCs    = 16; // number of ROCs per module
//static const Int_t numROCs    = 1;

static const Int_t numTemperatures = 11;
static const Int_t numTempRanges   = 8;

const Int_t temperatureValues_target[numTemperatures] = { -20, -15, -10, -5, 0, 5, 10, 15, 20, 25, 30 }; // degrees C

const Double_t vReference[numTempRanges]  = { 399.5, 423.0, 446.5, 470.0, 493.5, 517.0, 540.5, 564.0 }; // mV
const Double_t vCalibration = 470.0; // mV

const Double_t minADCvalue_graph =    0.;
const Double_t maxADCvalue_graph = 2000.;

Int_t    gADCvalue_blackLevel[numROCs][numTemperatures];
Int_t    gADCvalue_Measurement[numROCs][numTempRanges][numTemperatures];
Int_t    gADCvalue_Calibration[numROCs][numTempRanges][numTemperatures];

TGraph*  gADCgraph_Measurement[numROCs][numTempRanges];
TGraph*  gADCgraph_Calibration[numROCs][numTempRanges];

TCanvas* gCanvas = NULL; 
TPostScript* gPostScript = NULL; 

//---------------------------------------------------------------------------------------------------


void PIRE()
{
  const char *dirName="../output/CCEStudy/currentModule";
  int chipId=0;
  
  gSystem->Load("../psi46expert/.libs/libpsi46ana.so");
  
  PHCalibrationFit phFit = PHCalibrationFit(3);
  phFit.FitAllCurves(dirName);	
 
  SCurve sCurve;
  sCurve.FitSCurves(dirName);
  
  
  
  
  
  
  
  
  
        if (f && f->IsOpen()) f->Close();
	if (f1 && f1->IsOpen()) f1->Close();
	if (g && g->IsOpen()) g->Close();

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
	gStyle->SetNdivisions(6, "X");
	gStyle->SetNdivisions(8, "Y");
	gStyle->SetTitleFont(132);

	gROOT->ForceStyle();

	tl = new TLatex;
	tl->SetNDC(kTRUE);
	tl->SetTextSize(0.09);

	ts = new TLatex;
	ts->SetNDC(kTRUE);
	ts->SetTextSize(0.08);

	line = new TLine;
	line->SetLineColor(kRed);
	line->SetLineStyle(kSolid);
	
	box = new TBox;
	box->SetFillColor(kRed);
	box->SetFillStyle(3002);

	f = new TFile(Form("%s/%s", dirName, fileName), "READ");
	
	if (strcmp(fileName, adFileName) == 0) f1 = f;
	else f1 = new TFile(Form("%s/%s", dirName, adFileName), "READ");
	
	if (strcmp(fileName, trimFileName) == 0) g = f;
	else g = new TFile(Form("%s/%s", dirName, trimFileName), "READ");
 
	sprintf(fname, "../macros/criteria.dat");
	if ( !readCriteria(fname) ) { 
	  
	  printf("\nchipSummary> ----> COULD NOT READ GRADING CRITERIA !!!");
	  printf("chipSummary> ----> Aborting execution of chipgSummaryPage.C ... \n\n", fileName, dirName);  
	  break;
	}

	TH1D *h1;
	TH2D *h2;

	c1 = new TCanvas("c1", "", 800, 800);
	c1->Clear();
	c1->Divide(4,4, 0.01, 0.04);

  //	shrinkPad(0.1, 0.1, 0.1, 0.3);


        TString noslash(dirName);
        noslash.ReplaceAll("/", " ");
        noslash.ReplaceAll(".. ", "");
	
	
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Row 1
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	// -- Dead pixels
	TH2D *hpm = new TH2D("hpm", "", 80, 0., 80., 52, 0., 52.);

	int nDeadPixel(0);
	int nMaskDefect(0);
	int nNoisyPixel(0);
	int nRootFileProblems(0);

	c1->cd(1);
	h2 = (TH2D*)f->Get(Form("PixelMap_C%i", chipId));
        if (h2) {
	  for (int icol = 0; icol < 52; ++icol)
	  {
		for (int irow = 0; irow < 80; ++irow)
		{
		        hpm->SetBinContent(irow+1, icol+1, h2->GetBinContent(icol+1, irow+1));

			if (h2->GetBinContent(icol+1, irow+1)  == 0)
			{
				++nDeadPixel;
			}
			if (h2->GetBinContent(icol+1, irow+1)  > 10)
			{
				++nNoisyPixel;
			}
			if (h2->GetBinContent(icol+1, irow+1)  < 0)
			{
				++nMaskDefect;
			}
		}
	  }
	  h2->SetTitle("");
	  h2->Draw("colz");
	  tl->DrawLatex(0.1, 0.92, "Pixel Map");
	}

	else { ++nRootFileProblems; }

	// -- sCurve width and noise level
	TH1D *hw = new TH1D("hw", "", 100, 0., 600.);
	TH1D *hd = new TH1D("hd", "", 100, 0., 600.);  // Noise in unbonded pixel (not displayed)
	TH2D *ht = new TH2D("ht", "", 52, 0., 52., 80, 0., 80.);
	TH1D *htmp;

	float mN(0.), sN(0.), nN(0.), nN_entries(0.);
	int over(0), under(0);

	double htmax(255.), htmin(0.);
	
	float thr, sig;
	int a,b;
	FILE *inputFile;
	char string[200];
	sprintf(string, "%s/SCurve_C%i.dat", dirName, chipId);
	inputFile = fopen(string, "r");

	double minThrDiff(-5.);
	double maxThrDiff(5.);
	h2 = (TH2D*)f->Get(Form("vcals_xtalk_C%i", chipId));
        
	if (!inputFile)
	{
		printf("chipSummary> !!!!!!!!!  ----> SCurve: Could not open file %s to read fit results\n", string);
	}
	else
	{
	  for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);

		for (int icol = 0; icol < 52; ++icol)
		{
			for (int irow = 0; irow < 80; ++irow)
			{
				fscanf(inputFile, "%e %e %s %2i %2i", &thr, &sig, string, &a, &b);  //comment
//  				printf("chipSummary> sig %e thr %e\n", sig, thr);
				hw->Fill(sig);
				thr = thr / 65;

				ht->SetBinContent(icol+1, irow+1, thr); 
			
				if ( h2 ) {
				  if( h2->GetBinContent(icol+1, irow+1)  > minThrDiff)
				  {
				    hd->Fill(sig);
				  }
				}
			}
		}
		c1->cd(2);
		hw->Draw();
		tl->DrawLatex(0.1, 0.92, "S-Curve widths: Noise (e^{-})");
		

		/*		c1->cd(15);
		hd->SetLineColor(kRed);
		hd->Draw();
		tl->DrawLatex(0.1, 0.92, "S-Curve widths of dead bumps");
		if ( hd->GetEntries() > 0 ) {
		  ts->DrawLatex(0.55, 0.82, Form("entries: %4.0f", hd->GetEntries()));
		  ts->DrawLatex(0.55, 0.74, Form("#mu:%4.2f", hd->GetMean()));
		  ts->DrawLatex(0.55, 0.66, Form("#sigma: %4.2f", hd->GetRMS()));
		}
		*/

		mN =  hw->GetMean();
		sN =  hw->GetRMS();
		nN =  hw->Integral(hw->GetXaxis()->GetFirst(), hw->GetXaxis()->GetLast());
		nN_entries =  hw->GetEntries();

		under = hw->GetBinContent(0);
		over  = hw->GetBinContent(hw->GetNbinsX()+1);


		ts->DrawLatex(0.65, 0.82, Form("N: %4.0f", nN));
		ts->DrawLatex(0.65, 0.74, Form("#mu: %4.1f", mN));
		ts->DrawLatex(0.65, 0.66, Form("#sigma: %4.1f", sN));
			
		if ( under ) ts->DrawLatex(0.15, 0.55, Form("<= %i", under));			               
		if ( over  ) ts->DrawLatex(0.75, 0.55, Form("%i =>", over ));

		c1->cd(3);
		if ( ht->GetMaximum() < htmax ) { 
		  htmax = ht->GetMaximum();
		}
		if ( ht->GetMinimum() > htmin ) {
		  htmin = ht->GetMinimum();
		}
		ht->GetZaxis()->SetRangeUser(htmin,htmax);
		ht->Draw("colz");
		tl->DrawLatex(0.1, 0.92, "Vcal Threshold Untrimmed");
	}

	// -- Noise level map
	c1->cd(4);
        gPad->SetLogy(1);
 	gStyle->SetOptStat(1);
	
	float mV(0.), sV(0.), nV(0.), nV_entries(0.);
	over = 0.; under = 0.;

	if (!g->IsZombie())
	{
	      h1 = (TH1D*)g->Get(Form("VcalThresholdMap_C%iDistribution;7", chipId));
              if (h1) {
		h1->SetTitle("");
		h1->SetAxisRange(0., 100.);
		h1->Draw();

		mV = h1->GetMean();
		sV = h1->GetRMS();
		nV = h1->Integral(h1->GetXaxis()->GetFirst(), h1->GetXaxis()->GetLast());
		nV_entries = h1->GetEntries();

		under = h1->GetBinContent(0);
		over  = h1->GetBinContent(h1->GetNbinsX()+1);
              }
              else {

	        ++nRootFileProblems;
		mV = 0.;
		sV = 0.;
               
              }

	      ts->DrawLatex(0.15, 0.82, Form("N: %4.0f", nV));
	      ts->DrawLatex(0.15, 0.74, Form("#mu: %4.1f", mV));
	      ts->DrawLatex(0.15, 0.66, Form("#sigma: %4.1f", sV));
	      
	      if ( under ) ts->DrawLatex(0.15, 0.55, Form("<= %i", under));			               
	      if ( over  ) ts->DrawLatex(0.75, 0.55, Form("%i =>", over ));
	}

	tl->DrawLatex(0.1, 0.92, "Vcal Threshold Trimmed");
	
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Row 2
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	// -- Bump Map
	TH2D *hbm = new TH2D("hbm", "", 80, 0., 80., 52, 0., 52.);

	int nDeadBumps(0);

	c1->cd(5);
	gStyle->SetOptStat(0);
	h2 = (TH2D*)f->Get(Form("vcals_xtalk_C%i", chipId));
        
        if (h2) {

	  for (int icol = 0; icol < 52; ++icol)
	  {
		for (int irow = 0; irow < 80; ++irow)
		{
		        hbm->SetBinContent(irow+1, icol+1, h2->GetBinContent(icol+1, irow+1));

			if ( h2->GetBinContent(icol+1, irow+1)  >= minThrDiff )
			{
			   cout << Form("chipSummary> dead %3d %3d: %7.5f", icol, irow, h2->GetBinContent(icol, irow)) << endl;
				++nDeadBumps;
			}
		}
	  }

	  h2->SetTitle("");
	  h2->GetZaxis()->SetRangeUser(minThrDiff, maxThrDiff);
	  h2->Draw("colz");
	  tl->DrawLatex(0.1, 0.92, "Bump Bonding Problems");
	}

	else { ++nRootFileProblems; }

	// -- Bump Map
	c1->cd(6);  
	gPad->SetLogy(1);
	//gStyle->SetOptStat(1);
	h1 = (TH1D*)f->Get(Form("vcals_xtalk_C%iDistribution", chipId));
        if (h1) {
  	  h1->SetTitle("");
	  h1->SetAxisRange(-50., 50.); //DOES NOT WORK!!!???
	  h1->Draw();
	  tl->DrawLatex(0.1, 0.92, "Bump Bonding");
	}
	
	else { ++nRootFileProblems; }
	
	// -- Trim bits
	int trimbitbins(3);
	int nDeadTrimbits(0);
	c1->cd(7); 
	gPad->SetLogy(1);
	h1 = (TH1D*)f->Get(Form("TrimBit14_C%i", chipId));
	if (h1) {
	  h1->SetTitle("");
	  h1->SetAxisRange(0., 60.);
	  h1->SetMinimum(0.5);
	  h1->Draw("");
	  tl->DrawLatex(0.1, 0.92, "Trim Bit Test");
	  for (int i = 1; i <= trimbitbins; ++i) nDeadTrimbits += h1->GetBinContent(i);
	}

	else { ++nRootFileProblems; }

	h1 = (TH1D*)f->Get(Form("TrimBit13_C%i", chipId));
	if (h1) {
	  h1->SetLineColor(kRed);
	  h1->Draw("same");
	  for (int i = 1; i <= trimbitbins; ++i) nDeadTrimbits += h1->GetBinContent(i);
	}

	else { ++nRootFileProblems; }

	h1 = (TH1D*)f->Get(Form("TrimBit11_C%i", chipId));
	if (h1) {
	  h1->SetLineColor(kBlue);
	  h1->Draw("same");
	  for (int i = 1; i <= trimbitbins; ++i) nDeadTrimbits += h1->GetBinContent(i);
	}

	else { ++nRootFileProblems; }

	h1 = (TH1D*)f->Get(Form("TrimBit7_C%i", chipId));
	if (h1) {
	  h1->SetLineColor(kGreen);
	  h1->Draw("same");
	  for (int i = 1; i <= trimbitbins; ++i) nDeadTrimbits += h1->GetBinContent(i);
	}
	
	else { ++nRootFileProblems; }
	
	// -- For numerics and titels see at end


	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Row 3
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	// -- Address decoding
	TH2D *ham = new TH2D("ham", "", 80, 0., 80., 52, 0., 52.);

	int nAddressProblems(0);
	
	c1->cd(9);
	gStyle->SetOptStat(0);
	h2 = (TH2D*)f1->Get(Form("AddressDecoding_C%i", chipId));
        if (h2) {
	  for (int icol = 0; icol < 52; ++icol) {
	    for (int irow = 0; irow < 80; ++irow) {
	      
	      ham->SetBinContent(irow+1, icol+1, h2->GetBinContent(icol+1, irow+1));

	      if (h2 && h2->GetBinContent(icol+1, irow+1) < 1) {
	        cout << Form("chipSummary> address problem %3d %3d: %7.5f", icol, irow, h2->GetBinContent(icol, irow))
		     << endl;
	        ++nAddressProblems;
	      }
	    }
	  }
	  h2->SetTitle("");
	  h2->Draw("colz");
	  tl->DrawLatex(0.1, 0.92, "Address decoding");
	}

	else { ++nRootFileProblems; }

	// -- Address levels
	c1->cd(10); 
	gPad->SetLogy(1);
	h1 = (TH1D*)f1->Get(Form("AddressLevels_C%i", chipId));
        if (h1) {
	  h1->SetTitle("");
	  h1->SetAxisRange(-1200., 1000.);
	  h1->Draw();
	  tl->DrawLatex(0.1, 0.92, "Address Levels");
	}

	else { ++nRootFileProblems; }
		
	// -- PHCalibration (Gain & Pedesdtal)

	TH1D *hg = new TH1D("hg", "", 250, -1., 5.5);
	TH2D *hgm = new TH2D("hgm", "", 52, 0., 52., 80, 0., 80.);
	
	TH1D *hp = new TH1D("hp", "", 900, -300., 600.);
	hp->StatOverflows(kTRUE);
	TH1D *rp = new TH1D("rp", "", 900, -300., 600.);
	rp->StatOverflows(kFALSE);

	TH1D *htmp;
	
	float mG(0.), sG(0.), nG(0.), nG_entries(0.);
	float mP(0.), sP(0.), nP(0.), nP_entries(0.); 
	over = 0.; under = 0.;

	float par0, par1, par2, par3, par4, par5; // Parameters of Vcal vs. Pulse Height Fit
	float ped, gain;
	int a,b;
	
	int mPbin(0), xlow(-100), xup(255), extra(0);       // for restricted RMS
	double integral(0.);


	FILE *inputFile;
	char string[200];
	sprintf(string, "%s/phCalibrationFit_C%i.dat", dirName, chipId);
	inputFile = fopen(string, "r");

	if (!inputFile)
	{
		printf("chipSummary> !!!!!!!!!  ----> phCal: Could not open file %s to read fit results\n", string);
	}
	else
	{
		for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);

		for (int icol = 0; icol < 52; ++icol)
		{
			for (int irow = 0; irow < 80; ++irow)
			{
				fscanf(inputFile, "%e %e %e %e %e %e %s %2i %2i", &par0, &par1, &par2, &par3, &par4, &par5, string, &a, &b);
 			
				if (par2 != 0.)  // dead pixels have par2 == 0.
				{
				  // ped = -par3/par2;
					gain = 1./par2;
					ped = par3;
					hp->Fill(ped);
					hg->Fill(gain);
					hgm->SetBinContent(icol + 1, irow + 1, gain);
				}
			}
		}

		mG =  hg->GetMean();
		sG =  hg->GetRMS();
		nG =  hg->Integral(hg->GetXaxis()->GetFirst(), hg->GetXaxis()->GetLast());
		nG_entries = hg->GetEntries();

		under = hg->GetBinContent(0);
		over  = hg->GetBinContent(hp->GetNbinsX()+1);
		
		c1->cd(11);

		hg->Draw();
		tl->DrawLatex(0.1, 0.92, "PH Calibration: Gain (ADC/DAC)");
		
		if ( hg->GetMean() > 1.75 ) {
		  ts->DrawLatex(0.15, 0.82, Form("N: %4.0f", nG));
		  ts->DrawLatex(0.15, 0.74, Form("#mu: %4.2f", mG));
		  ts->DrawLatex(0.15, 0.66, Form("#sigma: %4.2f", sG));

		  if ( under ) ts->DrawLatex(0.15, 0.82, Form("<= %i", under));			               
		  if ( over  ) ts->DrawLatex(0.75, 0.82, Form("%i =>", over ));
		}
		else {
		  ts->DrawLatex(0.65, 0.82, Form("N: %4.0f", nG));
		  ts->DrawLatex(0.65, 0.74, Form("#mu: %4.2f", mG));
		  ts->DrawLatex(0.65, 0.66, Form("#sigma: %4.2f", sG));
	
		  if ( under ) ts->DrawLatex(0.15, 0.82, Form("<= %i", under));			               
		  if ( over  ) ts->DrawLatex(0.75, 0.82, Form("%i =>", over ));
		}


		mP =  hp->GetMean();
		sP =  hp->GetRMS();
		nP =  hp->Integral(hp->GetXaxis()->GetFirst(), hp->GetXaxis()->GetLast());
		nP_entries = hp->GetEntries();

		if ( nP > 0 ) {

		  // -- restricted RMS
		  integral = 0.;
		  mPbin = -1000; xlow = -1000; xup = 1000;
		  over = 0.; under = 0.;
		  
		  mPbin = hp->GetXaxis()->FindBin(mP);
				  
		  for (int i = 0; integral <  pedDistr; i++) { 
		    
		    xlow = mPbin-i;
		    xup =  mPbin+i;
		    integral = hp->Integral(xlow, xup)/nP;
		    
		  }
		  
		  extra = xup - xlow;
		}
		else {

		  xlow = -300; xup = 600; extra = 0;
		  over = 0.; under = 0.;
		}

		  
		hp->GetXaxis()->SetRange(xlow - extra, xup + extra);

		nP    = hp->Integral(hp->GetXaxis()->GetFirst(), hp->GetXaxis()->GetLast());
		under = hp->GetBinContent(0);
		over  = hp->GetBinContent(hp->GetNbinsX()+1);


		c1->cd(15);

		hgm->Draw("colz");
		tl->DrawLatex(0.1, 0.92, "PH Calibration: Gain (ADC/DAC)");

		
		c1->cd(12);

		hp->DrawCopy();

		rp->Add(hp);
		rp->GetXaxis()->SetRange(xlow, xup);

       		mP =  rp->GetMean();
		sP =  rp->GetRMS();

		// box->DrawBox( rp->GetBinCenter(xlow), 0, rp->GetBinCenter(xup), 1.05*rp->GetMaximum());
		rp->SetFillColor(kRed);
		rp->SetFillStyle(3002);
		rp->Draw("same");
		line->DrawLine(rp->GetBinCenter(xlow), 0, rp->GetBinCenter(xlow), 0.6*rp->GetMaximum());
		line->DrawLine(rp->GetBinCenter(xup),  0, rp->GetBinCenter(xup),  0.6*rp->GetMaximum());
	 
		tl->DrawLatex(0.1, 0.92, "PH Calibration: Pedestal (DAC)");
		
		if ( hp->GetMean() < 126. ) {

		  ts->DrawLatex(0.65, 0.82, Form("N: %4.0f", nP));
		  ts->SetTextColor(kRed);
		  ts->DrawLatex(0.65, 0.74, Form("#mu: %4.1f", mP));
		  ts->DrawLatex(0.65, 0.66, Form("#sigma: %4.1f", sP));
		  ts->SetTextColor(kBlack);

		  if ( under ) ts->DrawLatex(0.15, 0.55, Form("<= %i", under));			               
		  if ( over  ) ts->DrawLatex(0.75, 0.55, Form("%i =>", over ));
				
		}
		else {

		  ts->DrawLatex(0.16, 0.82, Form("N: %4.0f", nP));
		  ts->SetTextColor(kRed);
		  ts->DrawLatex(0.16, 0.74, Form("#mu: %4.1f", mP));
		  ts->DrawLatex(0.16, 0.66, Form("#sigma: %4.1f", sP));
		  ts->SetTextColor(kBlack);

		  if ( under ) ts->DrawLatex(0.15, 0.55, Form("<= %i", under));			               
		  if ( over  ) ts->DrawLatex(0.75, 0.55, Form("%i =>", over ));
		}


		
	}
	
	

	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Numerics and Titles
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	// -- Compute the final verdict on this chip  //?? FIXME (below is pure randomness)
	char finalVerdict(0);
	if (nDeadTrimbits > 40) finalVerdict += 1;
	if (nDeadPixel > 40) finalVerdict += 10;
	if (nNoisyPixel > 40) finalVerdict += 10;
	if (nAddressProblems > 40) finalVerdict += 10;
	if (nDeadBumps > 40) finalVerdict += 100;


	// -- Defects
	c1->cd(8);
	tl->SetTextSize(0.10);
	tl->SetTextFont(22);
	double y = 0.92;
	y -= 0.11;
	tl->DrawLatex(0.1, y, "Summary");
	tl->DrawLatex(0.7, y, Form("%d", finalVerdict));

	tl->SetTextFont(132);
	tl->SetTextSize(0.09);
	y -= 0.11;
	tl->DrawLatex(0.1, y, Form("Dead Pixels: "));
	tl->DrawLatex(0.7, y, Form("%4d", nDeadPixel));

	y -= 0.10;
	tl->DrawLatex(0.1, y, Form("Noisy Pixels: "));
	tl->DrawLatex(0.7, y, Form("%4d", nNoisyPixel));
	
	y -= 0.10;
	tl->DrawLatex(0.1, y, "Mask defects: ");
	tl->DrawLatex(0.7, y, Form("%4d", nMaskDefect));

	y -= 0.10;
	tl->DrawLatex(0.1, y, "Dead Bumps: ");
	tl->DrawLatex(0.7, y, Form("%4d", nDeadBumps));

	y -= 0.10;
	tl->DrawLatex(0.1, y, "Dead Trimbits: ");
	tl->DrawLatex(0.7, y, Form("%4d", nDeadTrimbits));

	y -= 0.10;
	tl->DrawLatex(0.1, y, "Address Probl: ");
	tl->DrawLatex(0.7, y, Form("%4d", nAddressProblems));

	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Row 4
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	
	TH2D *htm = new TH2D("htm", "", 80, 0., 80., 52, 0., 52.);

	c1->cd(13);
	
	gStyle->SetOptStat(0);
	h2 = (TH2D*)f->Get(Form("TrimMap_C%i;8", chipId));

        if (h2) {
	  for (int icol = 0; icol < 52; ++icol) {
	    for (int irow = 0; irow < 80; ++irow) {
	      
	      htm->SetBinContent(irow+1, icol+1, h2->GetBinContent(icol+1, irow+1));
	    }
	  }
  	  h2->SetTitle("");
	  h2->GetZaxis()->SetRangeUser(0., 16.);
	  h2->Draw("colz");
	}

	else { ++nRootFileProblems; }

	tl->DrawLatex(0.1, 0.92, "Trim Bits");


	FILE *tCalFile;
	sprintf(string, "%s/../T-calibration/TemperatureCalibration_C%i.dat", dirName, chipId);
	tCalFile = fopen(string, "r");
	char tCalDir[200];
	sprintf(tCalDir, "%s/../T-calibration", dirName);

	if ( tCalFile ) {
	
	  analyse(tCalDir, chipId);
	}
	else {

	  c1->cd(14);
	  TGraph *graph = (TGraph*)f->Get(Form("TempCalibration_C%i", chipId));
	  if ( graph ) { graph->Draw("A*"); }
	  else { ++nRootFileProblems; }
	  tl->DrawLatex(0.1, 0.92, "Temperature calibration");
	}


	// -- Operation Parameters
	c1->cd(16);
	
	y = 0.92;
	tl->SetTextSize(0.10);
	tl->SetTextFont(22);
	y -= 0.11;
	tl->DrawLatex(0.1, y, Form("Op. Parameters"));

	tl->SetTextFont(132);
	tl->SetTextSize(0.09);

	y -= 0.11;
	int vana(-1.);
        vana = dac_findParameter(dirName, "Vana", chipId);
	tl->DrawLatex(0.1, y, "VANA: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3i DAC", vana));
	else tl->DrawLatex(0.7, y, "N/A");

	y -= 0.10;
	int caldel(-1.);
        caldel = dac_findParameter(dirName, "CalDel", chipId);
	tl->DrawLatex(0.1, y, "CALDEL: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3d DAC", caldel));
	else tl->DrawLatex(0.7, y, "N/A");

	y -= 0.10;
	int vthrcomp(-1.);
        vthrcomp = dac_findParameter(dirName, "VthrComp", chipId);
	tl->DrawLatex(0.1, y, "VTHR: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3d DAC", vthrcomp));
	else tl->DrawLatex(0.7, y, "N/A");

	y -= 0.10;
	int vtrim(-1.);
        vtrim = dac_findParameter(dirName, "Vtrim", chipId);
	tl->DrawLatex(0.1, y, "VTRIM: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3d DAC", vtrim));
	else tl->DrawLatex(0.7, y, "N/A");
	
	y -= 0.10;
	int ibias(-1.);
        ibias = dac_findParameter(dirName, "Ibias_DAC", chipId);
	tl->DrawLatex(0.1, y, "IBIAS_DAC: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3d DAC", ibias));
	else tl->DrawLatex(0.7, y, "N/A");
       
	y -= 0.10;
	int voffset(-1.);
        voffset = dac_findParameter(dirName, "VoffsetOp", chipId);
	tl->DrawLatex(0.1, y, "VOFFSETOP: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3d DAC", voffset));
	else tl->DrawLatex(0.7, y, "N/A");

	// -- Page title
	c1->cd(0);
	tl->SetTextSize(0.04);
	tl->SetTextFont(22);
	tl->DrawLatex(0.02, 0.97, Form("%s (C%i)", noslash.Data(), chipId));

	TDatime date;
	tl->SetTextSize(0.02);
	tl->DrawLatex(0.75, 0.97, Form("%s", date.AsString()));

	c1->SaveAs(Form("%s/chipSummary_C%i.ps", dirName, chipId));
	c1->SaveAs(Form("%s/C%i.gif", dirName, chipId));
	
	// -- Dump into logfile
	ofstream OUT(Form("%s/summary_C%i.txt", dirName, chipId));
	OUT << "nDeadPixel: "         << nDeadPixel << endl;
	OUT << "nNoisyPixel: "        << nNoisyPixel << endl;
	OUT << "nDeadTrimbits: "      << nDeadTrimbits << endl;
	OUT << "nDeadBumps: "         << nDeadBumps << endl;
	OUT << "nMaskDefect: "        << nMaskDefect << endl;
	OUT << "nAddressProblems: "   << nAddressProblems << endl;
        OUT << "nRootFileProblems: "  << nRootFileProblems << endl;
	OUT << "SCurve "              << nN_entries << " " << mN << " " << sN << endl;
	OUT << "Threshold "           << nV_entries << " " << mV  << " " << sV << endl;
	OUT << "Gain "                << nG_entries << " " << mG << " " << sG << endl;
	OUT << "Pedestal "            << nP_entries << " " << mP << " " << sP << endl;
	OUT.close();
	
}



// ------------------------------------------------------------------------
void chipSummaries(const char *dirName, const char *module_type)
{
        printf("\nchipSummary> Starting ...\n");

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
        if (!inputFile) { printf("\nchipSummary> ----> COULD NOT FIND %s IN DIRECTORY %s\n", fileName, dirName);
                    printf("chipSummary> ----> Aborting execution  of chipSummaryPage.C ... \n\n", fileName, dirName);   
                    break; }
  
        sprintf(fname, "%s/%s", dirName, adFileName);
        inputFile = fopen(fname, "r");
        if (!inputFile) { sprintf(adFileName,"%s", fileName); }
        else { printf("chipSummary> ----> found separate address decoding file: %s\n", adFileName); fclose (inputFile); }

	sprintf(fname, "%s/%s", dirName, trimFileName);
	inputFile = fopen(fname, "r");
	if (!inputFile) { sprintf(trimFileName,"%s", fileName); }
	else { printf("chipSummary> ----> found separate trim file: %s\n", trimFileName); fclose (inputFile); }

	for (int i = startChip; i < startChip+nChips; i++)
	{
		chipSummary(dirName, i);
	}

	printf("\nchipSummary> ................................................ finished\n\n");
}

//-------------------------------------------------------------------------
int dac_findParameter(const char *dir, const char *dacPar, int chipId) {

  FILE *File, *File50, *File60;
  char fname[1000];
  char string[2000]; int a;
  int prm(-1);
   
  sprintf(fname, "%s/dacParameters_C%i.dat", dir, chipId);
  File = fopen(fname, "r");
  
  sprintf(fname, "%s/dacParameters50_C%i.dat", dir, chipId);
  File50 = fopen(fname, "r");
  
  sprintf(fname, "%s/dacParameters60_C%i.dat", dir, chipId);
  File60 = fopen(fname, "r");
  
  if ( File60 )
  {
    if ( !strcmp(dacPar,"Vana") ) {
      printf("\nchipSummary> Reading dac Parameters Vcal = 60 for chip %i ...\n", chipId);
    }
    for (int i = 0; i < 29; i++) {
  
      fscanf(File60, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;

    }
  }
  
  if ( File50 && !File60 )
  {
    if ( !strcmp(dacPar,"Vana") ) {
      printf("\nchipSummary> Reading dac Parameters Vcal = 50 for chip %i ...\n", chipId);
    }
    for (int i = 0; i < 29; i++) {
  
      fscanf(File50, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;

    }
  }
  
  if (!File50 && !File60)
  { 
    
    for (int i = 0; i < 29; i++) {
  
      fscanf(File, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;

    }
    
    if ( !File )
    {
      printf("\nchipSummary> !!!!!!!!!  ----> DAC Parameters: Could not find a file to read DAC parameter\n\n");
      return 0;
    }
    else
    {
    printf("\nchipSummary> No DAC Parameters after trimming available. Reading %s ...\n\n", dacPar);
    }

  }
  
  if (File)   fclose(File);
  if (File50) fclose(File50);
  if (File60) fclose(File60);
  
  return prm;
}

//----------------------------------------------------------------------------------------------
void initialize()
{
//--- initialise graphical output
//   gROOT->SetStyle("Plain");
//   gStyle->SetTitleBorderSize(0);
//   gStyle->SetPalette(1,0);

//   gCanvas->SetFillColor(10);
//   gCanvas->SetBorderSize(2);

//   gPostScript = new TPostScript("svTemperatureAnalysis.ps", 112);

//--- initialise internal data structures
  for ( Int_t iroc = 0; iroc < numROCs; iroc++ ){
    for ( Int_t itemprange = 0; itemprange < numTempRanges; itemprange++ ){
      for ( Int_t itemperature = 0; itemperature < numTemperatures; itemperature++ ){
	gADCvalue_blackLevel[iroc][itemperature]              = 0;
	gADCvalue_Measurement[iroc][itemprange][itemperature] = 0;
	gADCvalue_Calibration[iroc][itemprange][itemperature] = 0;
      }
    }
  }

//--- initialise histograms and graphs for:
//     o ADC value as function of calibration voltage
//       (histogram shown for one ROC at at time only and fitted with a linear function)
//     o ADC value as function of temperature
//       (histogram shown for one ROC at at time only and fitted with a linear function)
  for ( Int_t iroc = 0; iroc < numROCs; iroc++ ){
    for ( Int_t itemprange = 0; itemprange < numTempRanges; itemprange++ ){
      gADCgraph_Measurement[iroc][itemprange] = new TGraph();
      TString graphName = Form("gADCgraph_Measurement_C%i_TempRange%i", iroc, itemprange);
      gADCgraph_Measurement[iroc][itemprange]->SetName(graphName);
    
      gADCgraph_Calibration[iroc][itemprange] = new TGraph();
      TString graphName = Form("gADCgraph_Calibration_C%i_TempRange%i", iroc, itemprange);
      gADCgraph_Calibration[iroc][itemprange]->SetName(graphName);
    }
  }
}
//---------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------
void load(const char* directoryName, int iroc)
{
//--- read last DAC temperature information from file

  printf("chipSummary> Analysing last DAC temperature measurement for ROC %i\n", iroc);
    
  char inputFileName[255];
  sprintf(inputFileName, "%s/TemperatureCalibration_C%i.dat", directoryName, iroc);
  ifstream* inputFile = new ifstream(inputFileName);

  char dummyString[100];
  
  for ( Int_t itemperature = 0; itemperature < numTemperatures; itemperature++ ){
    Double_t actualTemperature;
    
    Int_t temperatureIndex = ((numTemperatures - 1) - itemperature);
    
    *inputFile >> dummyString;
    *inputFile >> dummyString;
    
    Char_t sign;
    *inputFile >> sign;
    if ( sign == '0' ){
      actualTemperature = 0;
    } else {
      *inputFile >> actualTemperature;
      if ( sign == '+' )
	actualTemperature *= +1;
      else if ( sign == '-' )
	actualTemperature *= -1;
      else 
	cerr << "Warning in <analyse>: cannot parse file " << inputFileName << " !" << endl;
    } 
    
    *inputFile >> dummyString;
    *inputFile >> dummyString;
    *inputFile >> dummyString;
    *inputFile >> gADCvalue_blackLevel[iroc][temperatureIndex];
    *inputFile >> dummyString;
    
    *inputFile >> dummyString;
    *inputFile >> dummyString;
    *inputFile >> dummyString;
    
    for ( Int_t itemprange = 0; itemprange < numTempRanges; itemprange++ ){
      *inputFile >> gADCvalue_Calibration[iroc][itemprange][temperatureIndex];
    }
    
    *inputFile >> dummyString;
    *inputFile >> dummyString;
    *inputFile >> dummyString;
    *inputFile >> dummyString;
    for ( Int_t itemprange = 0; itemprange < numTempRanges; itemprange++ ){
      *inputFile >> gADCvalue_Measurement[iroc][itemprange][temperatureIndex];
    }
    
    *inputFile >> dummyString;
  }
  
  delete inputFile;
}

//---------------------------------------------------------------------------------------------------


//---------------------------------------------------------------------------------------------------
void analyse(const char* directoryName, int chipId)
{
//--- initialize internal data-structures	
  initialize();

//--- read last DAC temperature information used as "training" data
  load(directoryName, chipId);

//--- prepare output graphs

  for ( Int_t itemprange = 0; itemprange < numTempRanges; itemprange++ ){
    TGraph* graph = gADCgraph_Measurement[chipId][itemprange];

    Int_t numPoints = 0;
    for ( Int_t itemperature = 0; itemperature < numTemperatures; itemperature++ ){
      Double_t adcValue   = gADCvalue_Measurement[chipId][itemprange][itemperature];
      Double_t blackLevel = gADCvalue_blackLevel[chipId][itemperature];
//--- only include measurements that correspond to a positive voltage difference
//    (i.e. have an ADC value above the black level)
//    and are within the amplification linear range, below the amplifier saturation
      if ( adcValue > minADCvalue_graph && adcValue < maxADCvalue_graph ){
	graph->SetPoint(numPoints, temperatureValues_target[itemperature], adcValue);
	numPoints++;
      }
    }
  }

  for ( Int_t itemprange = 0; itemprange < numTempRanges; itemprange++ ){
    TGraph* graph = gADCgraph_Calibration[chipId][itemprange];

    Int_t numPoints = 0;
    for ( Int_t itemperature = 0; itemperature < numTemperatures; itemperature++ ){
      Double_t adcValue   = gADCvalue_Calibration[chipId][itemprange][itemperature];
      Double_t blackLevel = gADCvalue_blackLevel[chipId][itemperature];
      //--- only include measurements that correspond to a positive voltage difference
      //    (i.e. have an ADC value above the black level)
      //    and are within the amplification linear range, below the amplifier saturation
      if ( adcValue > minADCvalue_graph && adcValue < maxADCvalue_graph ){
	graph->SetPoint(numPoints, temperatureValues_target[itemperature], adcValue);
	numPoints++;
      }
    }
  }


//--- initialise dummy histogram
//    (neccessary for drawing graphs)
  TH1F* dummyHistogram = new TH1F("dummyHistogram", "dummyHistogram", numTemperatures, temperatureValues_target[0] - 1, temperatureValues_target[numTemperatures - 1] + 1);
  dummyHistogram->SetTitle("");
  dummyHistogram->SetStats(false);
//   dummyHistogram->GetXaxis()->SetTitle("T / degrees");
  dummyHistogram->GetXaxis()->SetTitleOffset(1.2);
//   dummyHistogram->GetYaxis()->SetTitle("ADC");
  dummyHistogram->GetYaxis()->SetTitleOffset(1.3);
  dummyHistogram->SetMaximum(1.25*maxADCvalue_graph);

//--- prepare graph showing range in which the temperature has been measured
//    and the precision of the cooling-box of reaching the temperature setting
    dummyHistogram->GetXaxis()->SetTitle("T/C    ");
    dummyHistogram->GetXaxis()->SetTitleOffset(0.5);
    dummyHistogram->GetXaxis()->SetTitleSize(0.06);
//   dummyHistogram->GetYaxis()->SetTitle("T_{actual} / degrees");
  dummyHistogram->SetMinimum(minADCvalue_graph);
  dummyHistogram->SetMaximum(maxADCvalue_graph);

//--- draw output graphs
  TLegend* legendTempRanges = new TLegend(0.13, 0.47, 0.68, 0.87, NULL, "brNDC");
  legendTempRanges->SetFillColor(10);
  legendTempRanges->SetLineColor(10);

  c1->cd(14);
  //  TString title = Form("ADC Measurement for ROC%i", chipId);
  //  dummyHistogram->SetTitle(title);
  legendTempRanges->Clear();
  Int_t numGraphs = 0;
  for ( Int_t itemprange = 0; itemprange < numTempRanges; itemprange++ ){
    if ( gADCgraph_Measurement[chipId][itemprange]->GetN() >= 2 ){
      if ( numGraphs == 0 ) dummyHistogram->Draw();
      gADCgraph_Measurement[chipId][itemprange]->SetLineColor((itemprange % 8) + 1);
      gADCgraph_Measurement[chipId][itemprange]->SetLineStyle((itemprange / 8) + 1);
      gADCgraph_Measurement[chipId][itemprange]->SetLineWidth(2);
      gADCgraph_Measurement[chipId][itemprange]->Draw("L");
      numGraphs++;
      TString label = Form("Vref = %3.2f", vReference[itemprange]);
      legendTempRanges->AddEntry(gADCgraph_Measurement[chipId][itemprange], label, "l");
    }
  }
   
  tl->DrawLatex(0.12, 0.92, "ADC Measurement"); 
   if ( numGraphs > 0 ){
     legendTempRanges->Draw();
    
//     gCanvas->Update();
//     gPostScript->NewPage();
   }
  
  
  c1->cd(15);
//   TString title = Form("ADC Calibration for ROC%i", chipId);
//   dummyHistogram->SetTitle(title);
  legendTempRanges->Clear();
  Int_t numGraphs = 0;
  for ( Int_t itemprange = 0; itemprange < numTempRanges; itemprange++ ){
    if ( gADCgraph_Calibration[chipId][itemprange]->GetN() >= 2 ){
      if ( numGraphs == 0 ) dummyHistogram->Draw();
      gADCgraph_Calibration[chipId][itemprange]->SetLineColor((itemprange % 8) + 1);
      gADCgraph_Calibration[chipId][itemprange]->SetLineStyle((itemprange / 8) + 1);
      gADCgraph_Calibration[chipId][itemprange]->SetLineWidth(2);
      gADCgraph_Calibration[chipId][itemprange]->Draw("L");
      numGraphs++;
      TString label = Form("Vref = %3.2f", vReference[itemprange]);
      legendTempRanges->AddEntry(gADCgraph_Calibration[chipId][itemprange], label, "l");
    }
  }
 
  tl->DrawLatex(0.12, 0.92, "ADC Calibration");   
   if ( numGraphs > 0 ){
     legendTempRanges->Draw();
    
//     gCanvas->Update();
//     gPostScript->NewPage();
   }
   	
  //  delete gCanvas;
  //  delete gPostScript;
}

//---------------------------------------------------------------------------------------------------
// void shrinkPad(double b, double l, double r, double t) {
//   gPad->SetBottomMargin(b);
//   gPad->SetLeftMargin(l);
//   gPad->SetRightMargin(r);
//   gPad->SetTopMargin(t);
// }

//---------------------------------------------------------------------------------------------------


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
    
    printf("chipSummary> !!!!!!!!!  ----> GRADING: Could not find %s in directory macros\n", fname);
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
      
      ok = 0;
      
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


      if ( !ok ) { printf("*** ERROR: unknown criteria %s !!!\n", CritName); return 0;}
    
    }

    printf("\n");
    readVerbose = 0;

    return 1;
  }
}
