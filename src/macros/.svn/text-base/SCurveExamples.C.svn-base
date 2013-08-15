
// TFile f("...")
// SCurve a("SCurve_c50r73", "errf", 1)


// ----------------------------------------------------------------------
// Open a file beforehand!
void fitAccuracy() {

  SCurve *a;

  TH1D *hThr = new TH1D("hThr", "", 100, 0., 0.2);
  TH1D *hSig = new TH1D("hSig", "", 100, 0., 1.0);
  TH1D *hChi = new TH1D("hChi", "", 100, 0., 1.0);

  double par, parE;

  for (int ix = 0; ix < 52; ++ix) {
    cout << "column " << ix << endl;
    for (int iy = 0; iy < 80; ++iy) {

      a = (SCurve*)gFile->Get(Form("SCurve_c%dr%d", ix, iy));
      if (a) {
	// -- Threshold
	par  = a->GetFunction()->GetParameter(1);
	parE = a->GetFunction()->GetParError(1);
	if (par > 0.) {
	  hThr->Fill(parE/par);
	} else {
	  cout << "Thr at zero" << endl;
	}

	// -- Sigma
	par  = 1./(1.4142 * a->GetFunction()->GetParameter(2));
	parE = 1.4142 * par * par * a->GetFunction()->GetParError(2);
	if (par > 0.) {
	  hSig->Fill(parE/par);
	} else {
	  cout << "Sig at zero" << endl;
	}

      }
      
    }
  }
  
  
  // -- Define the following somewhere beforehand:
  // TCanvas c0("c0","--c0--",615,0,656,700);
  c0.Clear();
  c0.Divide(2,2);
  c0.cd(1); gPad->SetLogy(1);
  hThr->Draw();

  c0.cd(2); gPad->SetLogy(1);
  hSig->Draw();
  
}
