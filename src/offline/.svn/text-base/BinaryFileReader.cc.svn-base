#include <iostream>
#include <stdexcept>

#include <TSystem.h>
#include "TH1F.h"
#include "TH2F.h"
#include "TSpectrum.h"
#include "TMath.h"


using namespace std;

#include "BinaryFileReader.h"
#include "PHCalibration.h"


BinaryFileReader::BinaryFileReader(const char* f,int nroc,int ref){
  // r-type constructor, no layermap
  strncpy(fInputFileName,f,200);
  fNROC=nroc;
  fIsModule=(nroc==16);
  for(int r=0; r<nroc; r++){	 fLayerMap[r]=0;  }
  sprintf(fTag,"%06d",ref);
  fcfg=0;
  init();
}


BinaryFileReader::BinaryFileReader(const char* f,const char *layermap, 
				   const char *tag, 
				   ConfigReader* cfg, const char *cfgtag,
				   int levelMode) {
  // new constructor, layer-info passed as an array
  // layer number per roc (readout-order, not I2C chipID)
  // array terminated by -1
  strncpy(fInputFileName,f,200);
  fNROC=0;
  while( (fNROC<16) && !(layermap[fNROC]=='\0') )
  {
	 fLayerMap[fNROC]=int(layermap[fNROC]-'0');
	 fNROC++;
  }
  fIsModule=(fNROC==16);
  strncpy(fTag,tag,20);
  fcfg=cfg;
  strncpy(fcfgtag,cfgtag,20);
  fLevelMode=levelMode;  // 0 (default)=use levels from config, 1=re-determine levels
  
  init();
}



void BinaryFileReader::init(){
 
  selROC = 4;
  selROW = 24;
  selCOL = 15;

  fPHcal = NULL;
  fMaxEvent = 9999999;
  fHeader = fNextHeader = -1;
  fEOF = 0;
  // init run statistics
  fnRecord            = 0;
  fnTrig              = 0;
  fnTrigExternal      = 0;
  fnTrigInternal      = 0;
  fnTrig              = 0;
  fnData              = 0;
  fnDataWithHits      = 0;
  fnReset             = 0;
  fDtReset            = 0;
  fTlastReset         = 0;
  fnOvflw             = 0;
  fnInfiniteRO        = 0;
  fnBadTrailer        = 0;
  fnCorrupt           = 0;
  fnNoTokenPass       = 0;
  fnInvalidAddress    = 0;
  fnPixel000          = 0;
  fnCalInject         = 0;
  fnCalInjectHistogrammed=0;
  fnTruncated         =0;
  fnResync    =0;
  fMostRecentTrigger=-1;
  fnTmaxError=0;
  fTime=0;
  fTmin=1000000000000LL;
  fTmax=0;
  fNTS5=0;
  fAnaMin=-500;
  fCluCut =2;
  fDeconvTest=-1;
  fTCT=-1;
  fTBMTrigger=-1;
  fSyncOk=1;       // 
  fResync=0;       // wait until after next reset
  fRequireSync=0;

  // (un-)initialize levels
  fUbTBM=fUnInitialized;  
  for(int roc=0; roc<fNROC; roc++){
    fUbROC[roc]=fUnInitialized;
	 fDeconvolution[roc]=0.02;
	 fDeconvolution2[roc]=0;
	 fAnaOffset[roc]=-200;
	 fAnaSlope[roc]=1;
  }

  // double column readout loss bookkeeping
  for(int dc=0; dc<fNROC*26; dc++){
    fDCloss[dc]=0;
    fDCdeadUntil[dc]=0LL;
  }

  // get some configs, if available
  if(fcfg){
    fcfg->get(Form("%s.clusterCut",fcfgtag),fCluCut,2);
    unsigned int nroc;
    nroc=fNROC;
    fcfg->get(Form("%s.Latency",fcfgtag),fLatency,142);	      
    fcfg->geta(Form("%s.anaOffset",fcfgtag),nroc,fAnaOffset);	      
    fcfg->geta(Form("%s.anaSlope", fcfgtag),nroc,fAnaSlope);
    fcfg->geta(Form("%s.deconvolution",  fcfgtag),nroc,fDeconvolution);
    fcfg->geta(Form("%s.deconvolution2", fcfgtag),nroc,fDeconvolution2);
    // get levels from config file ultra black and levels in one row per TBM/ROC
    if(!fLevelMode){
      cout <<"reading levels from cfg file" << endl;
      int buf[10];  // assemble UB and Levels here
      fcfg->geta(Form("%s.tbmLevels", fcfgtag),6,buf);
      fUbTBM=buf[0];     for(int i=0; i<5; i++)fTBM[i]=buf[i+1];
      for(int roc=0; roc<fNROC; roc++){
	fcfg->geta(Form("%s.rocLevels", fcfgtag),roc,8,buf);
	fUbROC[roc]=buf[0];  for(int i=0; i<7; i++){ fROC[roc][i]=buf[i+1]; }
      }
    }else{
      cout << "ignoring levels from cfg file" << endl;
    }

    fcfg->get(Form("%s.CalTestRoc",fcfgtag),fCalTestRoc, 3);	      
    fcfg->get(Form("%s.CalTestCol",fcfgtag),fCalTestCol,20);	      
    fcfg->get(Form("%s.CalTestRow",fcfgtag),fCalTestRow,20);	      
  }

  // create control histograms

  char name[100], title[100];
  char module[10]="";
  if(fNROC==16){
	 sprintf(module,"module_");
  }
  /*
  cout << strlen(fTag) << endl;
  char tag[21]="";
  if(strlen(fTag)>0){
	 snprintf(tag,20,"_%s",fTag);  // prepend an underscore if we have a tag
  }
  */

  sprintf(name,"%subtbm%s",module,fTag);
  hUBTBM=new TH1F(name,"tbm ultrablack",nBinLH,LHMin,LHMax);

  sprintf(name,"%slvltbm%s",module,fTag);
  hLVLTBM=new TH1F(name,"tbm levles",nBinLH,LHMin,LHMax);

  sprintf(name,"nHits");
  hNHit=new TH1F(name,"number of hits",40,-0.5,39.5);

  sprintf(name,"%sGino%s,",module,fTag);
  hGino=new TH1F(name,"unexpected hits",10,-0.5, 9.5);

  hPH=new TH1F(Form("PH%s", fTag),Form("PH%s", fTag),4000,-2000., 2000.);
  hVcal=new TH1F(Form("Vcal%s", fTag),Form("Vcal%s", fTag),2000,-200.,1800.);
  
  for(int roc=0; roc<fNROC; roc++){
	 sprintf(name,"%sDeconv%d%s,",module,roc,fTag);
	 sprintf(title,"%sDeconv%d,",module,roc);
	 hDeconv[roc]=new TH2F(name,title,350,-400,900, 350, -400,900);
  }

  for(int roc=0; roc<fNROC; roc++){

    sprintf(name,"%subb_%d%s",module,roc,fTag);
    sprintf(title,"ultrablack/black roc %d",roc);
    hUBBROC[roc]=new TH1F(name,title,nBinLH,LHMin,LHMax);
    sprintf(name,"%slvl_%d%s",module,roc,fTag);
    sprintf(title,"levels roc %d",roc);
    hADROC[roc]=new TH1F(name,title,nBinLH,LHMin,LHMax);

    sprintf(name,"%sph_%d%s",module,roc,fTag);
    sprintf(title,"pulse height %d",roc);
    hPHROC[roc]=new TH1F(name,title,nBinLH,LHMin,LHMax);

    sprintf(name,"%s3rdclk_%d%s",module,roc,fTag);
    sprintf(title,"3rd clk %d",roc);
    h3rdClk[roc]=new TH1F(name,title,nBinLH,LHMin,LHMax);

    sprintf(name,"%sphVal_%d%s",module,roc,fTag);
    sprintf(title,"pulse height in vcal DAC units %d",roc);
    hPHVcalROC[roc]=new TH1F(name,title,nBinLH,-100., 800.);

    sprintf(name,"%sMap_%d%s,",module,roc,fTag);
    sprintf(title,"hitmap %d",roc);
    hRocMap[roc]=new TH2I(name,title,52,0,52,80,0,80);

    sprintf(name,"%sNHits_%d%s,",module,roc,fTag);
    sprintf(title,"number of hits %d",roc);
    hNHitRoc[roc]=new TH1F(name,title,20,-0.5,19.5);

    sprintf(name,"%sMapCal_%d%s,",module,roc,fTag);
    sprintf(title,"Calinject Map %d",roc);
    hRocMapCal[roc]=new TH2I(name,title,52,0,52,80,0,80);

    sprintf(name,"%sMapExt_%d%s,",module,roc,fTag);
    sprintf(title,"Ext Trigger Map %d",roc);
    hRocMapExt[roc]=new TH2I(name,title,52,0,52,80,0,80);

    sprintf(name,"%sMapInt_%d%s,",module,roc,fTag);
    sprintf(title,"Int Trigger Map %d",roc);
    hRocMapInt[roc]=new TH2I(name,title,52,0,52,80,0,80);

  }

  if(fIsModule){
	 sprintf(name,"hitmap");
	 sprintf(title,"hitmap");
	 hModMap=new TH2I(name,title,160,0,160,416,0,416);
	 sprintf(name,"ModMapCal%s,",fTag);
	 sprintf(title,"Calinject map");
	 hModMapCal=new TH2I(name,title,160,0,160,416,0,416);
	 sprintf(name,"ModMapExt%s,",fTag);
	 sprintf(title,"Ext Trigger Map");
	 hModMapExt=new TH2I(name,title,160,0,160,416,0,416);

	 sprintf(name,"ModMapInt%s,",fTag);
	 sprintf(title,"Int Trigger Map");
	 hModMapInt=new TH2I(name,title,160,0,160,416,0,416);

  }

  // clone histograms to show used decoding levels
  double somelargenumber=100000.;   // dummy for filling histos
  hLVLTBMUsed= new TH1F(*hLVLTBM);
  hLVLTBMUsed->SetName(Form("tbmlv%s",fTag));
  for(int i=0; i<5; i++){ 
    hLVLTBMUsed->Fill(fTBM[i],somelargenumber);
  }
  for(int roc=0; roc<fNROC; roc++){
    hADROCUsed[roc]=new TH1F(*hADROC[roc]);
    hADROCUsed[roc]->SetName(Form("roclvl_%d%s",roc,fTag));
    for(int i=0; i<7; i++){
      hADROCUsed[roc]->Fill(fROC[roc][i],somelargenumber);
    }
  }

  fnCalTestEvent=0;
  sprintf(name,"%sCalEff_%s",module,fTag);
  sprintf(title,"cal efficiency history  roc %d  col %d  row %d",
	  fCalTestRoc,fCalTestCol,fCalTestRow);
  hCalTestHistory=new TH1F(name,title,1000,0., 10000);

  sprintf(name,"%sDcolHist_%s",module,fTag);
  sprintf(title,"double column history  roc %d, dcol %d",
	  fCalTestRoc,int(fCalTestCol/2));
  hCalTestDcolHistory=new TH1F(name,title,1000,0., 10000);

  sprintf(name,"%sCalEffReset_%s",module,fTag);
  sprintf(title,"cal efficiency after reset  roc %d  col %d  row %d",
	  fCalTestRoc,fCalTestCol,fCalTestRow);
  hCalTestReset=new TH1F(name,title,100,0., 0.01);

  sprintf(name,"%sDcolReset_%s",module,fTag);
  sprintf(title,"dcol multiplicity  after reset  roc %d  col %d  row %d",
	  fCalTestRoc,fCalTestCol,fCalTestRow);
  hCalTestDcolReset=new TH1F(name,title,100,0., 0.01);

  sprintf(name,"%sDcolHistRest_%s",module,fTag);
  sprintf(title,"dcol hits after reset roc %d, dcol %d",
	  fCalTestRoc,int(fCalTestCol/2));
  hCalTestNReset=new TH1F(name,title,100,0., 0.01);

  sprintf(name,"%sTestFnd_%s",module,fTag);
  sprintf(title,"random hits in efficient dcol %d, dcol %d",
	  fCalTestRoc,int(fCalTestCol/2));
  hCalTestFnd=new TH1F(name,title,20,0., 20.);

  sprintf(name,"%sTestMiss_%s",module,fTag);
  sprintf(title,"random hits in inefficient dcol %d, dcol %d",
	  fCalTestRoc,int(fCalTestCol/2));
  hCalTestMiss=new TH1F(name,title,20,0., 20);

  sprintf(name,"%sOtherFnd_%s",module,fTag);
  sprintf(title,"random hits in other dcol %d, dcol %d",
	  fCalTestRoc,int(fCalTestCol/2));
  hCalOtherFnd=new TH1F(name,title,20,0., 20);

  sprintf(name,"%sOtherMiss_%s",module,fTag);
  sprintf(title,"random hits in other dcol %d, dcol %d",
	  fCalTestRoc,int(fCalTestCol/2));
  hCalOtherMiss=new TH1F(name,title,20,0., 20);

  // initialize geometry utitilty
  fRocGeometry=RocGeometry(27*0.0150, 40.5*0.0100);

  if(fIsModule){
  eventTree = new TTree("events", "events");
  eventTree->Branch("row", &tRow, "row/I");
  eventTree->Branch("col", &tCol, "col/I");
  eventTree->Branch("roc", &tRoc, "roc/I");
  eventTree->Branch("ph", &tPH, "ph/I");
  eventTree->Branch("vcal", &tVcal, "vcal/F");
  eventTree->Branch("eventNr", &fnDataWithHits, "eventNr/I");
  
  clusterTree = new TTree("clusters", "clusters");
  clusterTree->Branch("charge", &tCluCharge, "charge/F");
  clusterTree->Branch("size", &tCluSize, "size/I");

  }
}

// ----------------------------------------------------------------------
BinaryFileReader::~BinaryFileReader(){
  // delete the biggest chunks
  for(int i=0; i<fNROC; i++){
	 delete hRocMap[i];
	 delete hRocMapInt[i];
	 delete hRocMapExt[i];
	 delete hRocMapCal[i];
  }
  if(fIsModule){
	 delete hModMap;
	 delete hModMapCal;
	 delete hModMapExt;
	 delete hModMapInt;
  }
}

// ----------------------------------------------------------------------
int BinaryFileReader::open() {

  fInputBinaryFile = new ifstream(fInputFileName);

  if (fInputBinaryFile->is_open()) {

    cout << "--> reading from file " << fInputFileName << endl;

	 unsigned short word=readBinaryWord();
	 while( !((word&0xFF00)==0x8000) && (fEOF==0) ){
		word=readBinaryWord();
	 }

	 if( fEOF ) {
		fNextHeader=-1;
		cout << "--> file contains no headers:" << fInputFileName << endl;
		return 1;
	 }else if(word&0x8000){
		fNextHeader = word & 0x00FF;
		return 0;
	 }else{
		cout << "we should never be here" << endl;
		return 1;
	 }
  } else {
    cout << "--> ERROR: unable to open file " << fInputFileName << endl;
    return 1;
  }
};


// ----------------------------------------------------------------------
unsigned short BinaryFileReader::readBinaryWord() {
 
  if (fInputBinaryFile->eof()) { fEOF = 1; return 0; }
  unsigned char a = fInputBinaryFile->get();
  if (fInputBinaryFile->eof()) { fEOF = 1; return 0; }
  unsigned char b = fInputBinaryFile->get();
  if (fInputBinaryFile->eof()) { fEOF = 1; return 0; }
  unsigned short word  =  (b << 8) | a;

  //  cout << Form("readBinaryWord: %02x %02x word: %04x ", a, b, word) << endl;

  return word;
}


// ----------------------------------------------------------------------
void BinaryFileReader::nextBinaryHeader() {
  /* read data into fBuffer until the next header is encountered
	  afterwards :
	  fNextHeader = type of header encountered (=header of the next data block)
	  fHeader     = Header at the beginning of the block now in fBuffer
	  new triggers are put on the trigger stack 
  */
	  
  if(fEOF) return;
  
  //clear buffer
  for (int i = 0; i < NUM_DATA; ++i) {
	 fBuffer[i] = 0;
	 fData[i]=0;
  }

  // the header has already been read in by the previous call
  // it has been stored in fNextHeader
  fHeader=fNextHeader;
  // get at least three words (=time stamp)
  fBufferSize=0;
  for(fBufferSize=0; fBufferSize<3; fBufferSize++){
	 unsigned short word=0;
	 word=readBinaryWord();
	 if(fEOF) break;
	 fBuffer[fBufferSize]=word;
  }
  if(fEOF){
	 // no more data
	 return ;
  }


  while (fEOF==0) {
	 unsigned short word=0;
	 word = readBinaryWord();
	 if (fEOF) break;
	 
	 if( (word&0x8000)==0 ){
		// not a header, keep adding
		if( fBufferSize < NUM_DATA) {
		  fBuffer[fBufferSize++] = word;
		}else{
		  // skip to avoid overrun and warn
		  cout <<  msgId() << "internal buffer overflow" << endl;
		  fSyncOk=0;
                  fSyncOk=1;
		}
	 }else{
		// header bit was set, was it a valid header?
		if( (word&0x7F00)==0) {
		  fNextHeader = word & 0x00FF;
		  break;
		}else{
		  cout << msgId() 
				 << "illegal header word ignored " << Form("%4x",word) 
				 <<endl;
		  if( fBufferSize < NUM_DATA) {
			 fBuffer[fBufferSize++] = word;
		  }else{
			 // skip to avoid overrun and warn
			 cout << msgId() << "internal buffer overflow" << endl;
		  }
		}
	 }
  }
}

// ----------------------------------------------------------------------
int BinaryFileReader::decodeBinaryData() { 

  int j(0);

  fNHit=0;
  fBadTrailer=0;
  fNoTokenPass=0;
  fTruncated=0;

  for(int roc=0; roc<fNROC; roc++){ fHitROC[roc]=0; }

  if (fHeader > 0) {  
    //cout << Form(" Event at time  %04x/%08x with Header %d", fUpperTime, fLowerTime, fHeader) << endl;
  } else {
    cout << "No valid header, skipping this event" << endl;
    return -1;
  }
  


  for (int i = 3; i < fBufferSize; i++) {
    int value = fBuffer[i] & 0x0fff;
    if (value & 0x0800) value -= 4096;
    fData[i-3] = value;
    ++j;
  }

  fBufferSize -=3;
  
  if (0) {
    for (int i = 0; i < fBufferSize; ++i) { 
      cout << Form(" %04x ", fData[i]);
    }
    cout << endl;
    
    for (int i = 0; i < fBufferSize; ++i) { 
      cout << Form(" %6i ", fData[i]);
    }
    cout << endl;
  }
  

  if(fHeader&kData){
    // level bootstrap mode
    if(fUbTBM==fUnInitialized){
      // guess TBM levels using the first header
      float a=-(fData[0]+fData[1]+fData[2])/3./4.;
      fUbTBM = int( -3*a);
      fTBM[0]= int( -2*a);
      fTBM[1]= int( -0.5*a);
      fTBM[2]= int(  0.5*a);
      //cout << fData[0] << " "  << fData[1] << " " << fData[2] << " " << fData[3] <<endl;
      cout << Form("UB/B(TBM)") << (fData[0]+fData[1]+fData[2])/3.  << " "  << fData[3];
      cout << "  set levels  UB<" << fUbTBM
	   << "   D>"  << fTBM[0]  <<endl;
    }
    

    // tbm header??
    if((fData[0]>fUbTBM)||(fData[1]>fUbTBM)||(fData[2]>fUbTBM)){
      cout << "bad TBM header ? Check levels !!" << endl;
      throw std::runtime_error( "");
      return 0;
    }else{
      // tbm header ok, copy 
      for(int i=0; i<8; i++){  fTBMHeader[i]=fData[i]; }
    }

    // does a tbm trailer follow right behind?
    int i=8;  // pointer
    if((fData[8]<fUbTBM)&&(fData[9]<fUbTBM)&&
       (fData[10]>fTBM[1])&&(fData[10]<fTBM[2])){
      fNoTokenPass=1;
      fnNoTokenPass++;
    }else{
      // chop data along UBs
      for(int roc=0; roc<fNROC; roc++){
	// apply deconvolution for the black level and 3rd hit too
	fData[i+1]+= int (fDeconvolution[roc]*(fData[i+1]-fData[i  ]) + 0.49);
	fData[i+2]+= int (fDeconvolution[roc]*(fData[i+2]-fData[i+1]) + 0.49);
	// ROC header follows, are we in address level bootstrap mode?
	if(fUbROC[roc]==fUnInitialized){
	  cout << Form("UB/B(%3d)",roc) << fData[i] << " "  << fData[i+1];
	  fUbROC[roc] =int(fData[i]*0.8+fData[i+1]*0.2);
	  fROC[roc][0]=int(fData[i]*0.5+fData[i+1]*0.5);
	  cout << "  set levels  UB<" << fUbROC[roc] 
	       << "   D>"  << fROC[roc][0]  <<endl;
	}
	
	fOffs[roc]=i;   // keep pointers to chip data
	i=i+3;          // move on to data
	while((fData[i]>fROC[roc][0])&&(fNHit<MAX_PIXELS)&&(i<fBufferSize)){
	  // not an UB: hit data !!!
	  fNHit++;
	  fHitROC[roc]++;
	  //cout << "hit roc=" << roc << "   :";
	  for(int k=0; k<6; k++){ 
	    if(k>0){
	      fData[i]+= int (fDeconvolution[roc]*(fData[i]-fData[i-1]) +0.49); 
	    }else{
	      fData[i]+= int ( fDeconvolution[roc]*fData[i]
			       -fDeconvolution2[roc]*fData[i-1] +0.49); 
	    }
	    i++;
	  }
	  //	cout << endl;
	}
      }// decode roc data
    }
    
    // TBM trailer
    for(int k=0; k<8; k++){  fTBMTrailer[k]=fData[i++]; }
    if(   (fTBMTrailer[0]<fUbTBM)
	  &&(fTBMTrailer[1]<fUbTBM)
	  &&(fTBMTrailer[2]>fUbTBM)&&(fTBMTrailer[2]<-fUbTBM) ){
      fBadTrailer=0;
      if(fNoTokenPass){ printTrailer();}
    }else{
      //so, we expected to find the trailer here but found something else
      // was this readout truncated by a reset ?
      //for(int i=-10; i<20; i++) cout << i << ")" << fData[fBufferSize-i]<< endl;
      if(    (fData[fBufferSize-8]<fUbTBM)
	     &&(fData[fBufferSize-7]<fUbTBM)
	     &&(fData[fBufferSize-6]>fUbTBM)&&(fData[fBufferSize-6]<-fUbTBM) 
	     ){
	// ok, fill the trailer (e.g. for status decoding), 
	// but mark the event as truncated
	for(int i=0; i<8; i++){ fTBMTrailer[i]=fData[fBufferSize-8+i]; }
	fTruncated=1;
	fBadTrailer=0;
	cout << msgId() << "truncated event" 
	     << Form("  next header = %4x",fNextHeader) << endl;
	//cout << endl << "fUbTBM=" <<fUbTBM  << endl;
	//for(int i=0; i<8; i++){ cout << fTBMTrailer[i] << " "; }
	//cout << endl;
	
	//printTrailer();
      }else if (fEOF){
	fBadTrailer=1; // don't use this event, but don't complain either
      }else{
	// something else went wrong
	fBadTrailer=1;
	cout << msgId() << "decodeBinaryData: bad trailer" << endl;
	dump(1);
	// force resync, just to be on the safe side
	if(0){
	  cout << fTBMTrailer[0]; if (fTBMTrailer[0]<fUbTBM) cout << " ok " << endl;
	  cout << fTBMTrailer[1]; if (fTBMTrailer[1]<fUbTBM) cout << " ok " << endl;
	  cout << fTBMTrailer[2]; if ((fTBMTrailer[0]>fUbTBM)&&(fTBMTrailer[2]<fUbTBM)) ;
	  cout << " ok " << endl;
	  dump(1);
	}
      }
    }
  }
  return j;
}



// ----------------------------------------------------------------------
void BinaryFileReader::dump(int level){

}

// ----------------------------------------------------------------------
void BinaryFileReader::printTrailer(){

}
// ----------------------------------------------------------------------

char *BinaryFileReader::msgId(){
  snprintf(msgBuf,100,"BinaryFileReader:%s (%10lld): ",fInputFileName,fTime);
  return msgBuf;
}


// ----------------------------------------------------------------------
int BinaryFileReader::getType(){
  if((fHeader&kData)&&(fBadTrailer==1)) return 255;  // data with bad trailer 
  if(fNoTokenPass==1)                return   2;  // no token pass
  //  if((fHeader==80)&&(fNextHeader!=80)) return 1;  
  return fHeader;
};




// ----------------------------------------------------------------------
void BinaryFileReader::updateHistos(){
  if (fTBMHeader[0]==0) dump();
  hUBTBM->Fill(fTBMHeader[0]);
  hUBTBM->Fill(fTBMHeader[1]);
  hUBTBM->Fill(fTBMHeader[2]);
  hLVLTBM->Fill(fTBMHeader[3]);
  hLVLTBM->Fill(fTBMHeader[4]);
  hLVLTBM->Fill(fTBMHeader[5]);
  hLVLTBM->Fill(fTBMHeader[6]);
  hLVLTBM->Fill(fTBMHeader[7]);
  hNHit->Fill(fNHit);
  for(int roc=0; roc<fNROC; roc++){
	 hNHitRoc[roc]->Fill(fHitROC[roc]);
    int i = fOffs[roc];
    hUBBROC[roc]->Fill(fData[i]);
    hUBBROC[roc]->Fill(fData[i+1]);
	 h3rdClk[roc]->Fill(fData[i+2]);
    i+=3;  // move pointer to hit data
    for(int hit=0; hit<fHitROC[roc]; hit++){

      hADROC[roc]->Fill(fData[i  ]);
      hADROC[roc]->Fill(fData[i+1]);
      hADROC[roc]->Fill(fData[i+2]);
      hADROC[roc]->Fill(fData[i+3]);
      hADROC[roc]->Fill(fData[i+4]);
      hPHROC[roc]->Fill(fData[i+5]);

      i+=6;  //  next hit
    }
  }
}



// ----------------------------------------------------------------------
int BinaryFileReader::findLevels(TH1F* h, int n0, float* level, int algorithm){
  const int debug=0;
  int npeak=0;  // return number of peaks found
  if (debug) cout << "findLevels>" << endl;
  if(algorithm==1){

    //Use TSpectrum to find the peak candidates
    TSpectrum *s = new TSpectrum(2*n0);
    //Int_t nfound = s->Search(h,1,"new");
    Int_t nfound = s->Search(h,1,"goff");
    Float_t *xpeaks = s->GetPositionX();
    for (int p=0;p<nfound;p++) {
      Float_t xp = xpeaks[p];
      Int_t bin = h->GetXaxis()->FindBin(xp);
      Float_t yp = h->GetBinContent(bin);
      if (yp/TMath::Sqrt(yp) <2) continue;
      level[npeak]=xpeaks[p];
      npeak++;
      //    cout << npeak << " " << xpeaks[p] << endl;
    }

  }else{

    // clustering type peak finding
    const int nmiss=3;  // max number of missing/low  bins within a peak
    const float threshold=h->GetMaximum()/100.+1; // ignore bins below threshold
    int nlow=nmiss+1;  // to make sure the first entry starts a peak
    float s=0;
    float sx=0;
    if (debug) cout << " bin clustering threshold=" << threshold << endl;
    for(int b=0; b<h->GetNbinsX(); b++){
      if (debug) cout << Form("%5d  %10.1f   %10.1f   %3d %3d\n",b,h->GetBinCenter(b),h->GetBinContent(b),npeak,nlow);
      if(h->GetBinContent(b)>threshold){
		  if(nlow>nmiss){
			 // start a new peak
			 s=0;   // reset sums for mean 
			 sx=0;
			 npeak++;
		  }
		  // add to peak
		  s+=float(h->GetBinContent(b));
		  sx+=float(h->GetBinCenter(b))*float(h->GetBinContent(b));
		  level[npeak-1]=sx/s;
		  nlow=0;
      }else{
		  nlow++;
      }
    }
  }
  
  
  /* merge peaks if more than expected */
  while((n0>0)&&(npeak>n0)){
    int m=0;
    for(int i=1; i<(npeak-1); i++){
      if((level[i+1]-level[i])<level[m+1]-level[m]) m=i;
    }
    cout << "merged levels " << level[m] << " " << level[m+1] << endl;
    level[m]=(level[m]+level[m+1])/2.; // better if weighted?
    for(int i=m+1; i<(npeak-1); i++){
      level[i]=level[i+1];
    }
    npeak--;
  }
  return npeak;
}




// ----------------------------------------------------------------------
Double_t BinaryFileReader::findLowestValue(TH1F* h, Float_t threshold){
  const int debug=0;
  for(int b=0; b<h->GetNbinsX(); b++){
    if (debug) cout << Form("%5d  %10.1f   %10.1f\n",b,h->GetBinCenter(b),h->GetBinContent(b));
    if(h->GetBinContent(b)>threshold) return h->GetBinCenter(b);
  }
  return h->GetXaxis()->GetXmax();
}




// ----------------------------------------------------------------------
void BinaryFileReader::Levels(){
  float lubb[100];
  float l[100];
  int n;
   n=findLevels(hUBTBM, 1,l, 2 );
   if(n==1){
     fUbTBM=(int) (0.8*l[0]);
   }else{
     cout << "bad TBM ultrablack" << endl;
   }

   // TBM event counter levels
   n=findLevels(hLVLTBM,  4,l,2);
   if(n==4){
     //if(fUbTBM>l[0])  fUbTBM=l[0];
     fTBM[0]=(int) (0.5*(fUbTBM+l[0]));
     fTBM[1]=(int) (0.5*(l[0]+l[1]));
     fTBM[2]=(int) (0.5*(l[1]+l[2]));
     fTBM[3]=(int) (0.5*(l[2]+l[3]));
     fTBM[4]=(int) (0.5*(l[3]+LHMax));
   }else{
     cout << "problem with TBM levels" << endl;
     cout << " n= " << n << endl;
   }

   for (int roc=0; roc<fNROC; roc++){
     //cout <<" level finding roc " << roc << endl;
     n=findLevels(hUBBROC[roc], 2,lubb,2);
     if(n==2){
       fUbROC[roc]=(int) (0.8*lubb[0]+0.1*lubb[1]);
       Double_t phmin=findLowestValue(hPHROC[roc]);
       if(phmin <  fUbROC[roc]){
	 cout << "warning: lowest PH < UB cut    PHmin" << phmin
	      << "   Ub= "<< fUbROC[roc] << "    roc= "<< roc << endl; 
       }
     }else{
       cout << "problem with Roc UB  Roc="<< roc;
       cout << "   nlevel = " << n << endl;
     }
     
     n=findLevels(hADROC[roc],6,l,2);
     if(n!=6){
       cout <<" roc " << roc << "  retry level finding " << endl;
       n=findLevels(hADROC[roc],6,l,1);
     }
	  
     if(n==6){
       fROC[roc][0]=(int) (0.5*(lubb[0]+l[0]));
       fROC[roc][1]=(int) (0.5*(l[0]+l[1]));
       fROC[roc][2]=(int) (0.5*(l[1]+l[2]));
       fROC[roc][3]=(int) (0.5*(l[2]+l[3]));
       fROC[roc][4]=(int) (0.5*(l[3]+l[4]));
       fROC[roc][5]=(int) (0.5*(l[4]+l[5]));
       fROC[roc][6]=(int) (0.5*(l[5]+LHMax));
     }else{
       cout << "not enough Roc levels found for roc="<< roc;
		 cout << "   nlevel = " << n << endl;
     }
   }
}



// ----------------------------------------------------------------------
void BinaryFileReader::updateLevels() {
  if(fLevelMode==1){
    Levels();
    int buf[10];
    buf[0]=fUbTBM;  for(int i=0; i<5; i++){ buf[i+1]=fTBM[i];}
    fcfg->updatea(Form("%s.tbmLevels",fcfgtag), 6, buf,"%4d");
    for(int roc=0; roc< fNROC; roc++){
      buf[0]=fUbROC[roc];  for(int i=0; i<7; i++){ buf[i+1]=fROC[roc][i];}
      fcfg->updatea(Form("%s.rocLevels",fcfgtag), roc, 8, buf,"%4d");
    }
  }
}



// ----------------------------------------------------------------------
void BinaryFileReader::writeNewLevels() {
  if(fWriteNewLevelFile){
    Levels();
    writeLevels(fLevelFileName);
  }
  /*
  }else{
    Levels();
    writeLevels("levels-out.dat");
  }
  */
}

// ----------------------------------------------------------------------
void BinaryFileReader::writeLevels(const char* f) {
  //ofstream fout("levels-out.dat");
  ofstream fout(f);
  if(fout.is_open()){
    fout << fUbTBM;
    for(int i=0; i<5; i++) fout << " " << fTBM[i];
    fout << endl;
    for(int roc=0; roc<fNROC; roc++){
      fout << fUbROC[roc];
      for(int i=0; i<7; i++) fout << " " << fROC[roc][i];
      fout << endl;
    }
  }else{
    cout << "unable to save levels " << endl;
  }
}


// ----------------------------------------------------------------------
void BinaryFileReader::readLevels(const char* levelFile, int mode) {
  /* modes:
   0= default  :
	               read from levelFile  (normally  <path>/levels-xxx.dat)
						if levelFile does not exist, read from the local levels-xxx.dat
						and copy that to levelFile
   1=bootstrap:
	               determine levels from data and store them to levelFile
						user must call writeNewLevels() after reading all events
  */
  /* store the file-name */

  cout << "shout shout shout" << endl;
  strncpy(fLevelFileName,levelFile,200);
  bool rewrite=false;

  /* bootstrap mode, write new file afterwards, but read nothing */
  if(mode==1){
    fWriteNewLevelFile=1;
    return;
  }else{
	 fWriteNewLevelFile=0;
  }

  /* open an existing level file */
  ifstream* fin=new ifstream(levelFile);

  /* if the run level file is missing, use the level-xxx.dat file in 
     the current working directory */
  if(!(fin->is_open())){
    cout << "level file not found " << levelFile << endl;
    if(fNROC==16){
     fin=new ifstream("levels-module.dat");
	  if(fin->is_open()){
		 cout << "reading from levels-module.dat " << endl;
		 rewrite=true;
	  }else{
		 cout << "No level file, use -l option" << endl;
	  }
    }else{
     fin=new ifstream("levels-roc.dat");
	  if(fin->is_open()){
		 cout << "reading from levels-roc.dat " << endl;
		 rewrite=true;
	  }else{
		 cout << "No level file, use -l option" << endl;
	  }
    }      
  }else{
    fWriteNewLevelFile=0;
  }

  double somelargenumber=100000.;
  if(fin->is_open()){
    *fin >> fUbTBM;
    for(int i=0; i<5; i++){
      *fin >> fTBM[i];
      hLVLTBMUsed->Fill(fTBM[i],somelargenumber);
    }
    for(int roc=0; roc<fNROC; roc++){
      *fin >> fUbROC[roc];
      for(int i=0; i<7; i++){
	*fin >> fROC[roc][i];
	hADROCUsed[roc]->Fill(fROC[roc][i],somelargenumber);
      }
    }
  }else{
    cout << "unable to read levels from " << levelFile << endl;
  }
  if(rewrite) writeLevels(levelFile);
}
// ----------------------------------------------------------------------

int BinaryFileReader::decode(int adc, int nlevel, int* level){
  for(int i=0; i<nlevel; i++){
    if ( (adc>=level[i])&&(adc<level[i+1]) ) return i;
  }
  //cout << "level " << adc << " not found" << endl;
  //cout << level[0] << " " << level[1] << " " << level[2] << " " << level[3] << endl;
  if(adc<level[0]) return 0;
  return nlevel;
}

// ----------------------------------------------------------------------
int BinaryFileReader::getTBMTrigger(){
  int value=  
     (decode(fTBMHeader[4],4,fTBM)<<6)
    +(decode(fTBMHeader[5],4,fTBM)<<4)
    +(decode(fTBMHeader[6],4,fTBM)<<2)
    +(decode(fTBMHeader[7],4,fTBM));
  return value;
}

// ----------------------------------------------------------------------
int BinaryFileReader::getTBMStatus(){
  int value=  (decode(fTBMTrailer[4],4,fTBM)<<6)
    +(decode(fTBMTrailer[5],4,fTBM)<<4)
    +(decode(fTBMTrailer[6],4,fTBM)<<2)
             +(decode(fTBMTrailer[7],4,fTBM));
  return value;
}

/*****************************************************************/
//                                        from Daneks DecodeRawPacket
// Convert dcol&pix to col&row  
// Decodeing from "Weber" pixel addresses to rows for PSI46
// dcol = 0 - 25
// pix = 2 - 161 zigzag pattern.
// colAdd = 1-52   ! col&row start from 1
// rowAdd = 1-53
bool BinaryFileReader::convertDcolToCol(const int dcol,const int pix,
                                       int & colAdd, int & rowAdd) const
{
  const int ROCNUMDCOLS=26;
  const int ROCNUMCOLS=52;
  const int ROCNUMROWS=80;
  const int printWarning=1;
  if(dcol<0||dcol>=ROCNUMDCOLS||pix<2||pix>161)
	 {
		if(printWarning){
		  //		  cout << msgId() <<"wrong dcol or pix in user_decode "<<dcol<<" "<<pix<<endl;
		  cout << "wrong dcol or pix in user_decode "<<dcol<<" "<<pix<<endl;
		}
		rowAdd = -1;     // dummy row Address
		colAdd = -1;     // dummy col Address
		return false;
	 }
  
  // First find if we are in the first or 2nd col of a dcol.
  int colEvenOdd = pix%2;  // module(2), 0-1st sol, 1-2nd col.
  colAdd = dcol * 2 + colEvenOdd; // col address, starts from 0
  rowAdd = abs( int(pix/2) - 80);  // row addres, starts from 0
  if( colAdd<0 || colAdd>ROCNUMCOLS || rowAdd<0 || rowAdd>ROCNUMROWS )
	 {
		if(printWarning)
		  {
			 cout <<"wrong col or row in user_decode "<<colAdd<<" "<<rowAdd<<endl;
			 cout << "wrong dcol or pix in user_decode "<<dcol<<" "<<pix<<endl;
		  }
		rowAdd = -1;    // dummy row Address
		colAdd = -1;    // dummy col Address
		return false;
	 }
  return true;
}





// ----------------------------------------------------------------------
// basic conversion from row/column to roc/module coordinates

float BinaryFileReader::colToX(int col){
  const float x0=-54*0.0150/2.;
  if(col==0) return x0;
  if(col==51) return 52*0.0150+x0;
  return col*0.0150+0.0075+x0;
}

float BinaryFileReader::rowToY(int row){
  const float y0=-81*0.0100/2.;
  if(row==79) return 80*0.0100+y0;
  return row*0.0100+y0;
}


void BinaryFileReader::toLocal(pixel& p){
/* conversion of roc coordinates to module coordinates */
    if(fIsModule){
      if(p.roc<8){
        p.row=159-p.rowROC;
        p.col=p.roc*52 + p.colROC;
		  p.xy[0]=p.roc*54*0.0150 + colToX(p.colROC);
		  p.xy[1]= 160*0.0100- rowToY(p.rowROC);
      }else{//roc=8..16
        p.row=p.rowROC;
        p.col=(16-p.roc)*52-p.colROC-1;
		  p.xy[0]=(16-p.roc)*54*0.0150-colToX(p.colROC);
		  p.xy[1]= rowToY(p.rowROC);
      }
    }else{// single roc
      p.row=p.rowROC;
      p.col=p.colROC;
		p.xy[0]=colToX(p.colROC);
		p.xy[1]=rowToY(p.rowROC);
    }
}


// ----------------------------------------------------------------------
vector<cluster> BinaryFileReader::getHits(){
  /* returns clusters with local coordinates and layer IDs	added 
  the layermap is set at construction time
  */

  // decodePixels should have been called before to fill pixel buffer pb 

  // simple clusterization
  // cluster search radius fCluCut ( allows fCluCut-1 empty pixels)

  vector<cluster> v;
  if(fNHit==0) return v;
  int* gone = new int[fNHit];
  int* layer = new int[fNHit];  
  for(int i=0; i<fNHit; i++){
    gone[i]=0;
    layer[i]=fLayerMap[pb[i].roc];
  }
  int seed=0;
  while(seed<fNHit){
    // start a new cluster
    cluster c;
    c.vpix.push_back(pb[seed]); gone[seed]=1;
    c.charge=0.; c.size=0; c.col=0; c.row=0;
    c.xy[0]=0;
    c.xy[1]=0.;
    c.layer=layer[seed];
    // let it grow as much as possible
    int growing;
    do{
      growing=0;
      for(int i=0; i<fNHit; i++){
        if( (!gone[i]) && (layer[i]==c.layer) ){
          for(unsigned int p=0; p<c.vpix.size(); p++){
            int dr = c.vpix.at(p).row - pb[i].row;
            int dc = c.vpix.at(p).col - pb[i].col;
            if(    (dr>=-fCluCut) && (dr<=fCluCut) 
                    && (dc>=-fCluCut) && (dc<=fCluCut) )
            {
              c.vpix.push_back(pb[i]); gone[i]=1;
              growing=1;
              break;//important!
            }
          }
        }
      }
    }while(growing);
    
    // added all I could. determine position and append it to the list of clusters
    int nBig=0;
    for(  vector<pixel>::iterator p=c.vpix.begin();  p!=c.vpix.end();  p++){
      double Qpix=p->anaVcal;
      c.charge+=Qpix;
      c.col+=(*p).col*Qpix;
      c.row+=(*p).row*Qpix;
      c.xy[0]+=(*p).xy[0]*Qpix;
      c.xy[1]+=(*p).xy[1]*Qpix;
      if((*p).anaVcal>fAnaMin){nBig++;}
    } 
    c.size=c.vpix.size();
    if(!(c.charge==0)){
      c.col=c.col/c.charge;
      c.row=c.row/c.charge;
      c.xy[0]/=c.charge;
      c.xy[1]/=c.charge;
    }else{
      c.col=(*c.vpix.begin()).col;
      c.row=(*c.vpix.begin()).row;
      c.xy[0]=(*c.vpix.begin()).xy[0];
      c.xy[1]=(*c.vpix.begin()).xy[1];
      cout << "BinaryFileReader::GetHits>  cluster with zero charge" << endl;
    }
    if(nBig>0){
      v.push_back(c);
      tCluCharge = c.charge;
      tCluSize = c.size;
      clusterTree->Fill();
    }
	 //look for a new seed
    while((++seed<fNHit)&&(gone[seed]));
  }
  // nothing left,  return clusters
  delete layer;
  delete gone;
  return v;
}



// ----------------------------------------------------------------------
// fills pixels into the pixel buffer pb[]
void BinaryFileReader::decodePixels(){
  int k=0;
  for(int roc=0; roc<fNROC; roc++)
  {
    int j=fOffs[roc]+3;
    for(int i=0; i<fHitROC[roc]; i++)
    {
      pb[k].roc=roc;
      pb[k].ana = fData[j+5];
      pb[k].row=-1;
      pb[k].col=-1;
      int dcol =  decode(fData[j  ],6,fROC[roc])*6+decode(fData[j+1],6,fROC[roc]);
      int pix  =  decode(fData[j+2],6,fROC[roc])*6*6+decode(fData[j+3],6,fROC[roc])*6+decode(fData[j+4],6,fROC[roc]);

      if(!convertDcolToCol( dcol,pix, pb[k].colROC, pb[k].rowROC )) fnInvalidAddress++;

      if ((pb[k].colROC == selCOL) && (pb[k].rowROC == selROW) && (roc == selROC)) hPH->Fill(pb[k].ana);

      if(fPHcal!=NULL){
	if ((pb[k].colROC != -1) && (pb[k].rowROC != -1)){ 
	  pb[k].anaVcal = fPHcal->GetVcal(pb[k].ana, roc, pb[k].colROC, pb[k].rowROC);
	  hPHVcalROC[roc]->Fill(pb[k].anaVcal);

          if ((pb[k].colROC == selCOL) && (pb[k].rowROC == selROW) && (roc == selROC)) 
          {
            hPH->Fill(pb[k].ana);
            hVcal->Fill(pb[k].anaVcal);
          }
	}
      }
      else
      {
	pb[k].anaVcal=(pb[k].ana-fAnaOffset[roc])*fAnaSlope[roc];
	if(pb[k].anaVcal<=0) pb[k].anaVcal=1;

	hPHVcalROC[roc]->Fill(pb[k].anaVcal);
      }

      //fill eventTree
      tRoc = roc;
      tCol = pb[k].colROC;
      tRow = pb[k].rowROC;
      tPH = pb[k].ana;
      tVcal = pb[k].anaVcal;
      eventTree->Fill();
      
//       if (tRoc == 10) // && tCol == 51 && tRow == 71)
//       {
//          printf("%i %i %i\n", tRoc, tCol, tRow);
// 	 for (int l = 0; l < 100; l++) printf("%i ", fData[l]);
// 	 printf("\n");
//       }

      j+=6;
      k++;
    }
  }
  
  
  // convert to local/module coordinates
  for (int i=0; i<fNHit; i++){
    if(fIsModule){
      fRocGeometry.getModLocal(pb[i].roc, pb[i].colROC, pb[i].rowROC, pb[i].xy);
      if(pb[i].roc<8){
        pb[i].row=159-pb[i].rowROC;
        pb[i].col=pb[i].roc*52 + pb[i].colROC;
      }else{
        pb[i].row=pb[i].rowROC;
        pb[i].col=(16-pb[i].roc)*52-pb[i].colROC-1;
      }
    }else{
      pb[i].col=pb[i].colROC;
      pb[i].row=pb[i].rowROC;
      fRocGeometry.getRocLocal(pb[i].colROC, pb[i].rowROC, pb[i].xy);
    }
  }
  
}


// ----------------------------------------------------------------------
void BinaryFileReader::readoutLoss(){
}


// ----------------------------------------------------------------------
void BinaryFileReader::calTest(){
}


// ----------------------------------------------------------------------
void BinaryFileReader::fillPixelMaps(){
  // fill maps
  for (int i=0; i<fNHit; i++){
//     if(fIsModule){
      hModMap->Fill(pb[i].row, pb[i].col);
    }
//   }
}


// ----------------------------------------------------------------------
int BinaryFileReader::readRecord() {
  /* read the next record, i.e. header or data, and keep the
	  trigger stack up-to-date
	  histogram update is called for data records,
	  no address decoding or clustering
  */

  // slurp in data until the next Event header (0x8X) is found
  nextBinaryHeader();
  
  // decode and classify  header
  int words=0;
  if(fBufferSize>=3){
    words  = decodeBinaryData();
    if ((fHeader & kData)&&(fBadTrailer==0)&&(fTruncated==0)){
      updateHistos();
    }else if ((fHeader & kData)&&(fBadTrailer==1)){
      fnBadTrailer++;
    }else if ((fHeader & kData)&&(fTruncated==1)){
      fnTruncated++;
    }
  }else{
    fnCorrupt++;
  }


  // count header types
  fnRecord++;
  if( fHeader & (kInternalTrigger | kExternalTrigger) ){	 fnTrig++;  }
  if( fHeader & kInternalTrigger ){	 fnTrigInternal++;  }
  if( fHeader & kExternalTrigger ){	 fnTrigExternal++;  }
  if(( fHeader & kData )&&(fBadTrailer==0) ){
	 fnData++;
	 if (fNHit>0) fnDataWithHits++;
  }
  if (fHeader & kReset)     { fnReset++; }
  if (fHeader & kOvflw)     { fnOvflw++; }
  if (fHeader & kInfiniteRO){ fnInfiniteRO++; }
  
  return words;
}



// ----------------------------------------------------------------------
void BinaryFileReader::printHighEffPixels(int nevent, float thresh, TH2I** h){

}


// ----------------------------------------------------------------------
void BinaryFileReader::printVcalPeaks(const char* tag){
  if(fnCalInjectHistogrammed==0) return;
  float l[100];
  float ph[16];
  for(int roc=0; roc<fNROC; roc++){
	 int n=findLevels(hPHROC[roc], 1, l, 2);
	 if(n==1){
		ph[roc]=l[0];
	 }else{
		ph[roc]=-1000;
	 }
  }

  cout << "@@@" << tag << " ";
  for(int roc=0; roc<fNROC; roc++){	 cout << ph[roc] << " "; }
  cout << endl;
}



// ----------------------------------------------------------------------
int BinaryFileReader::readGoodDataEvent() {
  // return > 0 while good data is being read, return 0 when there is no more data
  int stat=readDataEvent();
  while(stat>0){
    if (stat==1) return 1;
    stat=readDataEvent();
  }
  return 0;
}


// ----------------------------------------------------------------------
int BinaryFileReader::readDataEvent() {
  /* get the next data event, return 0 if no more data is available,
     return 1 for good events, >1 for bad events (truncated etc)
  */
  bool good,data;

  // get records until the next data record
  do{
    readRecord();
    data=(fHeader & kData)==kData;
    
    good=data;
    
  }while( (!data) && (fEOF==0) );
  
  if ( fEOF !=0 ) {
    // no more data, don't call me again
    return 0;  // 
  }else if ( data ){
    if( good ){
      // good data record
      decodePixels();
      readoutLoss();
      fillPixelMaps();
      calTest();
      return 1;
    }else{
      // bad data record
      return 3;
    }
  }else{
    cout << msgId() << "readDataEvent : no clue how we got here" << endl;
    return 0; 
  }
}




// ----------------------------------------------------------------------
void BinaryFileReader::printRunSummary() { 
}

// ----------------------------------------------------------------------
void BinaryFileReader::printRunSummary2() { 
}

// ----------------------------------------------------------------------
void BinaryFileReader::printPixel(int col, int row) { 
  cout  << col << " " << row;
  for(int roc=0; roc<fNROC; roc++){
    double nhit=hRocMapCal[roc]->GetBinContent(col+1, row+1);
    printf(" %5f",nhit/double(fnCalInjectHistogrammed));
  } 
  cout << endl;
}

// ----------------------------------------------------------------------
double BinaryFileReader::getPixelCaleff(int roc, int col, int row) { 
  double nhit=hRocMapCal[roc]->GetBinContent(col+1, row+1);
  return nhit/double(fnCalInjectHistogrammed);
}
double BinaryFileReader::getPixelCaleffErr(int roc, int col, int row) { 
  double nhit=hRocMapCal[roc]->GetBinContent(col+1, row+1);
  return sqrt(nhit*(fnCalInjectHistogrammed-nhit)/double(fnCalInjectHistogrammed))
    /double(fnCalInjectHistogrammed);
}
// ----------------------------------------------------------------------

