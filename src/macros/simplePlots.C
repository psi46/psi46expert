// ----------------------------------------------------------------------
void makeAll() {

  noise();

  for (int i = 400; i < 800; i +=100) {
    nmod(i); 
  }

  for (int i = 4000; i < 4300; i +=100) {
    dac(i); 
  }

}


// ----------------------------------------------------------------------
void defects() {

  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gROOT->ForceStyle();
  c0.Clear();
  e100->SetLineWidth(2);
  e100->SetMinimum(0.5); 
  e100->SetAxisRange(-1.0, 14.0); 

  e100->Draw();
  gPad->SetLogy(1);
}

// ----------------------------------------------------------------------
void nmod(const int offset = 400) {
  
  c0.Clear();
  c0.Divide(2,8);

  tl->SetTextSize(0.2);
  tl->SetNDC(kTRUE);
  tl->SetTextAngle(90); 
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gROOT->ForceStyle();

  double min(-1), max(-1);
  if (offset == 600) {
    min = 120; 
    max = 200;
  }


  TProfile *p;
  for (int i = 0; i < 16; ++i) {
    c0.cd(i+1);
    p = (TProfile*)gFile->Get(Form("p%i", offset+i));
    if (min > -1) {
      p->SetMinimum(min);
      p->SetMaximum(max);
    }
    p->Draw();
    
    tl->DrawLatex(0.2, 0.11, Form("Chip %i", i)); 
  }

  if (offset == 400) c0.SaveAs("ped-nmod.ps");
  if (offset == 500) c0.SaveAs("gain-nmod.ps");
  if (offset == 600) c0.SaveAs("noise-nmod.ps");
  if (offset == 700) c0.SaveAs("pedgain-nmod.ps");

  
}


// ----------------------------------------------------------------------
void dac(const int offset = 4000) {
  
  c0.Clear();
  c0.Divide(2,8);

  tl->SetTextSize(0.2);
  tl->SetNDC(kTRUE);
  tl->SetTextAngle(90); 
  gStyle->SetOptTitle(0);
  gStyle->SetOptStat(0);
  gROOT->ForceStyle();

  TH1D *p;
  for (int i = 0; i < 16; ++i) {
    c0.cd(i+1);
    p = (TH1D*)gFile->Get(Form("p%i", offset+i));
    p->Draw("p");
    
    tl->DrawLatex(0.2, 0.11, Form("Chip %i", i)); 
  }

  if (offset == 4000) c0.SaveAs("voffsetop-nmod.ps");
  if (offset == 4100) c0.SaveAs("vana-nmod.ps");
  if (offset == 4200) c0.SaveAs("ibias_dac-nmod.ps");

  
}


// ----------------------------------------------------------------------
void noise() {
  c0.Clear();
  c0.Divide(1,4);

  c0.cd(1);
  H200->Draw("colz");


  c0.cd(2);
  H201->Draw("colz");

  c0.cd(3);
  h201->Draw();

  c0.cd(4);
  h202->Draw();

  c0.SaveAs("noise-1.ps");

}


// ----------------------------------------------------------------------
void ped() {
  c0.Clear();
  c0.Divide(1,2);

  c0.cd(1);
  P400->Draw("colz");

  c0.cd(2);
  p400->Draw("colz");

}


// ----------------------------------------------------------------------
void sarah(const int ichip = 0, const char *option = "") {
  if (!strcmp(option, "same")) {
  } else {
    c0.Clear();
    c0.Divide(2,2);
  }

  c0.cd(1);
  gPad->SetLogy(1);
  ((TH1D*)gFile->Get(Form("h%i", 1100+ichip)))->Draw(option);

  c0.cd(2);
  gPad->SetLogy(1);
  ((TH1D*)gFile->Get(Form("h%i", 1000+ichip)))->Draw(option);

  c0.cd(3);
  gPad->SetLogy(1);
  ((TH1D*)gFile->Get(Form("h%i", 1200+ichip)))->Draw(option);

  c0.cd(4);
  gPad->SetLogy(1);
  ((TH1D*)gFile->Get(Form("h%i", 1300+ichip)))->Draw(option);

}
