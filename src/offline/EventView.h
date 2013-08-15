#ifndef EVENTVIEW_H
#define EVENTVIEW_H

#include <TGFrame.h>
#include <TH1.h>
#include <TPad.h>
#include "Viewer.h"
#include <vector>
#include "Plane.h"
#include "TH2F.h"

using namespace std;

struct hit_t{
  int layer;
  //  int col;
  //  int row;
  double x,y,dx,dy;
};


class EventView: public View {
 private:
  vector<hit_t> vHit;
  vector<double*> vTrack;
  TH2F* fh;

 public:
  Plane *fPlane[5];
  EventView();
  void Draw(TCanvas* c);
  //void addPixel(int layer, int col, int row);
  void addPixel(int layer, double x, double y, double dx, double dy);
  void addTrack(double* par);
};

#endif// EVENTVIEW_H
