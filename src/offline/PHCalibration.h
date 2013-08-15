#ifndef PHCALIBRATION_H
#define PHCALIBRATION_H

#include <fstream>
#include <iostream>
#include <stdio.h>

class PHCalibration {

 public:
	PHCalibration();
	~PHCalibration();
	void LoadFitParameters(char *dirName, int phTrim);
	double GetVcal(int ph, int chip, int col, int row);

 private:

  int version[16];
  static const int nFitParams = 6;
  double parameter[nFitParams];
  float fitParameter[nFitParams][16][52][80];

};
#endif
