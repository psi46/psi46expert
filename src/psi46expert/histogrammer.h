#ifndef HISTOGRAMMER_H
#define HISTOGRAMMER_H

#include <fstream>
#include <iostream.h>
#include <stdio.h>

#include <TROOT.h>
#include <TApplication.h>
#include <TFile.h>
#include <TTree.h>
#include <TBranch.h>
#include <TChain.h>
#include <TObjString.h>
#include <TNtuple.h>
#include <TH2.h>
#include <TF1.h>
#include <TProfile.h>

#include "BasePixel/DecodedReadout.h"

class histogrammer {
 public:
  histogrammer(); 
  ~histogrammer();     
  void   close();

  void   openRootFile(const char *rootfilename);        // open root outputfile
  TFile* getRootFile() {return fRootFile;}
  void   setRootFile(TFile *f) {fRootFile = f;}
  void   init(const char *subdirectory);                // define/book histograms in subdirectory
  void   reset();                                // Reset all histograms in list
  void   fillRawDataHistograms(int n, int header = -1); // fill  histograms on raw data structure
  void   fillPixelHistograms(int n);                    // fill  histograms on pixel structure

  TH1*   getNextHistogram();
  TH1*   getHistogram(const char *hist = "h0");

  //void   setPixels(struct pixel p[]) {fPixels = p;}
  //void   setPixels(DecodedReadoutModule& p) {fPixels = p;}

  void   setRawData(int (*p)[DecodedReadoutConstants::MAX_PIXELSROC]);
  void   printRawData(int n);

 private:

  int          fOwner;
  TFile*       fRootFile;
  TString      fDir;

  TList*       lHistograms;
  TH1*         fpCurrent;

  int          (*fRawData)[DecodedReadoutConstants::MAX_PIXELSROC];

  //struct pixel *fPixels;
  //DecodedReadoutModule& fPixels;

};
#endif
