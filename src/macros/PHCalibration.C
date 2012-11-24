#include <iostream>
#include <TSystem.h>
#include "TF1.h"
#include "TGraph.h"
#include "TH1D.h"
#include "TFile.h"

void FitAllCurves(char *dirName, int fitMode = 0)
{
        gSystem->Load("libMinuit.so");
	gSystem->Load("libpsi46ana.so");
	
	PHCalibrationFit phFit = PHCalibrationFit(fitMode);
	phFit.FitAllCurves(dirName);
}


void FitCurve(char *dirName, int chip, int col, int row, int fitMode = 0)
{
	gSystem->Load("libpsi46ana.so");
	
	PHCalibrationFit phFit = PHCalibrationFit(fitMode);
	phFit.FitCurve(dirName, chip, col, row);
}
