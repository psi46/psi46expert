#include <iostream.h>
#include <TStyle.h>
#include "TString.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TPad.h"
#include "TLatex.h"
#include "TCanvas.h"
#include "TLegend.h"

class ntplAna
{

public:

  ntplAna(const char *filename1, const char *filename2, int *chipId);
  ~ntplAna();

  // ??? Aenderung des datentyps von Map fertig, von Defects noch nicht ganz...

  //  TH2D *Map(char testName[], char type[]);
  void Map(char testName[], char type[]);
  TH1D *Histo(char testName[], char type[]);
  TH2D * Defects(char testName[]);
  void Init();
  void InitCanvas(TPad *aPad, char* option = 0);
  void InitPad(char* option = 0);
  TLegend* NewLegend(double x1, double x2, double y1, double y2);
  TLatex* NewLatex();
  void DrawHisto(TH1 *histo, int color = kBlack, int lineStyle = 1, bool first = false, int width = 2, bool title = true);
  void DrawFirstHisto(TH1 *histo);
  void DrawSecondHisto(TH1 *histo);
  void DrawThirdHisto(TH1 *histo);
  void DrawFourthHisto(TH1 *histo);
  void DrawFifthHisto(TH1 *histo);

  // ----------------------------------------------------


  TFile *f1, *f2;
  TTree *initialTest, *reTest;
  TH2D *diffMap;

};



int main(int argc, char *argv[])
{

  char file1[200], file2[200];
  int chip;

  sprintf(file1, "~/sarah/rootFiles/retestedModules/module-T+17a-0047.root");
  sprintf(file2, "~/sarah/rootFiles/retestedModules/module-T+17a-0039.root");
  
  for (int i = 0; i < argc; i++)
    {
      if (!strcmp(argv[i],"-f1")) sprintf(file1, argv[++i]);
      if (!strcmp(argv[i],"-f2")) sprintf(file2, argv[++i]);
      if (!strcmp(argv[i],"-c")) chip = atoi(argv[++i]);
    }  
  
  ntplAna a(file1,file2, 0);

  return 0;
}



// -- main function


ntplAna::ntplAna(const char *filename1, const char *filename2, int *chipId)
{

  TCanvas *canvas = new TCanvas();

  InitCanvas(canvas);
  Init();

  f1 = new TFile(filename1);
  f2 = new TFile(filename2);


  initialTest = (TTree*)f1->Get("mod");
  reTest = (TTree*)f2->Get("mod");

  // -- Vcal --

  //   int testVcal[16];
  //   initialTest->SetBranchAddress("vcal",testVcal);
  //   initialTest->GetEntry(0);
  
  //   cout << "testVcal = " << testVcal[0] << endl;
  

  canvas->Clear();
  canvas->Divide(4,4);

  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // Row 1
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // -- Pixel Map

  canvas->cd(1);
  InitPad();
  TH2D *defectsMap = Defects("dp");
  defectsMap->DrawCopy("colz");

  // -- SCurve

  canvas->cd(2);
  InitPad();
  TH1D *noiseHist = (TH1D*)Histo("noise", "float");
  DrawFirstHisto(noiseHist);


  // -- Threshold Map

  canvas->cd(3);
  InitPad();
  Map("tthr", "int");
  TH2D *tthrMap = diffMap;
  tthrMap->DrawCopy("colz");

  //  TH2D *tthrMap = (TH2D*)Map("tthr", "int");
//   tthrMap->DrawCopy("colz");


  // -- Threshold Hist

  canvas->cd(4);
  InitPad();
  TH1D *tthrHist = (TH1D*)Histo("tthr", "int");
  DrawFirstHisto(tthrHist);

  
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // Row 2
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // -- Bump Bonding Map

  canvas->cd(5);
  InitPad();
  TH2D *bbMap = Defects("bb");
  bbMap->DrawCopy("colz");


  // -- Bump Bonding Hist

  canvas->cd(6);
  InitPad();
  Map("uthr", "int");
  TH2D *uthrMap = diffMap;
  uthrMap->DrawCopy("colz");


  // -- Trimbit Hist

  canvas->cd(7);
  InitPad();

  // -- Summary

  canvas->cd(8);
  InitPad();

  cout << "Summary" << endl;
  
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // Row 3
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // -- Address decoding Map

  canvas->cd(9);
  InitPad();


  // -- Address Levels

  canvas->cd(10);
  InitPad();


  // -- Gain Hist

  canvas->cd(11);
  InitPad();

  // -- Pedestal Hist

  canvas->cd(12);
  InitPad();
  
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
  // Row 4
  // %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

  // -- Trim Map

  canvas->cd(13);
  InitPad();


  // -- Temperature Calibration

  canvas->cd(14);
  InitPad();


  // -- empty

  canvas->cd(15);
  InitPad();

  // -- Operation Parameters

  canvas->cd(16);
  InitPad();

  canvas->SaveAs("chipDiff.ps");

   // canvas->Close();




}



ntplAna::~ntplAna()
{

}


// TH2D* ntplAna::Map(char testName[], char type[])
// {
void ntplAna::Map(char testName[], char type[])
{

  int ibuff1[16][4160], ibuff2[16][4160];
  float fbuff1[16][4160], fbuff2[16][4160];

  if (!strcmp(type, "int")) 
    {
      initialTest->SetBranchAddress(testName, &ibuff1);
      reTest->SetBranchAddress(testName, &ibuff2);
    }
  if (!strcmp(type, "float")) 
    {
      initialTest->SetBranchAddress(testName, &fbuff1);
      reTest->SetBranchAddress(testName, &fbuff2);
    }
  
  //  TH2D *diff = new TH2D("diff", testName, 52, 0., 52., 80, 0., 80.);
  diffMap = new TH2D("diffMap", testName, 52, 0., 52., 80, 0., 80.);
  int nb(0), nbytes(0);

  
  for (int i = 0; i < initialTest->GetEntries(); ++i) 
    {
      int nb = initialTest->GetEntry(i);   nbytes += nb;
      reTest->GetEntry(i);

      int icol = 0;
      int irow = 0;

      for (int j = 0; j < 4160; ++j)
	{
	  
	  icol = j/80;
	  irow = j%80;
	  if (!strcmp(type, "int"))
	    {
	      diffMap->SetBinContent(icol+1, irow+1, ibuff1[0][j]/*-ibuff2[0][j]*/);
	    }
	  if (!strcmp(type, "float")) 
	    {
	      diffMap->SetBinContent(icol+1, irow+1, fbuff1[0][j]/*-fbuff2[0][j]*/);
	    }
	}
    }
  //  return diff;
}


TH1D* ntplAna::Histo(char testName[], char type[])
{

  int ibuff1[16][4160], ibuff2[16][4160];
  float fbuff1[16][4160], fbuff2[16][4160];

  if (!strcmp(type, "int"))
    {
      initialTest->SetBranchAddress(testName, ibuff1);
      reTest->SetBranchAddress(testName, ibuff2);
    }
  if (!strcmp(type, "float"))
    {
      initialTest->SetBranchAddress(testName, fbuff1);
      reTest->SetBranchAddress(testName, fbuff2);
    }

  TH1D *hist = new TH1D("hist", testName, 100, -1000, 1000);
  int nb(0), nbytes(0);

  for (int i = 0; i < initialTest->GetEntries(); ++i) 
    {

      int nb = initialTest->GetEntry(i);   nbytes += nb;
      reTest->GetEntry(i);    
  
      int icol = 0;
      int irow = 0;

      for (int j = 0; j < 4160; ++j)
	{
	  
	  icol = j/80;
	  irow = j%80;
	  if (!strcmp(type, "int"))
	    {
	      hist->Fill(ibuff1[0][j]/*-ibuff2[0][j]*/);
	    }

	  if (!strcmp(type, "float"))
	    {
	      hist->Fill(fbuff1[0][j]/*-fbuff2[0][j]*/);
	    }
	}
    }


  return hist;
}



 TH2D* ntplAna::Defects(char testName[])
 {
  int buff1[16][4160], buff2[16][4160];
  initialTest->SetBranchAddress("defects", buff1);
  reTest->SetBranchAddress("defects", buff2);
  
  TH2D *diff = new TH2D(Form("diff-%s", testName), testName, 52, 0., 52., 80, 0., 80.);
  int nb(0), nbytes(0), mbytes(0);

  
  for (int i = 0; i < initialTest->GetEntries(); ++i) 
    {
      int nb = initialTest->GetEntry(i);   nbytes += nb;
      //      int mb = reTest->GetEntry(i); mbytes += mb;

      int icol = 0;
      int irow = 0;

      for (int j = 0; j < 4160; ++j)
	{
	  
	  icol = j/80;
	  irow = j%80;
	  int test = 0;
	  if(!(strcmp(testName, "dp"))) test = 1;
	  if(!(strcmp(testName, "np"))) test = 2;
	  if(!(strcmp(testName, "md"))) test = 4;
	  if(!(strcmp(testName, "bb"))) test = 8;
	  if(!(strcmp(testName, "ad"))) test = 16;
	  if(!(strcmp(testName, "tb1"))) test = 32;
	  if(!(strcmp(testName, "tb2"))) test = 64;
	  if(!(strcmp(testName, "tb3"))) test = 128;
	  if(!(strcmp(testName, "tb4"))) test = 256;
	  
// 	  if ((buff1[0][i] & test) == (buff2[0][i] & test))
// 	    {
// 	      diff->SetBinContent(icol+1, irow+1, 1);

// 	    }
// 	  else
// 	    {
// 	      diff->SetBinContent(icol+1, irow+1, 0);
	    
// 	    }

	  diff->SetBinContent(icol+1, irow+1, (buff1[0][j] & test));

	}
    }
  return diff;
}






// --- initializing functions

void ntplAna::Init()
{
  gROOT->SetStyle("Plain");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetPalette(1,0);
  gStyle->SetTitleW(0.5);
  gStyle->SetTitleH(0.08);
  gStyle->SetOptFit(0111);
  gStyle->SetOptStat(111111);
  gStyle->SetStatFormat("g");
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(0.6);
  gStyle->SetHistLineWidth(1);
  gROOT->ForceStyle();
  
}


void ntplAna::InitCanvas(TPad *aPad, char* option)
{
  if (!option) option = "";
   aPad->SetTickx();
   aPad->SetTicky();
   aPad->SetBottomMargin(.14);
   aPad->SetLeftMargin(.17);
   if (strcmp(option, "log") == 0) aPad->SetLogy();
   if (strcmp(option, "logz") == 0) aPad->SetLogz();
}


void ntplAna::InitPad(char* option)
{
  TPad *p1 = new TPad();
  InitCanvas(p1, option);
}


TLegend* ntplAna::NewLegend(double x1, double x2, double y1, double y2)
{
  TLegend *l = new TLegend(x1, x2, y1, y2);
  l->SetFillColor(0);
  l->SetBorderSize(0);
  return l;
}


TLatex* ntplAna::NewLatex()
{
  TLatex *latex = new TLatex;
  latex->SetNDC(kTRUE);
  latex->SetTextSize(0.05);
  return latex;
}



void ntplAna::DrawHisto(TH1 *histo, int color, int lineStyle, bool first, int width, bool title)
{
  histo->GetXaxis()->SetTitleSize(0.055);
  histo->GetYaxis()->SetTitleSize(0.055);
  histo->GetXaxis()->SetLabelSize(0.04);
  histo->GetYaxis()->SetLabelSize(0.05);
  histo->GetXaxis()->SetTitleOffset(1.15);
  histo->GetYaxis()->SetTitleOffset(1.55);

  if (!title) histo->SetTitle("");
  histo->SetLineColor(color);
  histo->SetMarkerColor(color);
  histo->SetLineStyle(lineStyle);
  histo->SetLineWidth(width);
  if (first) histo->Draw("hist");
  else histo->Draw("histsame");
}


void ntplAna::DrawFirstHisto(TH1 *histo)
{
  DrawHisto(histo, kBlack, 1, true);
}


void ntplAna::DrawSecondHisto(TH1 *histo)
{
  DrawHisto(histo, kRed, 2, false);
}


void ntplAna::DrawThirdHisto(TH1 *histo)
{
  DrawHisto(histo, kBlue, 3, false);
}


void ntplAna::DrawFourthHisto(TH1 *histo)
{
  DrawHisto(histo, kGreen+100, 5, false);
}


void ntplAna::DrawFifthHisto(TH1 *histo)
{
  DrawHisto(histo, kMagenta, 8, false);
}

