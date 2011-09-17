{
 TFile *iFile = new TFile("thistos.root");
 TNtuple *Digis = (TNtuple*)iFile->FindObjectAny("events");    
 
 TFile* oFile = new TFile("thistosNew.root", "RECREATE");
 TNtuple* DigisAndSignal = new TNtuple("DigisAndSignal", "DigisAndSignal","row:col:roc:ph:vcal:eventNr",1000000);

 Int_t NTlenght = Digis->GetEntries();
 Int_t MaxEvent;
 Digis->SetBranchAddress("eventNr", &MaxEvent);
 Digis->GetEntry(NTlenght -1);
 cout << " MaxEvent " << MaxEvent << endl;

 Int_t EvN, row, col, roc, ph;
 Float_t vcal;
 Digis->SetBranchAddress("row", &row);
 Digis->SetBranchAddress("col", &col);
 Digis->SetBranchAddress("roc", &roc);
 Digis->SetBranchAddress("ph", &ph);
 Digis->SetBranchAddress("vcal", &vcal);
 Digis->SetBranchAddress("eventNr", &EvN);
 
 Int_t OldEvN = -1;
 for(Int_t i =0; i < NTlenght; ++i){
   Digis->GetEntry(i);
   DigisAndSignal->Fill(row, col, roc, ph, vcal, EvN);
   if(EvN != OldEvN){
      OldEvN = EvN;
      for(Int_t j=0; j< 16; ++j){
         DigisAndSignal->Fill(5, 5, j, 100, 100, EvN);
      }
   }
 }
 oFile->Write();
 oFile->Close();
}
