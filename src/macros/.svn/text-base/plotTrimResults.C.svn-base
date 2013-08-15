trimResults(char *fileName)
{
  TFile *f = new TFile(fileName);
  
  char fname[1000];

  TCanvas *c1 = new TCanvas();
  c1->Divide(3,4);
  
  for (int i = 0; i < 4; i++)
  {
  
	c1->cd(i*3 + 1);
	gPad->SetLogy();
	sprintf(fname,"VcalThresholdMap_C%iDistribution;1", i);
	((TH1D*)f->Get(fname))->DrawCopy();
	
	c1->cd(i*3 + 2);
	gPad->SetLogy();
	sprintf(fname,"VcalThresholdMap_C%iDistribution;7", i);
	((TH1D*)f->Get(fname))->DrawCopy();
	
	c1->cd(i*3 + 3);
	gPad->SetLogy();
	sprintf(fname,"CalThresholdMap_C%iDistribution;1", i);
	((TH1D*)f->Get(fname))->DrawCopy();

  }
  

  TCanvas *c2 = new TCanvas();
  c2->Divide(3,4);
  
  for (int i = 4; i < 8; i++)
  {
  
	c2->cd(i*3 + 1 - 12);
	gPad->SetLogy();
	sprintf(fname,"VcalThresholdMap_C%iDistribution;1", i);
	((TH1D*)f->Get(fname))->DrawCopy();
	
	c2->cd(i*3 + 2 - 12);
	gPad->SetLogy();
	sprintf(fname,"VcalThresholdMap_C%iDistribution;7", i);
	((TH1D*)f->Get(fname))->DrawCopy();
	
	c2->cd(i*3 + 3 - 12);
	gPad->SetLogy();
	sprintf(fname,"CalThresholdMap_C%iDistribution;1", i);
	((TH1D*)f->Get(fname))->DrawCopy();
  }
  
  TCanvas *c3 = new TCanvas();
  c3->Divide(3,4);
  
  for (int i = 8; i < 12; i++)
  {
  
	c3->cd(i*3 + 1 - 24);
	gPad->SetLogy();
	sprintf(fname,"VcalThresholdMap_C%iDistribution;1", i);
	((TH1D*)f->Get(fname))->DrawCopy();
	
	c3->cd(i*3 + 2 - 24);
	gPad->SetLogy();
	sprintf(fname,"VcalThresholdMap_C%iDistribution;7", i);
	((TH1D*)f->Get(fname))->DrawCopy();
	
	c3->cd(i*3 + 3 - 24);
	gPad->SetLogy();
	sprintf(fname,"CalThresholdMap_C%iDistribution;1", i);
	((TH1D*)f->Get(fname))->DrawCopy();
  }
  
  
  TCanvas *c4 = new TCanvas();
  c4->Divide(3,4);
  
  for (int i = 12; i < 16; i++)
  {
  
	c4->cd(i*3 + 1 - 36);
	sprintf(fname,"VcalThresholdMap_C%iDistribution;1", i);
	((TH1D*)f->Get(fname))->DrawCopy();
	
	c4->cd(i*3 + 2 - 36);
	sprintf(fname,"VcalThresholdMap_C%iDistribution;7", i);
	((TH1D*)f->Get(fname))->DrawCopy();
	
	c4->cd(i*3 + 3 - 36);
	sprintf(fname,"CalThresholdMap_C%iDistribution;1", i);
	((TH1D*)f->Get(fname))->DrawCopy();
  }

}