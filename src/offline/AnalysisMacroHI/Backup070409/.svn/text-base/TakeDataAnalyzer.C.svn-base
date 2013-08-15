#include "TFile.h"
#include "TNtuple.h"
#include "TROOT.h"
//#include "stdio.h"
#include <iostream>
#include "ModuleMultiplicity.h"

//gSystem->Load("ModuleMultiplicity_cxx.so");


using namespace std;

void TakeDataAnalyzer(Int_t RunN){
 
  //Int_t RunN = 4591;   
  //string FileRoot = "/d00/icali/PixelHwStudies/PSIStuff/log/bt05r";
 string FileRoot = "/home/l_tester/log/bt05r";
 string FileInName = "thistos.root";
 string FileOutName = "ProcessedData.root";
 char RunNumber[6];
 sprintf(RunNumber, "%.6d", RunN);
 FileRoot = FileRoot + RunNumber + "/";
 FileInName = FileRoot + FileInName;
 FileOutName = FileRoot + FileOutName;
 
 TFile *iFile = new TFile((const char*)FileInName.c_str());
 TNtuple *Digis = (TNtuple*)iFile->FindObjectAny("events");
 Int_t EvN, row, col, roc, ph;
 Float_t vcal;
 Digis->SetBranchAddress("row", &row);
 Digis->SetBranchAddress("col", &col);
 Digis->SetBranchAddress("roc", &roc);
 Digis->SetBranchAddress("ph", &ph);
 Digis->SetBranchAddress("vcal", &vcal);
 Digis->SetBranchAddress("eventNr", &EvN);
 Int_t iNTlenght = Digis->GetEntries();
 
 TFile *oFile = new TFile((const char*)FileOutName.c_str(), "RECREATE");
 TNtuple *Multiplicity = new TNtuple("ModMult", "ModMult", "EvN:roc:NModHits:NROCHits:NRows:NCols:MeanPh:MeanPhPuls",10000000);
 
 //******************Calculating Number of Events**************
 Digis->GetEntry(iNTlenght -1);
 std::cout << " MaxEvent " << EvN << std::endl;
  
 //*****************Creating Multiplicity NTuple***************
 
 ModuleMultiplicity  ModMult;
 Int_t OldEvN = 1;
 for(Int_t i =0; i< iNTlenght; ++i){
    Digis->GetEntry(i);
    if(OldEvN != EvN ||( i == iNTlenght -1) ){
      for(Int_t j=0; j < 16; ++j){ 
        Float_t NModHits = ModMult.ModMultiplicity();
        Float_t NROCHits = ModMult.ROCMultiplicity(j);
        Float_t NRows = ModMult.NumberOfRows(j);
        Float_t NCols = ModMult.NumberOfColumns(j);
        Float_t MeanPh = ModMult.MeanPh(j);  
        Float_t MeanPhPuls = ModMult.MeanPhPuls(j); 
        if(NROCHits >0) Multiplicity->Fill(EvN-1, j,NModHits ,NROCHits ,NRows ,NCols ,MeanPh, MeanPhPuls); 
      }
      ModMult.Clean();
      OldEvN = EvN;
    }
    
    bool SelPixel = 0;
    if(row ==5 && col ==5)SelPixel = 1;
    if(roc>-1&&col>-1)ModMult.Add(roc, row, col, ph, SelPixel);
 }
 
 oFile->Write();
 oFile->Close();          
 
}

/*
Digis->Draw("eventNr>>h1(120001, -0.5, 120000.5)");
 TH1F h2("h2", "h2", 71, -0.5, 70.5);
 TH1F h3("h3", "h3", 551, -0.5, 550.5);
 Float_t storeBinC=0;
 int EvN=0;
 for(int i =0; i < 120001; ++i){
  Float_t binC = h1->GetBinContent(i);
  h2->Fill(binC);
  if(binC > storeBinC){
    // cout << "Event Number " << i-1 << " Multiplicity " << binC << endl;
    EvN = i-1;
    storeBinC = binC;
  } 
  if(binC != 0) h3->Fill(binC);
}
cout << "Biggest Event is: " << EvN << " With Module Mult: " << storeBinC << endl;
h3->Draw(); 
*/
