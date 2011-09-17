// =========================================================================== //
// This is a macro to analyse the results of a high Vcal range scan over       //
// various pixel. It can analyse the linear range, the linear range in         //
// Vcal direction, the offset and the pedestal.                                //
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
char color;
TF1 *fit = new TF1("Fit", "pol4");
TF1 *linFit = new TF1("linFit", "pol1");
TF1 *pol2Fit = new TF1("pol2Fit", "pol2");

const int mode = 1;  // 1: linear range

TCanvas *canvas;

void Init();
TLegend* NewLegend(double x1, double x2, double y1, double y2);
TH1D* Analysis(TFile *);


int FitStartPoint(TH1D *histo)
{
    int bin = 255;
    while ((histo->GetBinContent(bin) != 0) && (bin > 0)) bin--;
    if (bin > 255) return 255;
    return bin;
}


double QualityLowRange(TH1D *histo)
{
  int fitStart = FitStartPoint(histo), fitStop = 255;	
  pol2Fit->SetRange(fitStart, fitStop);
  histo->Fit("pol2Fit","RQ");

  double aoverb = 0;
  if (0 == histo->GetRMS()) aoverb = -99;
  else aoverb = pol2Fit->GetParameter(2)/pol2Fit->GetParameter(1);

  return aoverb;
}



void pixelVariationLowRange(const char *filenameOld = "../rootFiles/LRLScanOldSettings.root", const char *filenameNew = "../rootFiles/LRLScanNewSettings.root", const char *filenameMedium = "../rootFiles/LRLScanOldSettings.root")
{

    TFile *f1 = new TFile(filenameOld);
    TFile *f2 = new TFile(filenameNew);
    TFile *f3 = new TFile(filenameMedium);

    Init();
        
    TLegend *legend = new TLegend(0.65,0.6,0.95,0.95);

    cout << "test" << endl;

    color = 4;
    TH1D *oldSettings = Analysis(f1);
    color = 1;
    TH1D *newSettings = Analysis(f2);
    color = 2;
    TH1D *mediumSettings = Analysis(f3);

    cout << "Entries old settings = " << oldSettings->GetEntries() << endl;
    cout << "Entries medium settings = " << mediumSettings->GetEntries() << endl;
    cout << "Entries new settings = " << newSettings->GetEntries() << endl;

    legend->AddEntry(oldSettings,"oldSettings","L");
    legend->AddEntry(mediumSettings,"mediumSettings","L");
    legend->AddEntry(newSettings,"newSettings","L");

    newSettings->GetYaxis()->SetTitleOffset(1.2);

    canvas->Clear();
    newSettings->Draw();
    oldSettings->Draw("same");
    mediumSettings->Draw("same");

    legend->Draw();    
    
    canvas->SaveAs("aoverb.png");
    canvas->SaveAs("aoverb.root");
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
    const int lastRoc = 2;
    const int firstCol = 0;
    const int lastCol = 52;
    const int firstRow = 0;
    const int lastRow = 80;
        
    const int numRoc = lastRoc - firstRoc + 1;
    const int numCol = lastCol - firstCol + 1;
    const int numRow = lastRow - firstRow + 1;
        
    f->cd();


    TH1D *hist[numRoc][numCol][numRow];
    TH1D *qualityHist = new TH1D("quality","quality",1000,-0.5,0.5);
    
    qualityHist->SetLineColor(color);
    qualityHist->GetYaxis()->SetTitle("# pixels");


    qualityHist->GetXaxis()->SetTitle("a/b");
    qualityHist->SetTitle("a over b");
    qualityHist->GetXaxis()->SetRangeUser(-0.05,0.05);
    qualityHist->GetYaxis()->SetRangeUser(0,2500);
    
    int numRows = 0; int numCols = 0; int numRocs = 0;
    double quality;
    for (int roc = firstRoc; roc < lastRoc; roc++)
    {
	for (int col = firstCol; col < lastCol; col++)
	{ 
	    for (int row = firstRow; row < lastRow; row++)
	    {
		if (debug) cout << "roc = " << roc << "   col = " << col << "   row = " << row << endl;
		if (debug) cout << "numRocs = " << numRocs << "   numCols = " << numCols << "   numRows = " << numRows << endl;

		hist[numRocs][numCols][numRows] = (TH1D*)(f->Get(Form("PhVcal_c%ir%i_C%i;1",col,row,roc)))->Clone(Form("PhVcal_c%ir%i_C%i;1",col,row,roc)); 
 		quality = QualityLowRange(hist[numRocs][numCols][numRows]);

		if (debug) cout << "quality = " << quality << endl;

		if (-99 != quality) qualityHist->Fill(quality);		
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

    char file1[1000], file2[1000], file3[1000];
    
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
	if (!strcmp(argv[i],"-f3")) 
	{
	    sprintf(file3, argv[++i]);
	}
    }

    pixelVariationLowRange(file1, file2, file3) ;

}
