#ifndef PLANE_H
#define PLANE_H


class Plane{
 private:
  double fT[3][3];
  double fQ[3][3];
  double fX0[3];     // position
  double fDP[3];     // rotations

 public:
  Plane();
  Plane(double x, double y, double z);
  void init();
  void globalToLocal(double* xg, double* xl);
  void localToGlobal(double* xl, double* xg);
  void testTrafo();
  void invertTrafo();
  void updateAngles();
  void rotate(double px, double py, double pz);
  void interceptGlobal(double* start, double* direction, double* intercept);
  void interceptGlobal(double* par, double* intercept);
  void interceptLocal(double* start, double* direction, double* intercept);
  double getZ0(){return fX0[2];}
};

#endif
