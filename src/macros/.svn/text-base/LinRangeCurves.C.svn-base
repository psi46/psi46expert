// =========================================================== //
// Macro to overlay three curves                               //
// =========================================================== //

TCanvas *canvas;

Init()
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
        gStyle->SetMarkerSize(0.5);
        gStyle->SetHistLineWidth(1);
        gROOT->ForceStyle();

        canvas = new TCanvas();
        canvas->SetTickx();
        canvas->SetTicky();
        canvas->SetBottomMargin(.14);
        canvas->SetLeftMargin(.13);


}

TLegend* NewLegend(double x1, double x2, double y1, double y2)
{
        TLegend *l = new TLegend(x1, x2, y1, y2);
        l->SetFillColor(0);
        l->SetBorderSize(0);
        return l;
}


void LinRangeCurves(const char *filename1 = "../rootFiles/LRLScanOldSettings.root", const char *filename2 = "../rootFiles/LRLScanNewSettings.root", const char *filename3 = "../rootFiles/LRLScanNewSettings.root") 
{
  
  Init();
  
  TFile *f1 = new TFile(filename1);
  TFile *f2 = new TFile(filename2);
  TFile *f3 = new TFile(filename3);
  
  // -- get all histos
  
  histOld = (TH1D*)(f1->Get("singleAb112"));
  histMed = (TH1D*)(f2->Get("singleAb112"));
  histNew = (TH1D*)(f3->Get("singleAb112"));
  
  // -- set colors 
  
  leg = new TLegend(0.65,0.2,0.95,0.6);

  TLegendEntry *le;
  TLegend *l = NewLegend(0.60,0.2,0.88,0.4);
  le = l->AddEntry(histOld, "old settings", "l");
  le = l->AddEntry(histMed, "medium settings", "l");
  le = l->AddEntry(histNew, "new settings", "l");

  histOld->SetTitle("");
  histNew->SetTitle("");
  histMed->SetTitle("");
  histOld->SetStats(kFALSE);
  histOld->GetXaxis()->SetTitle("Par1");
  histOld->GetYaxis()->SetTitle("# events");
  histOld->SetLineColor(kBlue);
  histMed->SetLineColor(kRed);
  histNew->SetLineColor(kBlack);
  histOld->SetLineWidth(2);
  histNew->SetLineWidth(2);
  histMed->SetLineWidth(2);
  histOld->GetYaxis()->SetRangeUser(0,300);
  histMed->GetYaxis()->SetRangeUser(0,300);
  histNew->GetYaxis()->SetRangeUser(0,300);
  histOld->GetYaxis()->SetTitleOffset(1.2);

  canvas->Clear();
  histOld->Draw("");
  histMed->Draw("same");
  histNew->Draw("same");

  l->Draw();   
  
}

