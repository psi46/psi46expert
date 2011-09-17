#ifndef ANAQA_h
#define ANAQA_h

class TH1D;
class TLatex;

// ----------------------------------------------------------------------
class anaQA {
  
  public :
    anaQA(const char *rootfile = 0, int verbose = 0);
  ~anaQA();
  
  // -- Main analysis methods 
  void makeAll();

  void diffDAC(const char *baseDir);
  void drawDiff(const char *module, TString dirNames[], int ndir, TString parNames[], int npar);
  void getDiff(const char *dacParameter, int chipId, const char *dirN, const char *refN, TH1D *h);
  int  findParameter(const char *dir, const char *dacPar, int chipId);



  // -- Utilities
  void formatHisto(TH1D *h);
  void setStyle();

 private: 
  TLatex *ts;

};

#endif

