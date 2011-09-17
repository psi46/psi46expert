void ROCOccupancy(Int_t StartRun, Int_t EndRun){

    using namespace std;
       
   
   //string FileRoot = "/home/l_tester/log/bt05r";
   string FileInName = "ProcessedData.root";
   char RunNumber[6];
    
    
    Float_t j = 0;    
    for(int RunN = StartRun; RunN <=EndRun; ++RunN){
      string FileRoot = "/d00/icali/PixelHwStudies/PSIStuff/log/bt05r";        
      sprintf(RunNumber, "%.6d", RunN);
      FileRoot = FileRoot + RunNumber + "/";
      FileInName = FileRoot + FileInName;
        
      TFile *iFile = new TFile((const char*)FileInName.c_str());
      if(!iFile->IsZombie()){
        TNtuple *Digis = (TNtuple*)iFile->FindObjectAny("events");
        Int_t EvN, roc, ROCHits, MeanPh, NCols;
        Digis->SetBranchAddress("ROCHits", &ROCHits);
        Digis->SetBranchAddress("MeanPh", &MeanPh);
        Digis->SetBranchAddress("roc", &roc);
        Digis->SetBranchAddress("NCols", &NCols);
        Digis->SetBranchAddress("EvN", &EvN);
        Int_t iNTlenght = Digis->GetEntries();
        
        Int_t MaxROCHits =0;
        for(int i =0 ; i < iNTlenght; ++i){
          Digis->GetEntry(i);
          if(ROCHits > MaxROCHits) MaxROCHits = ROCHits;
        }
        cout << "Run: " << RunN << " MaxROCHits: " << MaxROCHits << endl;
      }
   }
}
    
      

