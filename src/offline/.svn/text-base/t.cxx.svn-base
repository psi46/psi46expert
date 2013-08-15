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

#include "EventReader.h"
#include "Viewer.h"
#include "EventView.h"
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
 *       program for offline analysis of november 2005 testbeam data  *
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
 *                                                                    *
 *********************************************************************/




 
/****************************************************************/
int main(int argc, char **argv)
{


  int run=0;
  int batch=0;
  int verbose=0;
  int usePHcal=0;
  int nEvent=0;
  int ed=0;
  double mua=0;
  int levelMode=0;
  int alignment=0; 
  int fit=1;
  char rtbfilename[101]="";
  char mtbfilename[101]="";
  int phTrim=0;
  char rootfileName[200]="";
  char path[100]="";

  // -- command line arguments
  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"-r")) {
      run=atoi(argv[++i]);
    }else if (!strcmp(argv[i],"-fr")) {
		strncpy(rtbfilename,argv[++i],100);
    }else if (!strcmp(argv[i],"-fm")) {
		strncpy(mtbfilename,argv[++i],100);
    }else if (!strcmp(argv[i],"-b")) {	
      batch=1; 
    }else if (!strcmp(argv[i],"-ph")) {	
      usePHcal=1; 
      phTrim=atoi(argv[++i]);
    }else if (!strcmp(argv[i],"-v")) {	
      verbose=1; 
    }else if (!strcmp(argv[i],"-a")) {	
      alignment=1; 
    }else if (!strcmp(argv[i],"-a2")) {	
      alignment=2; 
    }else if (!strcmp(argv[i],"-nf")) {	
      fit=0;
    }else if (!strcmp(argv[i],"-lr")) {	
      levelMode|=EventReader::initRocLevels;
    }else if (!strcmp(argv[i],"-lm")) {	
      levelMode|=EventReader::initModuleLevels;
    }else if (!strcmp(argv[i],"-l")) {	
      levelMode|=EventReader::initRocLevels|EventReader::initModuleLevels;
    }else if (!strcmp(argv[i],"-mua")){
      sscanf(argv[++i],"%lf",&mua);
    }else if (!strcmp(argv[i],"-n")){
      sscanf(argv[++i],"%d",&nEvent);
    }else if (!strcmp(argv[i],"-ed")) {
      ed=1;
	 }
  }

  sprintf(path,"/home/l_tester/log/bt05r%06d",run);
  sprintf(rootfileName, "%s/thistos.root",path);
  TFile tf(rootfileName,"RECREATE");
  cout << "rootfile = " << rootfileName << endl;

  
  
  EventReader* reader=new EventReader(run,"",levelMode,rtbfilename, mtbfilename);
  
   // get pulse-height calibration ----
  PHCalibration* phcal=NULL;
  if(usePHcal){
    phcal=new PHCalibration();
    phcal->LoadFitParameters(path, phTrim);
    reader->fMod->setPHCalibration(phcal);
  }

  
  if(alignment==2) reader->fAlignmentFile=new ofstream("alignment.dat");
  reader->setVerbose(verbose);
  if (reader->fMod) reader->fMod->setAnaMin(-500);
  reader->loop(nEvent);

  reader->printRunSummary();
  /*********************************************************************************************/

  if(reader->fMod){ reader->fMod->printVcalPeaks("phMod"); }


  tf.Write();

  /*********************************************************************************************/
  if(batch==0){
    // configure and pop up the histogram viewer
    int dummyargc=1;
    TApplication theApp("App", &dummyargc, argv);
   
    gStyle->SetErrorX(0.0);
    gStyle->SetPalette(1); 
    gStyle->SetOptFit(111); 
    
    Viewer* v=new Viewer(gClient->GetRoot(),700,800,Form("run%d",run));
    
    v->addTab("Maps");
    if(reader->fMod){
      reader->fMod->hModMap->SetOption("colz");
      v->addView("Module Map",1,1,reader->fMod->hModMap);
    }
    
      
    // **************   control plots for module rocs from BinaryFileReader  ***************
    if(reader->fMod){
      BinaryFileReader*f = reader->fMod;;
      
      v->addView("PH",1,1,reader->fMod->hPH);
      v->addView("Vcal",1,1,reader->fMod->hVcal);
      
      v->addView("TBM levels",1,3,reader->fMod->hUBTBM, reader->fMod->hLVLTBM);
      for(int roc=0; roc<16; roc++){
	char* label=new char[20];
	sprintf(label,"roc%d levels",roc);
	v->addOverlayView(label,1,3,1.,
			  f->hADROC[roc],    f->hADROCUsed[roc],
			  f->hPHROC[roc],    0,
			  f->hPHVcalROC[roc],0 );
      }
      v->addView("hits  all ",1,2, f->hNHit);
  
    }
    
    // **************  control plots for telescope rocs from BinaryFileReader  ****************
    
    v->addFwdBwdButtons();
    
    v->show();
    theApp.Run();
  }
  
  return 0;
}
