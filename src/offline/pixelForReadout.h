#ifndef TP_PIXELFORREADOUT_H
#define TP_PIXELFORREADOUT_H

// Define the pixel structure for raw data readout.
struct pixel {
  int col;
  int row;
  int ana;
  float anaVcal;
  int roc;
  int colROC;
  int rowROC;
  int raw[6];
  float xy[2];
};

struct cluster{
  vector<pixel> vpix;
  int size;
  float charge;
  float col,row;
  int layer;
  double xy[2]; // local coordinates
  double xyz[3];
};

#endif // TP_PIXELFORREADOUT_H
