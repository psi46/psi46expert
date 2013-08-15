
void pixelHistos(const char *filenames = "/export/data1/dambach/hardware/ntpls/gradeC/*.root", const char *cuts =  "defects == 0")
{

  Init();      // do not forget to load the Utilities.C

  TH1D *gainH = new TH1D("gainH", "Gain", 1000, 0., 10.);
  TH1D *pedH = new TH1D("pedH", "Pedestal", 1000, -1000., 1000.);
  TH1D *noiseH = new TH1D("noiseH", "Noise", 1100, -100., 1000.);
  TH1D *tthrH = new TH1D("tthrH", "Trimmed Threshold", 100, 0., 100.);
  
  TChain *c = new TChain("mod");
  c->Add(filenames);
  
  canvas->Clear();
  canvas->Divide(2,2);

  canvas->cd(1);
  InitPad("log");
  gainH->GetXaxis()->SetTitle("ADC units/DAC units");
  gainH->GetYaxis()->SetTitle("# pixels");
  gainH->GetXaxis()->SetTitleSize(0.055);
  gainH->GetYaxis()->SetTitleSize(0.055);
  c->Draw("gain>>gainH", cuts);

  TBox *box = new TBox();

  canvas->cd(2);
  InitPad("log");
  pedH->GetXaxis()->SetTitle("DAC units");
  pedH->GetYaxis()->SetTitle("# pixels");
  pedH->GetXaxis()->SetTitleSize(0.055);
  pedH->GetYaxis()->SetTitleSize(0.055);
  c->Draw("ped>>pedH", cuts);

  canvas->cd(3);
  InitPad("log");
  noiseH->GetXaxis()->SetTitle("e^{-}");
  noiseH->GetYaxis()->SetTitle("# pixels");
  noiseH->GetYaxis()->SetRangeUser(0.1,400000);
  noiseH->GetXaxis()->SetTitleSize(0.055);
  noiseH->GetYaxis()->SetTitleSize(0.055);
  c->Draw("noise>>noiseH", cuts);
  noiseH->Fit("gaus");
//   box->SetFillStyle(3003);
//   box->SetFillColor(kBlue);
//   box->DrawBox(0.1,0.,500.,400000.);
//   box->SetFillColor(kRed);
//   box->DrawBox(500.,0.,1000.,400000.);

  canvas->cd(4);
  tthrH->GetXaxis()->SetTitle("DAC units");
  tthrH->GetYaxis()->SetTitle("# pixels");
  tthrH->GetXaxis()->SetTitleSize(0.055);
  tthrH->GetYaxis()->SetTitleSize(0.055);
  InitPad("log");
  c->Draw("tthr>>tthrH", cuts);

}
