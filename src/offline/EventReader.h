#include "BinaryFileReader.h"
#include "ConfigReader.h"
#include <vector>
#include "pixelForReadout.h"
//#include "RocGeometry.h"
#include "EventView.h"
#include "Plane.h"
class TH2F;

using namespace std;

class EventReader{

 private:
  char fMtbfile[200];
  char fTtbfile[200];
  char fmtblevelfile[200];
  char frtblevelfile[200];
  char fConfigfile[200];
  char fTag[20];
  int fLevelMode;

  // tracking
  int fSeed1,fSeed2,fTLayer1,fTLayer2,fDUTLayer;
  vector<cluster> fHits[5];
  //  vector<Track> fTracks;
  Plane fPlane[5];
  double fzScint;
  double fSearchRadius;
  double fChiSqCut;
  double fCountingRegion1;
  int fNFound;
  int fShadowRowMin,fShadowRowMax,fShadowColMin,fShadowColMax;

  vector<EventView *> vev;
  EventView* fv;

 public:
  BinaryFileReader* fMod;
  BinaryFileReader* fRoc;
  ConfigReader* fConfig;
  int fRun;
  double fDeconvRoc[4];
  double fDeconv2Roc[4];
  double fDeconvMod;
  double fQminTrk,fQminEff;
  static const int initRocLevels=1;
  static const int initModuleLevels=2;
  int fRequireSync;
  int fVerbose;

  ofstream* fAlignmentFile;
  int fnAlignment;

  // counters
  int fnSkipRoc;
  int fnSkipMod;
  int fnSync;
  int fnRocOnly, fnModuleOnly;

  int fNCluModDcol[16][26];
  int fNTrkModDcol[16][26];
  int fNCluModSum,fNTrkModSum,fNCluModEvt;
;
  RocGeometry* fGeometry;
 
  //histos
 public:
  TH2F *hModHitMap;
  TH2F *hRocHitMap[4];
  TH1F* hNCluModRocExt[16];
  TH1F* hNCluModRocInt[16];
  TH1F *hNCluMod,*hNCluRoc,*hNCluRoc1,*hNCluRoc2,*hNCluRoc3,*hNCluRoc4;
  TH1F *hQCluLayer[5];
  TH1F *hSizeCluLayer[5];
  TH1F *hNCluShadowExt,*hNCluShadowInt,*hNCluShadowCal;
  TH1F *hNCluTracking;
  TH2F *hNCluCorr, *hNPixCorr;
  TH2F *hRocEmpty;
  TH1F *hNPixMod,*hNPixRoc;
  TH1F *hNSeedPair, *hNTracksPerSeed;
  TH1F *hFitChisq;
  TH1F *hDistTLayer1,  *hDistTLayer2, *hDistDUT, *hDistSig;
  TH1F *hDxTLayer1,    *hDxTLayer2,   *hDxDUT,   *hDxSig;
  TH1F *hDyTLayer1,    *hDyTLayer2,   *hDyDUT,   *hDySig;
  TH2F *hDxDyTLayer1,  *hDxDyTLayer2, *hDxDyDUT, *hDxDySig;
  TH2F *hDxSx, *hDySy;
  TH2F *hDxPhiTLayer1, *hDxPhiTLayer2,*hDxPhiDUT;
  TH2F *hDyPhiTLayer1, *hDyPhiTLayer2,*hDyPhiDUT;
  TH2F *hDUTh2;
  TH1F *hDUTh1, *hDUTnH1, *hDUTh1a, *hDUTnH2;
  TH2F *hMissing, *hFound;

  TH2F *hScint;

  // history
  TH1F *hNPixD, * hNPixT,  *hDeltaT, *hDeltaTt;

 public:
  EventReader(int run, const char* tag="", int levelMode=0,const char* telescope="", const char* module="");
  EventReader(const char* telescope, const char* module);
  void init();
  void loop(int nEvent=0);
  void printRunSummary();
  void findTracks();
  double fitTrack(vector<cluster>::iterator c1, vector<cluster>::iterator c2,
						vector<cluster>::iterator c3, vector<cluster>::iterator c4,
						double* par, double* cov);
  void goodTrack(double *par, int bg=0);
  void dumpHits(ofstream *f);
  double lineFit(double* x, double* y, int n, double* slope, double* offset);
  void setVerbose(int v){ fVerbose=v;};
  void setCountingRegionSize(double r){fCountingRegion1=r;};
  double getCountingRegionSize(){return fCountingRegion1;};
  ConfigReader* getConfig(){return fConfig;};
  int getTLayer1(){return fTLayer1;}
  int getTLayer2(){return fTLayer2;}
  int getDUTLayer(){return fDUTLayer;}

  EventView* addEventView();
  EventView* getEventView(unsigned int n);
  int getNEventView(){return vev.size();};
  double getShadowArea(){return (fShadowColMax-fShadowColMin+1)*(fShadowRowMax-fShadowRowMin+1)*0.0150*0.0100;}
  double getTrackingArea(){return 0.6*0.7;}
};
