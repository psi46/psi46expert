void PixelMergeSmallFiles(){

    using namespace std;
    TFile * oFilebDist = new TFile("bDistr2.root", "RECREATE");
    TNtuple* SimEventsGlobal = new TNtuple("SimEventGlob", "SimEventGlob", "EvN:b");
    
    TFile * oFileLinks = new TFile("Links2.root", "RECREATE");
    TNtuple* LinksGlobal = new TNtuple("LinksGlob", "LinksGlob", "EvN:fedid:linkn:nHits");
    
    char FileInNumber[5];
    string FileInPath = "/net/pstore01/d00/scratch/icali/CMSSW_2_1_11/PixelAnalysis/PixelNTuple_hydjet_x2_mb_oldPL_d20081106/";
    string FileInNameRoot= "hydjet_x2_mb_oldPL_d20081106_r0";
    
    Float_t j = 0;    
    for(int FileN = 901; FileN <1801; ++FileN){
    
         sprintf(FileInNumber, "%.5d", FileN);
         string FileInName = FileInPath+FileInNameRoot+FileInNumber+".root";
         TFile *iFile = new TFile((const char*)FileInName.c_str());
         if(!iFile->IsZombie()){
           PixelAnalyzer->cd();
    
           TNtuple *FEDLinks = (TNtuple*)iFile->FindObjectAny("Links");
           TNtuple *SimEvents = (TNtuple*)iFile->FindObjectAny("SimEvent");
    
           Float_t FEDEvN, fedid, linkn, nHits;
           FEDLinks->SetBranchAddress("EventN",&FEDEvN);
           FEDLinks->SetBranchAddress("fedid",&fedid);
           FEDLinks->SetBranchAddress("linkn",&linkn);
           FEDLinks->SetBranchAddress("nHits",&nHits);
           Long64_t FEDLinkLenght =FEDLinks->GetEntries(); 
    
           Float_t SimEvN, b;
           SimEvents->SetBranchAddress("EventN",&SimEvN);
           SimEvents->SetBranchAddress("mult",&b);
           Long64_t SimEventLenght =SimEvents->GetEntries(); 
           
           Long64_t i;
           for(i=0; i < SimEventLenght; ++i){
             SimEvents->GetEntry(i);
             SimEventsGlobal->Fill(i, b);
           } 
           
          Float_t OldEvN= -1;        
          for(i=0; i < FEDLinkLenght; ++i){
             FEDLinks->GetEntry(i);
             if(OldEvN != FEDEvN){
                OldEvN= FEDEvN;
                ++j;
             }
             LinksGlobal->Fill(j, fedid, linkn, nHits);
             
          }
      }
    }
    oFilebDist->Write();
    oFileLinks->Write();
}
    //bDistr->Draw();
      /*
      int FedN;
      cout <<"Fed Number : " << endl;
      cin >> FedN;
     
     
     
     
     
     TProfile * HitForEv_fd0_TP[9];
     TProfile * TimeHitForEv_fd0_TP[9];
     
     HitForEv_fd0_TP[0] = new TProfile("HitForEv_fd0ln1","HitForEv_fd0ln1", 100, 0, 99);
     HitForEv_fd0_TP[1] = new TProfile("HitForEv_fd0ln2","HitForEv_fd0ln2", 100, 0, 99);
     HitForEv_fd0_TP[2] = new TProfile("HitForEv_fd0ln3","HitForEv_fd0ln3", 100, 0, 99);
     HitForEv_fd0_TP[3] = new TProfile("HitForEv_fd0ln4","HitForEv_fd0ln4", 100, 0, 99);
     HitForEv_fd0_TP[4] = new TProfile("HitForEv_fd0ln5","HitForEv_fd0ln5", 100, 0, 99);
     HitForEv_fd0_TP[5] = new TProfile("HitForEv_fd0ln6","HitForEv_fd0ln6", 100, 0, 99);
     HitForEv_fd0_TP[6] = new TProfile("HitForEv_fd0ln7","HitForEv_fd0ln7", 100, 0, 99);
     HitForEv_fd0_TP[7] = new TProfile("HitForEv_fd0ln8","HitForEv_fd0ln8", 100, 0, 99);
     HitForEv_fd0_TP[8] = new TProfile("HitForEv_fd0ln9","HitForEv_fd0ln9", 100, 0, 99);
        
     TimeHitForEv_fd0_TP[0] = new TProfile("TimeHitForEv_fd0ln1","TimeHitForEv_fd0ln1", 100, 0, 99);
     TimeHitForEv_fd0_TP[1] = new TProfile("TimeHitForEv_fd0ln2","TimeHitForEv_fd0ln2", 100, 0, 99);
     TimeHitForEv_fd0_TP[2] = new TProfile("TimeHitForEv_fd0ln3","TimeHitForEv_fd0ln3", 100, 0, 99);
     TimeHitForEv_fd0_TP[3] = new TProfile("TimeHitForEv_fd0ln4","TimeHitForEv_fd0ln4", 100, 0, 99);
     TimeHitForEv_fd0_TP[4] = new TProfile("TimeHitForEv_fd0ln5","TimeHitForEv_fd0ln5", 100, 0, 99);
     TimeHitForEv_fd0_TP[5] = new TProfile("TimeHitForEv_fd0ln6","TimeHitForEv_fd0ln6", 100, 0, 99);
     TimeHitForEv_fd0_TP[6] = new TProfile("TimeHitForEv_fd0ln7","TimeHitForEv_fd0ln7", 100, 0, 99);
     TimeHitForEv_fd0_TP[7] = new TProfile("TimeHitForEv_fd0ln8","TimeHitForEv_fd0ln8", 100, 0, 99);
     TimeHitForEv_fd0_TP[8] = new TProfile("TimeHitForEv_fd0ln9","TimeHitForEv_fd0ln9", 100, 0, 99);
     
     TProfile * FIFOIEmptyTime1_TP = new TProfile("FIFOIEmptyTime1_fd0ln09","FIFOI RO Time ln1-4", 100, 0, 99);
     FIFOIEmptyTime1_TP->GetXaxis()->SetTitle("Event n");
     FIFOIEmptyTime1_TP->GetYaxis()->SetTitle("Time [us]");
     
     TProfile * FIFOIEmptyTime2_TP = new TProfile("FIFOIEmptyTime2_fd0ln09","FIFOI RO Time ln5-9", 100, 0, 99);
     FIFOIEmptyTime2_TP->GetXaxis()->SetTitle("Event n");
     FIFOIEmptyTime2_TP->GetYaxis()->SetTitle("Time [us]");
     
     TProfile * FIFOIEmptyTimeTotal_TP = new TProfile("FIFOIEmptyTimeTotal_fd0ln09","FIFOII RO Time", 100, 0, 99);
     FIFOIEmptyTime2_TP->GetXaxis()->SetTitle("Event n");
     FIFOIEmptyTime2_TP->GetYaxis()->SetTitle("Time [us]");
     
     TH1F * FIFOIIHits_TP = new TH1F("FIFOIIHits_fd0ln09","FIFOIIHits_fed0ln09", 100, 0, 99);
      
     Long64_t eventN = -1, HitInBuffer[100][9], TotalHits;
     float timeInData[100][9], timeOutData[100][9]; 
     Long64_t VecLnIndex;
     //for(int n=0; n < 100; ++n){
     //  for(int j =0; j< 9; ++j){
     //     HitInBuffer[n][j] = 0;
     //  }
     //}    
           
     for(Long64_t i=0; i < BuffLenght; ++i){
        FEDLinks->GetEntry(i);
        if(fedid==FedN && linkn<10) {
             if(linkn==1){
                 ++eventN; 
                 TotalHits=0;
             }
	     // nHits = 400;
             TotalHits += nHits;
             VecLnIndex = linkn -1;
             timeInData[eventN][VecLnIndex] =  nHits *0.150;
             timeOutData[eventN][VecLnIndex] = nHits * 0.025;
                   
             HitForEv_fd0_TP[VecLnIndex]->Fill(eventN,nHits);
             TimeHitForEv_fd0_TP[VecLnIndex]->Fill(eventN,  timeInData[eventN][VecLnIndex]);
             FIFOIIHits_TP->Fill(eventN, TotalHits);
        } 
     }
     
     float EvOutOfTime =0, EvOutOfTimeTotal =0 ;
     int Nevents = eventN + 1;
     Long64_t FIFOEmptyTime =0, FIFOEmptyTimeTotal =0 ;
     for( i=0; i<Nevents; ++i){
        FIFOEmptyTimeTotal = timeInData[i][0] + timeOutData[i][0];
        for(Long64_t j=1; j<9; ++j){
           if(FIFOEmptyTimeTotal < timeInData[i][j+1]) FIFOEmptyTimeTotal = timeInData[i][j+1];
           FIFOEmptyTimeTotal += timeOutData[i][j];
        }
        
        FIFOEmptyTime = timeInData[i][0] + timeOutData[i][0];
        for(Long64_t j=1; j<3; ++j){
           if(FIFOEmptyTime < timeInData[i][j+1]) FIFOEmptyTime = timeInData[i][j+1];
           FIFOEmptyTime += timeOutData[i][j];
        }
        FIFOIEmptyTime1_TP->Fill(i,FIFOEmptyTime);
        if(FIFOEmptyTime > 125) ++EvOutOfTime;
                
        FIFOEmptyTime = timeInData[4][0] + timeOutData[4][0];
        for(Long64_t j=5; j<9; ++j){
           if(FIFOEmptyTime < timeInData[i][j+1]) FIFOEmptyTime = timeInData[i][j+1];
           FIFOEmptyTime += timeOutData[i][j];
        }
        FIFOIEmptyTime2_TP->Fill(i,FIFOEmptyTime);
        
        FIFOIEmptyTimeTotal_TP->Fill(i,FIFOEmptyTimeTotal);
        if(FIFOEmptyTime > 125) ++EvOutOfTime;
        if(FIFOEmptyTimeTotal > 125) ++EvOutOfTimeTotal;
    }
    cout << "Events out of Time [%] " << EvOutOfTime/Nevents*100 << endl;
    cout << "Events out of Time Total[%] " << EvOutOfTimeTotal/Nevents*100 << endl;
    //Genral Plots
     
     BPixDistri->Draw("Mhits/(ncolumns*nrows)*100:layer>>BOccupancy","","box");
     BOccupancy->SetTitle("Barrel Occupancy");
     BOccupancy->GetXaxis()->SetTitle("Layer");
     BOccupancy->GetYaxis()->SetTitle("Occupancy [%]");

     FEDLinks->Draw("nHits:fedid>>HitsVsFedId","","box");
     HitsVsFedId->SetTitle("Hits vs. FED ");
     HitsVsF edId->GetXaxis()->SetTitle("FED n");
     HitsVsFedId->GetYaxis()->SetTitle("n Hits");
 
     FEDLinks->Draw("nHits:linkn>>HitsVsLinkn","","box");
     HitsVsLinkn->SetTitle("Hits vs. Link ");
     HitsVsLinkn->GetXaxis()->SetTitle("Link n");
     HitsVsLinkn->GetYaxis()->SetTitle("n Hits");
     
     FEDLinks->Draw("nHits>>NHits","","");
     NHits->SetTitle("Hits per RO link");
     NHits->GetXaxis()->SetTitle("Hits per RO link");
     NHits->GetYaxis()->SetTitle("Entries");
     
     */
//}

