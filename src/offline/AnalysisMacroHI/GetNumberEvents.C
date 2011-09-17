#include "TFile.h"
#include "TNtuple.h"
#include "TROOT.h"
//#include "stdio.h"
#include <iostream>





using namespace std;

void GetNumberEvents(){
 
 
 Float_t EvN;
 ModMult->SetBranchAddress("EvN", &EvN);
 Int_t iNTlenght = ModMult->GetEntries();
  
 ModMult->GetEntry(iNTlenght -1);
 std::cout << " MaxEvent " << EvN << std::endl;
 
}
