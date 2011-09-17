
void chipHistos(const char *filenames = "/export/data1/dambach/hardware/ntpls/gradeA/*.root")
{

  Init();      // do not forget to load the Utilities.C

  TH1D *gainH = new TH1D("gainH", "RMS(gain)/Mean(gain)", 100, 0., 0.2);
  TH1D *pedH = new TH1D("pedH", "RMS(pedestal)", 100, 0., 100.);
  TH1D *noiseH = new TH1D("noiseH", "RMS(trimmed threshold)", 100, 0., 8.);
  TH1D *tthrH = new TH1D("tthrH", "Mean (trimmed threshold)", 100, 50., 70.);
  
  TH1D *gainDist = new TH1D("gainDist", "gainR/gainM", 100, 0., 10.);
  TH1D *pedDist = new TH1D("pedDist", "pedR", 100, -1000., 1000.);
  
  TChain *c = new TChain("mod");
  c->Add(filenames);
  
  float ped[16][4160], gain[16][4160];
  int defects[16][4160];
  c->SetBranchAddress("ped", ped); 
  c->SetBranchAddress("gain", gain);
  c->SetBranchAddress("defects", defects);
  
  int nb(0), nbytes(0);
  
  for (int i = 0; i < c->GetEntries(); ++i)
    {
      nb = c->GetEntry(i);   nbytes += nb;
      
      
      for(int chipId = 0; chipId < 16; chipId++)
	{
	  gainDist->Reset();
	  pedDist->Reset();
	  
	  int allPix = 4160; 
	  for (int p = 0; p < allPix; p++)
	    {
	      if(defects[chipId][p] == 0)
		{
		  gainDist->Fill(gain[chipId][p]);
		  pedDist->Fill(ped[chipId][p]);	      
		}
	    }
	  double gainRMS = gainDist->GetRMS();
	  double pedRMS = pedDist->GetRMS();
	  double gainMean = gainDist->GetMean();
	  double pedMean = pedDist->GetMean();
	  
	  if (gainMean != 0) gainH->Fill(gainRMS/gainMean);
	  pedH->Fill(pedRMS);
	}
    }
  
  canvas->Clear();
  canvas->Divide(2,2);
  
  canvas->cd(1);
  InitPad("log");
  gainH->GetXaxis()->SetTitle("DAC units");
  gainH->GetYaxis()->SetTitle("# pixels");
  gainH->GetXaxis()->SetTitleSize(0.055);
  gainH->GetYaxis()->SetTitleSize(0.055);
  gainH->Draw();
  
  canvas->cd(2);
  InitPad("log");
  pedH->GetXaxis()->SetTitle("DAC units");
  pedH->GetYaxis()->SetTitle("# pixels");
  pedH->GetXaxis()->SetTitleSize(0.055);
  pedH->GetYaxis()->SetTitleSize(0.055);
  pedH->Draw();

  canvas->cd(3);
  InitPad("log");
  noiseH->GetXaxis()->SetTitle("DAC units");
  noiseH->GetYaxis()->SetTitle("# pixels");
  noiseH->GetXaxis()->SetTitleSize(0.055);
  noiseH->GetYaxis()->SetTitleSize(0.055);
  c->Draw("tthrR>>noiseH");

  canvas->cd(4);
  InitPad("log");
  tthrH->SetMinimum(0.5);
  tthrH->GetXaxis()->SetTitle("DAC units");
  tthrH->GetYaxis()->SetTitle("# pixels");
  tthrH->GetXaxis()->SetTitleSize(0.055);
  tthrH->GetYaxis()->SetTitleSize(0.055);
  c->Draw("tthrM>>tthrH");

}

