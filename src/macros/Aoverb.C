void Init()
{
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1,0);
    
    gStyle->SetTitleW(0.5);
    gStyle->SetTitleH(0.08);
    
    gStyle->SetOptFit(00000);
    gStyle->SetOptStat(111);
    gStyle->SetStatFormat("g");
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(0.5);
    gStyle->SetHistLineWidth(1);
    gROOT->ForceStyle();
}


void Aoverb(const char *dirName = "M0312-061109.11:12/T-10a") {

  Init();

  float par0, par1, par2, par3, par4, par5; // Parameters of Vcal vs. Pulse Height Fit
  int a,b;

  TH1D *quality = new TH1D("quality","quality",1000,-0.05,0.5);
  TH1D *chipQuality[16];


  for (int chipId = 0; chipId < 16; chipId++)
    {
      chipQuality[chipId] = new TH1D("quality","quality",1000,-0.05,0.5);
      FILE *inputFile;
      char string[200];
      sprintf(string, "%s/phCalibrationFit_C%i.dat", dirName, chipId);
      inputFile = fopen(string, "r");

	if (!inputFile)
	{
		printf("!!!!!!!!!  ----> phCal: Could not open file %s to read fit results\n", string);
	}
	else
	  {
	    for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);
	    
	    for (int icol = 0; icol < 52; ++icol)
	      {
		for (int irow = 0; irow < 80; ++irow)
		  {
		    fscanf(inputFile, "%e %e %e %e %e %e %s %2i %2i", &par0, &par1, &par2, &par3, &par4, &par5, string, &a, &b);
		    
		    if (par2 != 0.)  // dead pixels have par2 == 0.
		      {
//  			quality->Fill(par2);
//  			chipQuality[chipId]->Fill(par2);
//  			quality->Fill(par5);
//  			chipQuality[chipId]->Fill(par5);
  			quality->Fill(par5/TMath::Abs(par2));
  			chipQuality[chipId]->Fill(par5/TMath::Abs(par2));

		      }
		  }
	      }
		    cout << "quality chip " << chipId << ": " << chipQuality[chipId]->GetMean() << endl;
	  }
    }

  TCanvas *c1 = new TCanvas("c1");
  c1->Clear();
  c1->SetTickx();
  c1->SetTicky();
  gPad->SetLogy(1);
  quality->SetTitle("a over b whole module");
  quality->GetXaxis()->SetTitle("a over b");
  quality->GetYaxis()->SetTitle("number of pixels");
  quality->Draw();

  TCanvas *c2 = new TCanvas("c2");
  c2->Clear();

  c2->Divide(4,4);
  for (int chip = 0; chip < 16; chip++)
    {
      c2->cd(chip+1);
      gPad->SetLogy(1);
      gPad->SetTickx();
      gPad->SetTicky();
      chipQuality[chip]->SetTitle(Form("a over b chip %i", chip));
      chipQuality[chip]->GetXaxis()->SetTitle("a over b");
      chipQuality[chip]->GetYaxis()->SetTitle("number of pixels");
      chipQuality[chip]->Draw();
    }

}
