#ifndef VIEWER_H
#define VIEWER_H

#include <TGFrame.h>
#include <TRootEmbeddedCanvas.h>
#include <TH1.h>
#include <TPad.h>
#include <TGTab.h>

//class TH1;


class View {
  public:
  virtual void Draw(TCanvas* c){};
  virtual ~View(){};
};



class Viewer : public TGMainFrame {


 private: 
  static const int nTabMax=20;
  static const int nViewMax=500;

  char ViewerName[50];
  int currentView;

  // tab stuff
  int fNTab;
  int fCurrentTabForAdding;
  TGTab *fTab;
  TGCompositeFrame *fCurrentFrame;
  // components of each tab
  TRootEmbeddedCanvas *fECanvas[nTabMax];
  TGVerticalFrame *buttonFrame[nTabMax];
  bool fFwdBwdButtons[nTabMax];

  // components of a View
  Int_t nView;
  // simple views for now:
  char* vname[nViewMax];
  Int_t ncol[nViewMax], nrow[nViewMax], nHistView[nViewMax], viewType[nViewMax];
  Int_t tabNr[nViewMax];
  TH1* histView[nViewMax][12];
  View* extView[nViewMax];
  //  TString viewName[nViewMax];

 public:
  Viewer(const TGWindow *p, UInt_t w, UInt_t h, const char * name="Viewer");
  virtual ~Viewer();
  TCanvas* getCanvas();
  void ByeBye();
  void Print();
  void Inc();
  void Dec();
  void DoDraw(Int_t i);
  void DoSelect(Int_t i);
  void configureButtons(int tab=0);
  void show();
  void addView(const char* s, Int_t ncol, Int_t nrow, 
					TH1* h1=NULL, TH1*h2=NULL, TH1* h3=NULL, TH1* h4=NULL, TH1* h5=NULL, TH1* h6=NULL);
  void addOverlayView(const char* s, Int_t col, Int_t row, Float_t scale,
		    TH1* h1a=NULL, TH1* h1b=NULL, TH1* h2a=NULL, TH1* h2b=NULL, 
		    TH1* h3a=NULL, TH1* h3b=NULL, TH1* h4a=NULL, TH1* h4b=NULL);
  void addView(const char* s, View* v);
  void addTab(const char *tabName);
  void addFwdBwdButtons();
  // voodoo for rootcint:
  ClassDef(Viewer,1) //a simple histogram viewer
};

#endif// VIEWER_H
