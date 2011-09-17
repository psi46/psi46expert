void Init()
{
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1,0);
    
    gStyle->SetTitleW(0.5);
    gStyle->SetTitleH(0.08);
    
    gStyle->SetOptFit(0);
    //    gStyle->SetOptStat("emrou");
    gStyle->SetOptStat("emr");
    gStyle->SetStatFormat("g");
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(0.5);
    gStyle->SetHistLineWidth(1);
    gROOT->ForceStyle();

    canvas = new TCanvas();
    //    InitCanvas(canvas, "log");
    InitCanvas(canvas);

}

InitCanvas(TPad *aPad, char* option = 0)
{
  if (!option) option = "";
  aPad->SetTickx();
  aPad->SetTicky();
  aPad->SetBottomMargin(.14);
  aPad->SetLeftMargin(.15);
  if (strcmp(option, "log") == 0) aPad->SetLogy();
  if (strcmp(option, "logz") == 0) aPad->SetLogz();
}


InitPad(char* option = 0)
{
  InitCanvas(gPad, option);
}



void PrintHist(const char *filename = "test.root", const char *histname = "histname") {

  Init();

  TFile *f = new TFile(filename);
  TH1D *histo;

  histo = (TH1D*)(f->Get(histname));
  histo->GetXaxis()->SetTitleSize(0.055);
  histo->GetYaxis()->SetTitleSize(0.055);
  histo->GetXaxis()->SetLabelSize(0.04);
  histo->GetYaxis()->SetLabelSize(0.05);
  histo->GetXaxis()->SetTitleOffset(1.15);
  histo->GetYaxis()->SetTitleOffset(1.1);
  histo->SetTitle("");
  histo->SetLineStyle(1);
  histo->SetLineWidth(2);
//   histo->GetXaxis()->SetTitle("Vcal [low range DAC units]");
//   histo->GetYaxis()->SetTitle("PH [ADC units]");
     histo->GetXaxis()->SetTitle("par1");
     histo->GetYaxis()->SetTitle("# pixels");
//    histo->GetXaxis()->SetTitle("position resolution [#mum]");
//    histo->GetYaxis()->SetTitle("# pixels");
//     histo->GetXaxis()->SetTitle("Vana [DAC units]");
//     histo->GetYaxis()->SetTitle("par1");
//      histo->GetXaxis()->SetTitle("nChip");
//      histo->GetYaxis()->SetTitle("par1");


     histo->Draw();
     //histo->Draw("colz");

}
