// ================================================================= //
// This is a macro to overlay curves from different root files       //
// ================================================================= //


const int numberOfHistos = 7;
TCanvas *canvas;

Init()
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


void Curves(const char *filename1 = "expert.root",const char *filename2 = "expert.root",const char *filename3 = "expert.root",const char *filename4 = "expert.root",const char *filename5 = "expert.root",const char *filename6 = "expert.root",const char *filename7 = "expert.root") {

  Init();

  TFile *f1 = new TFile(filename1);
  TFile *f2 = new TFile(filename2);
  TFile *f3 = new TFile(filename3);
  TFile *f4 = new TFile(filename4);
  TFile *f5 = new TFile(filename5);
  TFile *f6 = new TFile(filename6);
  TFile *f7 = new TFile(filename7);


  TH1D *hist[numberOfHistos];
  TLegend *leg = new TLegend(0.6,0.55,0.8,0.95);

  hist[0] = (TH1D*)(f1->Get("PhVcal_c5r5_C0"));
  hist[1] = (TH1D*)(f2->Get("PhVcal_c5r5_C0"));
  hist[2] = (TH1D*)(f3->Get("PhVcal_c5r5_C0"));
  hist[3] = (TH1D*)(f4->Get("PhVcal_c5r5_C0"));
  hist[4] = (TH1D*)(f5->Get("PhVcal_c5r5_C0"));
  hist[5] = (TH1D*)(f6->Get("PhVcal_c5r5_C0"));
  hist[6] = (TH1D*)(f7->Get("PhVcal_c5r5_C0"));

  int value = 100;

   for(int n = 0; n < numberOfHistos; n++)
     {
       //       hist[n] = (TH1D*)(f->Get(Form("PhVcal_c5r5_C0;%i",n+1)))->Clone(Form("PhVcal_c5r5_C0;%i",n+1));
       hist[n]->SetLineColor(n+1);
       leg->AddEntry(hist[n], Form("Vsf = %i", value) ,"L");
       value+=10;
     }

  hist[0]->GetYaxis()->SetRangeUser(-500,500);

  canvas->Clear();
  hist[0]->Draw();
  for(int n = 1; n < numberOfHistos; n++)
    {
      hist[n]->Draw("same");
    }
  leg->Draw();

}

