void LinVsf()
{
 Init();
 TFile *f = new TFile("../psi46expert/testModule/LinVsf.root");

 TH1F *h = ((TH1F*)f->Get("hist3_ROC0"))->Clone();

 h->GetXaxis()->SetTitle("Vsf [DAC units]");
 h->GetYaxis()->SetTitle("p_{1}");
 h->SetTitle("");
 h->GetXaxis()->SetTitleSize(0.055);
 h->GetYaxis()->SetTitleSize(0.055);
 h->GetXaxis()->SetLabelSize(0.05);
 h->GetYaxis()->SetLabelSize(0.05);
 h->GetXaxis()->SetTitleOffset(1.15);
 h->GetYaxis()->SetTitleOffset(0.8);
 h->SetLineWidth(2);


 // DrawHisto(h);
  h->Draw();

} 
