TCanvas *canvas;
TFile *tFile;

void Init()
{
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1,0);
    
    gStyle->SetTitleW(0.5);
    gStyle->SetTitleH(0.08);
    
    gStyle->SetOptFit(00000);
    gStyle->SetOptStat(000);
    gStyle->SetStatFormat("g");
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(1.5);
    gStyle->SetHistLineWidth(1);
    gStyle->SetLineWidth(2);
    gROOT->ForceStyle();

    canvas = new TCanvas();
    InitCanvas(canvas);

}

InitCanvas(TPad *aPad, char* option = 0)
{
  if (!option) option = "";
  aPad->SetTickx();
  aPad->SetTicky();
  //  aPad->SetGridx();
  //  aPad->SetGridy();
  aPad->SetBottomMargin(.14);
  aPad->SetLeftMargin(.15);
  if (strcmp(option, "log") == 0) aPad->SetLogy();
  if (strcmp(option, "logz") == 0) aPad->SetLogz();
}


InitPad(char* option = 0)
{
  InitCanvas(gPad, option);
}


TLatex* NewLatex()
{
  TLatex *latex = new TLatex;
  latex->SetNDC(kTRUE);
  latex->SetTextSize(0.08);
  return latex;
}


void DrawGraph(TGraph *graph, int color = kBlack, int lineStyle = 1, bool axis = false, int width = 2, bool title = false)
{
  graph->GetXaxis()->SetTitleSize(0.055);
  graph->GetYaxis()->SetTitleSize(0.055);
  graph->GetXaxis()->SetLabelSize(0.04);
  graph->GetYaxis()->SetLabelSize(0.05);
  graph->GetXaxis()->SetTitleOffset(1.15);
  graph->GetYaxis()->SetTitleOffset(1.55);

/*   if (!title) graph->SetTitle(""); */
  graph->SetLineColor(color);
  graph->SetLineStyle(lineStyle);
  graph->SetLineWidth(width);
  if (axis) graph->Draw("AP");
  else graph->Draw("P");
}


void DrawHisto(TH1 *histo, int color = kBlack, int lineStyle = 1, bool first = false, int width = 2, bool title = true)
{
  histo->GetXaxis()->SetTitleSize(0.055);
  histo->GetYaxis()->SetTitleSize(0.055);
  histo->GetXaxis()->SetLabelSize(0.05);
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


void DrawFirstHisto(TH1 *histo)
{
  DrawHisto(histo, kBlack, 1, true);
}


void DrawSecondHisto(TH1 *histo)
{
  DrawHisto(histo, kRed, 2, false);
}


void DrawThirdHisto(TH1 *histo)
{
  DrawHisto(histo, kBlue, 3, false);
}


void DrawFourthHisto(TH1 *histo)
{
  DrawHisto(histo, kGreen+100, 5, false);
}


void DrawFifthHisto(TH1 *histo)
{
  DrawHisto(histo, kMagenta, 8, false);
}


void DrawHistos(TH1 *histo1, TH1 *histo2, TH1 *histo3 = 0)
{
  double max = histo1->GetMaximum();
  if (histo2->GetMaximum() > max) max = histo2->GetMaximum();
  if (histo3 && (histo3->GetMaximum() > max)) max = histo3->GetMaximum();

  histo1->SetMaximum(max*1.1);
  histo1->SetMinimum(0.);

  DrawFirstHisto(histo1);
  DrawSecondHisto(histo2);
  if (histo3) DrawThirdHisto(histo3);
}


