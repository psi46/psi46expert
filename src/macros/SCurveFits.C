
// ----------------------------------------------------------------------
// invoke sCurveFits() for all files in current directory
void allFiles(const char *wildcard = ".root", const char *dir = ".") {

  TList *lFiles = new TList();
  TString fname;
  char *file;
  TObjString *ofilename;
  TObject *pO;
  TFile *pFile;

  void *pDir = gSystem->OpenDirectory(dir);
  while (file = gSystem->GetDirEntry(pDir)) {
    fname = file;
    if (
	fname.Contains(".root") 
	&& fname.Contains(wildcard)
	&& !fname.Contains(".fittedScurves.root")
	) {
      ofilename = new TObjString(); ofilename->SetString((char*)fname);
      lFiles->Add((TObject*)ofilename);
    }
  }
  

  TIter ilFiles(lFiles);
  while (pO = ilFiles()) {
    cout << "Running sCurveFits() on file " << pO->GetName() << endl;
    sCurveFits(pO->GetName());
  }

}




// ----------------------------------------------------------------------
// Open "file" with scurve TH1D, convert them into sCurve objects and 
// save into a new file
void sCurveFits(const char *file, const char *newfile = "") {
  TString newFile;

  if (!strcmp(newfile, "")) {
    newFile = TString(file);
    newFile.ReplaceAll(".root", ".fittedScurves.root");
  }

  // -- Output file
  TFile g(newFile.Data(), "RECREATE");
  // -- Input file (needs to be gDirectory!)
  TFile f(file);

  // -- Loop over all keys in file
  TObject *key;
  TIter next(f.GetListOfKeys());
  sCurve *a;
  TString aname;
  TH1D *h1;
  while ((key = (TObject*)next())) {
    aname = TString(key->GetName());
    // -- Get TH1D's that are sCurves
    if (aname.Contains("SCurve_")) {
      h1 = (TH1D*)f.Get(key->GetName());
      cout << "key->GetName() "  <<  key->GetName() << "  " << aname.Data() << endl;
      // -- Instantiate sCurve fit object
      f.cd();
      a = new sCurve(aname.Data(), "errf", 1);
      // -- Dump to output file
      g.cd();
      a->Write(aname.Data());
    }
  }

  g.Write();
  g.Close();
}
