// =========================================================================== //
// This is a macro to analyse the results of PHRange                           //
// =========================================================================== //

void Init()
{
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1,0);
    
    gStyle->SetTitleW(0.5);
    gStyle->SetTitleH(0.08);
    
    gStyle->SetOptFit(0);
    //    gStyle->SetOptStat("emrou");
    gStyle->SetOptStat(0);
    gStyle->SetStatFormat("g");
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(0.5);
    gStyle->SetHistLineWidth(1);
    gROOT->ForceStyle();

    canvas = new TCanvas();
    //InitCanvas(canvas, "log");
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


void ValPlot(const char *filename = "expert.root") 
{
 gStyle->SetPalette(1);
 gROOT->SetStyle("Plain");

 TCanvas *c1, *c2, *c3, *c4[116] ;
 int NumberOfBins = 4000/10; 
 int lowValue = -2000;
 int upValue = 2000;
 int BinValue;


 TFile *f = new TFile(filename);
 
 // ----- ADC plot
 TH2D  *RocLevUb[16],*RocPh[16],*ValPlot[16], *RocLevelUb, *RocPH, *ValidPlot;
 RocLevelUb = new TH2D("RocLevUb","Roc Level and UB", 96, 0, 96, NumberOfBins, lowValue, upValue);
 RocLevelUb ->GetXaxis()->SetTitle("ROC 0 - ROC 16");   
 RocLevelUb ->GetYaxis()->SetTitle("Level and Ultra Black [ADC units]");
 RocPH = new TH2D("RocPh","Roc PH", 16, 0, 16, NumberOfBins, lowValue, upValue);
 RocPH ->GetXaxis()->SetTitle("ROC 0 - ROC 16");   
 RocPH ->GetYaxis()->SetTitle("Pulse high [ADC units]");
 ValidPlot = new TH2D("ValPlot","Validationplot", 70, 0, 70, NumberOfBins, lowValue, upValue);
 ValidPlot ->GetXaxis()->SetTitle("ADC");                 
 ValidPlot ->GetYaxis()->SetTitle("[ADC units]");
 
 
 for (int i = 0; i < 16; i++)
   {    
     RocLevUb[i] = (TH2D*)(f->Get(Form("Roc_%i_LevUb",i))->Clone(Form("Roc_%i_LevUb",i))); 
     RocPh[i] = (TH2D*)(f->Get(Form("Roc_%i_Ph",i))->Clone(Form("Roc_%i_Ph",i))); 
     ValPlot[i] = (TH2D*)(f->Get(Form("ValPlot_%i",i))->Clone(Form("ValPlot_%i",i))); 
     for(int x = 0; x < 70; x++)
       {
	 for(int y = lowValue; y < upValue; y++)
	   {
	     if(x > 0 && x < 7) // RocLevUb
	       {
		 BinValue = RocLevUb[i]->GetBinContent(x+1,y);  
		 RocLevelUb->SetBinContent(i*6+x,y,BinValue);
	       }	     
	     BinValue = ValPlot[i]->GetBinContent(x+1,y); // ADC
	     ValidPlot->Fill(x,y*10-2000,BinValue);
	     if(x==2) // RocPh
	       {
		 BinValue = RocPh[i]->GetBinContent(x,y);  
		 RocPH->SetBinContent(i+1,y,BinValue);
	       } 
	   }
       }
   }
 

 c1 = new TCanvas(); 
 c1->Clear();
 ValidPlot->SetStats(kFALSE);
 ValidPlot->Draw("colz");
 c2 = new TCanvas(); 
 c2->Clear();
 RocLevelUb->SetStats(kFALSE); 
 RocLevelUb->Draw("colz");
 c3 = new TCanvas();
 c3->Clear();
 RocPH->SetStats(kFALSE); 
 RocPH->Draw("colz");
		       
 
 c1->Print("postscripts/ValPlot.png");
 c2->Print("postscripts/ RocLevUb.png");
 c3->Print("postscripts/ RocPh.png");

}
