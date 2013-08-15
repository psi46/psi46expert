#include <TApplication.h>
#include <TGClient.h>
#include <TCanvas.h>
#include <TGButton.h>
#include <TH1.h>
#include <TF1.h>

#include "Viewer.h"
#include <fstream>
#include <iostream>
#include <stdio.h>
using namespace std;

Viewer::Viewer(const TGWindow *p, UInt_t w, UInt_t h, const char *name)
  :TGMainFrame(p,w,h) {
  nView=0;
  strncpy(ViewerName,name,50);
  fNTab=0;  
  fCurrentTabForAdding=0;
  fFwdBwdButtons[0]=false;
  fTab=NULL;
  fCurrentFrame=this;
}

void Viewer::addTab(const char *tabName){
  if(fTab==NULL){
	 fTab=new TGTab(this);
  }
  if(fNTab<nTabMax){
	 fNTab++;
	 fCurrentFrame=fTab->AddTab(tabName);
	 fCurrentTabForAdding=fNTab-1;  // doesn't change when called the first time
	 fFwdBwdButtons[fCurrentTabForAdding]=false;
  }
}


TCanvas* Viewer::getCanvas(){
  if(fTab==NULL){
	 return fECanvas[0]->GetCanvas();
  }else{
	 return fECanvas[fTab->GetCurrent()]->GetCanvas();
  }
}




void Viewer::configureButtons(Int_t tab){
  // configure the buttons
  char slot[100];
  TGTextButton *draw;
  for(Int_t i=0; i<nView; i++){
	 if(strlen(vname[i])>0){
		if(tabNr[i]==tab){
		  draw=new TGTextButton(buttonFrame[tab],vname[i]);
		  sprintf(slot,"DoDraw(=%d)",i);
		  draw->Connect("Clicked()","Viewer",this,slot);
		  buttonFrame[tab]->AddFrame(draw, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX,5,5,3,4));
		}
	 }
  }
  // forward/backward buttons (if requested)
  if (fFwdBwdButtons[tab]){
	 TGTextButton *inc=new TGTextButton(buttonFrame[tab],"+");
	 inc->Connect("Clicked()","Viewer",this,"Inc()");
	 buttonFrame[tab]->AddFrame(inc, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX,5,5,3,4));
	 TGTextButton *dec=new TGTextButton(buttonFrame[tab],"-");
	 dec->Connect("Clicked()","Viewer",this,"Dec()");
	 buttonFrame[tab]->AddFrame(dec, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX,5,5,3,4));
  }
  // print and exit buttons (move these to a menu ?)
  TGTextButton *print=new TGTextButton(buttonFrame[tab],"&Print");
  print->Connect("Clicked()","Viewer",this,"Print()");
  buttonFrame[tab]->AddFrame(print, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsBottom,5,5,3,4));
  TGTextButton *exit=new TGTextButton(buttonFrame[tab],"&Exit");
  exit->Connect("Clicked()","Viewer",this,"ByeBye()");
  buttonFrame[tab]->AddFrame(exit, new TGLayoutHints(kLHintsExpandX|kLHintsCenterX|kLHintsBottom,5,5,3,4));
  // done, attach buttons to the current frame

  fCurrentFrame->AddFrame(buttonFrame[tab], new TGLayoutHints(kLHintsExpandX|kLHintsCenterX,2,2,2,2));
}




void Viewer::show(){
  if(fTab==NULL){
	 // no tabs, put things directly into the top level Frame (this)
	 SetLayoutManager( new TGHorizontalLayout(this) );
	 fECanvas[0] = new TRootEmbeddedCanvas("Ecanvas",this,600,800);
	 AddFrame(fECanvas[0], new TGLayoutHints(kLHintsRight|kLHintsTop,10,10,10,1));
	 buttonFrame[0] = new TGVerticalFrame(this,200,800);
	 fECanvas[0]->GetCanvas()->ToggleEventStatus();
	 fCurrentFrame=this;
	 configureButtons(0);
  }else{
	 // using tabs, put buttons and canvasses into the Frames of the tabs
	 AddFrame(fTab);
	 fTab->SetLayoutManager( new TGTabLayout(fTab) );
	 fTab->Connect("Selected(Int_t)","Viewer",this,"DoSelect(Int_t)");
	 for(Int_t i=0; i<fNTab; i++){
		fCurrentFrame=fTab->GetTabContainer(i);
		fCurrentFrame->SetLayoutManager( new TGHorizontalLayout(fCurrentFrame) );
		fECanvas[i] = new TRootEmbeddedCanvas("Ecanvas",fCurrentFrame,600,800);
		fECanvas[i]->GetCanvas()->ToggleEventStatus();
		fCurrentFrame->AddFrame(fECanvas[i], new TGLayoutHints(kLHintsRight|kLHintsTop,10,10,10,1));
		buttonFrame[i] = new TGVerticalFrame(fCurrentFrame,200,800);
		configureButtons(i);
    }	
  }
  SetWindowName(ViewerName);
  MapSubwindows();
  Resize(GetDefaultSize());
  MapWindow();
  DoDraw(0);
  Connect("CloseWindow()", "Viewer", this, "ByeBye()");
}


Viewer::~Viewer(){
  //delete fECanvas;
}

void Viewer::ByeBye(){
  //  TCanvas *c = fECanvas->GetCanvas();
  // c->Clear();
  cout << "Trying to terminate" << endl;
  gApplication->Terminate(0) ;
}


void Viewer::DoSelect(Int_t tab){
  for(Int_t i=0; i<nView; i++){
	 if(tabNr[i]==tab){
		DoDraw(i);
		return;
	 }
  }
}


void Viewer::Print(){
  TCanvas *c = getCanvas();
  c->Print(Form("%s-%d.ps",ViewerName,currentView));
}

void Viewer::Inc(){
  if(currentView<nView){
	 currentView++;
	 DoDraw(currentView);
  }
}


void Viewer::Dec(){
  if(currentView>0){
	 currentView--;
	 DoDraw(currentView);
  }
}


void Viewer::DoDraw(Int_t view){
   TCanvas *c = getCanvas();
   c->Clear();
   if(viewType[view]==1){ // normal (histogram) view
	  c->Divide(ncol[view],nrow[view]);
     for(Int_t i=0; i<nHistView[view]; i++){
       c->cd(i+1);
       histView[view][i]->Draw();
		 TList* l=histView[view][i]->GetListOfFunctions();
		 if(l){
			TF1* f= (TF1 *)l->First();
			if(f) f->Draw("SAME");
		 }
     }
   }else if(viewType[view]==2){  // overlay
	  c->Divide(ncol[view],nrow[view]);
     for(Int_t i=0; i<nHistView[view]/2; i++){
       c->cd(i+1);
       histView[view][i*2  ]->Draw();
       if(histView[view][i*2+1]){
			histView[view][i*2+1]->SetLineColor(2);
			histView[view][i*2+1]->Draw("SAME");
       }
     }
   }else{// other view
	  if(extView[view]){ extView[view]->Draw(c); }
	}
   c->Update();
	currentView=view;
}



void Viewer::addView(const char* s, Int_t col, Int_t row,TH1* h1, TH1* h2,TH1* h3,TH1* h4,TH1* h5,TH1* h6){
  if(nView==nViewMax) return;
  vname[nView]=new char[20];
  strncpy(vname[nView],s,20);
  ncol[nView]=col;
  nrow[nView]=row;
  nHistView[nView]=0;
  viewType[nView]=1;
  tabNr[nView]=fCurrentTabForAdding;
  if(h1!=NULL){
    histView[nView][nHistView[nView]++]=h1;
  }
  if(h2!=NULL){
    histView[nView][nHistView[nView]++]=h2;
  }
  if(h3!=NULL){
    histView[nView][nHistView[nView]++]=h3;
  }
  if(h4!=NULL){
    histView[nView][nHistView[nView]++]=h4;
  }
  if(h5!=NULL){
    histView[nView][nHistView[nView]++]=h5;
  }
  if(h6!=NULL){
    histView[nView][nHistView[nView]++]=h6;
  }
  nView++;
}



void Viewer::addOverlayView(const char* s, Int_t col, Int_t row, Float_t scale,
			    TH1* h1a, TH1* h1b, TH1* h2a, TH1* h2b, 
			    TH1* h3a, TH1* h3b, TH1* h4a, TH1* h4b){
  if(nView==nViewMax) return;
  vname[nView]=new char[20];
  strncpy(vname[nView],s,20);
  ncol[nView]=col;
  nrow[nView]=row;
  nHistView[nView]=0;
  viewType[nView]=2;
  tabNr[nView]=fCurrentTabForAdding;
  if(h1a!=NULL){
    histView[nView][nHistView[nView]++]=h1a;
    histView[nView][nHistView[nView]++]=h1b;
    if(h1b) h1b->Scale(scale);
  }
  if(h2a!=NULL){
    histView[nView][nHistView[nView]++]=h2a;
    histView[nView][nHistView[nView]++]=h2b;
    if(h2b) h2b->Scale(scale);
  }
  if(h3a!=NULL){
    histView[nView][nHistView[nView]++]=h3a;
    histView[nView][nHistView[nView]++]=h3b;
    if(h3b) h3b->Scale(scale);
  }
  if(h4a!=NULL){
    histView[nView][nHistView[nView]++]=h4a;
    histView[nView][nHistView[nView]++]=h4b;
    if (h4b) h4b->Scale(scale);
  }
  nView++;
}



void Viewer::addView(const char* s, View* v){
  if(nView==nViewMax) return;
  vname[nView]=new char[20];
  strncpy(vname[nView],s,20);
  extView[nView]=v;
  viewType[nView]=3;
  tabNr[nView]=fCurrentTabForAdding;
  nView++;
}


void Viewer::addFwdBwdButtons(){
  fFwdBwdButtons[fCurrentTabForAdding]=true;
}
