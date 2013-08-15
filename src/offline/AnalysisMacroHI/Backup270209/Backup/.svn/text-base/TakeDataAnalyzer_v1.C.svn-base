{
Int_t NTlenght = events->GetEntries();
Int_t MaxEvent;
events->SetBranchAddress("eventNr", &MaxEvent);
events->GetEntry(NTlenght -1);
cout << " MaxEvent " << MaxEvent << endl;

events->Draw("eventNr>>h1(120001, -0.5, 120000.5)");
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
}
