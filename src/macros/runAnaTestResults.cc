#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include <math.h>
#include <stdlib.h>

#include "anaTestResults.hh"



int main(int argc, char *argv[]) {

  // -- command line arguments
  char dir[200];  sprintf(dir, "/data/ursl/moduleDB");
  char temp[200]; sprintf(temp, "T-10a");
  char file[200]; sprintf(file, "module-T-10a-0135.root");
  int  singleFile(0);
  bool abTest = false;
  int halfModules = 0;

  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-d")) sprintf(dir,  argv[++i]); // base directory
    if (!strcmp(argv[i],"-f")) {singleFile = 1; sprintf(file, argv[++i]);} // single file
    if (!strcmp(argv[i],"-t")) sprintf(temp, argv[++i]); // temperature
    if (!strcmp(argv[i],"-a")) abTest = true; // a over b check
    if (!strcmp(argv[i],"-ha")) halfModules = 1; // half modules type A
    if (!strcmp(argv[i],"-hb")) halfModules = 2; // half modules type B
  }  

  TChain *pA = new TChain("mod"); 
  if (singleFile == 1) {
    cout << "Using " << Form("%s", file) << endl;
    pA->Add(Form("%s", file)); 
  } else {    
    cout << "Using " << Form("%s/module-%s-*.root", dir, temp) << endl;
    pA->Add(Form("%s/module-%s-*.root", dir, temp)); 
  }

  anaTestResults b(pA);
  if (halfModules == 0) b.setFullModules();
  if (halfModules == 1) b.setHalfModulesA();
  if (halfModules == 2) b.setHalfModulesB();
  b.SetAbTest(abTest);
  b.bookHistograms(dir, temp);
  b.openResults();
  b.Loop();
  b.closeResults();
  b.summary();

  return 0;

}
