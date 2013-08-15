#include <iostream.h>
#include <fstream.h>
#include <iomanip.h>
#include <math.h>
#include <stdlib.h>

#include "myReproTestResults.hh"



int main(int argc, char *argv[]) {
  // -- command line arguments
  char dir[200];  sprintf(dir, "/data/ursl/moduleDB");
  char temp[200]; sprintf(temp, "T-10a");
  char file[200]; sprintf(file, "module-T-10a-0135.root");
  int  singleFile(0), grades(0);

  for (int i = 0; i < argc; i++){
    if (!strcmp(argv[i],"-d")) sprintf(dir,  argv[++i]); // base directory
    if (!strcmp(argv[i],"-f")) {singleFile = 1; sprintf(file, argv[++i]);} // single file
    if (!strcmp(argv[i],"-t")) sprintf(temp, argv[++i]); // temperature
    if (!strcmp(argv[i],"-c")) grades = 3; // only grade c
    if (!strcmp(argv[i],"-b")) grades = 2; // only grade b
    if (!strcmp(argv[i],"-a")) grades = 1; // only grade a
    if (!strcmp(argv[i],"-ab")) grades = 4; // only grade a and b
  }

  TChain *pA = new TChain("mod");
  if (singleFile == 1) {
    cout << "Using " << Form("%s", file) << endl;
    pA->Add(Form("%s", file)); 
  } else {    
    cout << "Using " << Form("%s/module-%s-*.root", dir, temp) << endl;
    pA->Add(Form("%s/module-%s-*.root", dir, temp)); 
  }

  MyReproTestResults b(pA);
  b.Init(dir, temp, grades);

  b.Loop();
  b.LoopEnd();
  b.Summary();

  return 0;

}
