// =========================================================================== //
// This is a macro to analyse the results of PhDacOverview                     //
// Usage: Set numberOfSteps same as PHNumberOfSteps in testParameters.dat      //
//        Adjust pruducePsFiles if needed                                      //
// =========================================================================== //


void Overlay(const char *filename = "expert.root") {

  Init();

  const int numberOfSteps = 5;
  bool producePsFiles = false;

  gStyle->SetPalette(1);
  gROOT->SetStyle("Plain");
 
  TFile *f = new TFile(filename);
  
  int scanMax = numberOfSteps+2;
  
  // -- get all histos

  TH1D *hist[28][numberOfSteps+2];
  TH1D *hist2[3][numberOfSteps+2];

  for (int i = 1; i < 28; i++)
    {
      for (int j = 1; j < scanMax; j++)
	{
     	  hist[i][j] = (TH1D*)(f->Get(Form("DAC%i_Value%i",i,j))->Clone(Form("DAC%i_Value%i",i,j))); 
	}
    }
  for (int i = 2; i < 5; i++)
    {
      for (int j = 1; j < scanMax; j++)
	{
     	  hist2[i-2][j] = (TH1D*)(f->Get(Form("TBM_DAC%i_Value%i",i,j))->Clone(Form("TBM_DAC%i_Value%i",i,j))); 
	}
    }

  // -- set colors 
 
  int col[26] = {1,2,3,4,5,6,7,8,9,11,14,16,20,23,25,28,30,31,33,35,38,40,41,42,44,46};
  
  TCanvas *c[28], *c2[3];
  TLegend *leg[28], *leg2[3];

  // -- draw the first histo per canvas
 
  for (int i = 1; i < 28; i++)
    {
      c[i] = new TCanvas();
      c[i]->Clear();
      c[i]->SetBottomMargin(.14);
      c[i]->SetLeftMargin(.16);
      c[i]->SetTickx();
      c[i]->SetTicky();
      leg[i] = new TLegend(0.65,0.2,0.95,0.6);
    
      hist[i][1]->SetStats(kFALSE);
      hist[i][1]->GetXaxis()->SetTitle("Vcal[DAC units]");
      hist[i][1]->GetYaxis()->SetTitle("PH [ADC units]");
      hist[i][1]->GetYaxis()->SetRangeUser(-2000,2000);
      char s[100];
      char title[100];
      int value;
      sprintf(s,hist[i][1]->GetTitle());
      sscanf(s,"%s=%i",title,&value);
      leg[i]->AddEntry(hist[i][1],value,"L");
      //leg[i]->AddEntry(hist[i][1],Form("VwllSh=%s",s),"L");      

      DrawHisto(hist[i][1],col[0],1,true,2, false);   

      // -- draw the other histos per canvas      

      for (int j = 2; j < scanMax; j++)
	{
	  
	  char s[100];
	  char title[100];
	  int value;
	  sprintf(s,hist[i][j]->GetTitle());
	  sscanf(s,"%s=%i",title,&value);
	  leg[i]->AddEntry(hist[i][j],value,"L");
	  //	  leg[i]->AddEntry(hist[i][j],Form("VwllSh=%s",s),"L");
 	  DrawHisto(hist[i][j],col[j+1],1,false,2, false);
	}

      // -- draw the legends and print the histos to files

      leg[i]->Draw();   
      if (producePsFiles) c[i]->Print(Form("postscripts/%i_%s.png",i,title));   
    }


   for (int i = 2; i < 5; i++)
     {
       c2[i-2] = new TCanvas();
       c2[i-2]->Clear();
       c2[i-2]->SetBottomMargin(.14);
       c2[i-2]->SetLeftMargin(.16);
       c2[i-2]->SetTickx();
       c2[i-2]->SetTicky();
       leg2[i-2] = new TLegend(0.65,0.2,0.95,0.6);
    
       hist2[i-2][1]->SetStats(kFALSE);
       hist2[i-2][1]->GetXaxis()->SetTitle("Vcal[DAC units]");
       hist2[i-2][1]->GetYaxis()->SetTitle("PH [ADC units]");
       hist2[i-2][1]->GetYaxis()->SetRangeUser(-2000,2000);
       char s[100];
       char title[100];
       int value;
       sprintf(s,hist2[i-2][1]->GetTitle());
       sscanf(s,"%s=%i",title,&value);
       leg2[i-2]->AddEntry(hist2[i-2][1],value,"L");
    
       DrawHisto(hist2[i-2][1],col[0],1,true,2, false);

       // -- draw the other histos per canvas      

       for (int j = 2; j < scanMax; j++)
 	{
	  
 	  char s[100];
 	  char title[100];
 	  int value;
 	  sprintf(s,hist2[i-2][j]->GetTitle());
 	  sscanf(s,"%s=%i",title,&value);
 	  leg2[i-2]->AddEntry(hist2[i-2][j],value,"L");
      	  DrawHisto(hist2[i-2][j],col[j+1],1,false,2, false);

 	}

       // -- draw the legends and print the histos to files

       leg2[i-2]->Draw();   
       if (producePsFiles) c2[i-2]->Print(Form("postscripts/%i_%s.png",i,title));   
     }








}

Double_t FitfcnTan( Double_t *x, Double_t *par)
{
  return par[3] + par[2] * TMath::TanH(par[0]*x[0] - par[1]);
}


void VsfVar(const char *filename = "expert.root")
{

  Init();

  TFile *f = new TFile(filename);

  TH1D *hist[6][10][10];
  TH1D *par1Hist[6];
  TH1D *par1VsfHist = new TH1D("Linearity vs. Vsf","linVsf",6,150,270);
  int vsfCount = 0;
  int par1, mean, rms;


  TF1 *tanFit = new TF1("tanFit", FitfcnTan, 50., 1500., 4);
  tanFit->SetNpx(1000);

  for (int vsf = 150; vsf < 255; vsf+=20)
    {
      par1Hist[vsfCount] = new TH1D(Form("Par1ForVsf%i\n",vsf),Form("Par1ForVsf%i\n",vsf),100,-5.,15.);
      for (int col = 0; col < 10; col++)
	{
	  printf("col = %d\n",col);
	  for (int row = 0; row < 10; row++)
	    {  
	      printf("row = %d\n",row);
	      printf ("vsfCount = %i\n",vsfCount);
	      hist[vsfCount][row][col] = (TH1D*)(f->Get(Form("Vsf%d_Col%d_Row%d",vsf,col,row))->Clone(Form("Vsf%d_Col%d_Row%d",vsf,col,row)));
	      
	      tanFit->SetParameter(0,0.00382);
	      tanFit->SetParameter(1,0.886);
	      tanFit->SetParameter(2,112.7);
	      tanFit->SetParameter(3,113.0);
	      tanFit->SetRange(50,1500);
	      
	      hist[0][0][0]->Fit("tanFit", "R", "");
	      par1 = tanFit->GetParameter(1);
	      par1Hist[vsfCount]->Fill(par1);
	    }
	}
      vsfCount++;
    }

  vsfCount = 0;

  for (int vsf = 150; vsf < 255; vsf+=20)
    {
      mean = par1Hist[vsfCount]->GetMean();
      rms = par1Hist[vsfCount]->GetRMS();
      par1VsfHist->SetBinContent(vsfCount+1,mean);
      vsfCount++;
    }

  par1VsfHist->Draw("hist");

}


