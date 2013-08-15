#include "TApplication.h"
#include "TCanvas.h"
#include "TLine.h"
#include "TPaveLabel.h"
#include "TF1.h"
#include "TH1F.h"
#include "TH2F.h"
#include "TSpectrum.h"
#include "TStyle.h"
#include "TFile.h"


#include "BinaryFileReader.h"
#include "Viewer.h"
#include "LangauFitter.h"
#include "TGClient.h"
#include "PHCalibration.h"
#include<stdio.h>
#include <iostream>
#include <fstream>
#include <vector>

// pixel hit and cluster struct
#include "pixelForReadout.h"

using namespace std;
TCanvas* gCanvas;
float greg[5]={0.,0.,0.,0.,0.};//counting region: col_min col_max row_min row_max print_region
int greg_shape=0; //shape counting region: 0=elliptical; 1=square
int gClucut=1; // cluster search radius (i.e. allows gClucut-1 empty pixels); WE 4
enum CountingRegion_t {kNormalRegion, kLargeRegion, kSmallRegion, kOutsideRegion, kEntireRegion, kSpecialRegion} gRegion;
float gTbin=0.2;
float fReset=1000.;


#include "rundb.h"
 
/*****************************************************************************************
 *       program for offline analysis of 2005 testbeam data                              *
 *                                                                                       *
 *  usage:                                                                               *
 *        ./r -r <run-number>  [options]                                                 *
 *  or                                                                                   *
 *        ./r <filename>                                                                 *
 *  Options:                                                                             *
 *     -roc                         read roc datafile from run directory (if existing)   *
 *     -v                           verbose mode                                         *
 *     -b                           don't pop up histogram window                        *
 *     -c                           use pulseheight calibration                          *
 *     -l                           bootstrap address levels (re-run without -l later)   *
 *     -ed                          ?                                                    *
 *     -large                       counting reg depends on run too                      *
 *     -small                       counting reg depends on run too                      *
 *     -outside                     counting reg outside shadow                          *
 *     -entire                      counting reg entire module                           *
 *     -reg <cmin cmax rmin rmax>   colmin colmax rowmin rowmax of fiducial region       *
 *     -mua                         correction for ?                                     *
 *     -clu <radius>                cluster search radius (radius-1 empty pixels)        *
 *                                                                                       *
 *                                                                                       *
 *   address level decoding is based on the files                                        *
 *         levels-module.dat                                                             *
 *  and    levels-roc.dat                                                                *
 *                                                                                       *
 *  which are in the data directory of the run                                           *
 *  if those files are not present, the files in the current working                     *
 *  directory are used and new files are created and placed in the                       *
 *  data directory.                                                                      *
 *  It may then be necessary to re-run the code to get correct                           *
 *  level/address identification only in the second pass                                 *
 *                                                                                       *
 *                                                                                       *
 *  Efficiencies can be calculated with or without a background                          *
 *  run. The regions for signal hit counting are defined in                              *
 *  the function   "float fiducial(...)" which must be updated                           *
 *  when the position/shape of the scintillator "shadow" moves.                          *
 *                                                                                       *
 *                                                                                       *
 *****************************************************************************************/




/************************************************************/


// simple clusterization
vector<cluster> clus(int n, pixel* apix){
  const int u=gClucut; // cluster search radius (i.e. allows u-1 empty pixels)
  const double anaOffs=-500;
  vector<cluster> v;
  if(n==0) return v;
  int* gone = new int[n];
  for(int i=0; i<n; i++) gone[i]=0;
  int seed=0;
  while(seed<n){
    // start a new cluster
    cluster c;
    c.vpix.push_back(apix[seed]); gone[seed]=1;
    c.charge=0.; c.size=0; c.col=0; c.row=0;
    // let it grow as much as possible
    int growing;
    do{
      growing=0;
      for(int i=0; i<n; i++){
		  if(!gone[i]){
			 for(unsigned int p=0; p<c.vpix.size(); p++){
				int dr = c.vpix.at(p).row - apix[i].row;
				int dc = c.vpix.at(p).col - apix[i].col;
				if((dr>=-u)&&(dr<=u)&&(dc>=-u)&&(dc<=u)){
				  c.vpix.push_back(apix[i]); gone[i]=1;
				  growing=1;
				  break;//important!
				}
			 }
		  }
      }
    }while(growing);
    // added all I could. determine position and append it to the list of clusters
    for(  vector<pixel>::iterator p=c.vpix.begin();  p!=c.vpix.end();  p++){
		double Qpix=(*p).anaVcal+anaOffs;
      c.charge+=Qpix;
		c.col+=(*p).col*Qpix;
      c.row+=(*p).row*Qpix;
    } 
    c.size=c.vpix.size();
    c.col=c.col/c.charge;
    c.row=c.row/c.charge;
    v.push_back(c);
    //look for a new seed
    while((++seed<n)&&(gone[seed]));
  }
  // nothing left,  return clusters
  return v;
}


/************************************************************/

void clutest(vector<cluster> vclu, TH1F* hsep, TH1F* hsep2){
  //cluster distances
    for(  vector<cluster>::iterator c1=vclu.begin();  c1!=vclu.end();  c1++){
		for(  vector<cluster>::iterator c2=c1+1;  c2!=vclu.end();  c2++){
		  float dc=(*c1).col-(*c2).col;
		  float dr=(*c1).row-(*c2).row;
		  hsep->Fill(sqrt(dc*dc+dr*dr));
		  if((*c2).charge<100){
			 hsep2->Fill(sqrt(dc*dc+dr*dr));
		  }

		  if(sqrt(dc*dc+dr*dr)<0){
			 cout << "split cluster?   distances=(" << dc << "," << dr << ")" <<endl;
			 for( vector<pixel>::iterator p=(*c1).vpix.begin();  p!=(*c1).vpix.end();  p++){
				cout << Form("(%3d,%3d,%4.0f)",(*p).col,(*p).row,(*p).anaVcal);
			 }
			 cout<< endl;
			 for( vector<pixel>::iterator p=(*c2).vpix.begin();  p!=(*c2).vpix.end();  p++){
				cout << Form("(%3d,%3d,%4.0f)",(*p).col,(*p).row,(*p).anaVcal);
			 }
			 cout<< endl;
		  }

		}
	 }
}


/************************************************************/


/* conversion of local roc coordinates to module coordinates */
void toGlobal(int module, int n, pixel* p){
  for (int i=0; i<n; i++){
    if(module){
      if(p[i].roc<8){
	p[i].row=159-p[i].rowROC;
	p[i].col=p[i].roc*52 + p[i].colROC;
      }else{
	p[i].row=p[i].rowROC;
	p[i].col=(16-p[i].roc)*52-p[i].colROC-1;
      }
    }else{
      p[i].row=p[i].rowROC;
      p[i].col=p[i].colROC;
    }
  }
}


/************************************************************/

/* define the counting region */
float fiducial(int module, int run, pixel* p,TH1F* hist){
  float rminModule,rmaxModule, cminModule,cmaxModule;
  if((run<2170)||(run>2225&&run<2356)){
	 rminModule=20; 
	 rmaxModule=160;
	 cminModule=110;
	 cmaxModule=240;
	 if(gRegion==kSmallRegion){
		rminModule=40;
		rmaxModule=140;
		cminModule=140;
		cmaxModule=210;
	 }else if(gRegion==kLargeRegion){
		rminModule=0;
		rmaxModule=180;
		cminModule= 80;
		cmaxModule=270;
	 }else if(gRegion==kOutsideRegion){
		rminModule=40;
		rmaxModule=140;
		cminModule=340;
		cmaxModule=410;
	 }else if(gRegion==kEntireRegion){
		rminModule=0;
		rmaxModule=160;
		cminModule=0;
		cmaxModule=416;
	 }else if(gRegion==kSpecialRegion){
		rminModule=greg[2];
		rmaxModule=greg[3];
		cminModule=greg[0];
		cmaxModule=greg[1];
	 }
  }else if(run>2535){
		rminModule=40;
		rmaxModule=120;
		cminModule=150;
		cmaxModule=220;
	 if(gRegion==kSmallRegion){
		rminModule=50;
		rmaxModule=110;
		cminModule=170;
		cmaxModule=200;
	 }else if(gRegion==kLargeRegion){
		rminModule=20;
		rmaxModule=140;
		cminModule=100;
		cmaxModule=270;
	 }else if(gRegion==kOutsideRegion){
		rminModule=40;
		rmaxModule=120;
		cminModule=340;
		cmaxModule=410;
	 }else if(gRegion==kEntireRegion){
		rminModule=0;
		rmaxModule=160;
		cminModule=0;
		cmaxModule=416;
	 }else if(gRegion==kSpecialRegion){
		rminModule=greg[2];
		rmaxModule=greg[3];
		cminModule=greg[0];
		cmaxModule=greg[1];
	 }
  }else{ // 45 degree beamspot 
    rminModule=20;
    rmaxModule=160;
    cminModule=90;
    cmaxModule=230;
  }

  if (greg[4]==0) {
    greg[4]=1;
    if (greg_shape==1) {
      cout << "square ";
    }
    else if (greg_shape==0) {
      cout << "elliptical ";
    }
    switch (gRegion) {
    case kNormalRegion: cout << "fiducial region (Normal): rmin=" << rminModule << " rmax=" << rmaxModule 
			     << " cmin=" << cminModule << " cmax=" << cmaxModule << " cluster=" << gClucut << endl; break;
    case kLargeRegion: cout << "fiducial region (Large): rmin=" << rminModule << " rmax=" << rmaxModule 
			    << " cmin=" << cminModule << " cmax=" << cmaxModule << " cluster=" << gClucut << endl; break;
    case kSmallRegion: cout << "fiducial region (Small): rmin=" << rminModule << " rmax=" << rmaxModule 
			    << " cmin=" << cminModule << " cmax=" << cmaxModule << " cluster=" << gClucut << endl; break;
    case kOutsideRegion: cout << "fiducial region (Outside): rmin=" << rminModule << " rmax=" << rmaxModule 
			      << " cmin=" << cminModule << " cmax=" << cmaxModule << " cluster=" << gClucut << endl; break;
    case kEntireRegion: cout << "fiducial region (Entire): rmin=" << rminModule << " rmax=" << rmaxModule 
			     << " cmin=" << cminModule << " cmax=" << cmaxModule << " cluster=" << gClucut << endl; break;
    case kSpecialRegion: cout << "fiducial region (Special): rmin=" << rminModule << " rmax=" << rmaxModule 
			      << " cmin=" << cminModule << " cmax=" << cmaxModule << " cluster=" << gClucut << endl; break;
    }
  }    

  greg[0]=cminModule;
  greg[1]=cmaxModule;
  greg[2]=rminModule;
  greg[3]=rmaxModule;

  float c0Module=(cmaxModule+cminModule)/2.;
  float r0Module=(rmaxModule+rminModule)/2.;
  float wcModule=(cmaxModule-cminModule)/2.;
  float wrModule=(rmaxModule-rminModule)/2.;

  if(module){
    if (greg_shape==0) {
      //    if(p->roc!=12) return 0;
      float dc=(p->col-c0Module)/wcModule;
      float dr=(p->row-r0Module)/wrModule;
      float r=sqrt(dr*dr+dc*dc);
      if( hist!=0 ) hist->Fill(r);
      if(r<1.){ 
	return 1.;
      }else{
	return 0.;
      }
    }
    else if (greg_shape==1) {
      float dc=(p->col-c0Module)/wcModule;
      float dr=(p->row-r0Module)/wrModule;
      float r=sqrt(dr*dr+dc*dc);
      //      float r;
      //      if (dc < 1 && dr < 1) {
      //	r = 0.9;
      //      } else {
      //	r = 1.;
      //      }
      if( hist!=0 ) hist->Fill(r);
      dc=dc*wcModule+c0Module;
      dr=dr*wrModule+r0Module;
      //      if(((cminModule <= dc) && (dc < cmaxModule)) && ((rminModule <= dr) && (dr < rmaxModule))){ 
      if(((cminModule < dc) && (dc < cmaxModule)) && ((rminModule < dr) && (dr < rmaxModule))){ 
	return 1.;
      }else{
	return 0.;
      }
    }
  }else{//roc
    if((p->row>15)&&(p->row<55)&&(p->col>10)&&(p->col<40)){
      return 1;
    }else{
      return 0;
    }
  }
  cout << " warning : fiducial reached end of function" << endl;
  return 1; // doesn't compile without
}


/************************************************************/


void display (int run, int event, int nh, pixel * p, vector<cluster> a){
  char hname[100];
  char htitle[100];
  sprintf(hname,"r%de%d",run,event);
  sprintf(htitle,"run %d  event %d",run,event);
  TH2F *h = new TH2F(hname,htitle,160,0,160,416,0,416);
  for(int i=0; i< nh; i++){
	 //h->Fill(p[i].row, p[i].col);
	 h->Fill(p[i].row, p[i].col, p[i].anaVcal>0 ? p[i].anaVcal : 1.);
  }
  gStyle->SetOptStat(kFALSE);
  h->Draw("colz");  gPad->Update();
  /*
  char junk[256];
  cout << "hit return to continue ";
  cin.getline(junk,256);
  cout << "you typed " << junk << endl;
  if(junk[0]=='p') gCanvas->Print("event.ps");
  if(junk[0]=='g') gCanvas->Print("event.gif");
  */
}

/************************************************************/

void printBC(BinaryFileReader* r, int doLF=1){
  cout << " at BC="<<r->getBC() << Form("(%6.3f s)",r->getBC()/40.e6);
  if (doLF) cout << endl;
}
/************************************************************/

// run statistics to be collected in the event loop
struct runStat {
  int nro;
  int nyes;
  int nyesFid;
  int nclu;
  int ncluFid;
  int ncluOut;
  int n0Fid;
  int n1Fid;
  int n2Fid;
  int n3Fid;
  int n4Fid;
  int nf_max;
  int ntrig;
  int maxdrotrig[100];
};

// histograms to be filled in the event loop
struct runHistos{
  TH1F* NPix;
  TH1F* NPixD; //ul
  TH1F* NPixT; //ul
  TH1F* DeltaT; //ul
  TH1F* DeltaTt;
  TH1F* DeltaTtlog;
  TH1F* DeltaTRO;
  TH2F* hitmap;
  TH2F* hitmap2;
  TH2F* noisemap;
  TH1F* mapRow;
  TH1F* mapRow312;
  TH1F* mapRowOther;
  TH1F* mapCol;
  TH1F* mapCol07;
  TH1F* mapCol815;
  TH1F* DColHits;
  TH1F* Fid;
  TH1F* FidClu;
  TH1F* NClu;
  TH1F* NCluFid;
  TH1F* SizeClu;
  TH1F* ChargeClu;
  TH1F* cluMapRow;
  TH1F* cluMapCol;
  TH1F* dcolSlice1;
  TH1F* dcolSlice2;
  TH1F* dcolSlice3;
  static const int ntbin=100;
  static const float wtbin=0.2;
  TH1F* NCluTime[ntbin];
  TH1F* NTTime[ntbin];
  TH1F* NCluTT[ntbin];
  TH1F* NCluTR[ntbin];
  TH1F* CluSep;
  TH1F* CluSep2;
  TH2F* dcolHistory;
  TH2F* dcolHistory07;
  TH2F* dcolHistory815;
};

// event loop
int loop(BinaryFileReader* f, int run, int module, int bg, int verbose,
	  runStat& stat, runHistos& h){

  //ul  h.NPix = new TH1F("npix","# pixels",20,-0.5,19.5);
  h.NPix = new TH1F("npix","# pixels",100,-0.5,99.5);
  h.NPixD = new TH1F("npixd","# readouts vs time",10000,0.,100.); //ul
  h.NPixT = new TH1F("npixt","trig vs time",10000,0.,100.); //ul
  h.DeltaT = new TH1F("deltat","time between triggers",5000,-0.5,4999.5); //ul
  h.DeltaTt = new TH1F("deltatt","time between trigger and data",500,-0.5,499.5);
  h.DeltaTtlog = new TH1F("deltattlog","time between trigger and data",100,0,6);
  h.DColHits = new TH1F("dcolhits","double column hits",
			     26,0,26);
  h.Fid = new TH1F("fid","fiducial",100,0.,10.);

  h.NClu = new TH1F("nclu","# clusters",100,-0.5,99.5);
  h.SizeClu = new TH1F("sizeclu","cluster size",20,-0.5,19.5);
  h.ChargeClu = new TH1F("chargeclu","cluster charge",150,0.,1500);
  h.NCluFid = new TH1F("nclufid","#clusters",20,-0.5,19.5);
  h.FidClu = new TH1F("fidclu","cluster fiduciality",100,0.,10.);
  for(int i=0; i<h.ntbin; i++){
	 h.NCluTime[i] = new TH1F(Form("nct%d",i),Form("nct%d",i),20,-0.5,19.5);
	 h.NTTime[i] = new TH1F(Form("ntt%d",i),Form("ntt%d",i),20,-0.5,19.5);
	 h.NCluTT[i] = new TH1F(Form("nctt%d",i),Form("nctt%d",i),20,-0.5,19.5);
	 h.NCluTR[i] = new TH1F(Form("nctr%d",i),Form("nctr%d",i),20,-0.5,19.5);
  }
  h.CluSep = new TH1F("clusep","cluster distance",100,0., 100.);
  h.CluSep2 = new TH1F("clusep2","cluster distance, small charge",100,0., 100.);

  if(module){
    h.hitmap = new TH2F("hitmap","hitmap",160,0,160,416,0,416);
    h.hitmap2 = new TH2F("hitmap2","hitmap ncluster in fid > 2",160,0,160,416,0,416);
    h.noisemap = new TH2F("noisemap","noisemap",160,0,160,416,0,416);
    h.mapRow = new TH1F("hitmapRow","rowmap all",160,0,160);
    h.mapRow312 = new TH1F("hitmapRow312","rowmap roc 3+12",160,0,160);
    h.mapRowOther = new TH1F("hitmapRowOther","rowmap roc w/o 3+12",160,0,160);
    h.mapCol = new TH1F("hitmapCol","column map all rocs",416,0,416);
    h.mapCol07 = new TH1F("hitmapCol07","column map roc 0-7",416,0,416);
    h.mapCol815 = new TH1F("hitmapCol815","column map roc 8-15",416,0,416);
    h.cluMapRow = new TH1F("clumapRow","rowmap",160,0,160);
    h.cluMapCol = new TH1F("clumapCol","column map",416,0,416);
	 h.dcolHistory = new TH2F("dcolhist","double column history",200, 0, 200*gTbin,26*16, 0, 26*16);
	 h.dcolHistory07 = new TH2F("dcolhist07","double column history ROC 0-7",200, 0, 200*gTbin,26*8, 0, 26*8);
	 h.dcolHistory815 = new TH2F("dcolhist815","double column history ROC 8-15",200, 0, 200*gTbin,26*8, 0, 26*8);
	 if(run<1959){
		f->setDeconvolution(0.06);  // TBM04
	 }else{
		f->setDeconvolution(0.02);  // TBM05
	 }
  }else{
    h.hitmap = new TH2F("hitmap","hitmap",52,0,52,80,0,80);
    h.noisemap = new TH2F("noisemap","noisemap",52,0,52,80,0,80);
    h.mapCol = new TH1F("hitmapCol","column map",52,0,52);
    h.mapRow = new TH1F("hitmapRow","row map",80,0,80);
    h.cluMapRow = new TH1F("clumapRow","rowmap",80,0,80);
    h.cluMapCol = new TH1F("clumapCol","column map",52,0,52);
	 h.dcolHistory = new TH2F("dcolhist","double column history",200, 0., 200*gTbin,26, 0, 26);
    f->setDeconvolution(0.0);
  }
  h.cluMapCol->SetMinimum(0.);
  h.cluMapRow->SetMinimum(0.);
  h.mapCol->SetMinimum(0.);
  h.mapRow->SetMinimum(0.);
  if(module){
	 h.mapCol07->SetMinimum(0.);
	 h.mapCol815->SetMinimum(0.);
	 h.mapRow312->SetMinimum(0.);
	 h.mapRowOther->SetMinimum(0.);
  }
  h.DColHits->SetMinimum(0.);
  

  h.dcolSlice1 = new TH1F("dcolSlic1","ds1",26,0,26);
  h.dcolSlice2 = new TH1F("dcolSlic2","ds2",26,0,26);
  h.dcolSlice3 = new TH1F("dcolSlic3","ds3",26,0,26);

  if(f->open()){ return 1;}

  pixel pixbuf[1001];

  stat.nro=0;
  stat.nyes=0;
  stat.nyesFid=0;
  stat.nclu=0;
  stat.ncluFid=0;
  stat.ncluOut=0;
  stat.n0Fid=0;
  stat.n1Fid=0;
  stat.n2Fid=0;
  stat.n3Fid=0;
  stat.n4Fid=0;
  stat.nf_max=0;
  stat.ntrig=0;
  int ntrig=0;
  int nreset=0; 
  int novlf=0;
  int drotrig=0;
  int ntp=0;
  for (int i=0; i<h.ntbin; i++) {stat.maxdrotrig[i]=0;}

  long long int oldTime=-1;
  long long int time(0);
  long long int dTime(0);
  long long int tReset=-10000;


  // event loop
  do{
	 int handled=0;
    f->readRecord();
	 //	 if(verbose) f->dump(1);
	 
    time = f->getBC();

    int header=f->getType();
    
    // valid readout 
    if((header==1)||(header==5)||(header==3)){
      handled=1;
      //		if(nreset==0) break;
      // good data events
      stat.nro++;
      int nh=f->getNHit();
      if(nh>0) stat.nyes++;
      h.NPix->Fill(nh);
      time = f->getTrigBC();
      dTime = time - oldTime;
      oldTime = time;
      h.NPixD->Fill(time/40.e6, 1); //ul
      if(oldTime>0) h.DeltaT->Fill(dTime); //ul
      h.DeltaTt->Fill((float)(f->getBC()-f->getTrigBC())); 
		
		//      if((f->getBC()-f->getTrigBC())>28) break;
		
      if(verbose){
		  cout << "data    " << stat.nro;  printBC(f,0);
		  cout << "  TBM " << f->getTBMTrigger() << " ";
		  cout << f->getTBMStatus() << " hits:" << nh << endl;
      }
		
      // hits		
      int nFid=0;
      if(nh<1000){
		  f->getPixels(pixbuf); toGlobal(module, nh, pixbuf);
		  for (int i=0; i<nh; i++){
			 if(module){
				h.hitmap->Fill(pixbuf[i].row, pixbuf[i].col);
				h.dcolHistory->Fill(time/40e6,pixbuf[i].roc*26+pixbuf[i].colROC/2);
				if(pixbuf[i].roc<8) {
				  h.dcolHistory07->Fill(time/40e6,pixbuf[i].roc*26+pixbuf[i].colROC/2);
				}
				if(pixbuf[i].roc>7) {
				  //				  h.dcolHistory815->Fill(time/40e6,((pixbuf[i].roc-8)*26)-(pixbuf[i].colROC/2)+26-1);
				  h.dcolHistory815->Fill(time/40e6,((abs(pixbuf[i].roc-15))*26)+abs(pixbuf[i].colROC/2-25));
				}
			 }else{
				h.hitmap->Fill(pixbuf[i].col, pixbuf[i].row);
				// double column slices 
				if (pixbuf[i].row<10) h.dcolSlice1->Fill(pixbuf[i].col/2);
				if ((pixbuf[i].row>20)&&(pixbuf[i].row<40)) h.dcolSlice2->Fill(pixbuf[i].col/2);
				if (pixbuf[i].row>60) h.dcolSlice3->Fill(pixbuf[i].col/2);
				h.dcolHistory->Fill(time/40e6,pixbuf[i].colROC/2);
			 }
			 h.mapRow->Fill(pixbuf[i].row);
			 h.mapCol->Fill(pixbuf[i].col);
			 if(module){
				if(pixbuf[i].roc<8) h.mapCol07->Fill(pixbuf[i].col);
				if(pixbuf[i].roc>7) h.mapCol815->Fill(pixbuf[i].col);
				if((pixbuf[i].roc==3)||(pixbuf[i].roc==12)){
				  h.mapRow312->Fill(pixbuf[i].row);
				}else{
				  h.mapRowOther->Fill(pixbuf[i].row);
				}
			 }
                         float r=fiducial(module, run, &pixbuf[i], h.Fid);
			 if(r>0) nFid++;
			 if(verbose){
				cout << i << ")" << pixbuf[i].roc << " " << pixbuf[i].colROC   << " " << pixbuf[i].rowROC << endl;
			 }
			 // double column rates
			 if((module==0)||(module==1)&&(pixbuf[i].roc==3)){
				//	    hDColHits->Fill(hitbuf[i*4+1]/2.);
				h.DColHits->Fill(pixbuf[i].colROC/2.);
			 }
		  }
		  
		  // clusters		
		  vector<cluster> a=clus(nh,pixbuf);
		  clutest(a, h.CluSep,h.CluSep2);
		  h.NClu->Fill(a.size());
		  stat.nclu+=a.size();
		  int nf=0;
		  for(unsigned int i=0; i<a.size(); i++){
			 h.SizeClu->Fill(a.at(i).size);
			 h.ChargeClu->Fill(a.at(i).charge);
			 h.cluMapRow->Fill(a.at(i).row);
			 h.cluMapCol->Fill(a.at(i).col);
			 float r=fiducial(module, run, &(a.at(i).vpix.at(0)), h.FidClu);
                         if(r>0){
				nf++;
			 }else{
				stat.ncluOut++;
			 }
			 if(a.at(i).charge<100){
				if(module){
				  h.noisemap->Fill(a.at(i).row,a.at(i).col);
				}else{
				  h.noisemap->Fill(a.at(i).col,a.at(i).row);
				}
			 }
		  }
		  if(gCanvas && (nh>100)) display(run, stat.nro, nh, pixbuf,a);
		  h.NCluFid->Fill(nf);
		  // time histograms
		  int tbin=int(time/40.e6/h.wtbin); // 0.1s time-slices
		  if (tbin<h.ntbin){h.NCluTime[tbin]->Fill(nf);}
		  // time to previous trigger
		  int ttbin=int(TMath::Log10(dTime/4)*10.);
		  if((ttbin>=0)&&(ttbin<h.ntbin)){h.NCluTT[ttbin]->Fill(nf);}
		  // time to previous reset
		  int trbin=int((time-tReset)/40E6*float(h.ntbin)*fReset);
		  if((trbin>=0)&&(trbin<h.ntbin)){h.NCluTR[trbin]->Fill(nf);}
		  
		  stat.ncluFid+=nf;
		  if(nf==0) stat.n0Fid++;
		  if(nf==1) stat.n1Fid++;
		  if(nf==2) stat.n2Fid++;
		  if(nf==3) stat.n3Fid++;
		  if(nf==4) stat.n4Fid++;
		  if (nf > stat.nf_max) {stat.nf_max = nf;}
      }
      if (nFid>0){
		  stat.nyesFid++;
      }
		
    }

      
    if(header==2){ // no  token pass
		handled=1;
      ntp++;
      if(verbose){
		  cout << "no token" << ntp; printBC(f,0);
		  cout << "  TBM " << f->getTBMTrigger() << " ";
		  cout << f->getTBMStatus() << endl;
      }
    }
		
    if((header==4)||(header==5)){
		handled=1;
      ntrig++;
      stat.ntrig++;
      h.NPixT->Fill(time/40.e6, 1); //ul
      if(verbose){
		  cout << "trigger " << ntrig; printBC(f);
      }
    }
		
   if(header==8){
	  handled=1;
	  nreset++;
	  tReset=f->getBC();
	  if(verbose){
		 cout << "reset   " << nreset; printBC(f);
	  }
	}
		
	if(header==0x80){
	  handled=1;
	  novlf++;
	  if(verbose){
		 cout << "overflow   " << novlf;  printBC(f);
	  }
	}
	
	if(handled==0){
	  cout << "exotic header =" << f->getType() << endl;
	}

	drotrig = stat.ntrig - stat.nro;
	int tbin=int(time/40.e6/h.wtbin); // 0.1s time-slices
	if ((tbin < h.ntbin) && (stat.maxdrotrig[tbin] < drotrig)) {
	  stat.maxdrotrig[tbin] = drotrig;
	}
	if ((header==4) || (header==5)) {
	  if (tbin < h.ntbin) {h.NTTime[tbin]->Fill(stat.ntrig);}
	}  
	
  }while((f!=NULL)&&(!f->eof()));


  f->printRunSummary();
  cout << "nevt " << stat.nro << endl;
  cout << "raw      Efficiency " << ((float)stat.nyes)/((float)stat.nro) << endl;
  cout << "fiducial Efficiency " << ((float)stat.nyesFid)/((float)stat.nro) << endl; 
  
  return 0;
}


/****************************************************************/


Double_t fitf(Double_t *x, Double_t *par)
{
  //parameters:
  Double_t eps  =par[0];  // epsilon
  Double_t mu0  =par[1];  // mu_t epsilon + mu_bg
  Double_t muA  =par[2];  // alpha * mu
              // par[3] = number of events
  Double_t gamma=TMath::Exp(-muA);
  Double_t n=int(*x+0.49);
  
  if(muA>0.0001){
    return par[3]*(      TMath::PoissonI(n, mu0         )
		  -gamma*TMath::PoissonI(n, mu0 - muA*eps)
		 )/(1.-gamma);
  }else if( muA < (mu0*eps*0.01))
	 {
		return par[3]*TMath::PoissonI(n,mu0)*(1.0-eps+eps*n/mu0);
	 }
  else  // muA and mu0 << 1
	 {
		switch (int(*x+0.49)) {
		  case 0: return (1-eps)*(1-mu0)+0.5*muA*eps*(1-eps);
		  case 1: return    eps *(1-mu0)-eps*mu0-0.5*muA*eps*(1-2.*eps);
		  case 2: return eps*mu0-0.5*muA*eps*eps;
		}
		return 0;
	 }
}


Double_t fitgg(Double_t *x, Double_t *par)
{
  //parameters:
  Double_t Ntot  =par[0];  // total
  Double_t mean  =par[1];  // position
  Double_t w1    =par[2];  // width of the central gaussian
  Double_t f     =par[3];   // fraction of the second gaussian
  Double_t w2    =par[4];   // width of the second gaussian
 
  Double_t arg=*x-mean;
  Double_t g1=Ntot*(1-f)/sqrt(2*3.14159)/w1 * TMath::Exp(-0.5*arg*arg/w1/w1);
  Double_t g2=Ntot*  f  /sqrt(2*3.14159)/w2 * TMath::Exp(-0.5*arg*arg/w2/w2);

  return g1+g2;
}






/****************************************************************/

int main(int argc, char **argv)
{


  char mtbfile[200]="mtb.bin";
  char rtbfile[200]="rtb.bin";
  char mtblevelfile[200];
  char rtblevelfile[200];
  char bgmtbfile[200];
  char bgrtbfile[200];
  char bgmtblevelfile[200];
  char bgrtblevelfile[200];
  char rootfileName[200];
  char phCalDir[200] = "";
  char path[200]=".";
  char bgpath[200]="";
  //  char title[200];
  int run=0;
  int batch=0;
  int module=1;
  int telescope=0;
  int NROC=1;
  int verbose=0;
  int bgrun=0;
  int usePHcal=0;
  int levelBootstrap=0;
  int ed=0;
  Double_t mua=0.037;
  gRegion=kNormalRegion;

#define PI 3.14159265

  // -- command line arguments
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"-r")) {
      run=atoi(argv[++i]);
      sprintf(path,"/home/l_tester/log/bt05r%06d",run);
      sprintf(mtbfile, "%s/mtb.bin",path);
      sprintf(rtbfile, "%s/rtb.bin",path);
      sprintf(mtblevelfile, "%s/levels-module.dat",path);
      sprintf(rtblevelfile, "%s/levels-roc.dat",path);
		mua=getrunmua(run,0.037);
		gTbin=0.1;//int(100*1/getruntrig(run,8.0))/100;
    }else if (!strcmp(argv[i],"-bg")) {
      bgrun=atoi(argv[++i]);
      sprintf(bgpath,"/home/l_tester/log/bt05r%06d",bgrun);
      sprintf(bgmtbfile, "%s/mtb.bin",bgpath); 
      sprintf(bgrtbfile, "%s/rtb.bin",bgpath); 
      sprintf(bgmtblevelfile, "%s/levels-module.dat",bgpath);
      sprintf(bgrtblevelfile, "%s/levels-roc.dat",bgpath);
    }else if (!strcmp(argv[i],"-b")) {	
      batch=1; 
    }else if (!strcmp(argv[i],"-c")) {	
      usePHcal=1; 
    }else if (!strcmp(argv[i],"-v")) {	
      verbose=1; 
    }else if (!strcmp(argv[i],"-t")) {	
      telescope=1; 
		NROC=4;
		module=0;
    }else if (!strcmp(argv[i],"-roc")) {
      if (getrunrocdata(run,0)==1) {
		  module=0;
		  //	cout << "module: " << module << endl;
      }
      else {
	cout << "no ROC data available in run " << run << " !!" << endl;
	exit (1);
      }  
    }else if (!strcmp(argv[i],"-ed")) {
      ed=1;
    }else if (!strcmp(argv[i],"-reg")) {
      gRegion=kSpecialRegion;
      greg[0]=atoi(argv[++i]);//cminModule
      greg[1]=atoi(argv[++i]);//cmaxModule
      greg[2]=atoi(argv[++i]);//rminModule
      greg[3]=atoi(argv[++i]);//rmaxModule
    }else if (!strcmp(argv[i], "-s")) {
	greg_shape=1;
    }else if (!strcmp(argv[i], "-e")) {
	greg_shape=0;
    }else if (!strcmp(argv[i],"-large")) {
      gRegion=kLargeRegion;
    }else if (!strcmp(argv[i],"-small")) {
      gRegion=kSmallRegion;
    }else if (!strcmp(argv[i],"-outside")) {
      gRegion=kOutsideRegion;
    }else if (!strcmp(argv[i],"-entire")) {
      gRegion=kEntireRegion;
    }else if (!strcmp(argv[i],"-mua")){
      sscanf(argv[++i],"%lf",&mua);
		cout << "setting mua=" << mua << endl;
    }else if (!strcmp(argv[i],"-clu")){
      sscanf(argv[++i],"%d",&gClucut);
    }else if (!strcmp(argv[i],"-l")) {
      levelBootstrap=1;
    }else if (!strcmp(argv[i],"-phCal")) {
      sprintf(phCalDir, argv[++i]);
    }else{
      sprintf(mtbfile,"%s/%s",path,argv[i]);
      sprintf(rtbfile,"%s/%s",path,argv[i]);
      //      strcpy( mtbfile, argv[i]);
      //strcpy( rtbfile, argv[i]);
      sprintf( mtblevelfile,"%s/levels-module.dat",path);
      sprintf( rtblevelfile,"%s/levels-roc.dat",path);
    }
  }

  if(module){
	 sprintf(rootfileName, "%s/rhistos-mod.root",path);
  }else{
	 sprintf(rootfileName, "%s/rhistos-roc.root",path);
  }


  // get pulse-height calibration ----
  PHCalibration* phcal=NULL;
  if(usePHcal){
    if (strcmp(phCalDir, "") == 0) sprintf(phCalDir, path);
    phcal=new PHCalibration();
    phcal->LoadFitParameters(phCalDir, 0);
  }

  int dummyargc=1;
  TApplication theApp("App", &dummyargc, argv);
  TFile tf(rootfileName,"RECREATE");
  cout << "rootfile = " << rootfileName << endl;
  if( ed ) {gCanvas=new TCanvas("a","b",400,800); gCanvas->cd();}

  // signal file ------
  BinaryFileReader* f;
  runStat sigStat;
  runHistos sigHist;
  if(module){
    f=new BinaryFileReader(mtbfile,16,run);
    f->readLevels(mtblevelfile, levelBootstrap);
    if(usePHcal) f->setPHCalibration(phcal);
    //    loop(f,  run, module,0, verbose&(!levelBootstrap), sigStat, sigHist);
    loop(f,  run, module,0, verbose&(!levelBootstrap), sigStat, sigHist);
    f->writeNewLevels();
  }else{
    f=new BinaryFileReader(rtbfile,NROC,run);
    f->readLevels(rtblevelfile);
    //    loop(f,  run, module,0, verbose, sigStat, sigHist);
    loop(f,  run, module,0, verbose, sigStat, sigHist);
    f->writeNewLevels();
  }

  if( ed ) {delete gCanvas;}

  if(usePHcal){
	 LangauFitter lf;
	 lf.Fit(sigHist.ChargeClu);
	 cout << "@@@mip  " << lf.GetParameter(1) << endl;
  }

  // background file ------
  BinaryFileReader* fbg;
  runStat bgStat;
  runHistos bgHist;
  if(bgrun>0){
    cout << "reading background file" << endl;
    if(module){
      fbg=new BinaryFileReader(bgmtbfile,16,bgrun);
      fbg->readLevels(bgmtblevelfile);
      loop(fbg,  run, module, 1, verbose, bgStat, bgHist);
    }else{
      fbg=new BinaryFileReader(bgrtbfile,1,bgrun);
      fbg->readLevels(bgrtblevelfile);
      loop(fbg,  run, module, 1, verbose, bgStat, bgHist);
    }
    cout << "done with background file" << endl;
  }

  Double_t p0=double(sigStat.n0Fid)/double(sigStat.nro);
  Double_t dp0=p0*(1-p0)/double(sigStat.nro);
  Double_t mut=sigHist.NCluFid->GetMean()-double(sigStat.nyesFid)/sigStat.nro;
  cout << "mut (guess) =" << mut << endl;
  cout << "@@@effdk   " << 1-p0*(1-exp(-mut))/(mut*exp(-mut))
		 << " +/- " << dp0*(1-exp(-mut))/(mut*exp(-mut)) <<endl;
  


  // cluster distance
  TF1 *fsep=new TF1("fsep","[0]*x+[1]*x*x",10,50.);
  fsep->SetParameter(0,0);
  fsep->SetParameter(1,0);
  sigHist.CluSep->Fit("fsep","QLR0I");
  sigHist.CluSep2->Fit("fsep","QLR0I");


  //
  if((run>2500)&&(run<2700)){mua=f->getTriggerRate()*5./40e6;}
  
  // efficiency determination
  TF1 *func = new TF1("fitf",fitf,-0.5,19.5,4);
  func->SetLineWidth(2);
  Double_t effraw=double(sigStat.nyesFid)/sigStat.nro;
  Double_t mu0guess=sigHist.NCluFid->GetMean()-effraw;
  func->SetParameters(effraw,mu0guess,0.0,sigHist.NCluFid->GetEntries());
  func->SetParNames("epsilon","mu0","muA","n");
  func->FixParameter(2,mua);
  func->FixParameter(3,sigHist.NCluFid->GetEntries());
  
  double q0c,dq0;
  if(bgrun>0){
	 //double mu1=double(sigStat.ncluFid)/double(sigStat.nro);
	 double mu0=double(bgStat.ncluFid)/double(bgStat.nro);
    func->FixParameter(1,mu0);
	 //cout << "@@@effsub  " << (double(sigStat.ncluFid)/double(sigStat.nro)-mu0)*(1+0.5*mua+mua*mua/6.) << endl;
	 double gamma=exp(-mua);
	 //	 cout << "LL " << mu1 << " " << mu0 << " " << endl;
	 cout << "@@@effsub  " << (double(sigStat.ncluFid)/double(sigStat.nro)-mu0)
		*(1-gamma)/(mua*gamma) << endl;
	 //double e0=exp(mu0)*p0;
	 //cout << "@@@effp0   " << 1-e0 + 0.5*mua*e0*(1-e0) << endl;
	 Double_t p0bg=double(bgStat.n0Fid)/double(bgStat.nro);
	 double dp0bg=p0bg*(1-p0bg)/double(bgStat.nro);
	 double q0=p0/p0bg;
	 q0c=q0*gamma/(1.-(1.-gamma)*q0);
	 dq0=q0*sqrt(dp0*dp0/p0/p0+dp0bg*dp0bg/p0bg/p0bg);
	 cout << "@@@effp0  " << 1-q0c << " +/- " << dq0 << endl;

  }
  sigHist.NCluFid->Fit("fitf","LR0");
  Double_t par[4];
  func->GetParameters(par);
  if(batch){
    cout << "@@@efficiency " << par[0] << " " << func->GetParErrors()[0] << endl;
    cout << "@@@clusize    " << sigHist.SizeClu->GetMean() << endl;
  }else{
    cout << "\n\n\n\n";
    cout << "====================  Fit  =========================" << endl;
    cout << Form("efficiency = %7.3f +/- %7.3f",par[0],func->GetParErrors()[0]) << endl;
    cout << "====================================================" << endl;
	 if(bgrun>0){
    cout << "\n\n\n\n";
    cout << "====================  p0   =========================" << endl;
    cout << Form("efficiency = %7.3f +/- %7.3f", 1-q0c, dq0) << endl;
    cout << "====================================================" << endl;
	 }
  }

		

  TH1F* cluDiffRow;
  TH1F* cluDiffCol;
  if(bgrun>0){
	 cluDiffRow = new TH1F(*sigHist.cluMapRow);
	 float scale=((float)sigStat.nro)/((float)bgStat.nro);
	 cluDiffRow->Add(bgHist.cluMapRow, -scale);
	 cluDiffRow->SetBinContent(80,0.5*cluDiffRow->GetBinContent(80));
	 cluDiffRow->SetBinContent(81,0.5*cluDiffRow->GetBinContent(81));
	 Float_t max=cluDiffRow->GetBinContent(80);
	 cluDiffRow->SetMaximum(max*1.1);
	 TF1 *funcggRow = new TF1("fitggr",fitgg, 20, 150,5);
	 funcggRow->SetParameter(0,cluDiffRow->Integral(40,120));
	 funcggRow->SetParameter(1,80.5);
	 funcggRow->SetParameter(2,4.8);
	 funcggRow->SetParameter(3,0.1);
	 funcggRow->SetParameter(4,10.);
	 cluDiffRow->Fit("fitggr","QR0");

	 cluDiffCol = new TH1F(*sigHist.cluMapCol);
	 cluDiffCol->Add(bgHist.cluMapCol, -scale);
	 max=cluDiffCol->GetBinContent(184);
	 TF1 *funcggCol = new TF1("fitggc",fitgg, 160, 205,5);
	 cluDiffCol->SetMaximum(max*1.1);
	 funcggCol->SetParameter(0,cluDiffCol->Integral(160,210));
	 funcggCol->SetParameter(1,184);
	 funcggCol->SetParameter(2,3.6);
	 funcggCol->SetParameter(3,0.1);
	 funcggCol->SetParameter(4,10.);
	 cluDiffCol->Fit("fitggc","QR0");


  }

  // fit in time slices
  cout << "fitting slices " << sigHist.ntbin << endl;
  TH1F* ehistory=new TH1F("ehist","efficiency vs time", sigHist.ntbin, 0, sigHist.ntbin*sigHist.wtbin);
  TH1F* nhistory=new TH1F("nhist","events vs time", sigHist.ntbin, 0, sigHist.ntbin*sigHist.wtbin);
  TH1F* mhistory=new TH1F("mhist","triggers vs time", sigHist.ntbin, 0, sigHist.ntbin*sigHist.wtbin);
  TH1F* thistory=new TH1F("thist","triggers vs time", sigHist.ntbin, 0, sigHist.ntbin*sigHist.wtbin);
  TH1F* tbmhistory=new TH1F("tbmhist","tbm stack vs time (ntrig-nro)", sigHist.ntbin, 0, sigHist.ntbin*sigHist.wtbin);
  nhistory->SetMinimum(0);
  mhistory->SetMinimum(0);
  thistory->SetMinimum(0);
  tbmhistory->SetMinimum(0);
  ehistory->SetOption("E1");
  TH1F* tthistory=new TH1F("tthist","efficiency vs time since previous trigger", sigHist.ntbin, 0, sigHist.ntbin/10.);
  TH1F* ntthistory=new TH1F("ntthist","number of clusters in fid region vs time since previous trigger", sigHist.ntbin, 0, sigHist.ntbin/10.);
  TH1F* trhistory=new TH1F("trhist","efficiency vs time since last reset", sigHist.ntbin, 0, 1./fReset);
  TH1F* ntrhistory=new TH1F("ntrhist","number of clusters vs time since last reset", sigHist.ntbin, 0, 1./fReset);
								  
  for(int i=0; i<sigHist.ntbin; i++){
	 // time
	 nhistory->SetBinContent(i+1,sigHist.NCluTime[i]->GetEntries());
	 mhistory->SetBinContent(i+1,sigHist.NCluTime[i]->GetMean());//?
	 thistory->SetBinContent(i+1,sigHist.NTTime[i]->GetEntries());
	 tbmhistory->SetBinContent(i+1,sigStat.maxdrotrig[i]);
	 if(sigHist.NCluTime[i]->GetEntries()>20){
		func->SetParameter(0,effraw);  // avoid correlations with previous results
		func->SetParameter(1,mu0guess);
		func->FixParameter(3,sigHist.NCluTime[i]->GetEntries());
		sigHist.NCluTime[i]->Fit("fitf","QLR0");
		if(batch){
		cout << "@@@etsli " << i << " " << func->GetParameter(0) << " +/- " << func->GetParErrors()[0] << endl;
		}
		ehistory->SetBinContent(i+1,func->GetParameter(0));
		ehistory->SetBinError(i+1,func->GetParErrors()[0]);
	 }
	 // time since prev trigger
	 if(sigHist.NCluTT[i]->GetEntries()>20){
		func->SetParameter(0,effraw);
		func->SetParameter(1,mu0guess);
		func->FixParameter(3,sigHist.NCluTT[i]->GetEntries());
		sigHist.NCluTT[i]->Fit("fitf","QLR0");
		tthistory->SetBinContent(i+1,func->GetParameter(0));
		tthistory->SetBinError(i+1,func->GetParErrors()[0]);
	 }
	 ntthistory->SetBinContent(i+1,sigHist.NCluTT[i]->GetMean());
	 if(sigHist.NCluTT[i]->GetEntries()>1){
		ntthistory->SetBinError(i+1,sigHist.NCluTT[i]->GetRMS()/
										sqrt(sigHist.NCluTT[i]->GetEntries()-1));
	 }
	 // time since prev reset
	 if(sigHist.NCluTR[i]->GetEntries()>20){
		func->SetParameter(0,effraw);
		func->SetParameter(1,mu0guess);
		func->FixParameter(3,sigHist.NCluTR[i]->GetEntries());
		sigHist.NCluTR[i]->Fit("fitf","QLR0");
		trhistory->SetBinContent(i+1,func->GetParameter(0));
		trhistory->SetBinError(i+1,func->GetParErrors()[0]);
	 }
	 ntrhistory->SetBinContent(i+1,sigHist.NCluTR[i]->GetMean());
	 if(sigHist.NCluTR[i]->GetEntries()>1){
		ntrhistory->SetBinError(i+1,sigHist.NCluTR[i]->GetRMS()/
										sqrt(sigHist.NCluTR[i]->GetEntries()-1));
	 }
  }

  cout << "ncluOut: " << sigStat.ncluOut << "; ncluFid: " << sigStat.ncluFid << "; totnclu: " 
       << sigStat.ncluFid+sigStat.ncluOut << " nro:" << sigStat.nro << " ntrig: " << sigStat.ntrig << endl;
  float area_tot = 16*54*81*0.015*0.01;
  cout << "@@flux_count_tot: " << (sigStat.ncluFid+sigStat.ncluOut) / area_tot / sigStat.nro * 40 << " MHz/cm^-2" << " area: " << area_tot << " cm^2 (bg run reasonable!" << endl;
  //  float area_out = ((16*54*81)-(PI*(greg[3]-greg[2])*(greg[1]-greg[0])))*0.015*0.01;
  //  cout << "@@flux_count_out: " << (sigStat.ncluOut) / area_out / sigStat.nro * 40 << " MHz/cm^-2" << " area: " << area_out << " cm^2" << endl;
  int maxdrotrig=0;
  for (int i=0; i<sigHist.ntbin; i++) {
    if (sigStat.maxdrotrig[i]>maxdrotrig) {maxdrotrig=sigStat.maxdrotrig[i];}
  }
  cout << "@@@maxdrotrig: " << maxdrotrig << endl;
cout << "n0Fid= " << sigStat.n0Fid << " n1Fid= " << sigStat.n1Fid << " n2Fid= " << sigStat.n2Fid << " n3Fid= " << sigStat.n3Fid << " n4Fid= " << sigStat.n4Fid << " nf_max= " << sigStat.nf_max << endl;

//   if(batch){
//    if(module==0){
//      cout << "@@@flux " << 
// 		  1.14*float(sigStat.ncluOut)/float(sigStat.nro)/((54*81-10*10)*0.0150*0.0100)*50 << endl;
//    }
//     f->printRunSummary2();
//   }

  tf.Write();

 
  /* histograms */

  f->printRunSummary2();

  TH2F* fiducialMap=new TH2F(*sigHist.hitmap);
  pixel p;
  int npfid=0;
  for(int i=0; i<fiducialMap->GetNbinsX()+1; i++){
    for(int j=0; j<fiducialMap->GetNbinsY()+1; j++){
      if(module){
	p.col=j; p.row=i;
      }else{
	p.col=i; p.row=j;
      }
      fiducialMap->SetBinContent(i,j,fiducial(module, run, &p, 0));
      if(fiducial(module, run, &p, 0)) {
	npfid++;
	if((j==80)||(j==79)||(i%52==0)||(i%52==51)) npfid++;
      }
    }
  }
  cout << "counting area = " << npfid * 0.01*0.015<< " cm**2" << endl;
  cout << "@@@flux ~ " << 1.14*par[1]/(npfid * 0.01*0.015)*40 << " MHz/cm**2" << endl;
  
  if(batch==0){
    
    //	 TApplication theApp("App", &argc, argv);
    Viewer *v=new Viewer(gClient->GetRoot(),700,800,Form("r-run%d",run));
    gStyle->SetPalette(1,0);
    gStyle->SetOptFit(1);
    gStyle->SetPaperSize(16,24);

    sigHist.hitmap->SetOption("colz");
    sigHist.noisemap->SetOption("colz");
    sigHist.dcolHistory->SetOption("colz");
    sigHist.dcolHistory07->SetOption("colz");
    sigHist.dcolHistory815->SetOption("colz");
    if(module){
      v->addView("map",2,1,sigHist.hitmap, sigHist.noisemap);
      v->addOverlayView("region",2,1,1.,sigHist.hitmap,fiducialMap);
    }else{
      v->addView("map",1,1,sigHist.hitmap);
      v->addOverlayView("region",1,1,1.,sigHist.hitmap);
		v->addView("slices",1,3,sigHist.dcolSlice1, 
					  sigHist.dcolSlice2, 
					  sigHist.dcolSlice3);
    }

    //   	 v->addView("dcols",1,1,sigHist.dcolHistory);
	 v->addView("dcols",2,1,sigHist.dcolHistory815, sigHist.dcolHistory07);
	 //	 v->addView("history",1,3,ehistory,nhistory,mhistory);
	 v->addView("history",1,3,ehistory,nhistory,thistory);
	 v->addView("structure",1,4,tthistory, ntthistory,trhistory,ntrhistory);
	 v->addView("cluster2",1,3,sigHist.CluSep,sigHist.CluSep2);
	 v->addView("TBM",1,3, nhistory, thistory, tbmhistory);

    if(bgrun==0){
       v->addView("hits",2,2,sigHist.NPix, sigHist.DColHits,
 					  sigHist.mapCol, sigHist.mapRow);
       v->addView("history",2,2,sigHist.DeltaTt, sigHist.NPixT,
       					  sigHist.DeltaT, sigHist.NPixD);
		 sigHist.NCluFid->SetOption("E1");
		 sigHist.NCluFid->SetMarkerStyle(20);
		 v->addView("clusters",2,2,
						sigHist.NClu,    sigHist.NCluFid, 
						sigHist.SizeClu, sigHist.ChargeClu );
		 v->addView("TBM",1,3,f->hUBTBM, f->hLVLTBM);
		 for(int roc=0; roc<((module==1) ? 16:1); roc++){
			char* label=new char[20];
			sprintf(label,"roc%d",roc);
			v->addOverlayView(label,1,4,1.,
									f->hADROC[roc],    f->hADROCUsed[roc],
									f->hUBBROC[roc],   f->hADROCUsed[roc],
									f->hPHROC[roc],    0,
									f->hPHVcalROC[roc],0 );
		 }
    }else{  // with background
      float scale=((float)sigStat.nro)/((float)bgStat.nro);
      v->addOverlayView("hits(all)",2,2,scale,
								sigHist.NPix,     bgHist.NPix,
								sigHist.DColHits, bgHist.DColHits,
								sigHist.mapCol,   bgHist.mapCol,
								new TH1F(*sigHist.mapRow),   new TH1F(*bgHist.mapRow));
		if(module){
		  v->addOverlayView("hits(3/12)",2,2,scale,
								  sigHist.mapCol07,    bgHist.mapCol07,
								  sigHist.mapCol815,   bgHist.mapCol815,
								  sigHist.mapRowOther, bgHist.mapRowOther,
								  sigHist.mapRow312,   bgHist.mapRow312);
		}
      bgHist.SizeClu->Scale(1/scale/scale);
      v->addOverlayView("clusters",2,2,scale,
								sigHist.NClu,    bgHist.NClu,
								//			sigHist.SizeClu, bgHist.SizeClu,
								sigHist.NCluFid, bgHist.NCluFid,
								sigHist.cluMapCol,   bgHist.cluMapCol,
								sigHist.cluMapRow,   bgHist.cluMapRow);
		v->addView("subtracted",1,2,
								cluDiffRow,cluDiffCol);
								
    }
    v->show();
	 theApp.Run();
  }


  return 0;
}
