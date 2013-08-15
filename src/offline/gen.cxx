#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
using namespace std;
#include <vector> 
#include "pixelForReadout.h"
#include "RocGeometry.h"
#include "Plane.h"
#include <stdlib.h>
#include "ConfigReader.h"


// some global variables
// roc/module geometry utility
RocGeometry gRocGeometry(27*0.0150, 40.5*0.0100);   
// the arguments put the center of the user coordinates system into the center of the ROC

ofstream *fRoc;
ofstream *fMod;
Plane gPlane[5];
double gThetaMS[5];
double gRclu=0.0050;
double gQthr=3000;


void writeBinaryWord(ofstream* f, int word){
  f->put(word&0xFF);
  f->put(word>>8);
}


void writeHeader(ofstream *f, int header, long long int timestamp){
  writeBinaryWord(f,0x8000+header);
  writeBinaryWord(f,(timestamp & 0xFFFF00000000LL)>> 32);
  writeBinaryWord(f,(timestamp & 0x0000FFFF0000LL)>> 16);
  writeBinaryWord(f,(timestamp & 0x00000000FFFFLL)     );
}

void writeLevel(ofstream *f, int level){
  // 0,1,2,3,4,5 for data
  // 1 for black
  // -3 for ub
  int value=(level-1)*120;
  if(value<0){
	 value+=4096;
  }
  writeBinaryWord(f,value&0xfff);
}


void writePulseheight(ofstream *f, int charge){
  int value=charge/65-300;
  if(value<-300){
    value= -300+4096;
  }else if(value<  0){
    value=value+4096;
  }else if (value>1000){
    value=1000;
  }
  writeBinaryWord(f,value&0xfff);
}



void writeTBMHeader(ofstream *f, int counter){
  writeLevel(f,-3);
  writeLevel(f,-3);
  writeLevel(f,-3);
  writeLevel(f,1);
  writeLevel(f,(counter&0xC0)>>6);
  writeLevel(f,(counter&0x30)>>4);
  writeLevel(f,(counter&0x0C)>>2);
  writeLevel(f,(counter&0x03)   );
}

void writeTBMTrailer(ofstream *f, int status){
  writeLevel(f,-3);
  writeLevel(f,-3);
  writeLevel(f,1);
  writeLevel(f,1);
  writeLevel(f,(status&0xC0)>>6);
  writeLevel(f,(status&0x30)>>4);
  writeLevel(f,(status&0x0C)>>2);
  writeLevel(f,(status&0x03)   );
}

void writeROCHeader(ofstream *f){
  writeLevel(f,-3);
  writeLevel(f, 1);
  writeLevel(f,2);// or whatever pleaseth you
}

void writeROCHit(ofstream *f,int col, int row, int ph){
  int dcol=col/2;
  writeLevel(f,dcol/6);
  writeLevel(f,dcol%6);
  int pix=2*(80-row)+col%2;
  writeLevel(f,pix/(6*6));
  writeLevel(f,int(pix/6)%6);
  writeLevel(f,pix%6);
  writePulseheight(f,ph);
}


void writeEvent(long long int timestamp, int trig, vector<pixel> *vp, int tbmStat=0){
  // write all hits of an event to file
  // takes an array of 5 vectors of <pixel>, one for each layer
  // layers 0..3 go to the roc file
  // layer 4 goes to the module
  // roc,colROC and rowROC must have been filled elsewhere
  // write roc stream
  writeHeader(fRoc, 2,timestamp);
  writeHeader(fRoc, 1,timestamp+20LL);
  writeTBMHeader(fRoc,trig%256);
  for(int i=0; i<4; i++){// loop over ROCs  = layers
    writeROCHeader(fRoc);
    //cout << "nhit layer " << i << " " << vp[i].size() << endl;
    for(vector<pixel>::iterator p=vp[i].begin(); p!=vp[i].end(); p++){
      if((*p).roc==i){
	if((*p).ana>gQthr){
	  writeROCHit(fRoc,(*p).colROC,(*p).rowROC,(*p).ana);
	}
	//cout << "writing "<<(*p).roc<<" " << (*p).colROC<< " " <<  (*p).rowROC << endl;
      }else{
	cout << "warning !!! roc/layer inconsistency" << endl;
      }
    }
  }
  writeTBMTrailer(fRoc,tbmStat);
  //done
  
  // write module stream
  writeHeader(fMod,2,timestamp);
  writeHeader(fMod,1,timestamp+20LL);
  writeTBMHeader(fMod,trig%256);
  for(int i=0; i<16; i++){
    writeROCHeader(fMod);
    // write all hits in layer 4
    for(vector<pixel>::iterator p=vp[4].begin(); p!=vp[4].end(); p++){
      //		  cout << "*"<<(*p).roc<< " " << (*p).colROC << " " << (*p).rowROC <<endl;
      if((*p).roc==i){
	if((*p).ana>gQthr){
	  writeROCHit(fMod,(*p).colROC,(*p).rowROC,(*p).ana);
	}
      }
    }
  }
  writeTBMTrailer(fMod,tbmStat);
  //done
}


void gauss2(double &x, double &y){
  // generate a pair of random numbers with gaussian distributions
  // with width 1 and mean 0
	 double v1=0;
	 double v2=0;
	 double r2=0;
	 while( (r2==0)||(r2>=1) ){ 
		v1=2.*double(rand())/double(RAND_MAX)-1.; 
		v2=2.*double(rand())/double(RAND_MAX)-1.; 
		r2=v1*v1+v2*v2; 
	 }
	 double z1=v1*sqrt(-2*log(r2)/r2);
	 double z2=v2*sqrt(-2*log(r2)/r2);
	 x=z1;
	 y=z2;
  }



/* simulate  track, deposit signal in the sensors */
/**************  OBSOLETE *******************/
void toss(vector<pixel> *vp, int trigger){
  double x0t[3]={0,0,-7.};
  double p0t[3];
  double xhit[3];


  if(trigger){
	 x0t[0]=0.2*(double(rand())/double(RAND_MAX)-0.5);
	 x0t[1]=0.2*(double(rand())/double(RAND_MAX)-0.5);
  }else{
	 x0t[0]=2.0*(double(rand())/double(RAND_MAX)-0.5);
	 x0t[1]=8.0*(double(rand())/double(RAND_MAX)-0.5);
  }
  double tx,ty;
  gauss2(tx,ty);
  p0t[0]=tx*0.01+0.01;
  p0t[1]=ty*0.01+0.01;
  p0t[2]=sqrt(1.-p0t[0]*p0t[0]-p0t[1]*p0t[1]);

  /*
  double theta=0.03*(double(rand())/double(RAND_MAX)-0.5);
  double phi=3.14159*(double(rand())/double(RAND_MAX)-0.5);

  p0t[0]=sin(theta)*cos(phi);
  p0t[1]=sin(theta)*sin(phi);
  p0t[2]=cos(theta);
  */


  bool inside;

  const int readout[5]={2,3,4,1,0};
  double dthx[5]={0};
  double dthy[5]={0};

  // i = tracking layer  (sorted in z)
  for(int i=0; i<5; i++){
    int layer=readout[i];  // readout index
    gPlane[layer].interceptLocal(x0t, p0t, xhit);
    if((layer==4)&&0){
      cout << "t0  " << x0t[0] << " " << x0t[1] << " " << x0t[2] << endl;
      cout << "hit "<<xhit[0] << " " << xhit[1] << " " << xhit[2] << endl;
    }
		

    // multiple scattering
    double dx=0;
    double dy=0;
    if(i>0){
      for(int j=0; j<i; j++){
	double dz=gPlane[layer].getZ0()-gPlane[readout[j]].getZ0();
	dx+=dz*dthx[j];
	dy+=dz*dthy[j];
      }
    }
    double tx,ty;
    gauss2(tx,ty);
    dthx[i]=tx*gThetaMS[layer]; // TODO correct for projections
    dthy[i]=ty*gThetaMS[layer];
    
    //printf("%d)   %10f   %10f\n",i,dx,dy);
    pixel p;
    p.xy[0]=xhit[0]+dx;  // fill local hit coordinates
    p.xy[1]=xhit[1]+dy;
    p.ana=0;             // dummy pulse-height
    
    if(layer<4){
      p.roc=layer;
      inside = gRocGeometry.getRocColRow(xhit, p.colROC, p.rowROC);
    }else{ // layer 4 = module
      inside = gRocGeometry.getModColRow(xhit, p.roc, p.colROC, p.rowROC);
    }
    //	 cout << layer << ")  " << inside << " " << p.roc << " " << p.colROC << " " << p.rowROC<< "    " << xhit[0] << " " << xhit[1] << endl;
    if(inside){
      vp[layer].push_back(p);
    }
    
    
  }
}





void addHit(int layer, vector<pixel> *vp,double x,double y, double Q){
    pixel p;
    p.xy[0]=x;  // fill local hit coordinates
    p.xy[1]=y;
    p.ana=int(Q); 
    double xhit[2];
    xhit[0]=x;
    xhit[1]=y;
        
    bool inside;

    if(layer<4){
      p.roc=layer;
      inside = gRocGeometry.getRocColRow(xhit, p.colROC, p.rowROC);
    }else{ // layer 4 = module
      inside = gRocGeometry.getModColRow(xhit, p.roc, p.colROC, p.rowROC);
    }
    /*
    cout << layer << ")  " << inside << " " << p.roc << " " 
    << p.colROC << " " << p.rowROC<< "    " << xhit[0] << " " << xhit[1] << endl;
    */
    if(inside){
      vp[layer].push_back(p);
    }
}




void printHits(vector<pixel> *vp){
  cout << "-------------------  Hits ---------------------" << endl;
  for(int layer=0; layer<5; layer++){
    for(vector<pixel>::iterator p=vp[layer].begin(); p!=vp[layer].end(); p++){
      printf(" %2d  %2d  %2d  %2d  %8d \n",layer,p->roc,p->colROC,p->rowROC,p->ana);
    }
  }
  cout << "-----------------------------------------------" << endl;
}




void mergeHits(vector<pixel> *vp){
  for(int layer=0; layer<5; layer++){
    
    vector<pixel>::iterator p=vp[layer].begin();

    while( !(p==vp[layer].end() )){

      for(vector<pixel>::iterator q=p+1; q!=vp[layer].end(); q++){

	if ( (q->roc==p->roc) && (q->colROC==p->colROC) && (q->rowROC==p->rowROC)){
	  // merge pixels, add charge to the first pixel ...
	  p->ana+=q->ana;
	  // ... and remove the second 
	  vector<pixel>::iterator qtemp=q;
	  q--;
	  vp[layer].erase(qtemp);
	}
      }
      p++;
    }
  }
}





void discardHits(double inefficency, vector<pixel> *vp){

  for(int layer=0; layer<5; layer++){
    int roc1,roc2;
    if(layer<4){
      roc1=layer; roc2=layer;
    }else{
      roc1=0; roc2=15;
    }

    for(int roc=roc1; roc<=roc2; roc++){

      // apply inefficency in double columns
      double rdc[26]={0};
      for(int i=0; i<26; i++) rdc[i]=double(rand())/double(RAND_MAX);
      // loop through pixels
      for( vector<pixel>::iterator p=vp[layer].begin();p!=vp[layer].end();){
	if(p->roc==roc){
	  int dcol = int((p->colROC)/2);
	  if (rdc[dcol] < inefficency ){
	    // discard
	    p=vp[layer].erase(p);
	  }else{ // not inefficient
	    p++;
	  }
	}else{ // different roc
	  p++;
	}
      }// pixel loop 
    }// roc loop
  }//layer loop
}



double tossCharge(){
  double r1,r2;
  gauss2(r1,r2);
  double Q;
  if( rand() < (0.8*RAND_MAX) ){
    Q=  24000+r1*5000;
  }else{
    Q=  48000+r1*10000;
  }
  if(Q >0){ return Q; } else { return 0;}
}



/* simulate a track, deposit signal in the pixels */
void simTrack(double * x0t, double* p0t, vector<pixel> *vp){
  double xhit[3];


  const int readout[5]={2,3,4,1,0};
  double dthx[5]={0};
  double dthy[5]={0};

  // i = tracking layer  (sorted in z)
  for(int i=0; i<5; i++){
    int layer=readout[i];  // readout index
    gPlane[layer].interceptLocal(x0t, p0t, xhit);
		
    // multiple scattering
    double dx=0;
    double dy=0;
    if(i>0){
      for(int j=0; j<i; j++){
	double dz=gPlane[layer].getZ0()-gPlane[readout[j]].getZ0();
	dx+=dz*dthx[j];
	dy+=dz*dthy[j];
      }
    }
    double tx,ty;
    gauss2(tx,ty);
    dthx[i]=tx*gThetaMS[layer]; // TODO correct for projections
    dthy[i]=ty*gThetaMS[layer];

    // very simple charge spreading, create multiple charge spots, merge pixels later
    double Qclu=tossCharge();
    const int nlump=10;
    for(int i=0; i<nlump; i++){
      gauss2(tx,ty);
      addHit(layer,vp,
	     xhit[0]+dx+tx*gRclu,
	     xhit[1]+dy+ty*gRclu,
	     Qclu/nlump);
    }
  }
}




/* simulate n tracks, deposit signal in the sensors */
void tossTracks(int nTrack, int requireTrigger, vector<pixel> *vp){
  double x0t[100][3];
  double p0t[3];

  
  int n=nTrack;  if(n>100) n=100;
  int trigger;

  do{
    trigger=0;
    for(int i=0; i<n; i++){
      x0t[i][0]=2.0*(double(rand())/double(RAND_MAX)-0.5);
      x0t[i][1]=8.0*(double(rand())/double(RAND_MAX)-0.5);
      x0t[i][2]=-7.; // start in the trigger plane
      //cout << "tossTrack>" << i << " " << x0t[i][0] << " " << x0t[i][1] << endl;
      if( (x0t[i][0]>-0.2) && (x0t[i][0]<0.2)&& (x0t[i][1]>-0.2) && (x0t[i][1]<0.2) ){
	trigger=1;
      }
    }
  }while(requireTrigger && (trigger==0) );

  double tx,ty;
  for(int i=0; i<n; i++){
    // toss a track direction
    gauss2(tx,ty);
    p0t[0]=tx*0.01+0.01;
    p0t[1]=ty*0.01+0.01;
    p0t[2]=sqrt(1.-p0t[0]*p0t[0]-p0t[1]*p0t[1]);
    // and simulate the track
    simTrack(x0t[i], p0t, vp);
  }
}












void writeConfig(const char* f) {
  ConfigReader* c=new ConfigReader(f);
  const int dl=120;
  const int UB=-3*dl;

  int buf[8]={UB, 0};
  for(int i=0; i<7; i++) buf[i+1]=int((i-1.5)*dl);
  int buf0[16]={0};
  // roc stuff
  c->updatea("roc.deconvolution", 4, buf0,"%5.2f");
  c->updatea("roc.deconvolution2", 4, buf0,"%5.2f");
  c->updatea("roc.tbmLevels", 6, buf,"%4d");
  for(int roc=0; roc<4; roc++){
    c->updatea("roc.rocLevels", roc, 8, buf,"%4d");
  }
  // module stuff
  c->updatea("mod.deconvolution", 16, buf0,"%5.2f");
  c->updatea("mod.deconvolution2", 16, buf0,"%5.2f");
  c->updatea("mod.tbmLevels", 6, buf,"%4d");
  for(int roc=0; roc<16; roc++){
    c->updatea("mod.rocLevels", roc, 8, buf,"%4d");
  }


  // alignment
  double dbuf[3]={0};
  c->updatea("shift", 3, 3, dbuf,"%8.4f");
  c->updatea("shift", 1, 3, dbuf,"%8.4f");
  dbuf[1]=0.55;
  c->updatea("shift", 4, 3, dbuf,"%8.4f");

  // dump it
  c->rewrite();
}


void writeLevels(const char* f, int nroc) {
  // obsolete
  const int dl=120;
  const int UB=-3*dl;
  ofstream fout(f);
  if(fout.is_open()){
    fout << UB;
    for(int i=0; i<5; i++) fout << " " << int((i-1.5)*dl);
    fout << endl;
    for(int roc=0; roc<nroc; roc++){
      fout << UB;
      for(int i=0; i<7; i++) fout << " " << int((i-1.5)*dl);
      fout << endl;
    }
  }else{
    cout << "unable to save levels:" << f << endl;
  }
}


int main(int argc, char **argv)
{
  char rocfileName[200];
  char modfileName[200];
  char configFileName[200];
  //  char roclevelfileName[200];
  char path[200];
  int run=42;
  int nEvent=100;
  int nTrack=1;
  double inefficiency=0;

  // geometry, this should match the EventReader 
  double zguess[5]={12.48-0.5, 10.32-0.5, 0-0.5, 2.16-0.5,  7.};
  double znom[5];
  // misalignment: 1. index layer, 2nd index dx,dy,dz,angles
  double dx[5][6]={{0}};
  dx[4][1]=0.55;  // module in y
  /*
  dx[0][5]= 0.010;
  dx[1][5]= 0.002;
  dx[2][5]=-0.007;
  dx[3][5]= 0.002;
  dx[4][5]= 0.001;
  */

  for(int i=0; i<5; i++){
	 znom[i]=zguess[i]-zguess[4];
	 double xX0= (0.0750+0.0300)/9.36  // ROC+Sensor, Si
		+0.1000/19.4;                     // Carrier, G10
	 double p=300; //MeV/c
	 double beta=1./sqrt(1.+139.*139./p/p);
	 gThetaMS[i]=13.6/(p*beta)*sqrt(xX0)*(1+0.038*log(xX0));
	 cout << i << ")" << gThetaMS[i]*1000 << " " << beta << " " << xX0 << " " << (1+0.038*log(xX0))<< endl;
  }

  ConfigReader* config=new ConfigReader("GeneratorConfig.dat");
  config->get("run",run,42);
  config->get("nEvent",nEvent,100);
  config->get("nTrack",nTrack,1);
  config->get("clusterradius",gRclu,0.0050);
  config->get("pixelThreshold",gQthr,3000.);
  config->get("inefficiency",inefficiency,0);
  for(int layer=0; layer<5; layer++){
	 config->geta("alignment",layer,6,dx[layer]);
  }
  

  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i],"-r")) {
      run=atoi(argv[++i]);
    }
  }

	 
  cout << "run = " << run << endl;
  sprintf(path,"/home/l_tester/log/bt05r%06d",run);
  sprintf(rocfileName,"%s/rtb.bin",path);
  sprintf(modfileName,"%s/mtb.bin",path);
  sprintf(configFileName,"%s/config.dat",path);

  // generate and print geometry
  printf("   %10s %10s %10s %10s %10s %10s \n","dx","dy","dz","phix","phiy","phiz");
  for(int layer=0; layer<5; layer++){
	 gPlane[layer]=Plane( dx[layer][0], dx[layer][1], znom[layer]+dx[layer][2] );
	 gPlane[layer].rotate( dx[layer][3], dx[layer][4], dx[layer][5] );
	 printf("%2d %10f %10f %10f %10f %10f %10f \n",layer,
			  dx[layer][0],dx[layer][1],dx[layer][2],
			  dx[layer][3],dx[layer][4],dx[layer][5]);
  }
	 



  vector<pixel> pixelHits[5];        // vector of hits for each layer
  fRoc = new ofstream(rocfileName,ofstream::out|ofstream::binary );
  fMod = new ofstream(modfileName,ofstream::out|ofstream::binary);


  long long int timeBC=0LL;
  // "send" resets to make "t" think this is sync'd
  writeHeader(fRoc, 0x08,timeBC);
  writeHeader(fMod, 0x08,timeBC);
  int tbmStat=0x20;   // give the next event the correct trailer
  timeBC+=1000LL;


  // event loop
  for(int trig=0; trig<nEvent; trig++){
	 timeBC+=10000LL;
	 // clear the charge
	 for(int i=0; i<5; i++) pixelHits[i].clear();

	 
	 tossTracks(nTrack, 1, pixelHits);
	 mergeHits(pixelHits);
	 discardHits(inefficiency, pixelHits);
	 writeEvent(timeBC,trig,pixelHits,tbmStat);

	 tbmStat=0;
  }

  fRoc->close();
  fMod->close();

  writeConfig(configFileName);

  
  }
