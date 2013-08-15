

void Print(const char *histname = "Timewalk_of_c5r5_C0") {

  Init();
  TFile *f = new TFile("../psi46expert/testModule/Timewalk.root");
  TH2D *histo;

  histo = (TH2D*)(f->Get(histname));
  histo->GetXaxis()->SetTitleSize(0.055);
  histo->GetYaxis()->SetTitleSize(0.055);
  histo->GetZaxis()->SetTitleSize(0.055);
  histo->GetXaxis()->SetLabelSize(0.04);
  histo->GetYaxis()->SetLabelSize(0.04);
  histo->GetZaxis()->SetLabelSize(0.05);
  histo->GetXaxis()->SetTitleOffset(1.15);
  histo->GetYaxis()->SetTitleOffset(1.1);
  histo->GetZaxis()->SetTitleOffset(0.8);
  histo->GetZaxis()->SetRangeUser(0,50);
  histo->SetTitle("");
  histo->SetLineStyle(1);
  histo->SetLineWidth(2);
  histo->GetXaxis()->SetTitle("VrgPr=VrgSh [DAC units]");
  histo->GetYaxis()->SetTitle("VwllPr=VwllSh [DAC units]");

  histo->Draw("colz");

}
