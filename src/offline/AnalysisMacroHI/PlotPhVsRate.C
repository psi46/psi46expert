//void PlotPhVsRate(Int_t StartRun, Int_t EndRun){
{
    using namespace std;
       
   
   //string FileRoot = "/home/l_tester/log/bt05r";
   string FileInNameOrig = "thistos.root";
   char RunNumber[6];
    
    TH1D* HistoVect[100];
    Float_t j = 0;    
    Int_t ph, roc, row, col;
    for(int RunN = 4863; RunN <=4870; ++RunN){
      string FileRoot = "/d00/icali/PixelHwStudies/PSIStuff/log/bt05r";        
      sprintf(RunNumber, "%.6d", RunN);
      string RunNumberSt = RunNumber ;
      string FileInName = FileRoot + RunNumber + "/" + FileInNameOrig;
      
      
      TFile *iFile = new TFile((const char*)FileInName.c_str());
      
      if(!iFile->IsZombie()){
	    cout<<"Opening File " << FileInName << endl;
        TNtuple *Digis = (TNtuple*)iFile->FindObjectAny("events");
        
        Digis->SetBranchAddress("ph", &ph);
        Digis->SetBranchAddress("roc", &roc);
        Digis->SetBranchAddress("row", &row);
        Digis->SetBranchAddress("col", &col);
        Int_t iNTlenght = Digis->GetEntries();
        
        TH1D* HistoVect[j] = new TH1D(RunNumber, RunNumber, 801, -400.5, 400.5);
        for(int i =0 ; i < iNTlenght; ++i){
          Digis->GetEntry(i);
          if(roc==4&&row==5&&col==5) HistoVect[j]->Fill(ph);
        }  
	    
        HistoVect[j]->SetLineColor(j);
        HistoVect[j]->Draw("same");
	    c1->SetLogy(1);  
	    c1->Draw();
	    j++;
      }
      
   }
  
   
}
    
      

