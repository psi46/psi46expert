void Init()
{
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1,0);
    
    gStyle->SetTitleW(0.5);
    gStyle->SetTitleH(0.08);
    
    gStyle->SetOptFit(00000);
    gStyle->SetOptStat(0);
    gStyle->SetStatFormat("g");
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(0.5);
    gStyle->SetHistLineWidth(1);
    gROOT->ForceStyle();

    canvas = new TCanvas();
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




void AllCurrents(const char *filename = "expert.root") {

  bool producePlots = true;

  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gROOT->SetStyle("Plain");
 
  TFile *f = new TFile(filename);

  // -- get all histos

  TH1D *hist[28];

  for(int i = 0; i < 28; i++)
    {

      hist[i] = ((TH1D*)(f->Get(Form("currentHist%i",i))));

    }

  int col[28] = {1,2,3,4,5,6,7,8,9,11,14,16,20,23,25,28,30,31,33,35,38,40,41,42,44,46,52,53};

  TCanvas *c[28];
  TLegend *leg[28];
  
  for (int i = 1; i < 28; i++)
    {
      c[i] = new TCanvas();
      c[i]->Clear();
      InitCanvas(c[i]);
      leg[i] = new TLegend(0.65,0.2,0.95,0.6);

      char s[100];
      char title[100];
      sprintf(s,hist[i]->GetTitle());
      sscanf(s,"%s",title);
      
      hist[i]->SetStats(kFALSE);
      hist[i]->GetXaxis()->SetTitle(Form("%s [DAC units]",title));
      //      hist[i]->GetYaxis()->SetTitle("digital current");
      hist[i]->GetYaxis()->SetTitle("analog current");
      hist[i]->SetTitle("");
      //      hist[i]->GetYaxis()->SetRangeUser(0.35,0.85);  // digital current
      hist[i]->GetYaxis()->SetRangeUser(0.,0.6);  // analog current
      
      hist[i]->Draw();

     if (producePlots) c[i]->Print(Form("plots/%s.png",title));

    }

  TCanvas *c1 = new TCanvas("c1");
  c1->Clear();
  hist[1]->Draw();
  for(int i = 1; i < 28; i++) 
    {
      hist[i]->SetLineColor(col[i]);
      hist[i]->Draw("same");

    }  
  if (producePlots) c1->Print("plots/all.png");
}



void VsfVar(const char *filename = "expert.root") {

  bool producePlots = false;

  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gROOT->SetStyle("Plain");
 
  TFile *f = new TFile(filename);

  // -- get all histos

  TH1D *hist[16];

  for(int i = 0; i < 16; i++)
    {

      hist[i] = ((TH1D*)(f->Get(Form("currentHist3_ROC%i",i))));

    }

  int col[28] = {1,2,3,4,5,6,7,8,9,11,14,16,20,23,25,28,30,31,33,35,38,40,41,42,44,46,52,53};

  TCanvas *c[16];
  TLegend *leg[16];
  
  for (int i = 0; i < 16; i++)
    {
      c[i] = new TCanvas();
      c[i]->Clear();
      InitCanvas(c[i]);
      leg[i] = new TLegend(0.65,0.2,0.95,0.6);

      char s[100];
      char title[100];
      sprintf(s,hist[i]->GetTitle());
      sscanf(s,"%s",title);
      
      hist[i]->SetStats(kFALSE);
      hist[i]->GetXaxis()->SetTitle(Form("%s [DAC units]",title));
      //      hist[i]->GetYaxis()->SetTitle("digital current");
      hist[i]->GetYaxis()->SetTitle("digital current");
      hist[i]->SetTitle("");
      //      hist[i]->GetYaxis()->SetRangeUser(0.35,0.85);  // digital current
      hist[i]->GetYaxis()->SetRangeUser(0.425,0.45);  // analog current
      
      hist[i]->Draw();

      printf("Idig at Vsf = 0 for ROC %i: %f\n", i, hist[i]->GetBinContent(1));

     if (producePlots) c[i]->Print(Form("plots/%s.png",title));

    }

  TCanvas *c1 = new TCanvas("c1");
  TLegend *legend = new TLegend(0.2,0.5,0.5,0.95);
  c1->Clear();
  //hist[0]->GetYaxis()->SetRangeUser(0.45,0.5);
  //  hist[0]->SetLineColor(col[0]);
  hist[0]->Draw();
  legend->AddEntry(hist[0],"ROC 0","L");
  for(int i = 1; i < 16; i++) 
    {
      hist[i]->SetLineColor(col[i]);
      hist[i]->Draw("same");
      legend->AddEntry(hist[i],Form("ROC %i", i),"L");
    }  
  legend->Draw();
  if (producePlots) c1->Print("plots/all.png");
}
