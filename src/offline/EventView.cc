#include "EventView.h"
#include <TBox.h>
#include <TLine.h>
#include <TCanvas.h>
#include <TEllipse.h>
#include <TPaveLabel.h>
#include <TH2F.h>
#include <TPad.h>
#include <stdio.h>
#include <iostream>

/*
void EventView::addPixel(int layer, int col, int row){
  //cout << "addPixel" << layer << " " <<  col << " " << row << endl;
  hit_t hit;
  hit.layer=layer;
  hit.col=col;
  hit.row=row;
  vHit.push_back(hit);
}
*/

void EventView::addPixel(int layer, double x, double y, double dx, double dy){
  hit_t hit;
  hit.layer=layer;
  hit.x=x;
  hit.y=y;
  hit.dx=dx;
  hit.dy=dy;
  vHit.push_back(hit);
}

EventView::EventView(){
  //fh=new TH2F("a","map",2,-0.80,0.80,2,-0.80,0.80);  
}


void EventView::addTrack(double* par){
  double* p=new double[4];
  for(int i=0; i<4; i++){ p[i]=par[i];}
  vTrack.push_back(p);
}


void EventView::Draw(TCanvas* c){
  TPad *pad1=new TPad("a","a",0.0,0.8, 1.0,1.0); pad1->Draw(); 
  TPad *pad2=new TPad("b","b",0.0,0.6, 1.0,0.8); pad2->Draw();
  TPad *pad3=new TPad("c","c",0.0,0.0, 1.0,0.6); pad3->Draw();

  TPaveLabel* pl;
  pad1->cd(); gPad->Range(-8, -0.5, 8., 0.5);
  pl=new TPaveLabel(-4.,0.4,-1.,0.5,"side view"); pl->Draw();
  pad2->cd(); gPad->Range(-8, -0.5, 8,  0.5);
  pl=new TPaveLabel(-4.,0.4,-1.,0.5,"top view"); pl->Draw();
  pad3->cd(); gPad->Range(-0.8, -0.6, 0.8, 0.6);
  pl=new TPaveLabel(-0.8,0.55,-0.4,0.6,"xy view"); pl->Draw();


  TBox *b1= new TBox(-26*0.015,-40*0.01, 26*0.015, 40*0.01);
  b1->SetLineColor(kBlack);
  b1->SetLineWidth(1);
  b1->SetLineStyle(1);
  b1->Draw();

  int color[5]={kRed,kGreen,kBlue,kMagenta,kBlack};

  for(vector<double*>::iterator t=vTrack.begin(); t!=vTrack.end(); t++){
	 double x2[3];  fPlane[2]->interceptGlobal(*t,x2);
	 double x0[3];	 fPlane[0]->interceptGlobal(*t,x0);
	 pad3->cd();
	 TLine *tl=new TLine(x2[0],x2[1],x0[0],x0[1]);
	 tl->Draw();
	 double x1[3];  fPlane[1]->interceptGlobal(*t,x1);
	 double x3[3];	 fPlane[3]->interceptGlobal(*t,x3);
	 double x4[3];	 fPlane[4]->interceptGlobal(*t,x4);
	 TEllipse *tc;
	 tc=new TEllipse(x0[0], x0[1],0.0100); tc->Draw();
	 tc=new TEllipse(x1[0], x1[1],0.0100); tc->Draw();
	 tc=new TEllipse(x2[0], x2[1],0.0100); tc->Draw();
	 tc=new TEllipse(x3[0], x3[1],0.0100); tc->Draw();
	 tc=new TEllipse(x4[0], x4[1],0.0100); tc->Draw();

	 pad1->cd();// side view
	 tl=new TLine(x2[2],x2[1],x0[2],x0[1]);
	 tl->Draw();
	 tc=new TEllipse(x0[2], x0[1],0.1,0.0200); tc->Draw();
	 tc=new TEllipse(x1[2], x1[1],0.1,0.0200); tc->Draw();
	 tc=new TEllipse(x2[2], x2[1],0.1,0.0200); tc->Draw();
	 tc=new TEllipse(x3[2], x3[1],0.1,0.0200); tc->Draw();
	 tc=new TEllipse(x4[2], x4[1],0.1,0.0200); tc->Draw();
	 pad2->cd(); // top view
	 tl=new TLine(x2[2],x2[0],x0[2],x0[0]);
	 tl->Draw();
	 tc=new TEllipse(x0[2], x0[0],0.1,0.0200); tc->SetLineStyle(1); tc->Draw();
	 tc=new TEllipse(x1[2], x1[0],0.1,0.0200); tc->SetLineStyle(1); tc->Draw();
	 tc=new TEllipse(x2[2], x2[0],0.1,0.0200); tc->SetLineStyle(1); tc->Draw();
	 tc=new TEllipse(x3[2], x3[0],0.1,0.0200); tc->SetLineStyle(1); tc->Draw();
	 tc=new TEllipse(x4[2], x4[0],0.1,0.0200); tc->SetLineStyle(1); tc->Draw();
  }


  // show the hits last
  for(vector<hit_t>::iterator h=vHit.begin(); h!=vHit.end(); h++){
	 double xl[3],xg1[3],xg2[3];
	 xl[0]=h->x-h->dx/2;	 xl[1]=h->y-h->dy/2;	 xl[2]=0;
	 fPlane[h->layer]->localToGlobal(xl, xg1);

	 xl[0]=h->x+h->dx/2;	 xl[1]=h->y+h->dy/2;	 xl[2]=0;
	 EventView::fPlane[h->layer]->localToGlobal(xl, xg2);

	 pad3->cd();
	 TBox *b= new TBox(xg1[0],xg1[1],xg2[0],xg2[1]);
	 b->SetFillColor(color[h->layer]);
	 b->Draw();

	 pad1->cd();
	 b= new TBox(xg1[2]-0.05,xg1[1],xg1[2]+0.05,xg2[1]);
	 b->SetFillColor(color[h->layer]);
	 b->Draw();
	 pad2->cd();
	 b= new TBox(xg1[2]-0.05,xg1[0],xg1[2]+0.05,xg2[0]);
	 b->SetFillColor(color[h->layer]);
	 b->Draw();
  }

}
