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
#include "TProfile.h"


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
 
/**********************************************************************
 *       program for offline analysis of 2005 testbeam data           *
 *                                                                    *
 *  usage:                                                            *
 *        ./r -r <run-number>  [-roc]                                 *
 *  or                                                                *
 *        ./r <filename>                                              *
 *  Options:                                                          *
 *     -roc      read roc datafile from run directory                 *
 *     -v        verbose mode                                         *
 *     -b        don't pop up histogram window                        *
 *     -l        bootstrap address levels (re-run without -l later)   *
 *                                                                    *
 *   address level decoding is based on the files                     *
 *         levels-module.dat                                          *
 *  and    levels-roc.dat                                             *
 *                                                                    *
 *  which are in the data directory of the run                        *
 *  if those files are not present, the files in the current working  *
 *  directory are used and new files are created and placed in the    *
 *  data directory.                                                   *
 *  It may then be necessary to re-run the code to get correct        *
 *  level/address identification only in the second pass              *
 *                                                                    *
 *                                                                    *
 *  Efficiencies can be calculated with or without a background       *
 *  run. The regions for signal hit counting are defined in           *
 *  the function   "float fiducial(...)" which must be updated        *
 *  when the position/shape of the scintillator "shadow" moves.       *
 *                                                                    *
 *                                                                    *
 *********************************************************************/




/************************************************************/


// simple clusterization
vector<cluster> clus(int n, pixel* apix){
  const int u=2; // cluster search radius (i.e. allows u-1 empty pixels)
  vector<cluster> v;
  if(n==0) return v;
  int* gone = new int[n];
  for(int i=0; i<n; i++) gone[i]=0;
  int seed=0;
  while(seed<n){
    // start a new cluster
    cluster c;
    c.vpix.push_back(apix[seed]); gone[seed]=1;
    c.charge=0; c.size=0; c.col=0; c.row=0;
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
      c.charge+=(*p).anaVcal;
      c.col+=(*p).col*(*p).anaVcal;
      c.row+=(*p).row*(*p).anaVcal;
    }
    c.size=c.vpix.size();
    c.col=c.col/c.charge;
    c.row=c.row/c.charge;
    v.push_back(c);
    //look for a new seed
    while((++seed<n)&&(gone[seed]));
  }
  // nothing left  return clusters
  return v;
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
  if((run<2170)||(run>2225)){
    rminModule=20;// 
    rmaxModule=160;//
    cminModule=120;//
    cmaxModule=240;// 
	 /*
    rminModule=40;// 
    rmaxModule=140;//
    cminModule=140;//
    cmaxModule=210;// 
	 */

    rminModule=0;// 
    rmaxModule=180;//
    cminModule=80;//
    cmaxModule=270;// 
  }else{ // 45 degree trigger spot 
    rminModule=20;// 
    rmaxModule=160;//
    cminModule=90;//
    cmaxModule=230;// 
  }
  float c0Module=(cmaxModule+cminModule)/2.;
  float r0Module=(rmaxModule+rminModule)/2.;
  float wcModule=(cmaxModule-cminModule)/2.;
  float wrModule=(rmaxModule-rminModule)/2.;

  if(module){
    float dc=(p->col-c0Module)/wcModule;
    float dr=(p->row-r0Module)/wrModule;
    float r=sqrt(dr*dr+dc*dc);
    if( hist!=0 ) hist->Fill(r);
    if(r<1.){ 
      return 1.;
    }else{
      return 0.;
    }
  }else{//roc
    if((p->row>15)&&(p->row<55)&&(p->col>10)&&(p->col<40)){
      return 1;
    }else{
      return 0;
    }
  }
}


/************************************************************/



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


/****************************************************************/

Double_t gp0(Double_t *x, Double_t *par){
  Double_t arg=(x[0]-par[1])/par[2];
  return par[0]*exp(-0.5*arg*arg)+par[3];
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
  char path[200]="";
  int run=0;
  int batch=0;
  int verbose=0;
  int bgrun=0;
  int usePHcal=0;
  int ed=0;
  int doPsf=0;
  double mua=0;

  // -- command line arguments
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"-r")) {
      run=atoi(argv[++i]);
      sprintf(path,"/home/l_tester/log/bt05r%06d",run);
      sprintf(mtbfile, "/home/l_tester/log/bt05r%06d/mtb.bin",run);
      sprintf(rtbfile, "/home/l_tester/log/bt05r%06d/rtb.bin",run);
      sprintf(mtblevelfile, "/home/l_tester/log/bt05r%06d/levels-module.dat",run);
      sprintf(rtblevelfile, "/home/l_tester/log/bt05r%06d/levels-roc.dat",run);
		sprintf(rootfileName, "/home/l_tester/log/bt05r%06d/rhistos2.root",run);
    }else if (!strcmp(argv[i],"-bg")) {
      bgrun=atoi(argv[++i]);
      sprintf(bgmtbfile, "/home/l_tester/log/bt05r%06d/mtb.bin",bgrun); 
      sprintf(bgrtbfile, "/home/l_tester/log/bt05r%06d/rtb.bin",bgrun); 
      sprintf(bgmtblevelfile, "/home/l_tester/log/bt05r%06d/levels-module.dat",bgrun);
      sprintf(bgrtblevelfile, "/home/l_tester/log/bt05r%06d/levels-roc.dat",bgrun);
    }else if (!strcmp(argv[i],"-b")) {	
      batch=1; 
    }else if (!strcmp(argv[i],"-c")) {	
      usePHcal=1; 
    }else if (!strcmp(argv[i],"-v")) {	
      verbose=1; 
    }else if (!strcmp(argv[i],"-psf")) {	
      doPsf=1; 
    }else if (!strcmp(argv[i],"-mua")){
      sscanf(argv[++i],"%lf",&mua);
    }else if (!strcmp(argv[i],"-ed")) {
      ed=1;
	 }
  }


  // get pulse-height calibration ----
  PHCalibration* phcal=NULL;
  if(usePHcal){
    phcal=new PHCalibration();
    phcal->LoadFitParameters();
  }

  int dummyargc=1;
  //  TApplication theApp("App", &dummyargc, argv);
  //  TFile tf(rootfileName,"RECREATE");


  // open event files ------
  BinaryFileReader* fmod=new BinaryFileReader(mtbfile,16,run);
  fmod->readLevels(mtblevelfile);
  if(run<1959){
	 fmod->setDeconvolution(0.06);  // TBM04
  }else{
	 fmod->setDeconvolution(0.02);  // TBM05
  }
  fmod->open();

  BinaryFileReader* froc=new BinaryFileReader(rtbfile,1,run);
  froc->readLevels(rtblevelfile);
  froc->open();
  

  int dtbmtrig=-1;
  int nSkipRoc=0;
  int nSkipMod=0;
  int nSync=0;
  pixel pixbuf[1001];


  TH2F* hNClu=new TH2F("nclu","# of clusters",10,-0.5,9.5,30,-0.5,29.5);
  TH2F* hNCluFid=new TH2F("nclufid","# of clusters in counting region",
								  5,-0.5,4.5,20,-0.5,19.5);
  TProfile* pNClu=new TProfile("ncluprof","# of clusters",10,-0.5,9.5,-1.,100.);
  TProfile* pNCluFid=new TProfile("ncluproffid","# of clusters in counting region",
										 5,-0.5,4.5,-1.,100.);

  // cluster counting for efficiency
  TH1F* hN00= new TH1F("hN00","module, all triggers ",10, -0.5, 9.5);
  TH1F* hN0 = new TH1F("hN0","module, roc confirmed events",10, -0.5, 9.5);
  TH1F* hN1 = new TH1F("hN1","module, roc confirmed (restricted)",10, -0.5, 9.5);
  TH1F* hN2 = new TH1F("hN2","module, roc confirmed (very restricted)",10, -0.5, 9.5);


  TH2F* hMapMiss =  new TH2F("miss", "missing", 52,0.,52., 80, 0., 80.);
  TH2F* hAll = new TH2F("all","all",160,0,160,416,0,416);
  TH2F* hEmpty = new TH2F("empty","empty",160,0,160,416,0,416);
  TH2F* hSingle = new TH2F("single","single",160,0,160,416,0,416);
  TH2F* hSingleTrig = new TH2F("singleTrig","singleTrig",160,0,160,416,0,416);
  TH2F* hMulti = new TH2F("multi","multi",160,0,160,416,0,416);
  TH2F* hxy0 =  new TH2F("xy0", "all", 160,0.,160., 416, 0., 416.);
  TH2F* hxyr =  new TH2F("xyr", "roc", 160,0.,160., 416, 0., 416.);
  TH2F* hxy1 =  new TH2F("xy1", "restricted", 160,0.,160., 416, 0., 416);
  TH2F* hDxy0 =  new TH2F("dxy", "dxy", 70,-20.,120., 100, 50.,250.);
  const int nhdxyMax=25;
  TH2F* hDxy[nhdxyMax];
  TH1F* hNxy[nhdxyMax];
  ifstream psfi("psf.dat");
  int colsel[nhdxyMax], rowsel[nhdxyMax];
  double dc[nhdxyMax], wc[nhdxyMax];
  double dr[nhdxyMax], wr[nhdxyMax];
  double effxy[nhdxyMax];
  //  double edum;
  int nhdxy=0;
  if(doPsf){
	 while (!psfi.eof()&&(nhdxy<nhdxyMax)){
		psfi >> colsel[nhdxy] >> rowsel[nhdxy] >> dc[nhdxy] >> wc[nhdxy] >> dr[nhdxy] >> wr[nhdxy];
		psfi.ignore(256,'\n'); // skip anything trailing
		nhdxy++;
	 } 
	 for(int i=0; i<nhdxy; i++){
		hDxy[i] = new TH2F(Form("dxy%d",i),Form("dxy%d",i),
								 80,0.,160,  100, 100.,300.);
		hNxy[i] = new TH1F(Form("nxy%d",i),Form("nxy%d",i),10,-0.5, 9.5);
	 }
  }



  // -----------------       event loop         -------------------------------

  while( fmod->readDataEvent() && froc->readDataEvent()){

	 //	 cout << "A: " <<fmod->getTrigBC() << " " << froc->getTrigBC() << endl;

	 //skip junk timestamps
	 while( (fmod->getTrigBC()>4000000000LL)&&fmod->readDataEvent() );
	 if(fmod->eof()) break;
	 while( (froc->getTrigBC()>4000000000LL)&&froc->readDataEvent() );
	 if(froc->eof()) break;

	 //cout << "B: "<<fmod->getTrigBC() << " " << froc->getTrigBC() << endl;

	 int t=0;
	 while( (fmod->getTrigBC()>(t=froc->getTrigBC()))&&froc->readDataEvent()){
		//		cout << "skipping roc event " << t << endl;
		nSkipRoc++;
	 }
	 while( (froc->getTrigBC()>(t=fmod->getTrigBC()))&&fmod->readDataEvent()){
		//		cout << "skipping module event " << t << endl;
		nSkipMod++;
	 }
	 //cout << "C: "<<fmod->getTrigBC() << " " << froc->getTrigBC() << endl;
	 //	 if( (fmod->getOverFlowCount()>0)||(froc->getOverFlowCount()>0) ) break;
	 if(   (fmod->getTrigBC()==froc->getTrigBC()) )
			 //		  &&(fmod->getBC()==froc->getBC()-2) )
		{
		//cout << fmod->getTriggerStack() << " " << froc->getTriggerStack() << endl;
		  //		cout << "D: "<<fmod->getTrigBC() << " " << froc->getTrigBC() << endl;
		  //		cout << "X: "<<fmod->getBC() << " " << froc->getBC() << endl;
		//fmod->readDataEvent();
		// ****  synchronized events ****

		  /*
		  cout << "TC: "<<fmod->getTBMTrigger() << " " 
				 << froc->getTBMTrigger() << " " 
				 << (256+fmod->getTBMTrigger()-froc->getTBMTrigger())%256
				 << Form(" Droc=%10d",froc->getBC()-froc->getTrigBC())
				 << Form(" Dmod=%10d",fmod->getBC()-fmod->getTrigBC())
				 << endl;
		  */
		  int dtb=(256+fmod->getTBMTrigger()-froc->getTBMTrigger())%256;
		  if(dtb!=dtbmtrig){
			 cout << "TBM trigger difference changed from " << dtbmtrig 
					<<" to " << dtb << endl;
			 dtbmtrig=dtb;
		  }			 
		  
		nSync++;
		// get hits and clusters
		int nhmod=fmod->getNHit();
		fmod->getPixels(pixbuf); toGlobal(1, nhmod, pixbuf);
		vector<cluster> mclu=clus(nhmod,pixbuf);
		int nhroc=froc->getNHit();
		froc->getPixels(pixbuf); toGlobal(0, nhroc, pixbuf);
		vector<cluster> rclu=clus(nhroc,pixbuf);
		hNClu->Fill(rclu.size(),mclu.size());
		pNClu->Fill(rclu.size(),mclu.size());

		// count module hits
		int nmfid=0;    // number of clusters in the module counting region
		int nm[nhdxyMax];	if(doPsf){for(int i=0; i<nhdxy; i++) nm[i]=0;};
		for(unsigned int im=0; im<mclu.size(); im++){
		  float r=fiducial(1, run, &(mclu.at(im).vpix.at(0)), NULL);
		  if(r>0) nmfid++;
		  if(doPsf){
			 for(int i=0; i<nhdxy; i++){
				double dy=(dc[i]-mclu.at(im).col)/wc[i];
				double dx=(dr[i]-mclu.at(im).row)/wr[i];
				if( (dx*dx+dy*dy)<9.) nm[i]++;
			 }
		  }
		}

		// count roc hits
		int nrfid=0;     // number of clusters in the ROC counting region
		int nrfid1=0;    // restricted region 1
		int nrfid2=0;    // restricted region 2
		int nr[nhdxyMax];	for(int i=0; i<nhdxy; i++) nr[i]=0;
		for(unsigned int ir=0; ir<rclu.size(); ir++){
		  if(nmfid==0) hMapMiss->Fill(rclu.at(ir).col, rclu.at(ir).row);
		  float r=fiducial(0, run, &(rclu.at(ir).vpix.at(0)), NULL);
		  if(r>0) nrfid++;
		  if((abs(rclu.at(ir).col-24)<5) && (abs(rclu.at(ir).row-34)<8)) nrfid1++;
		  if((abs(rclu.at(ir).col-24)<2) && (abs(rclu.at(ir).row-34)<2)) nrfid2++;
		  if(doPsf){
			 for(int i=0; i<nhdxy; i++){
				if( (abs(rclu.at(ir).row-rowsel[i])<2) && (abs(rclu.at(ir).col-colsel[i])<2)  )
				  nr[i]++;
			 }
		  }
		}
		hNCluFid->Fill(nrfid1, nmfid);
		pNCluFid->Fill(nrfid1, nmfid);
		hN00->Fill(nmfid);
		if(nrfid)  hN0->Fill(nmfid);
		if(nrfid1) hN1->Fill(nmfid);
		if(nrfid2) hN2->Fill(nmfid);
		if(doPsf){
		  for(int i=0; i< nhdxy; i++){ if (nr[i]) hNxy[i]->Fill(nm[i]); }
		}

		if(nrfid>nmfid){
		  //		  cout << "hello " << nSync << endl;
		}

		for(vector<cluster>::iterator cm=mclu.begin(); cm!=mclu.end(); cm++){
		  hAll->Fill((*cm).row, (*cm).col);
		  if (rclu.size()==0) hEmpty->Fill((*cm).row, (*cm).col);
		  if (rclu.size()==1) hSingle->Fill((*cm).row, (*cm).col);
		  if (rclu.size()>1) hMulti->Fill((*cm).row, (*cm).col);
		  if ((rclu.size()==1)&&(nrfid==1)) hSingleTrig->Fill((*cm).row, (*cm).col);
		}

		for(vector<cluster>::iterator cm=mclu.begin(); cm!=mclu.end(); cm++){
		  hxy0->Fill((*cm).row , (*cm).col); 
		  if(rclu.size()>0) hxyr->Fill((*cm).row , (*cm).col); 
		  if(nrfid1) hxy1->Fill((*cm).row , (*cm).col); 
		  for(vector<cluster>::iterator cr=rclu.begin(); cr!=rclu.end(); cr++){
			 hDxy0->Fill((*cm).row - (*cr).col*1.5, (*cm).col - (*cr).row/1.5); 
			 if(doPsf){
				for(int i=0; i<nhdxy; i++){
				  if( (abs((*cr).row-rowsel[i])<2)&&(abs((*cr).col-colsel[i])<2) )
					 hDxy[i]->Fill((*cm).row - (*cr).col*0., (*cm).col - (*cr).row*0.); 
				}
			 }
		  }
		}

	 }// sync'ed events
	 else
		{ cout << " hmm! " << fmod->getTrigBC() << " " << froc->getTrigBC() << endl; }

  }// event loop

	 // efficiency
  TF1 *func = new TF1("fitf",fitf,-0.5,19.5,4);
  func->SetLineWidth(1);
  Double_t mu0guess=hN0->GetMean()-1.;
  func->SetParameters(0.9,mu0guess,0.0,hN0->GetEntries());
  func->SetParNames("epsilon","mu0","muA","n");
  func->FixParameter(2,0.008);
  func->FixParameter(3,hN00->GetEntries());
  hN00->Fit("fitf","LR0");
  hN00->SetOption("E1");
  func->FixParameter(3,hN0->GetEntries());
  hN0->Fit("fitf","LR0");
  hN0->SetOption("E1");
  func->FixParameter(3,hN1->GetEntries());
  hN1->Fit("fitf","LR0");
  hN1->SetOption("E1");
  func->FixParameter(3,hN2->GetEntries());
  hN2->Fit("fitf","LR0");
  hN2->SetOption("E1");

  
  if(doPsf){
	 for(int i=0; i<nhdxy; i++){
		func->FixParameter(3,hNxy[i]->GetEntries());
		hNxy[i]->Fit("fitf","LR0");
		hNxy[i]->SetOption("E1");
		effxy[i]=func->GetParameter(1);

	 }
  }

  cout << "syncd " << nSync << endl;
  cout << "skipped " << nSkipRoc << "/" << nSkipMod << endl;

  // configure and pop up the histogram viewer
  TApplication theApp("App", &dummyargc, argv);

  gStyle->SetErrorX(0.0);
  gStyle->SetPalette(1);

  Viewer* v=new Viewer(gClient->GetRoot(),700,800,Form("r2-run%d",run));

  /*  all clusters  */
  hNClu->SetOption("box");
  pNClu->SetMinimum(0);
  //  pNClu->Fit("pol1");  // don't fit, makes no sense, just confuses
  v->addView("clusters1",2,2,
				 hNClu->ProjectionY("nclumod"),hNClu,
				 pNClu,                        hNClu->ProjectionX("ncluroc"));

  /*  clusters in counting region */
  hNCluFid->SetOption("box");
  pNCluFid->SetOption("P");
  pNCluFid->SetMinimum(0);
  pNCluFid->Fit("pol1","0");
  pNCluFid->SetMarkerStyle(20);
  pNCluFid->SetMarkerSize(1.0);
  pNCluFid->SetMarkerColor(kRed);
  v->addView("clusters2",2,2,
				 hNCluFid->ProjectionY("nclumodfid"),hNCluFid,
				 pNCluFid,                    hNCluFid->ProjectionX("nclurocfid"));


  /*  confirmed clusters in counting region */
  gStyle->SetOptFit(1);
  hN00->SetMarkerStyle(20);
  hN0->SetMarkerStyle(20);
  hN1->SetMarkerStyle(20);
  hN2->SetMarkerStyle(20);
  v->addView("efficiency",2,2, hN00, hN0, hN1, hN2);

  /* full and restricted cluster map */
  hxy0->SetOption("colz");
  hxyr->SetOption("colz");
  hxy1->SetOption("colz");
  v->addView("maps",2,1,hxy0,hxyr);
  v->addView("maps",2,1,hxy0,hxy1);

  /* cluster position correlations */
  TF1* fGP0=new TF1("gp0",gp0,-1000.,1000.,4);
  TH1D* hDxyY0 = hDxy0->ProjectionY("pdxyY");
  fGP0->SetParameters(hDxyY0->GetMaximum(),hDxyY0->GetMean(),10.,100.);
  hDxyY0->Fit(fGP0,"0");
  TH1D* hDxyX0 = hDxy0->ProjectionX("pdxyX");
  fGP0->SetParameters(hDxyX0->GetMaximum(),hDxyX0->GetMean(),10.,100.);
  hDxyX0->Fit(fGP0,"0");
  v->addView("dxy",2,2,hDxy0, hDxyY0, hDxyX0);

  /* restricted cluster position correlations */
  if(doPsf){
	 TH1D* hDxyX[nhdxy];
	 TH1D* hDxyY[nhdxy];
	 ofstream psf("psf.dat");
	 for(Int_t i=0; i<nhdxy; i++){
		psf << colsel[i] << " "  << rowsel[i] << " ";
		hDxyY[i] = hDxy[i]->ProjectionY(Form("pdxyY%d",i));
		fGP0->SetParameters(hDxyY[i]->GetMaximum(),hDxyY[i]->GetMean(),5.,0.);
		hDxyY[i]->Fit(fGP0,"0");
		psf << fGP0->GetParameter(1) << " " <<  fGP0->GetParameter(2) << " "; 
		hDxyX[i] = hDxy[i]->ProjectionX(Form("pdxyX%d",i));
		fGP0->SetParameters(hDxyX[i]->GetMaximum(),hDxyX[i]->GetMean(),5.,0.);
		hDxyX[i]->Fit(fGP0,"0");
		psf << fGP0->GetParameter(1) << " " <<  fGP0->GetParameter(2);
		if(i<9)	 v->addView(Form("dxy%d",i),2,2,hDxy[i], hDxyY[i], hDxyX[i],hNxy[i]);
		psf << " " << effxy[i] << endl;
	 }
  }


  hMapMiss->SetOption("colz");
  v->addView("missing",1,1,hMapMiss);

  hAll->SetOption("colz");
  hEmpty->SetOption("colz");
  hSingle->SetOption("colz");
  hSingleTrig->SetOption("colz");
  hMulti->SetOption("colz");
  v->addView("empty",2,1,hAll,   hEmpty);
  v->addView("Single",2,1,hSingle,hMulti);
  v->addView("SingleTrig",2,1,hSingleTrig);

  v->show();
  theApp.Run();

  
  return 0;
  }
