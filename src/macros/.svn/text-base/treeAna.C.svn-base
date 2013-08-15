void plot() {

  TH2D *h2 = new TH2D("h2", "tthr", 52, 0., 52., 80, 0., 80.);

  TFile *f = TFile::Open("/home/ursl/tmp/moduleDB/module-T-10a-0240.root");
  
  TTree *t = (TTree*)f->Get("mod");

  int tthr[16][4160];
  t->SetBranchAddress("tthr", tthr);

  int nb(0), nbytes(0);

  for (int i = 0; i < t->GetEntries(); ++i) {
    nb = t->GetEntry(i);   nbytes += nb;
    
    for (int j = 0; j < 4160; ++j) {

      icol = j/80;
      irow = j%80;

      h2->SetBinContent(icol+1, irow+1, tthr[0][j]);

    }
  }

  h2->Draw("colz");
}
