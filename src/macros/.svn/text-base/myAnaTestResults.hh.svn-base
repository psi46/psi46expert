#ifndef MYANATESTRESULTS
#define MYANATESTRESULTS

#include <vector>

#include "TROOT.h"
#include "TChain.h"
#include "TFile.h"
#include "TF1.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TH2D.h"



struct triplet {
  int chip;
  int col, row;
};


class MyAnaTestResults {
public :

  MyAnaTestResults(TTree *tree=0);
  ~MyAnaTestResults();

  void Loop();
  void Summary();
  void Init(const char *dir, const char *temp, int grades);
  void Analyze();

  void TBMErrors();
  void DACHistos();
  void MaskDefects();
  void Bumpbonding();
  void TrimBitsTest();
  void Trimming();
  void Noise();
  void IV();
  void Defects();
  void PHCalibration();
  void TempVoltage();
  void Grading();
  void CurrentsVoltages();
  int BReason();

  void ModuleTable();
  void ReadModuleList();

  void setHalfModulesA();
  void setHalfModulesB();
  void setFullModules();
  int GetHalfModule();

  char *gradeLetter(int aGrade);
  const char *PrintNumber(Double_t number, Int_t nDigis, bool noExp = false);
  double GetTempVoltage(int i);
  int GetModuleNr();

  Int_t    GetEntry(Long64_t entry);
  Long64_t LoadTree(Long64_t entry);


private:

  FILE *resultsFile, *moduleTableFile;
  TFile *fFile;
  TGraph *graph;
  TF1 *phFit, *tanFit;
  TString tempNoSlash;

  Int_t    Cut(Long64_t entry);

  void     Init(TTree *tree);
  Bool_t   Notify();
  void     Show(Long64_t entry = -1);

  int nModules, nHalfModulesA, nHalfModulesB, nA, nB, nC, allGrades[2000], allOverallGrades[2000], allFinalGrades[2000], allShortGrades[2000], allDates[2000], allAllTestsDone[2000], moduleList[2000], gradeOption;
  int nGoodBumps, nBadBumps, nAllPixels, nDeadPixels, nTrim1Defects, nTrim2Defects, nTrim3Defects, nTrim4Defects, nAddressProblems;

  double x[10], y[10], xErr[10], yErr[10];
  double gainRMSDC, pedRMSDC;
  int nPHDC;

  static const int nFitParams = 6;
  static const double vcalConversion = 65.;

  int NCHP, FCHP, LCHP; // number of chips, first chip, last chip
  
  TH1F *hCaldel;
  TH1F *hBumpbonding;
  TH2F *hBumpbondingM0086;
  TH1F *hTrimBitDefects;
  TH1F *hTrimMean, *hTrimRMS, *hTrimmedThreshold, *hUntrimmedThreshold, *hTrimBits;
  TH1F *hIV150, *hIVSlope;
  TH1F *hNoise, *hNoiseEdge, *hNoiseCorner;
  TH1F *hDefects;
  TH1F *hGainMean, *hPedestalMean, *hGainRMS, *hPedestalRMS, *hGain, *hPedestal, *hPar1, *hRelGainSpread;
  TH1F *hTempVoltage;
  TH1F *hCFailures, *hBFailures;
  TH1F *hIA, *hID, *hVA, *hVD;
  
  TTree          *fChain;   //!pointer to the analyzed TTree or TChain
  Int_t           fCurrent; //!current Tree number in a TChain
  
  // Declaration of leave types
  Int_t           moduleNr;
  Int_t           grade, finalGrade, overallGrade, shortGrade;
  Int_t           halfModule;
  Int_t           allTestsDone;
  Int_t           tbmError1, tbmError2;
  Int_t           date, hour, minute;
  Int_t           nRocsB[20], nRocsC[20];
  Float_t         iv100, iv150, ivSlope, ivi[150], ivv[150];
  Float_t         ia, id, va, vd;
  Float_t         temperatureVoltage[16];
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
  TBranch        *b_grade, *b_finalGrade, *b_overallGrade, *b_shortGrade;   //!
  TBranch        *b_halfModule;   //!
  TBranch        *b_allTestsDone;   //!
  TBranch        *b_date;   //!
  TBranch        *b_hour;   //!
  TBranch        *b_minute;   //!
  TBranch        *b_tbmError1;   //!
  TBranch        *b_tbmError2;   //!
  TBranch        *b_ia;   //!
  TBranch        *b_id;   //!
  TBranch        *b_va;   //!
  TBranch        *b_vd;   //!
  TBranch        *b_iv100, *b_iv150, *b_ivSlope, *b_ivv, *b_ivi;   //!
  TBranch        *b_temperatureVoltage;
  TBranch        *b_nRocsB, *b_nRocsC;
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

