//
// Author: Peter Trüb
// 
// Code to fit SCurves
//

#ifndef SCurve_H
#define SCurve_H

#include "TF1.h"
#include "TGraph.h"
#include <stdio.h>


class SCurve: public TObject
{

public :
	SCurve();

	void Fit();
	void FitSCurves(char *dirName, int nRocs = 16);
	bool OpenInputFile(char *dirName, int roc);
	void FitSCurve(char *dirName, int roc, int col, int row);
	bool ReadData(int &n, double x[], double y[], double xErr[], double yErr[]);

	
private :

	FILE *outputFile, *inputFile;
	TF1 *fit;
	TGraph *graph;
	int mode;
	bool debug;
	double thr, sig, ePerVcal, slope;

	ClassDef(SCurve, 0);
};

#endif

