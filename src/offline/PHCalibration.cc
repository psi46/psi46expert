#include <iostream>
#include <TSystem.h>
#include "PHCalibration.h"
#include "TMath.h"

using namespace std;

const double xCut = TMath::Pi()/2. - 0.0005;
const double tanXCut = TMath::Tan(xCut);


Double_t FitfcnOld( Double_t *x, Double_t *par)
{
	return TMath::Exp(par[1]*x[0] - par[0]) + par[2]*x[0]*x[0]*x[0] + par[3]*x[0]*x[0] + par[4]*x[0] + par[5];
}


Double_t Fitfcn( Double_t *x, Double_t *par)
{
	if (par[0]*x[0] - par[4] > xCut) return tanXCut + (x[0] - (xCut + par[4])/par[0])* 1e8;
	return TMath::Tan(par[0]*x[0] - par[4]) + par[1]*x[0]*x[0]*x[0] + par[5]*x[0]*x[0] + par[2]*x[0] + par[3];
}


Double_t FitfcnTan( Double_t *x, Double_t *par)
{
//  return par[3] + par[2] * TMath::TanH(par[0]*x[0] - par[1]);
  return (TMath::ATanH((x[0] - par[3])/par[2]) + par[1])/par[0];
}


PHCalibration::PHCalibration()
{
}


PHCalibration::~PHCalibration()
{
}

void PHCalibration::LoadFitParameters(char *dirName, int phTrim)
{
	FILE *file;
	char fname[1000], string[500];
	int a,b;

	for (int chip = 0; chip < 16; chip++)
	{
                if (phTrim != 0) sprintf(fname, "%s/phCalibrationFit%i_C%i.dat", dirName, phTrim, chip);
                else sprintf(fname, "%s/phCalibrationFit_C%i.dat", dirName, chip);
		
                file = fopen(fname, "r");
                printf("reading calibration %s for chip %i: ", fname, chip);

		if (!file)
		{
			printf("!!!!!!!!!  ----> PHCalibration: Could not open file %s to read fit parameters\n", fname);
			return;
		}

		fgets(string, 500, file);
		fgets(string, 500, file);
		if (strcmp(string, "TMath::Exp(par[1]*x[0] - par[0]) + par[2]*x[0]*x[0]*x[0] + par[3]*x[0]*x[0] + par[4]*x[0] + par[5]\n") == 0) 
		{
			version[chip] = 0;
		}
                else if (strcmp(string, "par[3] + par[2] * TMath::TanH(par[0]*x[0] - par[1])\n") == 0) 
                {
                        version[chip] = 2;
                }
                else
		{
			version[chip] = 1;
		}
		
		fgets(string, 500, file);
		printf("PhCalibration version %i\n", version[chip]);
		
		for (int iCol = 0; iCol < 52; iCol++)
		{
			for (int iRow = 0; iRow < 80; iRow++)
			{
                                if (version[chip] == 0 || version[chip] == 1) fscanf(file, "%e %e %e %e %e %e", &fitParameter[0][chip][iCol][iRow], &fitParameter[1][chip][iCol][iRow], &fitParameter[2][chip][iCol][iRow], &fitParameter[3][chip][iCol][iRow], &fitParameter[4][chip][iCol][iRow], &fitParameter[5][chip][iCol][iRow]);
                                else if (version[chip] == 2)  fscanf(file, "%e %e %e %e", &fitParameter[0][chip][iCol][iRow], &fitParameter[1][chip][iCol][iRow], &fitParameter[2][chip][iCol][iRow], &fitParameter[3][chip][iCol][iRow]);
				fscanf(file, "%s %2i %2i", string, &a, &b);
			}
		}

		fclose(file);
	}
}


double PHCalibration::GetVcal(int ph, int chip, int col, int row)
{
	double x[] = {(double)ph};
	for (int i = 0; i < nFitParams; i++) parameter[i] = fitParameter[i][chip][col][row];
	
	if (version[chip] == 0) return FitfcnOld(x, parameter);
        else if (version[chip] == 1) return Fitfcn(x, parameter);
        else if (version[chip] == 2) return FitfcnTan(x, parameter);
        
        return 0.;

}


