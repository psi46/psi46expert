#include <TMath.h>

// -- data files ----------------------------------------------------------------------------------
const char* fileName = "ShortTest.root";
const char* adFileName = "ShortTest.root"; 
const char* trimFileName = "ShortTest.root" ;

float defectsB, defectsC, maskdefB,  maskdefC;
float currentB, currentC, slopeivB,  slopeivC;
float noiseB,   noiseC,   trimmingB, trimmingC;
float gainB,    gainC,    pedestalB, pedestalC, parB, parC;
float gainMin, parMin, parMax;
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


void chipSummary(const char *dirName, int chipId)
{
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
 
	sprintf(fname, "%s/../../../macros/criteria-short.dat", dirName);
	if ( !readCriteria(fname) ) { 
	  
	  printf("\nchipSummaryShort> ----> COULD NOT READ GRADING CRITERIA !!!");
	  printf("chipSummaryShort> ----> Aborting execution of chipSummaryPage.C ... \n\n", fileName, dirName);  
	  break;
	}

	TGraph *g;
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
	int nGainDefect(0);
	int nPedDefect(0);
	int nParDefect(0);
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

	
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Row 2
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	// -- For numerics and titels see at end


	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Row 3
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	// -- Address levels
	c1->cd(2); 
	gPad->SetLogy(1);
	h1 = (TH1D*)f1->Get(Form("AddressLevels_C%i", chipId));
        if (h1) {
	  h1->SetTitle("");
	  h1->SetAxisRange(-1200., 1200.);
	  h1->Draw();
	  tl->DrawLatex(0.1, 0.92, "Address Levels");
	}

	else { ++nRootFileProblems; }
		
	// -- PHCalibration (Gain & Pedestal)

	TH1D *hg = new TH1D("hg", "", 250, -1., 5.5);
	TH2D *hgm = new TH2D("hgm", "", 52, 0., 52., 80, 0., 80.);
	
	TH1D *hp = new TH1D("hp", "", 1200, -300., 900.);
        TH2D *hpm = new TH2D("hpm", "", 52, 0., 52., 80, 0., 80.);
	hp->StatOverflows(kTRUE);
	TH1D *rp = new TH1D("rp", "", 1200, -300., 900.);
	rp->StatOverflows(kFALSE);

	TH1D *htmp;
	
	float mG(0.), sG(0.), nG(0.), nG_entries(0.);
	float mP(0.), sP(0.), nP(0.), nP_entries(0.); 
	int over = 0., under = 0.;

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
		printf("chipSummaryShort> !!!!!!!!!  ----> phCal: Could not open file %s to read fit results\n", string);
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
					if (gain < gainMin ) { nGainDefect++; }
					ped = par3;
					hp->Fill(ped);
					hg->Fill(gain);
					hgm->SetBinContent(icol + 1, irow + 1, gain);
                                        hpm->SetBinContent(icol + 1, irow + 1, ped);
				}
			}
		}

		mG =  hg->GetMean();
		sG =  hg->GetRMS();
		nG =  hg->Integral(hg->GetXaxis()->GetFirst(), hg->GetXaxis()->GetLast());
		nG_entries = hg->GetEntries();

		under = hg->GetBinContent(0);
		over  = hg->GetBinContent(hp->GetNbinsX()+1);
		
		c1->cd(5);
                gPad->SetLogy(1);
		hg->Draw();
		tl->DrawLatex(0.1, 0.92, "PH Calibration: Gain (ADC/DAC)");
		
		if ( hg->GetMean() > 1.75 ) {
		  ts->DrawLatex(0.15, 0.82, Form("N: %4.0f", nG));
		  ts->DrawLatex(0.15, 0.74, Form("#mu: %4.2f", mG));
		  ts->DrawLatex(0.15, 0.66, Form("#sigma: %4.2f", sG));

		  if ( under ) ts->DrawLatex(0.15, 0.55, Form("<= %i", under));			               
		  if ( over  ) ts->DrawLatex(0.75, 0.55, Form("%i =>", over ));
		}
		else {
		  ts->DrawLatex(0.65, 0.82, Form("N: %4.0f", nG));
		  ts->DrawLatex(0.65, 0.74, Form("#mu: %4.2f", mG));
		  ts->DrawLatex(0.65, 0.66, Form("#sigma: %4.2f", sG));
	
		  if ( under ) ts->DrawLatex(0.15, 0.55, Form("<= %i", under));			               
		  if ( over  ) ts->DrawLatex(0.75, 0.55, Form("%i =>", over ));
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

		} else {

		  xlow = -300; xup = 900; extra = 0;
		  over = 0.; under = 0.;
		}

		  
		hp->GetXaxis()->SetRange(xlow - extra, xup + extra);

		nP    = hp->Integral(hp->GetXaxis()->GetFirst(), hp->GetXaxis()->GetLast());
		under = hp->GetBinContent(0);
		over  = hp->GetBinContent(hp->GetNbinsX()+1);

		nPedDefect = under + over;

		c1->cd(6);

		hgm->Draw("colz");
		tl->DrawLatex(0.1, 0.92, "PH Calibration: Gain (ADC/DAC)");

		
		c1->cd(7);
                gPad->SetLogy(1);

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

		c1->cd(8);

		hpm->Draw("colz");
		tl->DrawLatex(0.1, 0.92, "PH Calibration: Pedestal (ADC/DAC)");

		
	}
	
	float nPar1(0.), mPar1(0.), sPar1(0.);

        TH1D *hPar1 = new TH1D("par1", "", 350, -1., 6.);
	sprintf(string, "%s/phCalibrationFitTan_C%i.dat", dirName, chipId);
	inputFile = fopen(string, "r");

	if (!inputFile)
	{
		printf("chipSummaryShort> !!!!!!!!!  ----> phCal: Could not open file %s to read fit results\n", string);
	}
	else
	{
		for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);

		for (int icol = 0; icol < 52; ++icol)
		{
			for (int irow = 0; irow < 80; ++irow)
			{
				fscanf(inputFile, "%e %e %e %e %s %2i %2i", &par0, &par1, &par2, &par3, string, &a, &b);		
        			hPar1->Fill(par1);
				if ( par1 < parMin || par1 > parMax ) { nParDefect++; }
			}
		}
        }
        
        c1->cd(4);
        gPad->SetLogy(1);
        hPar1->Draw();
        tl->DrawLatex(0.1, 0.92, "PH Par1");
        mPar1 =  hPar1->GetMean();
        sPar1 =  hPar1->GetRMS();
        nPar1 =  hPar1->Integral(hPar1->GetXaxis()->GetFirst(), hPar1->GetXaxis()->GetLast());

        under = hPar1->GetBinContent(0);
        over  = hPar1->GetBinContent(hPar1->GetNbinsX()+1);
		
        ts->DrawLatex(0.15, 0.82, Form("N: %4.0f", nPar1));
        ts->DrawLatex(0.15, 0.74, Form("#mu: %4.2f", mPar1));
        ts->DrawLatex(0.15, 0.66, Form("#sigma: %4.2f", sPar1));

        if ( under ) ts->DrawLatex(0.15, 0.55, Form("<= %i", under));			               
        if ( over  ) ts->DrawLatex(0.75, 0.55, Form("%i =>", over ));
        
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Numerics and Titles
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

	// -- Compute the final verdict on this chip  //?? FIXME (below is pure randomness)
	char finalVerdict(0);
	if (nDeadPixel > 40) finalVerdict += 10;


	// -- Defects
	c1->cd(3);
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
	tl->DrawLatex(0.1, y, "Mask defects: ");
	tl->DrawLatex(0.7, y, Form("%4d", nMaskDefect));

	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	// Row 4
	// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
	
	TH2D *htm = new TH2D("htm", "", 80, 0., 80., 52, 0., 52.);

	// -- Operation Parameters
	c1->cd(9);
	
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

	y -= 0.11;
	int vana(-1.);
        vana = dac_findParameter(dirName, "Vsf", chipId);
	tl->DrawLatex(0.1, y, "VSF: ");
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
	int ibias(-1.);
        ibias = dac_findParameter(dirName, "Ibias_DAC", chipId);
	tl->DrawLatex(0.1, y, "IBIAS_DAC: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3d DAC", ibias));
	else tl->DrawLatex(0.7, y, "N/A");

	y -= 0.10;
	int ibias(-1.);
        ibias = dac_findParameter(dirName, "VIbias_PH", chipId);
	tl->DrawLatex(0.1, y, "VIBIAS_PH: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3d DAC", ibias));
	else tl->DrawLatex(0.7, y, "N/A");
               
	y -= 0.10;
	int voffset(-1.);
        voffset = dac_findParameter(dirName, "VoffsetOp", chipId);
	tl->DrawLatex(0.1, y, "VOFFSETOP: ");
	if (vana >= 0.) tl->DrawLatex(0.6, y, Form("%3d DAC", voffset));
	else tl->DrawLatex(0.7, y, "N/A");
        
        
	c1->cd(11);
	gPad->SetLeftMargin(0.2);
	g  = (TGraph*)f->Get(Form("VanaIana_C%i", chipId));
        if (g) {
	  g->SetTitle("");
	  g->SetMarkerColor(109);
	  g->SetMarkerStyle(29);
	  g->SetLineColor(109);
	  g->SetLineWidth(2);
	  g->Draw("ALP");
	  tl->DrawLatex(0.2, 0.92, "Iana vs. Vana");
	}
        
	c1->cd(12);
	h2 = (TH2D*)f->Get(Form("ValPlot_C%i", chipId));
        if (h2) {
	  h2->SetTitle("");
	  h2->Draw("colz");
	  tl->DrawLatex(0.1, 0.92, "PHRange");
	}

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
	OUT << "nMaskDefect: "        << nMaskDefect << endl;

	OUT << "nGainDefect: "          << nGainDefect << endl;
	OUT << "nPedDefect: "           << nPedDefect << endl;
	OUT << "nParDefect: "           << nParDefect << endl;

        OUT << "nRootFileProblems: "  << nRootFileProblems << endl;
	OUT << "Gain "                << nG         << " " << mG    << " " << sG    << endl;
	OUT << "Pedestal "            << nP         << " " << mP    << " " << sP    << endl;
	OUT << "Parameter1 "          << nPar1      << " " << mPar1 << " " << sPar1 << endl;
	OUT.close();
	
}



// ------------------------------------------------------------------------
void chipSummaries(const char *dirName, const char *module_type)
{
        printf("\nchipSummaryShort> Starting ...\n");

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
        if (!inputFile) { printf("\nchipSummaryShort> ----> COULD NOT FIND %s IN DIRECTORY %s\n", fileName, dirName);
                    printf("chipSummaryShort> ----> Aborting execution  of chipSummaryPage.C ... \n\n", fileName, dirName);   
                    break; }
  
        sprintf(fname, "%s/%s", dirName, adFileName);
        inputFile = fopen(fname, "r");
        if (!inputFile) { sprintf(adFileName,"%s", fileName); }
        else { printf("chipSummaryShort> ----> found separate address decoding file: %s\n", adFileName); fclose (inputFile); }

	sprintf(fname, "%s/%s", dirName, trimFileName);
	inputFile = fopen(fname, "r");
	if (!inputFile) { sprintf(trimFileName,"%s", fileName); }
	else { printf("chipSummaryShort> ----> found separate trim file: %s\n", trimFileName); fclose (inputFile); }

	for (int i = startChip; i < startChip+nChips; i++)
	{
		chipSummary(dirName, i);
	}

	printf("\nchipSummaryShort> ................................................ finished\n\n");
}

//-------------------------------------------------------------------------
int dac_findParameter(const char *dir, const char *dacPar, int chipId) {

  FILE *File, *File40;
  char fname[1000];
  char string[2000]; int a;
  int prm(-1);
   
  sprintf(fname, "%s/dacParameters_C%i.dat", dir, chipId);
  File = fopen(fname, "r");
  
  sprintf(fname, "%s/dacParameters40_C%i.dat", dir, chipId);
  File40 = fopen(fname, "r");
  
  if ( File40 )
  {
    if ( !strcmp(dacPar,"Vana") ) {
      printf("\nchipSummaryShort> Reading dac Parameters Vcal = 40 for chip %i ...\n", chipId);
    }
    for (int i = 0; i < 29; i++) {
  
      fscanf(File40, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;

    }
  }
  
  if (!File40)
  { 
    
    for (int i = 0; i < 29; i++) {
  
      fscanf(File, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;

    }
    
    if ( !File )
    {
      printf("\nchipSummaryShort> !!!!!!!!!  ----> DAC Parameters: Could not find a file to read DAC parameter\n\n");
      return 0;
    }
    else
    {
    printf("\nchipSummaryShort> No DAC Parameters after trimming available. Reading %s ...\n\n", dacPar);
    }

  }
  
  if (File)   fclose(File);
  if (File40) fclose(File40);
  
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

  printf("chipSummaryShort> Analysing last DAC temperature measurement for ROC %i\n", iroc);
    
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
    
    printf("chipSummaryShort> !!!!!!!!!  ----> GRADING: Could not find %s in directory macros\n", fname);
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
