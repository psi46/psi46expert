// =========================================================================== //
// This is a macro to analyse the results of DacOverview                       //
// Usage: Set numberOfSteps same as PHNumberOfSteps in testParameters.dat      //
//        Adjust pruducePsFiles if needed                                      //
// =========================================================================== //

// OverlayRocDac does histos for RocDacs

void OverlayRocDac(const char *filename = "expert.root") 
{
   const int numberOfSteps = 15;
   bool producePsFiles = true;
   char *saveIn = "postscripts";

   gStyle->SetPalette(1);
   gROOT->SetStyle("Plain");

   TFile *f = new TFile(filename);

   // -- get all histos for RocDacs for RocLev, RocUb, TbmLev, TbmUb
   TH1D *histRoc[30][10];
   TH1D *histTbm[30][10];
     
   for (int i = 1; i < 28; i++) 
     {
       // histRoc for RocLev & RocUb
       
       for (int j = 0; j < 5; j++)
	 {
	   histRoc[i][j] = (TH1D*)(f->Get(Form("ROCDAC%i_RocLev%i",i,j))->Clone(Form("ROCDAC%i_RocLev%i",i,j))); 
	 }
       histRoc[i][5] = (TH1D*)(f->Get(Form("ROCDAC%i_RocUb",i))->Clone(Form("ROCDAC%i_RocUb",i)));
            
        // histTbm for TbmLev & TbmUb
     
        for (j = 0; j < 4; j++)
 	 {
 	   histTbm[i][j] = (TH1D*)(f->Get(Form("ROCDAC%i_TbmLev%i",i,j))->Clone(Form("ROCDAC%i_TbmLev%i",i,j))); 
 	 }
        histTbm[i][4] = (TH1D*)(f->Get(Form("ROCDAC%i_TbmUb",i))->Clone(Form("ROCDAC%i_TbmUb",i)));
     } 
 
    // -- set colors 
    int col[26] = {1,2,3,4,5,6,7,8,9,11,14,16,20,23,25,28,30,31,33,35,38,40,41,42,44,46};
	  
    TCanvas *cRoc[28];
    TLegend *legRoc[28];
    TCanvas *cTbm[28];
    TLegend *legTbm[28];

    // -- draw the first histo per canvas
    for (int i = 1; i < 28; i++)
      {
        int scanMax = numberOfSteps+2;
  
        // **** RocLev & RocUb histo **** 
  
        cRoc[i] = new TCanvas();
        cRoc[i]->Clear();
        legRoc[i] = new TLegend(0.15,0.65,0.3,0.95);

        histRoc[i][0]->SetStats(kFALSE);
        histRoc[i][0]->GetYaxis()->SetRangeUser(-2000,2600);
        histRoc[i][0]->SetLineColor(col[1]);
	char histTitle[100];
	sprintf(histTitle,histRoc[i][0]->GetTitle());
        legRoc[i]->AddEntry(histRoc[i][0],histTitle, "L");
        histRoc[i][0]->SetTitle("");
        histRoc[i][0]->Draw();
  
    
        // -- draw the other histos per canvas for RocLev & RocUb     
        for (int j = 1; j < 6; j++)
  	{
	  sprintf(histTitle,histRoc[i][0]->GetTitle());
	  legRoc[i]->AddEntry(histRoc[i][j], histTitle, "L");
  	  histRoc[i][j]->SetTitle("");
  	  histRoc[i][j]->SetStats(kFALSE);
  	  histRoc[i][j]->SetLineColor(col[j+4]);
  	  histRoc[i][j]->Draw("same");
  	}

        // -- draw the legends and print the histos RocLev & RocUb to files
        legRoc[i]->Draw();  
        if (producePsFiles) cRoc[i]->Print(Form("%s/%i_ROCDAC_RocLev_RocUb.png",saveIn,i));  

	  
        // **** TbmLev & TbmUb histo ****
	
        cTbm[i] = new TCanvas();
        cTbm[i]->Clear();
        legTbm[i] = new TLegend(0.15,0.65,0.3,0.95);
  
        histTbm[i][0]->SetStats(kFALSE);
        histTbm[i][0]->GetYaxis()->SetRangeUser(-2000,2600);
        histTbm[i][0]->SetLineColor(col[1]);
	char histTitle[100];
	sprintf(histTitle,histTbm[i][0]->GetTitle());
        legTbm[i]->AddEntry(histTbm[i][0],histTitle, "L");
        histTbm[i][0]->SetTitle("");
        histTbm[i][0]->Draw();   

	// -- draw the other histos per canvas for TbmLev & TbmUb     
        for (int j = 1; j < 5; j++)
  	{	  
	  sprintf(histTitle,histTbm[i][0]->GetTitle());
	  legTbm[i]->AddEntry(histTbm[i][j],histTitle, "L");
  	  histTbm[i][j]->SetTitle("");
  	  histTbm[i][j]->SetStats(kFALSE);
  	  histTbm[i][j]->SetLineColor(col[j+4]);
  	  histTbm[i][j]->Draw("same");
  	}
	
        // -- draw the legends and print the histos TbmLev & TbmUb to files
        legTbm[i]->Draw();   
        if (producePsFiles) cTbm[i]->Print(Form("%s/%i_ROCDAC_TbmLev_TbmUb.png",saveIn,i));  
      }
}

// ############################################################################################################
// ############################################################################################################
// ############################################################################################################



// OverlayTbmDac does histos for TbmDacs
 
void OverlayTbmDac(const char *filename = "expert.root") 
{
  const int numberOfSteps = 15;
  bool producePsFiles = true;
  char *saveIn = "postscripts";

  gStyle->SetPalette(1);
  gROOT->SetStyle("Plain");
 
  TFile *f = new TFile(filename);

  // -- get all histos for TbmDacs for RocLev, RocUb, TbmLev, TbmUb
 
 	  
  TH1D *histRoc[7][10];
  TH1D *histTbm[7][10];
  
  for (int i = 2; i < 5; i++) 
    {
       // histRoc for RocLev & RocUb
 
       for (int j = 0; j < 5; j++)
	 {
	   histRoc[i][j] = (TH1D*)(f->Get(Form("TBMDAC%i_RocLev%i",i,j))->Clone(Form("TBMDAC%i_RocLev%i",i,j))); 
	 }
       histRoc[i][5] = (TH1D*)(f->Get(Form("TBMDAC%i_RocUb",i))->Clone(Form("TBMDAC%i_RocUb",i)));
       
       // histTbm for TbmLev & TbmUb
       
       for (j = 0; j < 4; j++)
	  {
 	   histTbm[i][j] = (TH1D*)(f->Get(Form("TBMDAC%i_TbmLev%i",i,j))->Clone(Form("TBMDAC%i_TbmLev%i",i,j))); 
 	 }
        histTbm[i][4] = (TH1D*)(f->Get(Form("TBMDAC%i_TbmUb",i))->Clone(Form("TBMDAC%i_TbmUb",i)));
     } 
  
  // -- set colors 
  int col[26] = {1,2,3,4,5,6,7,8,9,11,14,16,20,23,25,28,30,31,33,35,38,40,41,42,44,46};
  
  TCanvas *cRoc[28];
  TLegend *legRoc[28];
  TCanvas *cTbm[28];
  TLegend *legTbm[28];
	  
  for (int i = 2; i < 5; i++) 
    {
      int scanMax = numberOfSteps+2;
  
      // **** RocLev & RocUb histo **** 
      
      cRoc[i] = new TCanvas();
      cRoc[i]->Clear();
      legRoc[i] = new TLegend(0.15,0.65,0.3,0.95);
      
      histRoc[i][0]->SetStats(kFALSE);
      histRoc[i][0]->GetYaxis()->SetRangeUser(-700,700);
      histRoc[i][0]->SetLineColor(col[1]);

      char histTitle[100];
      sprintf(histTitle,histRoc[i][0]->GetTitle());
      histRoc[i][0]->SetTitle("");
      legRoc[i]->AddEntry(histRoc[i][0],histTitle, "L");
      histRoc[i][0]->Draw();
      
      // -- draw the other histos per canvas for RocLev & RocUb     
      for (int j = 1; j < 6; j++)
  	{
	  sprintf(histTitle,histRoc[i][j]->GetTitle());
	  legRoc[i]->AddEntry(histRoc[i][j], histTitle, "L");
	  histRoc[i][j]->SetTitle("");
  	  histRoc[i][j]->SetStats(kFALSE);
  	  histRoc[i][j]->SetLineColor(col[j+4]);
  	  histRoc[i][j]->Draw("same");
  	}

      
      // -- draw the legends and print the histos RocLev & RocUb to files
      legRoc[i]->Draw();  
      if (producePsFiles) cRoc[i]->Print(Form("%s/%i_TBMDAC_RocLev_RocUb.png",saveIn,i));  
      
      
      // **** TbmLev & TbmUb histo ****
      
      cTbm[i] = new TCanvas();
      cTbm[i]->Clear();
      legTbm[i] = new TLegend(0.15,0.65,0.3,0.95);
      
      histTbm[i][0]->SetStats(kFALSE);
      histTbm[i][0]->GetYaxis()->SetRangeUser(-2000,2000);
      histTbm[i][0]->SetLineColor(col[1]);
     
      char histTitle[100];
      sprintf(histTitle,histTbm[i][0]->GetTitle());
      legTbm[i]->AddEntry(histTbm[i][0],histTitle, "L");
      histTbm[i][0]->SetTitle("");
      histTbm[i][0]->Draw();
      
      // -- draw the other histos per canvas for TbmLev & TbmUb     
      for (int j = 1; j < 5; j++)
  	{	  
	  sprintf(histTitle,histTbm[i][0]->GetTitle());
	  legTbm[i]->AddEntry(histTbm[i][j], histTitle, "L");
	  histTbm[i][j]->SetTitle("");
  	  histTbm[i][j]->SetStats(kFALSE);
  	  histTbm[i][j]->SetLineColor(col[j+4]);
  	  histTbm[i][j]->Draw("same");
  	}
      
      // -- draw the legends and print the histos TbmLev & TbmUb to files
      legTbm[i]->Draw();   
      if (producePsFiles) cTbm[i]->Print(Form("%s/%i_TBMDAC_TbmLev_TbmUb.png",saveIn,i));     
    }
}



// ############################################################################################################
// ############################################################################################################
// ############################################################################################################



void FitDac(const char *filename1 = "expert.root", const char *filename2 = "expert.root") 
{
  char *saveIn = "postscripts";
  TF1 *linFitTBM = new TF1("linFit", "pol1",100,200);  // definition des fits und angabe, in welchen bereich er fitten soll

  gStyle->SetPalette(1);
  gStyle->SetOptFit(111);
  gROOT->SetStyle("Plain");
 
   TFile *f1 = new TFile(filename1);
   TFile *f2 = new TFile(filename2);
   double slope[100]; 
   char *histName[100];


   // ********  FitDac for TBM Dacs *******
   TCanvas *cTbm[3];
   TH1D *histTbm[10];
 

   // -- get all histos for TbmDacs for RocLev, RocUb, TbmLev, TbmUb
   for(int j = 2; j < 5; j++)
     {
       
       cTbm[j-2] = new TCanvas();
       cTbm[j-2]->Clear();
        
       histTbm[0] = (TH1D*)(f1->Get(Form("TBMDAC%i_RocLev1",j))->Clone(Form("TBMDAC%i_RocLev1",j))); 
       histTbm[1] = (TH1D*)(f1->Get(Form("TBMDAC%i_RocUb",j))->Clone(Form("TBMDAC%i_RocUb",j))); 
       histTbm[2] = (TH1D*)(f1->Get(Form("TBMDAC%i_TbmLev3",j))->Clone(Form("TBMDAC%i_TbmLev3",j))); 
       histTbm[3] = (TH1D*)(f1->Get(Form("TBMDAC%i_TbmUb",j))->Clone(Form("TBMDAC%i_TbmUb",j))); 
 
       cTbm[j-2]->Clear();
       cTbm[j-2]->Divide(2,2);
       for (int i = 0; i < 4; i++)
 	{
 	  cTbm[j-2]->cd(i+1);
 	  histTbm[i]->Draw();
 	  histTbm[i]->Fit("linFit","rq");
	  sprintf(histName,histTbm[i]->GetName());
 	  slope[i] = linFitTBM->GetParameter(1);
 	  printf("slope_%s = %f\n",histName, slope[i]);
 	}
       cTbm[j-2]->Print(Form("%s/TBMDAC%i_Slopes.png",saveIn,j)); 
     }



  // ********  FitDac for ROC  Dacs *******
   TCanvas *cRoc[3];
   TH1D *histRoc[10];
   TF1 *linFitROC = new TF1("linFit", "pol1",10,200);  // definition des fits und angabe, in welchen bereich er fitten soll
 

   // -- get all histos for RocDacs for RocLev, RocUb, TbmLev, TbmUb
   for(int j = 19; j < 22; j++)
     {    
       cRoc[j-19] = new TCanvas();
       cRoc[j-19]->Clear();
        
       histRoc[0] = (TH1D*)(f2->Get(Form("ROCDAC%i_RocLev1",j))->Clone(Form("ROCDAC%i_RocLev1",j))); 
       histRoc[1] = (TH1D*)(f2->Get(Form("ROCDAC%i_RocUb",j))->Clone(Form("ROCDAC%i_RocUb",j))); 
       histRoc[2] = (TH1D*)(f2->Get(Form("ROCDAC%i_TbmLev2",j))->Clone(Form("ROCDAC%i_TbmLev2",j))); 
       histRoc[3] = (TH1D*)(f2->Get(Form("ROCDAC%i_TbmUb",j))->Clone(Form("ROCDAC%i_TbmUb",j))); 
 
       cRoc[j-19]->Clear();
       cRoc[j-19]->Divide(2,2);
       for (int i = 0; i < 4; i++)
 	{
 	  cRoc[j-19]->cd(i+1);
	  histRoc[i]->GetYaxis()->SetRangeUser(-2000,2000);
 	  histRoc[i]->Draw();
 	  histRoc[i]->Fit("linFit","rq");
	  sprintf(histName,histRoc[i]->GetName());
 	  slope[i] = linFitROC->GetParameter(1);
 	  printf("slope_%s = %f\n",histName, slope[i]);
 	}
       cRoc[j-19]->Print(Form("%s/ROCDAC%i_Slopes.png",saveIn,j)); 
     }
}



