// =========================================================================== //
// This is a macro to analyse the results of DacOverview                       //
// Usage: set  dacNumber (ROC:19,20,21;TBM:2,3,4 )                             //                      
// =========================================================================== //



void LevDeviation(const char *filename = "expert.root", const char *dacType = "ROCDAC", int dacNumber) 
{
  const int numberOfSteps = 15;
  gStyle->SetPalette(1);
  gROOT->SetStyle("Plain");
  gStyle->SetOptFit(111);


  TFile *f = new TFile(filename);
  TH1D *histoRoc[6], *histoTbm[5];
  

  // ********* get  deviation graph for chosen DAC for RocLev ************
  for (int i = 0; i < 5; i++)
    {
      histoRoc[i] = (TH1D*)(f->Get(Form("%s%i_RocLev%i",dacType,dacNumber,i))->Clone(Form("%s%i_RocLev%i",dacType,dacNumber,i))); 
    }
  histoRoc[5] = (TH1D*)(f->Get(Form("%s%i_RocUb",dacType,dacNumber))->Clone(Form("%s%i_RocUb",dacType,dacNumber))); 
  
  char *dacNameRoc =  histoRoc[1]->GetXaxis()->GetTitle();
  double dacValue[numberOfSteps], rocLevDev[numberOfSteps], rocUbDev[numberOfSteps];
  
  // loop over all bins of the histo
  for (int scanValue = 0; scanValue < 256; scanValue+=((int)256/numberOfSteps))
    { 
      int binNumber = histoRoc[1]->FindBin(scanValue);
            
      //get value of upper and lower Level (Lev4, Lev 0 for Roc)
      double Lev4 = histoRoc[4]->GetBinContent(binNumber);
      double Lev0 = histoRoc[0]->GetBinContent(binNumber);
    
      // calculate Black Level and distance between levels
      double black;
      double distance;
      black = (Lev4 + Lev0)/2;
      //      distance = ((black - Lev0) + (Lev4 - black))/2;      
      distance = (Lev4 - Lev0)/2; //xxx      
    
      // expected value for highest and lowest levels (Lev1, Ub for Roc)
      double Lev1_calc, Ub_calc;
      Lev1_calc = black + 4*distance;      
      Ub_calc = black - 4*distance;
          
      // deviation in percent
      double deviation, Lev1, Ub;      
    
      Lev1 = histoRoc[1]->GetBinContent(binNumber);
      if (Lev1_calc != black)      
	{
	  deviation = (Lev1 - Lev1_calc)/(Lev1_calc - black)*100.; // Lev1 deviation
	}      
      else deviation = 0;
      if (binNumber < 16) 
	{
	  dacValue[binNumber-1]=scanValue;
	  rocLevDev[binNumber-1]=deviation;
	}    

      Ub = histoRoc[5]->GetBinContent(binNumber);
      if (Ub_calc != black)      
	{
	  deviation = (Ub - Ub_calc)/(Ub_calc - black)*100.; // Ub deviation
	}
      else deviation = 0;
      if (binNumber < 16) 
	{
	  dacValue[binNumber-1]=scanValue;
	  rocUbDev[binNumber-1]=deviation;
	} 
    }





  // ********* get  deviation graph for chosen DAC for TbmLev ************
  for (int i = 0; i < 4; i++)
    {
      histoTbm[i] = (TH1D*)(f->Get(Form("%s%i_TbmLev%i",dacType,dacNumber,i))->Clone(Form("%s%i_TbmLev%i",dacType,dacNumber,i))); 
    }
  histoTbm[4] = (TH1D*)(f->Get(Form("%s%i_TbmUb",dacType,dacNumber))->Clone(Form("%s%i_TbmUb",dacType,dacNumber))); 
  
  char *dacNameTbm =  histoTbm[1]->GetXaxis()->GetTitle();
  double  tbmLevDev[numberOfSteps], tbmUbDev[numberOfSteps];
  
  // loop over all bins of the histo
  for (int scanValue = 0; scanValue < 256; scanValue+=((int)256/numberOfSteps))
    { 
      int binNumber = histoTbm[1]->FindBin(scanValue);
            
         
      // calculate distance beween levels
      double black = histoTbm[1]->GetBinContent(binNumber);

      printf("black %f \n",black);
      double Lev0 = histoTbm[0]->GetBinContent(binNumber);
      double Lev2 = histoTbm[2]->GetBinContent(binNumber);
      //      double distance = ((black - Lev0) + (Lev2 - black))/2;
      double distance = (Lev2 - Lev0)/2;  //xxx
      
    
      // expected value for highest and lowest levels (Lev3, Ub for Tbm)
      double Lev3_calc, Ub_calc;
      Lev3_calc = black + 2*distance;      
      Ub_calc = black - 4*distance;
          
      // deviation in percent
      double deviation, Lev3, Ub;      
    
      Lev3 = histoTbm[3]->GetBinContent(binNumber);
      if (Lev3_calc != black)      
	{
	  deviation = (Lev3 - Lev3_calc)/(Lev3_calc - black)*100.; // Lev3 deviation
	}  
      else deviation = 0;
      if (binNumber < 16) 
	{
	  tbmLevDev[binNumber-1]=deviation;
	}    

      Ub = histoTbm[4]->GetBinContent(binNumber);
      if (Ub_calc != black)      
	{
	  deviation = (Ub - Ub_calc)/(Ub_calc - black)*100.; // Ub deviation
	}
      else deviation = 0;
      if (binNumber < 16) 
	{
	  tbmUbDev[binNumber-1]=deviation;
	} 
    }



  TCanvas *c = new TCanvas("c");
  leg = new TLegend(0.65,0.25,0.95,0.65);
  c->Clear();
  c->Divide(2,2);
  
  c->cd(1);
  leg = new TLegend(0.65,0.25,0.95,0.65);
  graphRocLev = new TGraph(numberOfSteps, dacValue, rocLevDev);
  graphRocLev->SetTitle(Form("RocLev Deviation for %s",dacNameRoc));  
  graphRocLev->Draw("A*");
  graphRocLev->GetXaxis()->SetTitle(Form("%s",dacNameRoc));
  graphRocLev->GetYaxis()->SetTitle(Form("[%]"));
  graphRocLev->GetYaxis()->SetRangeUser(-20,20);
  leg->AddEntry(graphRocLev,"RocLev Deviation [%]","l");
  //  leg->Draw();
  
  
  c->cd(2);
  leg = new TLegend(0.65,0.35,0.95,0.75);
  graphRocUb = new TGraph(numberOfSteps, dacValue, rocUbDev);
  graphRocUb->Draw("A*");
  graphRocUb->GetXaxis()->SetTitle(Form("%s",dacNameRoc));
  graphRocUb->GetYaxis()->SetTitle(Form("[%]"));
  graphRocUb->SetTitle(Form("Roc Ub Deviation for %s",dacNameRoc));  
  graphRocUb->GetYaxis()->SetRangeUser(-20,20);
  leg->AddEntry(graphRocUb,"Roc Ub Deviation [%]","l");
  //  leg->Draw();


  c->cd(3);
  leg = new TLegend(0.65,0.25,0.95,0.65);
  graphTbmLev = new TGraph(numberOfSteps, dacValue, tbmLevDev);
  graphTbmLev->SetTitle(Form("TbmLev Deviation for %s",dacNameTbm));  
  graphTbmLev->Draw("A*");
  graphTbmLev->GetXaxis()->SetTitle(Form("%s",dacNameTbm));
  graphTbmLev->GetYaxis()->SetTitle(Form("[%]"));
  graphTbmLev->GetYaxis()->SetRangeUser(-20,20);
  leg->AddEntry(graphTbmLev,"Tbm Lev Deviation [%]","l");
  //  leg->Draw();


  c->cd(4);
  leg = new TLegend(0.65,0.35,0.95,0.75);
  graphTbmUb = new TGraph(numberOfSteps, dacValue, tbmUbDev);
  graphTbmUb->Draw("A*");
  graphTbmUb->GetXaxis()->SetTitle(Form("%s",dacNameTbm));
  graphTbmUb->GetYaxis()->SetTitle(Form("[%]"));
  graphTbmUb->SetTitle(Form("Tbm Ub Deviation for %s",dacNameTbm)); 
  graphTbmUb->GetYaxis()->SetRangeUser(-20,20);
  leg->AddEntry(graphTbmUb,"Tbm Ub Deviation [%]","l");
  //  leg->Draw();

  c->Print(Form("postscripts/LevDeviation_%s%i.png",dacType,dacNumber));  
}

