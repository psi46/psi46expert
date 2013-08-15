// =========================================================================== //
// This is a macro to analyse the results of a low Vcal range scan over        //
// various pixel. It can analyse the curvature                                 //
// Usage: Set mode and scan range and adjust the input files if needed,        //
//        compile first                                                        //
// =========================================================================== //


#include <iostream.h>

#include "TF1.h"
#include "TH1.h"
#include "TCanvas.h"
#include "TLegend.h"
#include "TStyle.h"
#include "TFile.h"


bool debug = false;
double minPh;
int color;
TF1 *fit = new TF1("Fit", "pol4");
TF1 *linFit = new TF1("linFit", "pol1");
TF1 *pol2Fit = new TF1("pol2Fit", "pol2");

const int mode = 3;  // 1: linear range; 2: linear Vcal range; 3: pedestal; 4: gain; 5: linear PH range

TCanvas *canvas;

void Init();
TLegend* NewLegend(double x1, double x2, double y1, double y2);
TH1D* Analysis(TFile *);

int FitStartPoint(TH1D *histo)
{
    int bin = 255;
    while ((histo->GetBinContent(bin+1) != 7777) && (bin > 0)){
	bin--;
    }
    bin += 2;
    
    while ( histo->GetBinContent(bin) == 0 && (bin < 255)) bin++;
    
    if (bin > 255) return 255;
    return bin;
}

int FitStopPoint(TH1D *histo, int fitStart)
{
    int ibin = 256;
    while ( histo->GetBinContent(ibin) == 0 && (ibin > 0)) ibin--;
    double saturation = histo->GetBinContent(ibin);
    double saturationLow = histo->GetBinContent(fitStart);
    double diff = 0.1*(saturation - saturationLow);
    if (diff < 8) diff = 8;
    int bin = 255;
    
    //while (((histo->GetBinContent(bin+1) > saturation - diff) || histo->GetBinContent(bin+1) == 0) && (bin > 0)) bin--;
    while (((histo->GetBinContent(bin) > saturation - diff) || histo->GetBinContent(bin) == 0) && (bin > 0)) bin--;
    bin += 40;
    
    if (bin > 255) return 255;
    return bin;
}

int PH(int vcal, TH1D *histo, TF1* fit)
{
    int binHigh = vcal;
    while ( histo->GetBinContent(binHigh) == 0 && binHigh < 255 ) binHigh++;
    int binLow = vcal;
    while ( histo->GetBinContent(binLow) == 0 && binLow > 0 ) binLow--;
    
    int bin = 0;
    if ( (binHigh - vcal) < (vcal - binLow) ) bin = binHigh;
    else bin = binLow;
    
    //if (TMath::Abs(histo->GetBinContent(vcal+1) - fit->Eval(vcal)) < 10) return (int)fit->Eval(vcal);
    //else return TMath::Nint(histo->GetBinContent(vcal+1));
    
    if (TMath::Abs(histo->GetBinContent(bin) - fit->Eval(bin)) < 10) return (int)fit->Eval(vcal);
    else return TMath::Nint(histo->GetBinContent(bin));
}


double FindLinearRange(TH1D *histo)
{
    // start and stop point for fit
    
    int fitStart = FitStartPoint(histo);
    int fitStop = FitStopPoint(histo, fitStart);
    
    if (debug) printf("fitStart %i fitStop %i\n", fitStart, fitStop);
    
    if (histo->GetBinContent(fitStop+1) - histo->GetBinContent(fitStart+1) < 40) return 0;
    
    fit->SetRange(fitStart, fitStop);
    double slope = (histo->GetBinContent(fitStop+1)-histo->GetBinContent(fitStart+1))/(fitStop - fitStart);
    fit->SetParameter(0, histo->GetBinContent(fitStart+1)-slope*fitStart);
    fit->SetParameter(1, -3.);
    if (debug) histo->Fit("Fit","R"); else histo->Fit("Fit","RQN");
    
    // find inflection point
    
    double a = fit->GetParameter(4);
    double b = fit->GetParameter(3);
    double c = fit->GetParameter(2);
    double d = fit->GetParameter(1);
    
    int minOffset = (fitStop - fitStart) / 5;
    if (debug) cout << "minOffset " << minOffset << endl; 
    
    double derivative;
    int x, x2;
    for (x = fitStop - 40; x > fitStart + minOffset; x--)
    {
	derivative = 12*a*x*x + 6*b*x + 2*c;
	if (derivative > 0) break;
    }
    
    for (x2 = fitStart + minOffset; x2 < fitStop - 40; x2++)
    {
      derivative = 12*a*x2*x2 + 6*b*x2 + 2*c;
      if (derivative < 0) break;
    }
    x2--;
    
    if ((x2 != fitStop - 2) && (x != x2) && (x != fitStart + minOffset - 1)) 
    {
	if (debug) printf("Second inflection point found %i %i\n", x, x2);  
	double mean = (histo->GetBinContent(fitStop+1) + histo->GetBinContent(fitStart+1))/2;
	if (TMath::Abs(histo->GetBinContent(x2) - mean) < TMath::Abs(histo->GetBinContent(x) - mean)) x = x2;	
    }
    
    if (debug) printf("inflection point %i\n", x);
    
    if (x < fitStart + minOffset) x = fitStart+minOffset;
    if (debug) printf("x %i\n", x);
    
    // create linear fit in inflection point
    
    slope = 4*a*x*x*x + 3*b*x*x + 2*c*x + d;
    linFit->SetRange(fitStart, fitStop);
    linFit->FixParameter(1, slope);
    //double phx = (histo->GetBinContent(x) + histo->GetBinContent(x+1) + histo->GetBinContent(x+2))/3;
    int ibin = x;
    while ( histo->GetBinContent(ibin) == 0 && (ibin < 255)) ibin++;
    double phx = histo->GetBinContent(ibin);
    linFit->FixParameter(0, phx - slope*x);
    
    if (debug)
    {
	TH1D *histo2 = (TH1D*)histo->Clone();
	histo2->Fit("linFit","QRB");
    }
    minPh = fit->Eval(20);
    
    
    // find beginning and end of the linear range
    
    //double diff = (histo->GetBinContent(fitStop+1) - histo->GetBinContent(fitStart+1)) / 10;
    double diff = (histo->GetBinContent(fitStop) - histo->GetBinContent(fitStart)) / 10;
    if (debug) cout << "bin-content(fitStart) = " << histo->GetBinContent(fitStart) << endl;
    if (debug) cout << "bin-content(fitStop) = " << histo->GetBinContent(fitStop) << endl;
    int stopVcal = x, stopPh = PH(stopVcal, histo, fit);
    while((TMath::Abs(stopPh - linFit->Eval(stopVcal)) < diff) && (stopVcal < 255))
    {
	stopVcal++;
	stopPh = PH(stopVcal, histo, fit);
    }
    
    int startVcal = x;
    int startPh = PH(startVcal, histo, fit);
    while(TMath::Abs(startPh - linFit->Eval(startVcal)) < diff)
    {
	startVcal--;
	startPh = PH(startVcal, histo, fit);
    }
    startVcal++;
    startPh = PH(startVcal, histo, fit);
    
    if (debug) printf("linear range in Vcal direction from %i to %i, distance %i\n", startVcal, stopVcal, stopVcal-startVcal);
    if (debug) printf("linear range in PH direction from %i to %i, distance %i \n", startPh, stopPh, stopPh-startPh);
    
    if (mode == 1) return sqrt((double)((stopVcal-startVcal)*(stopVcal-startVcal) + (stopPh-startPh)*(stopPh-startPh)));  
    if (mode == 2) return stopVcal - startVcal;
    if (mode == 3) return linFit->Eval(0);
    if (mode == 4) return slope;
    if (mode == 5) return stopPh-startPh;  
}


void pixelVariationHighRange(const char *filenameOld = "../../../rootFiles/LRScanOldSettings2.root", const char *filenameNew = "../../../rootFiles/LRScanNewSettings.root")
{

    TFile *f1 = new TFile(filenameOld);
    TFile *f2 = new TFile(filenameNew);

    Init();
        
    TLegend *legend = new TLegend(0.65,0.6,0.95,0.95);

    color = 3;
    TH1D *oldSettings = Analysis(f1);
    color = 2;
    TH1D *newSettings = Analysis(f2);

    legend->AddEntry(oldSettings,"oldSettings","L");
    legend->AddEntry(newSettings,"newSettings","L");

    canvas->Clear();
    oldSettings->Draw();
    newSettings->Draw("same");

    legend->Draw();    
    
    canvas->SaveAs("qualityHist.png");
}

void Init()
{
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1,0);
    
    gStyle->SetTitleW(0.5);
    gStyle->SetTitleH(0.08);
    
    gStyle->SetOptFit(00000);
    gStyle->SetOptStat(00000);
    gStyle->SetStatFormat("g");
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(0.5);
    gStyle->SetHistLineWidth(1);
    gROOT->ForceStyle();
    
    canvas = new TCanvas();
    canvas->SetTickx();
    canvas->SetTicky();
    canvas->SetBottomMargin(.14);
    canvas->SetLeftMargin(.13);
    canvas->SetFillColor(0);
    
    
}

TLegend* NewLegend(double x1, double x2, double y1, double y2)
{
    TLegend *l = new TLegend(x1, x2, y1, y2);
    l->SetFillColor(0);
    l->SetBorderSize(0);
    return l;
}


TH1D* Analysis(TFile *f)
{
    
    const int firstRoc = 1;
    const int lastRoc = 15;
    const int firstCol = 0;
    const int lastCol = 24;
    const int firstRow = 0;
    const int lastRow = 24;
        
    const int numRoc = lastRoc - firstRoc + 1;
    const int numCol = lastCol - firstCol + 1;
    const int numRow = lastRow - firstRow + 1;
        
    f->cd();

    TH1D *hist[numRoc][numCol][numRow];
    TH1D *qualityHist = new TH1D("quality","quality",4000,-2000,2000);
    
    qualityHist->SetLineColor(color);
    qualityHist->GetYaxis()->SetTitle("# pixels");


    if (mode == 1)
    {
	qualityHist->GetXaxis()->SetTitle("linear range");
	qualityHist->SetTitle("Linear Range");
	qualityHist->GetXaxis()->SetRangeUser(800,1800);
    }

    if (mode == 2)
    {
	qualityHist->GetXaxis()->SetTitle("Vcal linearity [DAC units]");
	qualityHist->SetTitle("Vcal Linearity");
	qualityHist->GetXaxis()->SetRangeUser(0,150);
    }

    if (mode == 3)
    {
	qualityHist->GetXaxis()->SetTitle("pedestal [ADC units]");
	qualityHist->SetTitle("Pedestal");
	qualityHist->GetXaxis()->SetRangeUser(-1000,0);
    }

    if (mode == 4)
    {
	qualityHist->GetXaxis()->SetTitle("gain");
	qualityHist->SetTitle("Gain");
	qualityHist->GetXaxis()->SetRangeUser(0,40);
    }

    if (mode == 5)
    {
	qualityHist->GetXaxis()->SetTitle("PH linearity [ADC units]");
	qualityHist->SetTitle("PH Linearity");
	qualityHist->GetXaxis()->SetRangeUser(0,2000);
    }

    int numRows = 0; int numCols = 0; int numRocs = 0;
    double quality;
    for (int roc = firstRoc; roc < lastRoc; roc++)
    {
	for (int col = firstCol; col < lastCol; col++)
	{ 
	    for (int row = firstRow; row < lastRow; row++)
	    {
		
		hist[numRocs][numCols][numRows] = (TH1D*)(f->Get(Form("ROC%i_Col%i_Row%i;1",roc,col,row)))->Clone(Form("ROC%i_Col%i_Row%i",roc,col,row)); 
		
		if (debug) cout << "roc = " << roc << "   col = " << col << "   row = " << row << endl;
		
		quality = FindLinearRange(hist[numRocs][numCols][numRows]);
		if (debug) cout << "quality = " << quality << endl;

		qualityHist->Fill(quality);		
		numRows++;
	    }
	    numCols++;
	    numRows = 0;
	}
	numRocs++;
	numCols = 0;
    }
    numRocs = 0;    

    return qualityHist;    

}




int main(int argc, char* argv[]) {

    canvas = new TCanvas();

    char file1[1000], file2[1000];
    
    for (int i = 0; i < argc; i++)
    {
	if (!strcmp(argv[i],"-f1")) 
	{
	    sprintf(file1, argv[++i]);
	}
	if (!strcmp(argv[i],"-f2")) 
	{
	    sprintf(file2, argv[++i]);
	}
    }

    pixelVariationHighRange(file1, file2) ;

}
