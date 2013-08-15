void bla(const char *fileName = "times.txt") {
  
  //  /export/data3/moduleDB/M0109-060705.16:56: 9.36666666666667


  TH1D *h1 = new TH1D("h1", "Test durations ", 100, 0., 5.);
  setTitles(h1, "t [hours]", "modules");

  TCanvas *c0 = new TCanvas("c0", "", 500, 400);
  c0->Clear();


  char buffer[200];
  char bla[200];
  ifstream is(fileName);
  float time(0.);

  while (is.getline(buffer, 200, '\n')) {
    time = -1; 
    sscanf(buffer, "%f: %s", &time, &bla);
    cout << Form("time: %4.3f bla: %s", time, bla) << endl;
    h1->Fill(time);
  }

  setStyle();
  gStyle->SetOptStat(1111);
  shrinkPad(0.18, 0.15);
  setFilledHist(h1);
  h1->Draw();

  c0->SaveAs("testDurations.pdf");
  c0->SaveAs("testDurations.ps");
}

//-----------------------------------------------------------------------------------------
void setStyle() {

  gROOT->SetStyle("Plain");
  gStyle->SetPalette(1);
  gStyle->SetOptStat(0);
  gStyle->SetOptTitle(0);
  gStyle->SetTitle(0);

  gStyle->SetStatFont(132);
  gStyle->SetTextFont(132);
  gStyle->SetLabelFont(132, "X");
  gStyle->SetLabelFont(132, "Y");
  gStyle->SetLabelOffset(0.02, "X");
  gStyle->SetLabelOffset(0.02, "Y");
  gStyle->SetTitleFont(132);

  gROOT->ForceStyle();

}
// ----------------------------------------------------------------------
void setHist(TH1 *h, Int_t color = 1, Int_t symbol = 20, Double_t size = 1.6, Double_t width = 1.) {
  h->SetLineColor(color);   h->SetLineWidth(width);
  h->SetMarkerColor(color); h->SetMarkerStyle(symbol);  h->SetMarkerSize(size);
  h->SetStats(kFALSE);
  h->SetFillStyle(0); h->SetFillColor(color);
}


// ----------------------------------------------------------------------
void setTitles(TH1 *h, const char *sx, const char *sy, float size=0.06,
               float xoff=-2.0, float yoff=-2.0, float lsize=0.06, int font=132) {
  if (h == 0) {
    cout << " Histogram not defined" << endl;
  } else {
    h->SetXTitle(sx);                  h->SetYTitle(sy);
    h->SetTitleOffset(xoff, "x");      h->SetTitleOffset(yoff, "y");
    h->SetTitleSize(size, "x");        h->SetTitleSize(size, "y");
    h->SetLabelSize(lsize, "x");       h->SetLabelSize(lsize, "y");
    h->SetLabelFont(font, "x");        h->SetLabelFont(font, "y");
    h->GetXaxis()->SetTitleFont(font); h->GetYaxis()->SetTitleFont(font);
    h->SetNdivisions(508, "X");
  }
}

// ----------------------------------------------------------------------
void setFilledHist(TH1 *h, Int_t color=kBlack, Int_t fillcolor=kYellow, Int_t fillstyle=1000, Int_t width=1) {
  // Note: 3004, 3005 are crosshatches
  // ----- 1000       is solid
  //       kYellow    comes out gray on bw printers
  h->SetLineColor(color);     h->SetLineWidth(width);
  h->SetFillStyle(fillstyle); h->SetFillColor(fillcolor);
}

// ----------------------------------------------------------------------
void shrinkPad(double b, double l, double r=0.1, double t=0.1) {
  gPad->SetBottomMargin(b);
  gPad->SetLeftMargin(l);
  gPad->SetRightMargin(r);
  gPad->SetTopMargin(t);
}



