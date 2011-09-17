#include <iostream>
#include <TSystem.h>
#include "TH1F.h"
#include "TH2F.h"
#include "BinaryFileReader.h"
//#include "ConfigReader.h"
#include "EventReader.h"
#include "PHCalibration.h"
#include<stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <math.h>

// pixel hit and cluster struct
#include "pixelForReadout.h"

using namespace std;



/************************************************************/

EventReader::EventReader(int run, const char* tag, int levelMode, const char* telescope, const char* module){
  char path[200];
  sprintf(path,"/home/l_tester/log/bt05r%06d",run);
  if(strlen(module)==0){
	 sprintf(fMtbfile, "%s/mtb.bin",path);
  }else{
	 sprintf(fMtbfile, "%s/%s",path,module);
  }	 
  if(strlen(telescope)==0){
	 sprintf(fTtbfile, "%s/rtb.bin",path);
  }else{
	 sprintf(fTtbfile, "%s/%s",path,telescope);
  }	 
  //sprintf(frtblevelfile, "%s/levels-roc.dat",path);
  //sprintf(fmtblevelfile, "%s/levels-module.dat",path);
  sprintf(fConfigfile, "%s/config.dat",path);
  sprintf(fTag,"%s",tag);
  fRun=run;
  fLevelMode=levelMode;
  init();
}

/************************************************************/

EventReader::EventReader(const char* telescope, const char* module){
  sprintf(fMtbfile,"%s",module);
  sprintf(fTtbfile,"%s",telescope);
  fRun=0;
  fLevelMode=0;
  init();
}


/************************************************************/

void EventReader::init(){

  fConfig=new ConfigReader(fConfigfile,"config.dat");
  fConfig->get("QminTrk",fQminTrk,100.);
  fConfig->get("QminEff",fQminEff,100.);
  // get pulse-height calibration ----
  /*
  PHCalibration* phcal=NULL;
  if(usePHcal){
    phcal=new PHCalibration();
    phcal->LoadFitParameters();
  }
  */
  fAlignmentFile=0;
  

  // re-do level files if requested
  if(fLevelMode&initModuleLevels){
	 fMod=new BinaryFileReader(fMtbfile,"4444444444444444","MtbTemp",
				   fConfig,"mod",1);
	 //fMod->readLevels(fmtblevelfile,1);
	 fMod->requireSync(0);
	 if((fMod->open()==0) ){
		while( (fMod!=NULL)&&(!fMod->eof()) ){fMod->readRecord();}
		fMod->updateLevels();
	 }
	 delete fMod;
  }
  if(fLevelMode&(initRocLevels|initModuleLevels)){
    fConfig->rewrite();
  }

  // open event files ------
  fMod=new BinaryFileReader(fMtbfile,"4444444444444444",Form("mod%05d",fRun),
			    fConfig,"mod");
  //fMod->readLevels(fmtblevelfile);
  if(!(fMod->open()==0) ){
	 fMod=NULL;
  }

  fRoc=new BinaryFileReader(fTtbfile,"0123", Form("rocs%05d",fRun),
			    fConfig,"roc");
  //fRoc->readLevels(frtblevelfile);
  if( !(fRoc->open()==0) ){
	 fRoc=NULL;
  }
  
  fConfig->get("seed1",fSeed1,2);
  fConfig->get("seed2",fSeed2,0);
  fConfig->get("tracking1",fTLayer1,3);
  fConfig->get("tracking2",fTLayer2,1);
  fConfig->get("DUT",fDUTLayer,4);
  fConfig->get("searchRadius",fSearchRadius,0.05);
  fConfig->get("chi2Cut",fChiSqCut,4.);
  fConfig->get("countingRegion1",fCountingRegion1,0.1);
  fConfig->get("requireSync",fRequireSync,1);
  fConfig->get("shadowRowMin",fShadowRowMin,34);
  fConfig->get("shadowRowMax",fShadowRowMax,53);
  fConfig->get("shadowColMin",fShadowColMin,16);
  fConfig->get("shadowColMax",fShadowColMax,30);
  //  fConfig->get("verbose",fVerbose,0);
  fVerbose=0;

  // telescope connector distances  21.6mm/81.6mm/21.6mm
  double zguess[5]={12.48-0.5, 10.32-0.5, 0-0.5, 2.16-0.5,  7.};
  double znom[5];
  //double dxyz[5][3]={{0}};
  double dxyz[5][3]={{0,0,0},
		     {0.0111,-0.0100,0},
		     {0,0,0},
		     {0.0036,0.0148,0},
		     {-0.0017,0.5507,0} };
  
  for(int i=0; i<5; i++){
	 znom[i]=zguess[i]-zguess[4];
	 fConfig->geta("shift",i,3,dxyz[i]);
	 fPlane[i]=Plane(dxyz[i][0],dxyz[i][1], dxyz[i][2] + znom[i]);
	 //printf("%d  %10f  %10f  %10f\n",i,dxyz[i][0],dxyz[i][1], dxyz[i][2] + znom[i]);
	 //fConfig->get("phi",
  }
  fzScint=znom[2]-1.0;
  fGeometry=new RocGeometry(0,0);

  fnSync=0;
  fnRocOnly=0;
  fnModuleOnly=0;

  // create some histos
  /*
  hModHitMap   = new TH2F("hitmap","hitmap",160,0,160,416,0,416);
  hRocHitMap[0]= new TH2F("hitmap0","hitmap0",52,0,51,80,0,79);
  hRocHitMap[1]= new TH2F("hitmap1","hitmap1",52,0,51,80,0,79);
  hRocHitMap[2]= new TH2F("hitmap2","hitmap2",52,0,51,80,0,79);
  hRocHitMap[3]= new TH2F("hitmap3","hitmap3",52,0,51,80,0,79);
  */
  hRocEmpty   = new TH2F("hitmap0","hitmap0",162,-0.81,0.81,432,-3.24,3.24);
  hMissing   = new TH2F("Missing","missing hits",162,-0.81,0.81,432,-3.24,3.24);
  hFound     = new TH2F("Found  ","found   hits",162,-0.81,0.81,432,-3.24,3.24);

  for(int i=0; i<16; i++){
    hNCluModRocExt[i]=new TH1F(Form("NCluModExt%2d_%s",i,fTag), 
			    "# of clusters", 30,-0.5,29.5);
    hNCluModRocInt[i]=new TH1F(Form("NCluModInt%2d_%s",i,fTag), 
			    "# of clusters", 30,-0.5,29.5);
    for(int j=0; j<26; j++) fNCluModDcol[i][j]=0;
    for(int j=0; j<26; j++) fNTrkModDcol[i][j]=0;
  }
  fNCluModSum=0;
  fNCluModEvt=0;
  fNTrkModSum=0;

  hNCluMod=new TH1F(Form("NCluMod_%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNCluRoc=new TH1F(Form("NCluRoc_%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNCluCorr=new TH2F(Form("NCluCorr_%s",fTag), "# of clusters", 
							30,-0.5,29.5,30,-0.5,29.5);
  hNCluRoc1=new TH1F(Form("NCluRoc_1%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNCluRoc2=new TH1F(Form("NCluRoc_2%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNCluRoc3=new TH1F(Form("NCluRoc_3%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNCluRoc4=new TH1F(Form("NCluRoc_4%s",fTag), "# of clusters", 30,-0.5,29.5);
  
  hNCluTracking=new TH1F(Form("NCluTracking_%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNCluShadowExt=new TH1F(Form("NCluShadowExt_%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNCluShadowInt=new TH1F(Form("NCluShadowInt_%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNCluShadowCal=new TH1F(Form("NCluShadowCal_%s",fTag), "# of clusters", 30,-0.5,29.5);
  hNPixMod=new TH1F(Form("NPixMod_%s",fTag), "# of pixels", 30,-0.5,29.5);
  hNPixRoc=new TH1F(Form("NPixRoc_%s",fTag), "# of pixels", 30,-0.5,29.5);
  hNPixCorr=new TH2F(Form("NPixCorr_%s",fTag), "# of pixels", 30,-0.5,29.5,30,-0.5,29.5);

  for(int layer=0; layer<5; layer++){
    hQCluLayer[layer]=new TH1F(Form("QCluRoc_%d%s",layer,fTag), 
			       "Q(clusters)", 50,0.,1000);
    hSizeCluLayer[layer]=new TH1F(Form("SizeCluRoc_%d%s",layer,fTag), 
			       "cluster size", 20,0.,20.);
  }

  // history
  hNPixD = new TH1F("npixd","# readouts vs time",10000,0.,100.); 
  hNPixT = new TH1F("npixt","trig vs time",10000,0.,100.);
  hDeltaT = new TH1F("deltat","time between triggers",5000,-0.5,4999.5); 
  hDeltaTt = new TH1F("deltatt","time between trigger and data",500,-0.5,499.5);
  //hDeltaTtlog = new TH1F("deltattlog","time between trigger and data",100,0,6);
  

  // 
  hNSeedPair=new TH1F(Form("seedpairs_%s",fTag), "seed pairs", 40,-0.5,39.5);
  hNTracksPerSeed=new TH1F(Form("trksperseed_%s",fTag), "Tracks per seed pair", 
									20,-0.5,19.5);

  // tracking control-plots
  const int nbin=100;
  hFitChisq=new TH1F(Form("Chisq_%s",fTag), "track fit chi**2", 30,0.,30.);
  hDistTLayer1=new TH1F(Form("distL1_%s",fTag), "distance L1", nbin,0.,0.1);
  hDistTLayer2=new TH1F(Form("distL2_%s",fTag), "distance L2", nbin,0.,0.1);
  hDistDUT=new TH1F(Form("distDUT_%s",fTag), "distance", nbin,0.,0.1);
  hDistSig=new TH1F(Form("distSig_%s",fTag), "distance", nbin,0.,0.1);

  hDxTLayer1=new TH1F(Form("dxL1_%s",fTag), "dx L1", nbin,-0.2,0.2);
  hDxTLayer2=new TH1F(Form("dxL2_%s",fTag), "dx L2", nbin,-0.2,0.2);
  hDxDUT=new TH1F(Form("dxDUT_%s",fTag), "dx", nbin,-0.2,0.2);
  hDxSig=new TH1F(Form("dxSig_%s",fTag), "dx", nbin,-0.2,0.2);

  hDyTLayer1=new TH1F(Form("dyL1_%s",fTag), "dy L1", nbin,-0.2,0.2);
  hDyTLayer2=new TH1F(Form("dyL2_%s",fTag), "dy L2", nbin,-0.2,0.2);
  hDyDUT=new TH1F(Form("dyDUT_%s",fTag), "dy", nbin,-0.2,0.2);
  hDySig=new TH1F(Form("dySig_%s",fTag), "dy", nbin,-0.2,0.2);

  hDxDyTLayer1=new TH2F(Form("dxdyL1_%s",fTag),"dx vs dy L1",
			100,-0.2, 0.2, 100, -0.2, 0.2);;
  hDxDyTLayer2=new TH2F(Form("dxdyL2_%s",fTag),"dx vs dy L2",
			100,-0.2, 0.2, 100, -0.2, 0.2);;
  hDxDyDUT=new TH2F(Form("dxdyDUT_%s",fTag),"dx vs dy",
		    100,-0.2, 0.2, 100, -0.2, 0.2);
  hDxDySig=new TH2F(Form("dxdySig_%s",fTag),"dx vs dy",
		    100,-0.2, 0.2, 100, -0.2, 0.2);
  
  double const pi=3.14159;
  const int nbin1=10;
  hDxPhiTLayer1=new TH2F(Form("dxphiL1_%s",fTag),"dx vs phi L1",
			 2*nbin1,-0.00300*nbin1, 0.00300*nbin1, 20, -pi, pi);
  hDxPhiTLayer2=new TH2F(Form("dxphiL2_%s",fTag),"dx vs phi L2",
			 2*nbin1,-0.00300*nbin1, 0.00300*nbin1, 20, -pi, pi);
  hDxPhiDUT=new TH2F(Form("dxphiDUT_%s",fTag),"dx vs phi",
		     2*nbin1,-0.00200*nbin1, 0.00200*nbin1, 20, -pi, pi);
  
  hDyPhiTLayer1=new TH2F(Form("dyphiL1_%s",fTag),"dy vs phi L1",
			 2*nbin1,-0.00200*nbin1, 0.00200*nbin1, 20, -pi, pi);
  hDyPhiTLayer2=new TH2F(Form("dyphiL2_%s",fTag),"dy vs phi L2",
			 2*nbin1,-0.00200*nbin1, 0.00200*nbin1, 20, -pi, pi);
  hDyPhiDUT=new TH2F(Form("dyphiDUT_%s",fTag),"dy vs phi",
		     2*nbin1,-0.00300*nbin1, 0.00300*nbin1, 20, -pi, pi);
  
  
  hDxSx=new TH2F(Form("dxSxDUT_%s",fTag),"dx vs Sx", 20, -0.02, 0.02,
		 2*nbin1,-0.00200*nbin1, 0.00200*nbin1);
  hDySy=new TH2F(Form("dySyDUT_%s",fTag),"dy vs Sy", 20, -0.02, 0.02,
		 2*nbin1,-0.00200*nbin1, 0.00200*nbin1);


  hDUTh2=new TH2F(Form("hits2_%s",fTag),"",100,-0.2, 0.2, 100, -0.2, 0.2);;
  hDUTh1=new TH1F(Form("hits1_%s",fTag),"",100,0., 0.4);
  hDUTh1a=new TH1F(Form("hits1a_%s",fTag),"",100,0., 0.2);
  hDUTnH1=new TH1F(Form("nhit1_%s",fTag), "hits in signal region", 20,-0.5,19.5);
  hDUTnH2=new TH1F(Form("nhit2_%s",fTag), "hits in signal region", 20,-0.5,19.5);
  //  hDUTnB1=new TH1F(Form("nhitb1_%s",fTag), "hits in background region", 20,-0.5,19.5);

  hScint=new TH2F("Scintillator","Scintillator",20,-0.4, 0.4, 20, -0.4, 0.4);;
  
}
/************************************************************/




/****************************************************************/
void EventReader::loop(int nEvent){
  int dtbmtrig=-1;
  //  int t=0;


  int useMod=0;
  int useRoc=0;
  int readMod=0;
  int readRoc=0;

  long long int oldTime=-1;
  long long int time=0;
  long long int dTime=0;

  if(nEvent==0) nEvent=0xFFFFFFF; // maxint?

  int mStat=0;  // 0 means no more data, 1=good >1=corrupt
  int rStat=0;

  if( fMod ) mStat=fMod->readGoodDataEvent();
  if( fRoc ) rStat=fRoc->readGoodDataEvent();

  // cout << "EventReader::loop> first read  mStat=" << mStat << "  rSTat=" << rStat << endl;
  bool more=( mStat | rStat );
  
  while( more && (fnSync<nEvent) ){
    
    bool bMod=fMod && !fMod->eof();
    bool bRoc=fRoc && !fRoc->eof();
    /*
    cout << Form("EventReader::loop>  (%10lld,%2d)    (%10lld,%2d) ",
		 fMod->getTrigBC(),mStat,
		 fRoc->getTrigBC(),rStat) << endl;
    */
    // sort out what kind of data we have here
    if( bMod && bRoc ){

      if (  (fMod->getTrigBC() == fRoc->getTrigBC()) ){
	// module and roc for the same event
	readMod=1; readRoc=1;
	if(   (fMod->getTrigType() == BinaryFileReader::kExternalTrigger)
	      &&(fRoc->getTrigType() == BinaryFileReader::kExternalTrigger) ){
	  // we have a sync'd event with external triggers
	  if( mStat==1 && rStat==1 ){
	    useMod=1; useRoc=1;
	    fnSync++;
	  }else{
	    // sync'd, but at least one bad guy, pass
	    useMod=0; useRoc=0;
	  }
	}else{
	  // equal time-stamps but only one is external ??
	  // cout << "EventReader::loop> This should never happen" << endl;
	  // don't use those events
	  cout <<  "EventReader  mTrig=" << fMod->getTrigBC() 
	       << "  rTrig=" << fRoc->getTrigBC()<< endl;
	  cout << "EventReader> equal time-stamps but only one external trigger. Event not used" << endl;
	  readMod=1; useMod=0; readRoc=1; useRoc=0;
	}
      }else if ( fMod->getTrigBC() < fRoc->getTrigBC() ){
	// unsync'd, module only
	readMod=1; readRoc=0; useRoc=0;
	if (mStat==1) {
	  useMod=1;
	  fnModuleOnly++;
	}else{
	  useRoc=0;
	}	  
	// keep track of the TBM event counter offset
	if(dtbmtrig>=0){
	  dtbmtrig--; if(dtbmtrig==-1){ dtbmtrig=255;}
	}
      }else if ( fMod->getTrigBC() > fRoc->getTrigBC() ){
	// unsync'd, roc only
	readMod=0; useMod=0; readRoc=1;
	if(rStat==1){
	  useRoc=1;
	  fnRocOnly++;
	}else{
	  useRoc=0;
	}
	// keep track of the TBM event counter offset
	if(dtbmtrig>=0){
	  dtbmtrig++; if(dtbmtrig>255){ dtbmtrig=0;}
	}
      }
    }else if (bMod){
      if(mStat==1){
	readMod=1; useMod=1; readRoc=0; useRoc=0;
      }else{
	readMod=1; useMod=0; readRoc=0; useRoc=0;
      }
    }else if (bRoc){
      if(rStat==1){
	readMod=0; useMod=0; readRoc=1; useRoc=1;
      }else{
	readMod=0; useMod=0; readRoc=1; useRoc=0;
      }
    }else{
      // neither module nor roc, what are we doing here?????
      cout << "EventReader::loop> nothing to be read, bailing out" << endl;
      exit(1);
    }
    
    
    if(fVerbose && fMod && fRoc){
      cout << "EventReader> ";
      if(readMod){
	char cal=' ';
	if (fMod->getTBMStatus()&0x02){ cal='c';}
	cout << Form("Module:%10lld(x%2x)  TBM=%3x%C  ",
		     fMod->getTrigBC(),fMod->getTrigType(),fMod->getTBMTrigger(),cal);
      }else{
	cout << "                                  ";
      }
      if(readRoc){
	char cal=' ';
	if (fRoc->getTBMStatus()&0x02){ cal='c';}
	cout << Form("ROC:%10lld(x%2x)  TBM=%3x%C",
		     fRoc->getTrigBC(),fRoc->getTrigType(),fRoc->getTBMTrigger(),cal);
      }else{
	cout << "                       ";
      }
      cout << endl;
    }
    
    // get hits/clusters and sort them by layer
    for(int i=0; i<5; i++){fHits[i].clear();  }
    
    double xl[3];
    int nCluMod, nCluRoc;
    if(fMod && useMod){
      int nhmod=fMod->getNHit();
      vector<cluster> mclu = fMod->getHits();
      hNPixMod->Fill(nhmod);
      nCluMod=mclu.size();
      hNCluMod->Fill(mclu.size());
      int nHitTracking=0;
      int nHitModRoc[16]={0};
      for(vector<cluster>::iterator c=mclu.begin(); c!=mclu.end(); c++){
	xl[0]=(*c).xy[0]; xl[1]=(*c).xy[1];  xl[2]=0;// set local z=0
	fPlane[(*c).layer].localToGlobal(xl, (*c).xyz);
	fHits[(*c).layer].push_back((*c));
	hQCluLayer[c->layer]->Fill(c->charge);
	hSizeCluLayer[c->layer]->Fill(c->size);
	if( (xl[0]>-0.3) && (xl[0]<0.3) && (xl[1]>-1.0) && (xl[1]<-0.3) ){
	  nHitTracking++;
	}
	int r=c->vpix.begin()->roc;
	int d=c->vpix.begin()->colROC/2;
	nHitModRoc[r]++;
       if( fMod->getTrigType() == BinaryFileReader::kExternalTrigger){
	 fNCluModDcol[r][d]++;
	 fNCluModSum++;
       }
      }
      if( fMod->getTrigType() == BinaryFileReader::kExternalTrigger){
	fNCluModEvt++;
	hNCluTracking->Fill(nHitTracking);
	for(int i=0; i<16; i++) hNCluModRocExt[i]->Fill(nHitModRoc[i]);
      }else{
	for(int i=0; i<16; i++) hNCluModRocInt[i]->Fill(nHitModRoc[i]);
      }
    }
    
    
    if(fRoc && useRoc){
      int nhroc=fRoc->getNHit();
      vector<cluster> rclu=fRoc->getHits();
      nCluRoc=rclu.size();
      hNPixRoc->Fill(nhroc);
      
      hNCluRoc->Fill(rclu.size());
      int nc[4]={0};
      int nShadow=0;
      for(vector<cluster>::iterator c=rclu.begin(); c!=rclu.end(); c++){
	if(c->charge>fQminTrk){nc[c->layer]++;}
	hQCluLayer[c->layer]->Fill(c->charge);
	hSizeCluLayer[c->layer]->Fill(c->size);
	if( (c->layer==2) && (c->charge>fQminTrk) &&
	    (c->col>=fShadowColMin) && (c->col<=fShadowColMax) &&
	    (c->row>=fShadowRowMin) && (c->row<=fShadowRowMax) ){
	  nShadow++;
	}
      }
      hNCluRoc1->Fill(nc[0]);
      hNCluRoc2->Fill(nc[1]);
      hNCluRoc3->Fill(nc[2]);
      hNCluRoc4->Fill(nc[3]);
      if(fRoc->getTrigType() == BinaryFileReader::kExternalTrigger){
	hNCluShadowExt->Fill(nShadow);
      }else if(fRoc->getTrigType() == BinaryFileReader::kInternalTrigger){
	hNCluShadowInt->Fill(nShadow);
      }else if(fRoc->getTrigType() == BinaryFileReader::kCalInject){
	hNCluShadowCal->Fill(nShadow);
      }
      
      
      if(nCluRoc==0){
	if(fMod && useMod){
	  for(vector<cluster>::iterator c=fHits[4].begin(); c!=fHits[4].end(); c++){
	    hRocEmpty->Fill(c->xy[0], c->xy[1]);
	  }
	}
      }
      
      for(vector<cluster>::iterator c=rclu.begin(); c!=rclu.end(); c++){
	xl[0]=(*c).xy[0]; xl[1]=(*c).xy[1];  xl[2]=0; // set local z=0
	fPlane[(*c).layer].localToGlobal(xl, (*c).xyz);
	if( isnan((*c).xyz[0])) cout <<"zeter und mordio "<< xl[0] << " " << xl[1] << endl;
	fHits[(*c).layer].push_back((*c));
      }
    }
    
    
    
    if( fMod && useMod && fRoc && useRoc){
      
      hNCluCorr->Fill(nCluRoc, nCluMod);
      hNPixCorr->Fill(fRoc->getNHit(),fMod->getNHit());
      if (fAlignmentFile){dumpHits(fAlignmentFile);}
      time=fMod->getTrigBC();
      dTime=time-oldTime;
      oldTime=time;
      hNPixD->Fill(time/40.e6);
      if(oldTime>0) hDeltaT->Fill(dTime);
      hDeltaTt->Fill((float)(fRoc->getBC()-fRoc->getTrigBC()));
      // check tbm event counter
      int dtbm=fRoc->getTBMTrigger()-fMod->getTBMTrigger();		if(dtbm<0){dtbm+=256;}
      if(!(dtbm==dtbmtrig)){
	if(!(dtbmtrig==-1)){
	  cout << "!!! Warning !!! :  TBM trigger count offset changed from " << dtbmtrig << " to " << dtbm << endl;
	}
	dtbmtrig=dtbm;
      }
      
      fv=NULL;
      findTracks();
    }
    
    
    
    if( fMod && readMod ) mStat=fMod->readDataEvent();
    if( fRoc && readRoc ) rStat=fRoc->readDataEvent();
    more=( mStat | rStat );
    
  }// event loop
  if (fAlignmentFile) {fAlignmentFile->close();}
}//void loop()

/************************************************************/


void EventReader::printRunSummary(){
  if(fRoc){
	 cout << endl << endl << fTtbfile << endl;
	 fRoc->printRunSummary();
  }
  if (fMod){
	 cout << endl << endl << fMtbfile << endl;
	 fMod->printRunSummary();
  }
  
  cout << "------------------------------------------------------------------"<<endl;
  cout << "synchronized events: " << fnSync 
		 << "    roc only: " << fnRocOnly 
		 << "    module only: " << fnModuleOnly 
		 <<endl;
  cout << "------------------------------------------------------------------"<<endl;

}

/************************************************************/

void EventReader::dumpHits(ofstream *f){

  fnAlignment++;
  int nevent=0;
  for(int i=0; i<5; i++){	 nevent+=fHits[i].size();  }
  (*f) << fnAlignment << " " << nevent << endl;

  for(int i=0; i<5; i++){
    for(vector<cluster>::iterator c=fHits[i].begin(); c!=fHits[i].end(); c++){
      *f << Form("%2d  %10f %10f %10f",
		 (*c).layer,	(*c).xyz[0]*10,(*c).xyz[1]*10,(*c).xyz[2]*10) << endl;		
    }
  }
}

/************************************************************/
void EventReader::findTracks(){
  // 
  double par[4];
  double cov[4*4];
  double n[3];
  double x0[3];
  // loop over hits in seed layers to form track candidates
  int nSeedPair=0;
  fNFound=0;
  vector<cluster>::iterator c1,c2,c3,c4;
  for(c1=fHits[fSeed1].begin(); c1!=fHits[fSeed1].end(); c1++){

	 //if( abs((*c1).xy[0])>0.3) break;
	 //if( abs((*c1).xy[1])>0.3) break;
	 for(c2=fHits[fSeed2].begin(); c2!=fHits[fSeed2].end(); c2++){
		//if( abs((*c2).xy[0])>0.3) break;
		//if( abs((*c2).xy[1])>0.3) break;

		nSeedPair++;
		// start with a straight line through both seeds
		double dx=(*c2).xyz[0]-(*c1).xyz[0];
		double dy=(*c2).xyz[1]-(*c1).xyz[1];
		double dz=(*c2).xyz[2]-(*c1).xyz[2];
		x0[0]=( (*c1).xyz[0]*(*c2).xyz[2] - (*c2).xyz[0]*(*c1).xyz[2] )/dz; // x @ z=0
		x0[1]=( (*c1).xyz[1]*(*c2).xyz[2] - (*c2).xyz[1]*(*c1).xyz[2] )/dz; // y @ z=0
		x0[2]=0;
		//		cout << Form("par  %10f  %10f",par[0],par[1]) << endl;
		par[0]=x0[0];
		par[1]=x0[1];
		par[2]=dx/dz;   
		par[3]=dy/dz;
		n[0]=dx/sqrt(dx*dx+dy*dy+dz*dz);
		n[1]=dy/sqrt(dx*dx+dy*dy+dz*dz);
		n[2]=dz/sqrt(dx*dx+dy*dy+dz*dz);

		// look for nearby hits in both tracking layers
		int TracksPerSeedPair=0;

		double xl3[3];
		fPlane[fTLayer1].interceptLocal(x0, n,xl3);

		for(c3=fHits[fTLayer1].begin(); c3!=fHits[fTLayer1].end(); c3++){

		  double dx3=(*c3).xy[0]-xl3[0];
		  double dy3=(*c3).xy[1]-xl3[1];
		  double dist3=hypot(dx3, dy3 );
		  double phi3=atan2((*c3).xy[0], (*c3).xy[1]);


		  double xl4[3];
		  fPlane[fTLayer2].interceptLocal(x0,n,xl4);
		  int n4=0;
		  
		  for(c4=fHits[fTLayer2].begin(); c4!=fHits[fTLayer2].end(); c4++){

			 double dx4=(*c4).xy[0]-xl4[0];
			 double dy4=(*c4).xy[1]-xl4[1];				
			 double dist4=hypot(dx4, dy4 );
			 double phi4=atan2((*c4).xy[0], (*c4).xy[1]);
			 if( dist4 < fSearchRadius*0.5){n4++;}

			 if( dist3  < fSearchRadius*0.5){
				hDistTLayer2->Fill(dist4);
				hDxTLayer2->Fill(dx4);
				hDyTLayer2->Fill(dy4);
				hDxDyTLayer2->Fill(dx4,dy4);
				hDxPhiTLayer2->Fill(dx4,phi4);
				hDyPhiTLayer2->Fill(dy4,phi4);
			 }
				
			 if(( dist4 < fSearchRadius)&&(dist3<fSearchRadius) ){
				// we have found a four hit candidate, fit it
				double chisq = fitTrack(c1,c2,c3,c4,par,cov);
				hFitChisq->Fill(chisq);
				//if ( (chisq<fChiSqCut)&&(par[1]>(-0.7+0.64))&&(par[0]>-0.04) ){
				if ( (chisq<fChiSqCut)&&(par[1]>(-0.7+0.64)) ){
				//if ( chisq<fChiSqCut){
				  TracksPerSeedPair++;
				  goodTrack(par, 0);
				  hScint->Fill(par[0]+fzScint*par[2], par[1]+fzScint*par[3]);
				}
			 } // c4 in road

		  }//c4 loop
			 
		  // only fill c3 histos when c4-confirmed
		  if(n4>0){
			 hDistTLayer1->Fill(dist3);
			 hDxTLayer1->Fill(dx3);
			 hDyTLayer1->Fill(dy3);
			 hDxDyTLayer1->Fill(dx3,dy3);
			 hDxPhiTLayer1->Fill(dx3,phi3);
			 hDyPhiTLayer1->Fill(dy3,phi3);
		  }

		}//c3 loop
		hNTracksPerSeed->Fill(TracksPerSeedPair);

	 }//c2 loop
  }//c1 loop
  hNSeedPair->Fill(nSeedPair);
  
}


double EventReader::fitTrack(vector<cluster>::iterator c1, 
			     vector<cluster>::iterator c2,
			     vector<cluster>::iterator c3,
			     vector<cluster>::iterator c4,
			     double* par, double* cov)
{
  double x[4],y[4],z[4];
  x[0]=(*c1).xyz[0];  y[0]=(*c1).xyz[1];  z[0]=(*c1).xyz[2];
  x[1]=(*c2).xyz[0];  y[1]=(*c2).xyz[1];  z[1]=(*c2).xyz[2];
  x[2]=(*c3).xyz[0];  y[2]=(*c3).xyz[1];  z[2]=(*c3).xyz[2];
  x[3]=(*c4).xyz[0];  y[3]=(*c4).xyz[1];  z[3]=(*c4).xyz[2];
  double chisqx=lineFit(z,x,4,&par[2], &par[0]);
  double chisqy=lineFit(z,y,4,&par[3], &par[1]);
  const double sigmax=0.013;
  const double sigmay=0.013;
  return chisqx/(sigmax*sigmax)+chisqy/(sigmay*sigmay);
}


/* try a straight line fit through the n points in arrays x,y */
/* returns the chisquare of the fit, negative if fit failed */
/*double lineFit(double* x, double* y, int n, double* slope, double* offset){*/
double EventReader::lineFit(double* x, double* y, int n, double* slope, double* offset)
{
  double s = 0;
  double sx = 0;
  double sxx = 0;
  double sy = 0;
  double sxy = 0;
  double det = 0;
  double chisquare = 0;
  int j;

  s=0; sx = 0; sxx=0; sy=0; sxy=0;

   for(j=0; j<n; j++){
     s += 1;
     sx += x[j];
     sxx += x[j]*x[j];
     sy += y[j];
     sxy += y[j]*x[j];
   }
   det = s*sxx - sx*sx;
   if (det == 0){ return (-1);}
   *offset = (sy*sxx - sxy*sx)/det;
   *slope = (sxy*s - sy*sx)/det;

   chisquare = 0;
   for(j=0; j<n; j++){
     chisquare += (*offset + *slope*x[j]-y[j])*(*offset + *slope*x[j]-y[j]);
   }
	return chisquare;
}



void EventReader::goodTrack(double* par, int bg){
// called for good/selected tracks, 
// do the accounting for efficiency and such

  int nHit1=0;
  int nHit2=0;

  double xl[3];
  double x0[3];
  double n[3];
  x0[0]=par[0];
  x0[1]=par[1];
  x0[2]=0;
  n[0]=par[2]/sqrt(1+par[2]*par[2]+par[3]*par[3]);//sint*cos(par[2]);
  n[1]=par[3]/sqrt(1+par[2]*par[2]+par[3]*par[3]);//sint*sin(par[2]);
  n[2]=     1/sqrt(1+par[2]*par[2]+par[3]*par[3]);//cost;
  fPlane[fDUTLayer].interceptLocal(x0,n,xl);
  int roc,col,row;
  if(fGeometry->getModColRow(xl,roc,col,row)){
    fNTrkModDcol[roc][col/2]++;
    fNTrkModSum++;
  }
  

  for(vector<cluster>::iterator c=fHits[fDUTLayer].begin(); 
		c!=fHits[fDUTLayer].end(); c++){
	 double dx=(*c).xy[0]-xl[0];
	 double dy=(*c).xy[1]-xl[1];
	 double dr=hypot(dx,dy);
	 double phi=atan2((*c).xy[0], (*c).xy[1]);
	 hDistDUT->Fill(dr);
	 hDxDUT->Fill(dx);
	 hDyDUT->Fill(dy);
	 hDxDyDUT->Fill(dx,dy);
	 hDxPhiDUT->Fill(dx,phi);
	 hDyPhiDUT->Fill(dy,phi);
	 hDxSx->Fill(n[0],dx);
	 hDySy->Fill(n[1],dy);
	 hDUTh2->Fill(dx,dy);
	 hDUTh1->Fill(dr);
	 hDUTh1a->Fill(dr*dr);
	 if(dr<fCountingRegion1){
		nHit1++;
	 }
	 if((dr<fCountingRegion1)&&(c->charge>fQminEff)){
		nHit2++;
	 }
  }
  hDUTnH1->Fill(nHit1);
  hDUTnH2->Fill(nHit2);
  if(nHit1==0){
	 hMissing->Fill(xl[0],xl[1]);
	 if(!fv) fv=addEventView();
  }else{
	 hFound->Fill(xl[0],xl[1]);
	 fNFound++;
  }
  if(fv){ fv->addTrack(par); } // event display
}	 
		


EventView* EventReader::addEventView(){
  pixel* pb;
  EventView* v=NULL;
  if(vev.size()<100){
	 v=new EventView();
	 for(int i=0;  i<5; i++){v->fPlane[i]=&fPlane[i];}
	 //v->fPlane[i]=&fPlane[i];

	 if(fRoc){
		pb=fRoc->getPixels();
		int np=fRoc->getNHit();
		for(int i=0; i<np; i++){
		  double w=0.0150;
		  if((pb[i].colROC==0)||(pb[i].colROC==51)){w=0.0300;}
		  double h=0.0100;
		  if(pb[i].rowROC==80){h=0.0200;}
		  v->addPixel(pb[i].roc, pb[i].xy[0], pb[i].xy[1],w,h);
		  //v->addPixel(pb[i].roc, pb[i].col, pb[i].row, pb[i]);
		}
	 }
	 if(fMod){
		pb=fMod->getPixels();
		for(int i=0; i<fMod->getNHit(); i++){
		  double w=0.0150;
		  if((pb[i].colROC==0)||(pb[i].colROC==51)){ w=0.0300;}
		  double h=0.0100;
		  if(pb[i].rowROC==80){h=0.0200;}
		  v->addPixel(4, pb[i].xy[0], pb[i].xy[1],h,w);
		  //v->addPixel(4, pb[i].col, pb[i].row);
		}
	 }
	 vev.push_back(v);
  }
  return v;
}


EventView* EventReader::getEventView(unsigned int n){
  if(n<vev.size()){
	 return vev[n];
  }else{
	 return NULL;
  }
}

