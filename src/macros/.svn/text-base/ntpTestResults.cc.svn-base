#include <stdio.h>
#include <iostream.h>
#include <fstream.h>
#include <string.h>
#include <dirent.h>

#include <vector>

#include "TSystem.h"
#include "TDirectory.h"
#include "TString.h"
#include "TFile.h"
#include "TH1.h"
#include "TH2.h"
#include "TTree.h"
#include "TParameter.h"
#include "TGraph.h"
#include "TF1.h"

using namespace::std;


class ntpTestResults {

public:
  ntpTestResults(const char *baseDir, const char *temperature = "T-10a", int force = 0);
  ~ntpTestResults();
  void setupNtuple();
  void fillNtuple();

  void setHalfModulesA();
  void setHalfModulesB();
  void setFullModules();
  void fillMeansRMS();
  void fillNoise();
  void fillTrims();
  void fillGainPed();
  void fillTanhFits();
  void fillPulseHeights();
  void fillThresholds();
  void fillAddressLevels();
  void fillPixelDefects();
  void fillTBMErrors();
  void fillCurrents();
  void fillIV();
  void fillPHValidation();
  void fillTemperatureCalibration();

  int dac_findParameter(const char *dir, const char *dacPar, int chipId);
  void findHalfModules(const char *dir);
  double calcMean(TH2 *h);
  double calcRMS(TH2 *h, double mean);


  int isOK() {return fIsOK;}

  // ----------------------------------------------------------------------
  TString fBaseDir, fTemperature, fDirectory;
  int     fModule;
	bool    fShortTest;

  int     fIsOK;
  int     fForceOverwrite;

  
  TFile *fFullTest;
  TFile *fFile;
  TTree *fBpix;

  static const int NPIX = 4160;
  static const int MAXNCHP = 16;

  int NCHP, FCHP, LCHP; // number of chips, first chip, last chip

  // ----------------------------------------------------------------------
  // -- Vectors for histograms
  vector<TH2D*> vHuthr, vHtthr;


  // ----------------------------------------------------------------------
  // -- Arrays for tree 
  int  
    vdig[MAXNCHP],       
    vana[MAXNCHP],       
    vsf[MAXNCHP],        
    vcomp[MAXNCHP],
    vleak_comp[MAXNCHP],
    vrgpr[MAXNCHP],
    vwllpr[MAXNCHP],     
    vrgsh[MAXNCHP],      
    vwllsh[MAXNCHP],
    vhlddel[MAXNCHP],
    vtrim[MAXNCHP],
    vthrcomp[MAXNCHP],
    vibias_bus[MAXNCHP],
    vbias_sf[MAXNCHP],
    voffsetop[MAXNCHP],
    vibiasop[MAXNCHP],   
    voffsetr0[MAXNCHP],
    vion[MAXNCHP],
    vibias_ph[MAXNCHP],
    ibias_dac[MAXNCHP],
    vibias_roc[MAXNCHP], 
    vicolor[MAXNCHP],
    vnpix[MAXNCHP],
    vsumcol[MAXNCHP],    
    vcal[MAXNCHP],
    caldel[MAXNCHP],     
    rangetemp[MAXNCHP],  
    ctrlreg[MAXNCHP],
    wbc[MAXNCHP];

  // -- defects
  //    0 OK
  //    1 dead
  //    2 noisy (Beat's criterium)
  //    4 bump bonding
  //    8 address levels
  //   16 masking
  //   32 trim bits 1
  //   64 trim bits 2
  //  128 trim bits 3
  //  256 trim bits 4

  int defects[MAXNCHP][NPIX];

  int   nmod;
  int   grade, overallGrade, finalGrade, shortGrade; // A = 1, B = 2, C = 3
  int   allTestsDone;

  int halfModule;
  int date, hour, minute;
  int tbmError1, tbmError2;

  int nRocsB[20], nRocsC[20];

  int   trims[MAXNCHP][NPIX];
  float noiseM[MAXNCHP], noiseR[MAXNCHP];
  float uthrM[MAXNCHP],  uthrR[MAXNCHP];     // M = mean, r = RMS
  float tthrM[MAXNCHP],  tthrR[MAXNCHP];
  float noise[MAXNCHP][NPIX];
  int   tthr[MAXNCHP][NPIX];
  int   uthr[MAXNCHP][NPIX];
  int   bumpThrDiff[MAXNCHP][NPIX];

  float gainM[MAXNCHP], gainR[MAXNCHP], pedM[MAXNCHP], pedR[MAXNCHP];
  float ph[MAXNCHP][NPIX][10];
  float tanhFit[MAXNCHP][NPIX][4];
  float gain[MAXNCHP][NPIX];
  float ped[MAXNCHP][NPIX];
  float addlvl[MAXNCHP][NPIX];
  float pixmap[MAXNCHP][NPIX];
  float phValidation[MAXNCHP][10];

  float iv150, iv100, ivSlope;
  float ivv[150], ivi[150];
  float ia, id, va, vd;

  float temperatureVoltage[MAXNCHP];
};




// ----------------------------------------------------------------------
ntpTestResults::ntpTestResults(const char *baseDir, const char *temperature, int force) {

  fShortTest = false;
  fIsOK = 0;
  fForceOverwrite = force;

  // -- remove path until last '/'
  TString filename(baseDir);
  int index = filename.Last('/');
  filename.Remove(0, index+1);

  sscanf((char*)filename.Data(), "M%d-%d.%d:%d", &fModule, &date, &hour, &minute);

  fBaseDir     = TString(baseDir);
  fTemperature = TString(temperature);
  fDirectory   = fBaseDir + TString(Form("/%s", fTemperature.Data()));
  if (opendir(fDirectory.Data())) {
    cout << "Found directory " << fDirectory.Data() << endl;
  } else {
    cout << "Error: Directory " << fDirectory.Data() << " not found" << endl;
    return;
  }


  fFullTest = new TFile(Form("%s/%s/FullTest.root", fBaseDir.Data(), fTemperature.Data()));
  if (!fFullTest->IsOpen())
	{
	  fFullTest    = new TFile(Form("%s/%s/ShortTest.root", fBaseDir.Data(), fTemperature.Data()));
		printf("ShortTest\n");
		fShortTest = true;
  }
  if (fFullTest) setupNtuple();
	else
  {
    allTestsDone = 0;
    cout << "Error: No file " << Form("%s/%s/FullTest.root",  fBaseDir.Data(), fTemperature.Data()) << "or " << Form("%s/%s/ShortTest.root",  fBaseDir.Data(), fTemperature.Data()) << endl;
  }
}


// ----------------------------------------------------------------------
ntpTestResults::~ntpTestResults() {

  if (fIsOK) {
    fBpix->Write();
    fFile->Write();
    fFile->Close();
    //   delete fBpix;
    delete fFile;

    if (fFullTest)
    {
      fFullTest->Close();
      delete fFullTest;
    }
  }
}


//-----------------------------------------------------------------------------------------
// see macros/moduleSummaryPage.C
double ntpTestResults::calcMean(TH2 *h) {

  double mean(0.);
  for (int m = 0; m < h->GetNbinsX(); m++) {
    for (int n = 0; n < h->GetNbinsY(); n++) {
      mean += h->GetBinContent(m+1, n+1);
    }
  }
  mean /= (h->GetNbinsX()*h->GetNbinsY());

  return mean;
}



// ----------------------------------------------------------------------
void ntpTestResults::setHalfModulesA()
{
  cout << "Using half modules typ A " << endl;
  FCHP = 0;
  LCHP = 8;
  NCHP = LCHP - FCHP;
  halfModule = 1;
}


// ----------------------------------------------------------------------
void ntpTestResults::setHalfModulesB()
{
  cout << "Using half modules typ B " << endl;
  FCHP = 8;
  LCHP = 16;
  NCHP = LCHP - FCHP;
  halfModule = 2;
}


// ----------------------------------------------------------------------
void ntpTestResults::setFullModules()
{
  cout << "Using full modules " << endl;

  FCHP = 0;
  LCHP = 16;
  NCHP = LCHP - FCHP;
  halfModule = 0;
}



//-----------------------------------------------------------------------------------------
// see macros/moduleSummaryPage.C
double ntpTestResults::calcRMS(TH2 *h, double mean) {

  double rms(0.);
  for (int m = 0; m < h->GetNbinsX(); m++) {
    for (int n = 0; n < h->GetNbinsY(); n++) {
      rms += (mean - h->GetBinContent(m+1, n+1))*(mean - h->GetBinContent(m+1, n+1));
    }
  }

  rms = TMath::Sqrt(rms / (h->GetNbinsX()*h->GetNbinsY()));

  return rms;

}


// ----------------------------------------------------------------------
void ntpTestResults::fillNtuple() {

  cout << "Fill ntuple " << fBaseDir.Data() << "/" << fTemperature.Data()
       << " in directory " << fDirectory.Data()
       << endl;

  nmod = fModule;

  if (!fShortTest)
	{
    fillThresholds();
    fillMeansRMS();
    fillNoise();
    fillTrims();
    fillTBMErrors();
    fillIV();
    fillTemperatureCalibration();
  }
  else
  {
    fillPHValidation();
  }

  fillGainPed();
  fillTanhFits();
  fillPulseHeights();
  fillAddressLevels();
  fillPixelDefects();
  fillCurrents();

  // -- Grading
  char line[200], buffer[200], sgrade[10], sOverallGrade[10], sFinalGrade[10], sShortGrade[10];
  sprintf(line, "%s/summaryTest.txt", fDirectory.Data());
  ifstream is(line);
  int tmp;
  while (is.getline(buffer, 200, '\n')) {
    if (strstr(buffer, "OVERALL GRADE")) {
      sscanf(buffer, "OVERALL GRADE %s", sOverallGrade);
    }
    if (strstr(buffer, "FINAL DB-GRADE")) {
      sscanf(buffer, "FINAL DB-GRADE %s", sFinalGrade);
      cout << "Grade: " << sFinalGrade << endl;
    }
    if (strstr(buffer, "Grade")) {
      sscanf(buffer, "Grade %s", sgrade);
    }
    if (strstr(buffer, "shortTest GRADE")) {
      sscanf(buffer, "shortTest GRADE %s", sShortGrade);
    }

    if (strstr(buffer, "ROCS with defects > 1%")) {
      sscanf(buffer + 22, "%i %i %i", &tmp, &nRocsB[0], &nRocsC[0]);
    }

    if (strstr(buffer, "DEAD")) {
      sscanf(buffer, "DEAD %i %i", &nRocsB[1], &nRocsC[1]);
    }
    if (strstr(buffer, "MASK")) {
      sscanf(buffer, "MASK %i %i", &nRocsB[2], &nRocsC[2]);
    }
    if (strstr(buffer, "BUMPS")) {
      sscanf(buffer, "BUMPS %i %i", &nRocsB[3], &nRocsC[3]);
    }
    if (strstr(buffer, "TRIMBITS")) {
      sscanf(buffer, "TRIMBITS %i %i", &nRocsB[4], &nRocsC[4]);
    }
    if (strstr(buffer, "ADDRESS")) {
      sscanf(buffer, "ADDRESS %i %i", &nRocsB[5], &nRocsC[5]);
    }
    if (strstr(buffer, "Noise")) {
      sscanf(buffer, "Noise %i %i", &nRocsB[6], &nRocsC[6]);
    }
    if (strstr(buffer, "VcalThrWidth")) {
      sscanf(buffer, "VcalThrWidth %i %i", &nRocsB[7], &nRocsC[7]);
    }
    if (strstr(buffer, "RelGainWidth")) {
      sscanf(buffer, "RelGainWidth %i %i", &nRocsB[8], &nRocsC[8]);
    }
    if (strstr(buffer, "PedSpread")) {
      sscanf(buffer, "PedSpread %i %i", &nRocsB[9], &nRocsC[9]);
    }
    if (strstr(buffer, "I150V")) {
      sscanf(buffer, "I150V %i %i", &nRocsB[10], &nRocsC[10]);
    }
    if (strstr(buffer, "Iratio")) {
      sscanf(buffer, "Iratio %i %i", &nRocsB[11], &nRocsC[11]);
    }
    if (strstr(buffer, "MeanParameter1")) {
      sscanf(buffer, "MeanParameter1 %i %i", &nRocsB[12], &nRocsC[12]);
    }
  }
  is.close();

  grade = 3; finalGrade = 3; overallGrade = 3; shortGrade = 3;

  if (!strcmp(sgrade, "A")) grade = 1;
  if (!strcmp(sgrade, "B")) grade = 2;

  if (!strcmp(sFinalGrade, "A")) finalGrade = 1;
  if (!strcmp(sFinalGrade, "B")) finalGrade = 2;

  if (!strcmp(sOverallGrade, "A")) overallGrade = 1;
  if (!strcmp(sOverallGrade, "B")) overallGrade = 2;

  if (!strcmp(sShortGrade, "A")) shortGrade = 1;
  if (!strcmp(sShortGrade, "B")) shortGrade = 2;

  if (!strcmp(sFinalGrade, "TBD")) finalGrade = overallGrade;
  if (!strcmp(sFinalGrade, "tbd")) finalGrade = overallGrade;

  // -- DAC parameters
  for (int iChip = FCHP; iChip < LCHP; ++iChip) {
    vdig[iChip] = dac_findParameter(fDirectory.Data(), "Vdig", iChip);
    vana[iChip] = dac_findParameter(fDirectory.Data(), "Vana", iChip);
    vsf[iChip] = dac_findParameter(fDirectory.Data(), "Vsf", iChip);
    vcomp[iChip] = dac_findParameter(fDirectory.Data(), "Vcomp", iChip);
    vleak_comp[iChip] = dac_findParameter(fDirectory.Data(), "Vleak_comp", iChip);
    vrgpr[iChip] = dac_findParameter(fDirectory.Data(), "VrgPr", iChip);
    vwllpr[iChip] = dac_findParameter(fDirectory.Data(), "VwllPr", iChip);
    vrgsh[iChip] = dac_findParameter(fDirectory.Data(), "VrgSh", iChip);
    vwllsh[iChip] = dac_findParameter(fDirectory.Data(), "VwllSh", iChip);
    vhlddel[iChip] = dac_findParameter(fDirectory.Data(), "VhldDel", iChip);
    vtrim[iChip] = dac_findParameter(fDirectory.Data(), "Vtrim", iChip);
    vthrcomp[iChip] = dac_findParameter(fDirectory.Data(), "VthrComp", iChip);
    vibias_bus[iChip] = dac_findParameter(fDirectory.Data(), "VIBias_Bus", iChip);
    vbias_sf[iChip] = dac_findParameter(fDirectory.Data(), "Vbias_sf", iChip);
    voffsetop[iChip] = dac_findParameter(fDirectory.Data(), "VoffsetOp", iChip);
    vibiasop[iChip] = dac_findParameter(fDirectory.Data(), "VIbiasOp", iChip);
    voffsetr0[iChip] = dac_findParameter(fDirectory.Data(), "VOffsetR0", iChip);
    vion[iChip] = dac_findParameter(fDirectory.Data(), "VIon", iChip);
    vibias_ph[iChip] = dac_findParameter(fDirectory.Data(), "VIbias_PH", iChip);
    ibias_dac[iChip] = dac_findParameter(fDirectory.Data(), "Ibias_DAC", iChip);
    vibias_roc[iChip] = dac_findParameter(fDirectory.Data(), "VIbias_roc", iChip);
    vicolor[iChip] = dac_findParameter(fDirectory.Data(), "VIColOr", iChip);
    vnpix[iChip] = dac_findParameter(fDirectory.Data(), "Vnpix", iChip);
    vsumcol[iChip] = dac_findParameter(fDirectory.Data(), "VSumCol", iChip);
    vcal[iChip] = dac_findParameter(fDirectory.Data(), "Vcal", iChip);
    caldel[iChip] = dac_findParameter(fDirectory.Data(), "CalDel", iChip);
    rangetemp[iChip] = dac_findParameter(fDirectory.Data(), "RangeTemp", iChip);
    ctrlreg[iChip] = dac_findParameter(fDirectory.Data(), "CtrlReg", iChip);
    wbc[iChip] = dac_findParameter(fDirectory.Data(), "WBC", iChip);
  }

  fBpix->Fill();

}


// ----------------------------------------------------------------------
void ntpTestResults::setupNtuple() {

  if (0 == fForceOverwrite) {
    TFile *f    = TFile::Open(Form("module-%s-%04d.root", fTemperature.Data(), fModule));
    if (f) {
      cout << "--> No overwriting of existing file " << Form("module-%s-%04d.root", fTemperature.Data(), fModule) << endl;
      fIsOK = 0;
      return;
    }
  }

  fFile = new TFile(Form("module-%s-%04d.root", fTemperature.Data(), fModule), "RECREATE");
  fBpix = new TTree("mod", Form("%s/%s", fBaseDir.Data(), fTemperature.Data()));


  fBpix->Branch("nmod",   &nmod,    "nmod/I");
  fBpix->Branch("grade",  &grade,   "grade/I");
  fBpix->Branch("overallGrade",  &overallGrade,   "overallGrade/I");
  fBpix->Branch("finalGrade",  &finalGrade,   "finalGrade/I");
  fBpix->Branch("shortGrade",  &shortGrade,   "shortGrade/I");
  fBpix->Branch("halfModule",  &halfModule,   "halfModule/I");
  fBpix->Branch("allTestsDone",  &allTestsDone,   "allTestsDone/I");
  fBpix->Branch("date",  &date,   "date/I");
  fBpix->Branch("hour",  &hour,   "hour/I");
  fBpix->Branch("minute",  &minute,   "minute/I");

  fBpix->Branch("tbmError1",  &tbmError1,   "tbmError1/I");
  fBpix->Branch("tbmError2",  &tbmError2,   "tbmError2/I");

  fBpix->Branch("ia",  &ia,   "ia/F");
  fBpix->Branch("id",  &id,   "id/F");
  fBpix->Branch("va",  &va,   "va/F");
  fBpix->Branch("vd",  &vd,   "vd/F");

  fBpix->Branch("iv100",  &iv100,   "iv100/F");
  fBpix->Branch("iv150",  &iv150,   "iv150/F");
  fBpix->Branch("ivSlope",  &ivSlope,   "ivSlope/F");
  fBpix->Branch("ivi",  &ivi,   "ivi[150]/F");
  fBpix->Branch("ivv",  &ivv,   "ivv[150]/F");

  fBpix->Branch("nRocsB",  nRocsB,   "nRocsB[20]/I");
  fBpix->Branch("nRocsC",  nRocsC,   "nRocsC[20]/I");

  fBpix->Branch("temperatureVoltage",  temperatureVoltage,   "temperatureVoltage[16]/F");

  fBpix->Branch("trims",  trims,    "trims[16][4160]/I");

  fBpix->Branch("uthrM",  uthrM,    "uthrM[16]/F");
  fBpix->Branch("uthrR",  uthrR,    "uthrR[16]/F");
  fBpix->Branch("uthr",   uthr,     "uthr[16][4160]/I");

  fBpix->Branch("tthrM",  tthrM,    "tthrM[16]/F");
  fBpix->Branch("tthrR",  tthrR,    "tthrR[16]/F");
  fBpix->Branch("tthr",   tthr,     "tthr[16][4160]/I");
  fBpix->Branch("bumpThrDiff",   bumpThrDiff,     "bumpThrDiff[16][4160]/I");

  fBpix->Branch("noiseM", noiseM ,  "noiseM[16]/F");
  fBpix->Branch("noiseR", noiseR ,  "noiseR[16]/F");
  fBpix->Branch("noise",  noise ,   "noise[16][4160]/F");


  fBpix->Branch("gainM",  gainM,    "gainM[16]/F");
  fBpix->Branch("gainR",  gainR,    "gainR[16]/F");
  fBpix->Branch("pedM",   pedM,     "pedM[16]/F");
  fBpix->Branch("pedR",   pedR,     "pedR[16]/F");
  fBpix->Branch("ph",     ph,       "ph[16][4160][10]/F");
  fBpix->Branch("tanhFit",tanhFit,  "tanhFit[16][4160][4]/F");
  fBpix->Branch("gain",   gain,     "gain[16][4160]/F");
  fBpix->Branch("ped",    ped,      "ped[16][4160]/F");
  fBpix->Branch("addlvl", addlvl,   "addlvl[16][4160]/F");
  fBpix->Branch("pixmap", pixmap,   "pixmap[16][4160]/F");
  fBpix->Branch("phValidation", phValidation,   "phValidation[16][10]/F");

  fBpix->Branch("defects",defects,  "defects[16][4160]/I");


  fBpix->Branch("vdig",       vdig,        "vdig[16]/I");
  fBpix->Branch("vana",       vana,        "vana[16]/I");
  fBpix->Branch("vsf",        vsf,         "vsf[16]/I");
  fBpix->Branch("vcomp",      vcomp,       "vcomp[16]/I");
  fBpix->Branch("vleak_comp", vleak_comp,  "vleak_comp[16]/I");
  fBpix->Branch("vrgpr",      vrgpr,       "vrgpr[16]/I");
  fBpix->Branch("vwllpr",     vwllpr,      "vwllpr[16]/I");
  fBpix->Branch("vrgsh",      vrgsh,       "vrgsh[16]/I");
  fBpix->Branch("vwllsh",     vwllsh,      "vwllsh[16]/I");
  fBpix->Branch("vhlddel",    vhlddel,     "vhlddel[16]/I");
  fBpix->Branch("vtrim",      vtrim,       "vtrim[16]/I");
  fBpix->Branch("vthrcomp",   vthrcomp,    "vthrcomp[16]/I");
  fBpix->Branch("vibias_bus", vibias_bus,  "vibias_bus[16]/I");
  fBpix->Branch("vbias_sf",   vbias_sf,    "vbias_sf[16]/I");
  fBpix->Branch("voffsetop",  voffsetop,   "voffsetop[16]/I");
  fBpix->Branch("vibiasop",   vibiasop,    "vibiasop[16]/I");
  fBpix->Branch("voffsetr0",  voffsetr0,   "voffsetr0[16]/I");
  fBpix->Branch("vion",       vion,        "vion[16]/I");
  fBpix->Branch("vibias_ph",  vibias_ph,   "vibias_ph[16]/I");
  fBpix->Branch("ibias_dac",  ibias_dac,   "ibias_dac[16]/I");
  fBpix->Branch("vibias_roc", vibias_roc,  "vibias_roc[16]/I");
  fBpix->Branch("vicolor",    vicolor,     "vicolor[16]/I");
  fBpix->Branch("vnpix",      vnpix,       "vnpix[16]/I");
  fBpix->Branch("vsumcol",    vsumcol,     "vsumcol[16]/I");
  fBpix->Branch("vcal",       vcal,        "vcal[16]/I");
  fBpix->Branch("caldel",     caldel,      "caldel[16]/I");
  fBpix->Branch("rangetemp",  rangetemp,   "rangetemp[16]/I");
  fBpix->Branch("ctrlreg",    ctrlreg,     "ctrlreg[16]/I");
  fBpix->Branch("wbc",        wbc,         "wbc[16]/I");

  for (int iChip = FCHP; iChip < LCHP; ++iChip) {
    vdig[iChip] = 0;
    vana[iChip] = 0;
    vsf[iChip] = 0;
    vcomp[iChip] = 0;
    vleak_comp[iChip] = 0;
    vrgpr[iChip] = 0;
    vwllpr[iChip] = 0;
    vrgsh[iChip] = 0;
    vwllsh[iChip] = 0;
    vhlddel[iChip] = 0;
    vtrim[iChip] = 0;
    vthrcomp[iChip] = 0;
    vibias_bus[iChip] = 0;
    vbias_sf[iChip] = 0;
    voffsetop[iChip] = 0;
    vibiasop[iChip] = 0;
    voffsetr0[iChip] = 0;
    vion[iChip] = 0;
    vibias_ph[iChip] = 0;
    ibias_dac[iChip] = 0;
    vibias_roc[iChip] = 0;
    vicolor[iChip] = 0;
    vnpix[iChip] = 0;
    vsumcol[iChip] = 0;
    vcal[iChip] = 0;
    caldel[iChip] = 0;
    rangetemp[iChip] = 0;
    ctrlreg[iChip] = 0;
    wbc[iChip] = 0;
  }

  for (int ic = FCHP; ic < LCHP; ++ic) {
    noiseM[ic] = 0;
    noiseR[ic] = 0;
    uthrM[ic] = 0;
    uthrR[ic] = 0;
    tthrM[ic] = 0;
    tthrR[ic] = 0;
    gainM[ic] = 0;
    gainR[ic] = 0;
    pedM[ic] = 0;
    pedR[ic] = 0;
    for (int k = 0; k < 10; k++) phValidation[ic][k] = 0;

    temperatureVoltage[ic] = 0.;

    for (int ix = 0; ix < NPIX; ++ix) {
      addlvl[ic][ix]  = 0;
      ped[ic][ix]     = 0;
      gain[ic][ix]    = 0;
      tthr[ic][ix]    = 0;
      uthr[ic][ix]    = 0;
      noise[ic][ix]   = 0;
      trims[ic][ix]   = 0;
      bumpThrDiff[ic][ix] = -1000;

      defects[ic][ix] = 0;
      pixmap[ic][ix]  = -9999.;
      for(int i = 0; i < 10; i++) ph[ic][ix][i]   = 0;
      for(int i = 0; i < 4; i++) tanhFit[ic][ix][i]   = 0;
    }
  }

  for (int i = 0; i < 20; i++)
  {
    nRocsB[i] = -1; //initialize to bad value
    nRocsC[i] = -1;
  }

  iv100 = 0.;
  iv150 = 0.;
  ivSlope = 0.;
  for (int i = 0; i < 150; i++)
  {
    ivi[i] = 0.;
    ivv[i] = 0.;
  }

  ia = 0.;
  id = 0.;
  va = 0.;
  vd = 0.;

  grade = 3;  //C
  finalGrade = 3; //C
  overallGrade = 3; //C
  allTestsDone = 1;
  tbmError1 = 0;
  tbmError2 = 0;

  fIsOK = 1;

}



// ----------------------------------------------------------------------
void ntpTestResults::fillMeansRMS() {
  
  float n,a,b;
  char fname[200];
  char string[200];
  
  FILE *inputFile;

  for (int ichip = FCHP; ichip < LCHP; ++ichip) {
    sprintf(fname, "%s/summary_C%d.txt", fDirectory.Data(), ichip);
    inputFile = fopen(fname, "r");
    if (inputFile) {

      for (int i = 0; i < 7; i++){
        fscanf(inputFile, "%s %f %f", string, &a, &b);   // unused data in summary_Cx.dat
      }

      // SCurve 170.072 17.3699
      fscanf(inputFile, "%s %f %f %f", string, &n, &a, &b);
      noiseM[ichip] = a; 
      noiseR[ichip] = b; 

      // Threshold 60.6058 1.44294
      fscanf(inputFile, "%s %f %f %f", string, &n, &a, &b);
      tthrM[ichip] = a;
      tthrR[ichip] = b; 

      // Gain 2.99519 0.103743
      fscanf(inputFile, "%s %f %f %f", string, &n, &a, &b);
      gainM[ichip] = a;
      gainR[ichip] = b;

      // Pedestal -990.25 50.543
      fscanf(inputFile, "%s %f %f %f", string, &n, &a, &b);
      pedM[ichip] = a;
      pedR[ichip] = b; 
      
      fclose(inputFile);
    } else {
      allTestsDone = 0;
      cout << "-----> File " << fname << " not found " << endl;
    }

  }

}


// ----------------------------------------------------------------------
void ntpTestResults::fillNoise() {
  float thr, sig;
  int a,b;
  char string[200];

  for (int ichip = FCHP; ichip < LCHP; ++ichip) {
    sprintf(string, "%s/SCurve_C%d.dat", fDirectory.Data(), ichip);
    FILE *inputFile = fopen(string, "r");
    if (inputFile) {
      for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);
      for (int icol = 0; icol < 52; ++icol) {
        for (int irow = 0; irow < 80; ++irow) {
          fscanf(inputFile, "%e %e %s %2i %2i", &thr, &sig, string, &a, &b);  //comment
          noise[ichip][icol*80 + irow] = sig;
        }
      }
      fclose(inputFile);
    } else {
      allTestsDone = 0;
      cout << "???? " << string << endl;
    }
  }

}



// ----------------------------------------------------------------------
void ntpTestResults::fillTrims() {
  int trim;
  int a,b;
  char string[200];

  for (int ichip = FCHP; ichip < LCHP; ++ichip) {
    sprintf(string, "%s/trimParameters60_C%d.dat", fDirectory.Data(), ichip);
    FILE *inputFile = fopen(string, "r");
    if (!inputFile) {
      cout << "Did not find trimfiles for VCAL=60. Try with VCAl = 50" << endl;
      sprintf(string, "%s/trimParameters50_C%d.dat", fDirectory.Data(), ichip);
      inputFile = fopen(string, "r");
    }
    
    if (inputFile) {
      for (int icol = 0; icol < 52; ++icol) {
        for (int irow = 0; irow < 80; ++irow) {
          fscanf(inputFile, "%2i %s %2i %2i", &trim, string, &a, &b);  //comment
          trims[ichip][icol*80 + irow] = trim;
        }
      }
      fclose(inputFile);
    } else {
      allTestsDone = 0;
      cout << "???? " << string << endl;
    }
    
  }

}


// ----------------------------------------------------------------------
void ntpTestResults::fillAddressLevels() {
  if (!fFullTest) return;
  char hname[200];
  TH1D *h1, *H1;
  for (int ichip = FCHP; ichip < LCHP; ++ichip) {
    sprintf(hname, "AddressLevels_C%i", ichip);
    h1 = (TH1D*)fFullTest->Get(hname);
    if (h1) {
      H1 = (TH1D*)h1->Clone();
      H1->SetName(Form("ADDLVL%i", ichip));
    } else {
      cout << ">>>>>> No histogram " <<  hname << endl;
      continue;
    }

    for (int i = 0; i < H1->GetNbinsX(); ++i) {
      addlvl[ichip][i] = H1->GetBinContent(i+1);
    }
    if (H1) delete H1;
  }
}

// ----------------------------------------------------------------------
void ntpTestResults::fillTanhFits() {
  float par0, par1, par2, par3; //Parameters of Vcal vs. Pulse Height Fit
  int a,b;
  char string[200];

  for (int ichip = FCHP; ichip < LCHP; ++ichip) {
    sprintf(string, "%s/phCalibrationFitTan_C%d.dat", fDirectory.Data(), ichip);
    FILE *inputFile = fopen(string, "r");
    if (inputFile) {

      for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);
      for (int icol = 0; icol < 52; ++icol) {
        for (int irow = 0; irow < 80; ++irow) {
          fscanf(inputFile, "%e %e %e %e %s %2i %2i", &par0, &par1, &par2, &par3, string, &a, &b);
          tanhFit[ichip][icol*80 + irow][0] = par0;
          tanhFit[ichip][icol*80 + irow][1] = par1;
          tanhFit[ichip][icol*80 + irow][2] = par2;
          tanhFit[ichip][icol*80 + irow][3] = par3;
          //	  printf("par1 = %f\n",tanhFit[ichip][icol*80 + irow][1]);
        }
      }
      fclose(inputFile);
      //cout << "par1 filled " << endl;
    } else {
      allTestsDone = 0;
      cout << "???? " << string << endl;
    }
  }
}


// ----------------------------------------------------------------------
void ntpTestResults::fillPHValidation() {
  char hname[100];

  for (int ichip = FCHP; ichip < LCHP; ++ichip) {
    sprintf(hname, "ValPlot_C%d;1", ichip);
    TH2D *h2 = (TH2D*)fFullTest->Get(hname);
    if (h2) {
      int n = 0;
      for (int i = 1; i < h2->GetNbinsX(); i++)
      {
        for (int k = 1; k < h2->GetNbinsY(); k++)
        {
          if (h2->GetBinContent(i,k) > 0 && n < 10)
          {
            phValidation[ichip][n] = h2->GetYaxis()->GetBinCenter(k);
            n++;
          }
        }
      }
    } else {
      cout << ">>>>>> No histogram " <<  hname << endl;
      continue;
    }
  }
}


// ----------------------------------------------------------------------
void ntpTestResults::fillGainPed() {
  float par0, par1, par2, par3, par4, par5; //Parameters of Vcal vs. Pulse Height Fit
  float tped, tgain;
  int a,b;
  char string[200];


  for (int ichip = FCHP; ichip < LCHP; ++ichip) {
    sprintf(string, "%s/phCalibrationFit_C%d.dat", fDirectory.Data(), ichip);
    FILE *inputFile = fopen(string, "r");
    if (inputFile) {

      for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);
      for (int icol = 0; icol < 52; ++icol) {
        for (int irow = 0; irow < 80; ++irow) {
          fscanf(inputFile, "%e %e %e %e %e %e %s %2i %2i", &par0, &par1, &par2, &par3, &par4, &par5, string, &a, &b);
          if (par2 != 0.) {
            //	    tped  = -par3/par2;
            tped  = par3;
            tgain = 1./par2;
            gain[ichip][icol*80 + irow] = tgain;
            ped[ichip][icol*80 + irow]  = tped;
          }
        }
      }
      fclose(inputFile);
    } else {
      allTestsDone = 0;
      cout << "???? " << string << endl;
    }    
  }
}

// ----------------------------------------------------------------------
void ntpTestResults::fillPulseHeights()
{
  int a,b;
  char string[200];

  for (int ichip = FCHP; ichip < LCHP; ++ichip)
    {
      sprintf(string, "%s/phCalibration_C%d.dat", fDirectory.Data(), ichip);
      FILE *inputFile = fopen(string, "r");
      if (inputFile)
	{
	  for (int i = 0; i < 4; i++) fgets(string, 200, inputFile);
	  for (int icol = 0; icol < 52; ++icol)
	    {
	      for (int irow = 0; irow < 80; ++irow)
        {
          for (int i = 0; i < 10; i++)
            {
              fscanf(inputFile, "%s", string);

              if (strcmp(string, "N/A") == 0)  ph[ichip][icol*80 + irow][i] = 7777;
              else
          {
            ph[ichip][icol*80 + irow][i]  = atoi(string);
          }
            }
          fscanf(inputFile, "%s %2i %2i", string, &a, &b);  //comment
        }
	    }
	  fclose(inputFile);
    }

    else
    {
      allTestsDone = 0;
      cout << "???? " << string << endl;
    }
  }
}


// ----------------------------------------------------------------------
void ntpTestResults::fillThresholds() {

  char hname[200];
  TH2D *h2, *hu, *ht, *hb;
  if (!fFullTest) return;

  for (int ichip = FCHP; ichip < LCHP; ++ichip) {

    sprintf(hname, "VcalThresholdMap_C%d;8", ichip);
    h2 = (TH2D*)fFullTest->Get(hname);
    if (h2) {
      ht = (TH2D*)h2->Clone();
      ht->SetName(Form("ht%i", ichip));
    } else {
      cout << ">>>>>> No histogram " <<  hname << endl;
      continue;
    }

    sprintf(hname, "VcalThresholdMap_C%d;1", ichip);
    h2 = (TH2D*)fFullTest->Get(hname);
    if (h2) {
      hu = (TH2D*)h2->Clone();
      hu->SetName(Form("hu%i", ichip));
    } else {
      cout << ">>>>>> No histogram " <<  hname << endl;
      continue;
    }

    sprintf(hname, "vcals_xtalk_C%d;1", ichip);
    h2 = (TH2D*)fFullTest->Get(hname);
    if (h2) {
      hb = (TH2D*)h2->Clone();
      hb->SetName(Form("hb%i", ichip));
    } else {
      cout << ">>>>>> No histogram " <<  hname << endl;
      continue;
    }


    for (int icol = 0; icol < 52; ++icol) {
      for (int irow = 0; irow < 80; ++irow) {
        uthr[ichip][icol*80 + irow] = int(hu->GetBinContent(icol+1, irow+1));
        tthr[ichip][icol*80 + irow] = int(ht->GetBinContent(icol+1, irow+1));
        bumpThrDiff[ichip][icol*80 + irow] = int(hb->GetBinContent(icol+1, irow+1));
      }
    }

    uthrM[ichip] = calcMean(hu);
    uthrR[ichip] = calcRMS(hu, uthrM[ichip]);
    // -- the trimmed versions are filled in

    if (ht) delete ht;
    if (hu) delete hu;
    if (hb) delete hb;
  }

}



// ----------------------------------------------------------------------
// -- defects
//    0 OK
//    1 dead
//    2 noisy (Beat's criterium)
//    4 masking
//    8 bump bonding
//   16 address levels
//   32 trim bits 1
//   64 trim bits 2
//  128 trim bits 3
//  256 trim bits 4
void ntpTestResults::fillPixelDefects() {
  
  double minDThr(-5.);

  char hname[200];
  TH2D *h2 = 0, *pm = 0, *bb = 0, *al = 0, *tb0 = 0, *tb1 = 0, *tb2 = 0, *tb3 = 0, *tb4 = 0, *htmp = 0;
  if (!fFullTest) return;

  for (int iChip = FCHP; iChip < LCHP; ++iChip) {

    sprintf(hname, "PixelMap_C%i", iChip); 
    h2 = (TH2D*)fFullTest->Get(hname);
    if (h2) {
      pm = (TH2D*)h2->Clone();
      pm->SetName(Form("PM%i", iChip));
    } else {
      cout << ">>>>>> No histogram " <<  hname << endl;
      pm = 0;
    }

    if (!fShortTest)
    {
      sprintf(hname, "vcals_xtalk_C%i", iChip);
      h2 = (TH2D*)fFullTest->Get(hname);
      if (h2) {
        bb = (TH2D*)h2->Clone();
        bb->SetName(Form("BB%i", iChip));
      } else {
        cout << ">>>>>> No histogram " <<  hname << endl;
        bb = 0;
      }

      sprintf(hname, "AddressDecoding_C%i", iChip);
      h2 = (TH2D*)fFullTest->Get(hname);
      if (h2) {
        al = (TH2D*)h2->Clone();
        al->SetName(Form("AL%i", iChip));
      } else {
        cout << ">>>>>> No histogram " <<  hname << endl;
        al = 0;
      }


      for (int i = 1; i < 6; ++i) {
        sprintf(hname, "CalThresholdMap_C%i;%i", iChip, i);
        h2 = (TH2D*)fFullTest->Get(hname);
        if (h2) {
          htmp = (TH2D*)h2->Clone();
          htmp->SetName(Form("TB0C%i", i));
          if (i == 1) tb0 = htmp;
          if (i == 2) tb1 = htmp;
          if (i == 3) tb2 = htmp;
          if (i == 4) tb3 = htmp;
          if (i == 5) tb4 = htmp;
              } else {
          cout << ">>>>>> No histogram " <<  hname << endl;
          tb0 = 0;
          tb1 = 0;
          tb2 = 0;
          tb3 = 0;
          tb4 = 0;
        }
      }
  }

    for (int icol = 0; icol < 52; ++icol) {
      for (int irow = 0; irow < 80; ++irow) {

      if (pm) {
        pixmap[iChip][icol*80 + irow] = pm->GetBinContent(icol+1, irow+1);
      }

      // -- dead
      if (pm && pm->GetBinContent(icol+1, irow+1) ==  0) {
        defects[iChip][icol*80 + irow] += 1;
        //	  cout << Form("  dead pixel     Chip%i: %i/%i", iChip, icol, irow) << endl;
      }

      // -- beat's noise
      if (pm && pm->GetBinContent(icol+1, irow+1)  > 10) {
        defects[iChip][icol*80 + irow] += 2;
        //	  cout << Form("  noisy pixel    Chip%i: %i/%i", iChip, icol, irow) << endl;
      }

      // -- mask defects
      if (pm && pm->GetBinContent(icol+1, irow+1) == -1) {
        defects[iChip][icol*80 + irow] += 4;
        //	  cout << Form("  mask-defect    Chip%i: %i/%i", iChip, icol, irow) << endl;
      }
      // -- bump bonding
      if (bb && (bb->GetBinContent(icol+1, irow+1) > minDThr)) {
        defects[iChip][icol*80 + irow] += 8;
        //	  cout << Form("  bumpbonding    Chip%i: %i/%i", iChip, icol, irow) << endl;
      }
      // -- address level decoding
      if (al && al->GetBinContent(icol+1, irow+1) < 1) {
        defects[iChip][icol*80 + irow] += 16;
        //	  cout << Form("  address levels Chip%i: %i/%i", iChip, icol, irow) << endl;
      }

      // -- trim bit 1
      if (tb0 && tb1 && TMath::Abs(tb0->GetBinContent(icol+1, irow+1) - tb1->GetBinContent(icol+1, irow+1)) <= 2) {
        defects[iChip][icol*80 + irow] +=  32;
        // 	  cout << Form("  dead trimbit1  Chip%i: %i/%i: %i",
        //      iChip, icol, irow, tb1->GetBinContent(icol+1, irow+1)) << endl;
          }
      // -- trim bit 2
      if (tb0 && tb2 && TMath::Abs(tb0->GetBinContent(icol+1, irow+1) - tb2->GetBinContent(icol+1, irow+1)) <= 2) {
        defects[iChip][icol*80 + irow] +=  64;
        //	  cout << Form("  dead trimbit2  Chip%i: %i/%i: %i",
        //      iChip, icol, irow, tb2->GetBinContent(icol+1, irow+1)) << endl;
      }
      // -- trim bit 3
      if (tb0 && tb3 && TMath::Abs(tb0->GetBinContent(icol+1, irow+1) - tb3->GetBinContent(icol+1, irow+1)) <= 2) {
        defects[iChip][icol*80 + irow] += 128;
        //	  cout << Form("  dead trimbit3  Chip%i: %i/%i: %i",
        //      iChip, icol, irow, tb3->GetBinContent(icol+1, irow+1)) << endl;
      }
      // -- trim bit 4
      if (tb0 && tb4 && TMath::Abs(tb0->GetBinContent(icol+1, irow+1) - tb4->GetBinContent(icol+1, irow+1)) <= 2) {
        defects[iChip][icol*80 + irow] += 256;
        //	  cout << Form("  dead trimbit4  Chip%i: %i/%i: %i",
        //      iChip, icol, irow, tb4->GetBinContent(icol+1, irow+1)) << endl;
      }

      }
    }

    if (pm) delete pm;
    if (bb) delete bb;
    if (al) delete al;
    if (tb0) delete tb0;
    if (tb1) delete tb1;
    if (tb2) delete tb2;
    if (tb3) delete tb3;
    if (tb4) delete tb4;
  }


  if (0) {
    for (int ic = FCHP; ic < LCHP; ++ic) {
      for (int ix = 0; ix < 52; ++ix) {
        for (int iy = 0; iy < 80; ++iy) {
          if (defects[ic][ix*80 + iy] > 0) {
            cout <<  Form(" .. Chip%i: %i/%i: %i", ic, ix, iy, defects[ic][ix*80 + iy]) << endl;
          }
        }
      }
    }
  }

}



// -------------------------------------------------------------------------
void ntpTestResults::fillTBMErrors() {
  if (!fFullTest) return;

  TParameter<int> *par;
  par = (TParameter<int>*)fFullTest->Get("TBM1");
  if (par) tbmError1 = par->GetVal();
  par = (TParameter<int>*)fFullTest->Get("TBM2");
  if (par) tbmError2 = par->GetVal();
}


// -------------------------------------------------------------------------
void ntpTestResults::fillCurrents() {
  TParameter<double> *par;
  par = (TParameter<double>*)fFullTest->Get("IA");
  if (par) ia = par->GetVal();
  par = (TParameter<double>*)fFullTest->Get("ID");
  if (par) id = par->GetVal();
  par = (TParameter<double>*)fFullTest->Get("VA");
  if (par) va = par->GetVal();
  par = (TParameter<double>*)fFullTest->Get("VD");
  if (par) vd = par->GetVal();
}


// -------------------------------------------------------------------------
int ntpTestResults::dac_findParameter(const char *dir, const char *dacPar, int chipId) {

  FILE *File, *File50, *File60;
  char fname[1000];
  char string[2000]; int a;
  int prm(-1);

  sprintf(fname, "%s/dacParameters_C%i.dat", dir, chipId);
  File = fopen(fname, "r");

  sprintf(fname, "%s/dacParameters50_C%i.dat", dir, chipId);
  File50 = fopen(fname, "r");

  sprintf(fname, "%s/dacParameters60_C%i.dat", dir, chipId);
  File60 = fopen(fname, "r");

  if ( File60 )
  {
    //    printf("chipSummary> Reading %s from dac Parameters 60 ...\n", dacPar);
    for (int i = 0; i < 29; i++) {

      fscanf(File60, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;

    }
  }

  if ( File50 && !File60 )
  {
    //    printf("chipSummary> Reading %s from dac Parameters 50 ...\n", dacPar);
    for (int i = 0; i < 29; i++) {

      fscanf(File50, "%i %s %i", &a, string, &prm);
      if ( strcmp(dacPar,string) == 0 )  break;

    }
  }

  if (!File50 && !File60)
  {

    if (File)
    {
      for (int i = 0; i < 29; i++) {

        fscanf(File, "%i %s %i", &a, string, &prm);
        if ( strcmp(dacPar,string) == 0 )  break;

      }
    }

    if ( !File )
    {
      allTestsDone = 0;
      printf("chipSummary> !!!!!!!!!  ----> DAC Parameters: Could not find a file to read DAC parameter\n");
      return 0;
    }
    else if (!fShortTest)
    {
      allTestsDone = 0;
      printf("chipSummary> No DAC Parameters after trimming available. Reading %s ...\n", dacPar);
    }

  }

  if (File)   fclose(File);
  if (File50) fclose(File50);
  if (File60) fclose(File60);

  return prm;
}

void ntpTestResults::fillIV()
{
  FILE *File;
  char fname[1000];
  float V, A;
  int i = 0;

  sprintf(fname, "%s/iv.dat", fDirectory.Data());
  File = fopen(fname, "r");

  if (File)
  {
    fclose(File);
    ifstream is(fname);

    char  buffer[200];

    while (is.getline(buffer, 200, '\n'))
    {

      // check that line starts with a number
      if (buffer[0] != '1' && buffer[0] != '2' && buffer[0] != '3' && buffer[0] != '4' && buffer[0] != '5' && buffer[0] != '6' && buffer[0] != '7' && buffer[0] != '8' && buffer[0] != '9') continue;

      sscanf(buffer, "%e %e", &V, &A);

      ivv[i] = V;
      ivi[i] = 1e6*A;

      if ( i > 0 )
      {
        // check that voltage is increasing & find current at 150 V
        if ( ivv[i] < ivv[i-1] ) continue;
	      if ( ivv[i] >= 100. && ivv[i-1] <= 100. ) iv100 = ivi[i-1] + (100. - ivv[i-1])*(ivi[i] - ivi[i-1])/(ivv[i] - ivv[i-1]);
	      if ( ivv[i] >= 150. && ivv[i-1] <= 150. ) iv150 = ivi[i-1] + (150. - ivv[i-1])*(ivi[i] - ivi[i-1])/(ivv[i] - ivv[i-1]);
      }
      if ( iv100 != 0. ) ivSlope = iv150/iv100;
      i++;
    }
  }
  else
  {
    if (strcmp(fTemperature.Data(), "T-10a")) allTestsDone = 0; //only if not t-10a
  }
}


// ----------------------------------------------------------------------
void ntpTestResults::fillTemperatureCalibration() {
  if (!fFullTest) return;
  char hname[200];
  TGraph *temp, *calib, *gr;

  for (int ichip = FCHP; ichip < LCHP; ++ichip)
  {
    sprintf(hname, "TempCalibration_C%i", ichip);
    gr = (TGraph*)fFullTest->Get(hname);
    if (gr) calib = (TGraph*)gr->Clone();
    else {
      cout << ">>>>>> No temperature graph " <<  hname << endl;
      continue;
    }

    sprintf(hname, "TempMeasurement_C%i", ichip);
    gr = (TGraph*)fFullTest->Get(hname);
    if (gr) temp = (TGraph*)gr->Clone();
    else {
      cout << ">>>>>> No temperature graph " <<  hname << endl;
      continue;
    }

    // Compute voltage

    int n = 0;
    double x[8], y[8], a, b;

    for (int i = 0; i < 8; i++)
    {
      calib->GetPoint(i, a, b);
      if (TMath::Abs(b) > 150.)
      {
        x[n] = b;
        y[n] = 70. - a * 23.5; // measured = 470 - reference
        n++;
      }
    }
    if (n == 0) return;

    TGraph *graph = new TGraph(n, x, y);
    TF1 *fit = new TF1("fit", "pol1(0)", 2000., 2);
    fit->SetParameter(0, -70.);
    fit->SetParameter(1, 0.1);
    graph->Fit("fit", "NRQ");

//     for (int i = 0; i < 8; i++)
//     {
    int i = 0; //always use point 0
    temp->GetPoint(i, a, b);
    if (TMath::Abs(b) > 150.)
    {
      temperatureVoltage[ichip] = 440. + 23.5*i + fit->Eval(b);  //measured = sensor - reference
      //printf("%i voltage %e\n", i, temperatureVoltage[ichip]);
    }
//     }

    if (graph) delete graph;
    if (temp) delete temp;
    if (calib) delete calib;
  }
}


void ntpTestResults::findHalfModules(const char *dir) {

  FILE *File;
  char fname[1000];
  char string[2000];
  int halfModule = 0, a;

  sprintf(fname, "%s/configParameters.dat", dir);
  File = fopen(fname, "r");

  if ( File )
  {
    for (int i = 0; i < 50; i++) {

      fscanf(File, "%s %i", string, &a);
      if ( strcmp("halfModule",string) == 0 ) halfModule = a;
    }
    fclose(File);
  }
  else printf("Cannot open file %s\n", fname);

  if (halfModule == 0) setFullModules();
  else if (halfModule == 1) setHalfModulesA();
  else if (halfModule == 2) setHalfModulesB();
}

// ======================================================================
int main(int argc, char *argv[]) {

  // -- command line arguments
  char baseDir[1000];     sprintf(baseDir, ".");
  char temperature[100];  sprintf(temperature, "T-10a");
  int all(0), force(0);

  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-a"))  all   = 1;
    if (!strcmp(argv[i],"-f"))  force = 1;
    if (!strcmp(argv[i],"-d"))  sprintf(baseDir, argv[++i]);     // base directory
    if (!strcmp(argv[i],"-m"))  sprintf(baseDir, argv[++i]);     // base directory
    if (!strcmp(argv[i],"-t"))  sprintf(temperature, argv[++i]); // temperature
  }

  if (all) {
  // -- Get all directories in baseDir
    chdir(baseDir);

    const int MAXMOD(2000);
    TString fname;
    TString aDirs[MAXMOD];
    for (int i = 0; i < MAXMOD; ++i) {
      aDirs[i] = "-";
    }

    int module, bla;
    const char *file;
    void *pDir = gSystem->OpenDirectory(".");

    while ((file = gSystem->GetDirEntry(pDir))) {
      fname = TString(file);
      module = -1;
      if (!strcmp(fname.Data(), ".") || !strcmp(fname.Data(), "..")) continue;
      sscanf(fname.Data(), "M%d-%d.%d:%d", &module, &bla, &bla, &bla);
      cout << fname << "  " << module << endl;

      if (module < 0) {continue;}

      if (!strcmp(aDirs[module].Data(), "-")) {
  aDirs[module] = fname;
      } else {
  if (fname.Data() > aDirs[module].Data()) {
    cout << "replacing " << aDirs[module] << "  with " << fname << endl;
    aDirs[module] = fname;
  }
      }
    }

    char dir[1000];
    for (int i = 0; i < MAXMOD; ++i) {
      if (!strcmp(aDirs[i].Data(), "-")) continue;
      fname = aDirs[i];
      sprintf(dir, fname.Data());
      fname += TString(Form("/%s", temperature));
      cout << fname.Data() << endl;

      cout << Form("-> ntuple(%s, %s)", dir, temperature) << endl;
      //      ntuple(dir, temperature);
      ntpTestResults a(dir, temperature, force);
      if (a.isOK()) {
        a.findHalfModules(Form("%s/dtlscan", dir));
        a.fillNtuple();
            } else {
        cout << Form("..... problem with  ntuple(%s, %s)", dir, temperature)
            << endl;
            }
    }

  } else {
    //    ntuple(baseDir, temperature);
    ntpTestResults a(baseDir, temperature, force);
    a.findHalfModules(Form("%s/dtlscan", baseDir));
    a.fillNtuple();
  }

  return 0;

}

