#ifndef ANATESTRESULTS
#define ANATESTRESULTS

#include <vector>

#include <TROOT.h>
#include <TChain.h>
#include <TFile.h>
#include "TF1.h"
#include "TGraph.h"



struct triplet {
  int chip;
  int col, row;
};


class anaTestResults {
public :

  anaTestResults(TTree *tree=0);
  ~anaTestResults();

  void Loop();
  void openResults();
  void closeResults();
  void bookHistograms(const char *dir, const char *temp);
  void fillHistograms();
  void summary();
  double FitPol2();
  double FitTanH();
  void SetAbTest(bool abTest);
  void setHalfModulesA();
  void setHalfModulesB();
  void setFullModules();
  void FillVtrimUthrWidths(int iChip);


private:

  FILE *resultsFile;
  TFile *fFile;
  TGraph *graph;
  TF1 *phFit, *tanFit;

  Int_t    Cut(Long64_t entry);
  Int_t    GetEntry(Long64_t entry);
  Long64_t LoadTree(Long64_t entry);
  void     Init(TTree *tree);
  Bool_t   Notify();
  void     Show(Long64_t entry = -1);

  std::vector<triplet> dead, noisy, mask, bb, al, tb1, tb2, tb3, tb4;

  double fMeanTrim;
  int    fNmod, fNchip;

  double x[10], y[10], xErr[10], yErr[10];
  int n;

  static const int nFitParams = 6;
  static const int maxModNr = 600;

  bool abCheck;

  int NCHP, FCHP, LCHP; // number of chips, first chip, last chip



  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain

  // Declaration of leave types
  Int_t           nmod;
  Int_t           grade;
  Float_t         iv100, iv150, ivSlope, ivi[150], ivv[150];
  Int_t           trims[16][4160];
  Float_t         uthrM[16];
  Float_t         uthrR[16];
  Int_t           uthr[16][4160];
  Float_t         tthrM[16];
  Float_t         tthrR[16];
  Int_t           tthr[16][4160];
  Int_t           bumpThrDiff[16][4160];
  Float_t         noiseM[16];
  Float_t         noiseR[16];
  Float_t         noise[16][4160];
  Float_t         gainM[16];
  Float_t         gainR[16];
  Float_t         pedM[16];
  Float_t         pedR[16];
  Float_t         ph[16][4160][10];
  Float_t         tanhFit[16][4160][4];
  Float_t         gain[16][4160];
  Float_t         ped[16][4160];
  Float_t         addlvl[16][4160];
  Float_t         pixmap[16][4160];
  Int_t           defects[16][4160];
  Int_t           vdig[16];
  Int_t           vana[16];
  Int_t           vsf[16];
  Int_t           vcomp[16];
  Int_t           vleak_comp[16];
  Int_t           vrgpr[16];
  Int_t           vwllpr[16];
  Int_t           vrgsh[16];
  Int_t           vwllsh[16];
  Int_t           vhlddel[16];
  Int_t           vtrim[16];
  Int_t           vthrcomp[16];
  Int_t           vibias_bus[16];
  Int_t           vbias_sf[16];
  Int_t           voffsetop[16];
  Int_t           vibiasop[16];
  Int_t           voffsetr0[16];
  Int_t           vion[16];
  Int_t           vibias_ph[16];
  Int_t           ibias_dac[16];
  Int_t           vibias_roc[16];
  Int_t           vicolor[16];
  Int_t           vnpix[16];
  Int_t           vsumcol[16];
  Int_t           vcal[16];
  Int_t           caldel[16];
  Int_t           rangetemp[16];
  Int_t           ctrlreg[16];
  Int_t           wbc[16];

  // List of branches
  TBranch        *b_nmod;   //!
  TBranch        *b_grade;   //!
  TBranch        *b_iv100, *b_iv150, *b_ivSlope, *b_ivv, *b_ivi;   //!
  TBranch        *b_trims;   //!
  TBranch        *b_uthrM;   //!
  TBranch        *b_uthrR;   //!
  TBranch        *b_uthr;   //!
  TBranch        *b_tthrM;   //!
  TBranch        *b_tthrR;   //!
  TBranch        *b_tthr;   //!
  TBranch        *b_bumpThrDiff;   //!
  TBranch        *b_noiseM;   //!
  TBranch        *b_noiseR;   //!
  TBranch        *b_noise;   //!
  TBranch        *b_gainM;   //!
  TBranch        *b_gainR;   //!
  TBranch        *b_pedM;   //!
  TBranch        *b_pedR;   //!
  TBranch        *b_ph;   //!
  TBranch        *b_tanhFit;   //!
  TBranch        *b_gain;   //!
  TBranch        *b_ped;   //!
  TBranch        *b_addlvl;   //!
  TBranch        *b_pixmap;   //!
  TBranch        *b_defects;   //!
  TBranch        *b_vdig;   //!
  TBranch        *b_vana;   //!
  TBranch        *b_vsf;   //!
  TBranch        *b_vcomp;   //!
  TBranch        *b_vleak_comp;   //!
  TBranch        *b_vrgpr;   //!
  TBranch        *b_vwllpr;   //!
  TBranch        *b_vrgsh;   //!
  TBranch        *b_vwllsh;   //!
  TBranch        *b_vhlddel;   //!
  TBranch        *b_vtrim;   //!
  TBranch        *b_vthrcomp;   //!
  TBranch        *b_vibias_bus;   //!
  TBranch        *b_vbias_sf;   //!
  TBranch        *b_voffsetop;   //!
  TBranch        *b_vibiasop;   //!
  TBranch        *b_voffsetr0;   //!
  TBranch        *b_vion;   //!
  TBranch        *b_vibias_ph;   //!
  TBranch        *b_ibias_dac;   //!
  TBranch        *b_vibias_roc;   //!
  TBranch        *b_vicolor;   //!
  TBranch        *b_vnpix;   //!
  TBranch        *b_vsumcol;   //!
  TBranch        *b_vcal;   //!
  TBranch        *b_caldel;   //!
  TBranch        *b_rangetemp;   //!
  TBranch        *b_ctrlreg;   //!
  TBranch        *b_wbc;   //!

};

#endif

