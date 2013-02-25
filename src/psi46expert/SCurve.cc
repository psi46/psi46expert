#include <TF1.h>
#include <TGraph.h>
#include <TGraphErrors.h>
#include "BasePixel/CalibrationTable.h"
#include "SCurve.h"
#include "TMinuit.h"
#include <TMath.h>

using namespace::std;


ClassImp(SCurve)


Double_t Erffcn( Double_t *x, Double_t *par)
{
	return par[0]*TMath::Erf(par[2]*(x[0]-par[1]))+par[3];
}


SCurve::SCurve()
{
	fit = new TF1("Fit", Erffcn, 0., .230, 4);
	ePerVcal = 65.;  // charge (electrons) per Vcal DAC
	slope = gCalibrationTable->VcalDAC(0, 255)/256.;  // voltage per Vcal DAC
	
	debug = false;
}


bool SCurve::OpenInputFile(char *dirName, int roc)
{
	char fname[1000], string[500];

	sprintf(fname, "%s/SCurveData_C%i.dat", dirName, roc);
	inputFile = fopen(fname, "r");
	if (!inputFile)
	{
		printf("!!!!!!!!!  ----> SCurve: Could not open file %s to read the SCurve data results\n", fname);
		return false;
	}
	fscanf(inputFile, "%s %i", string, &mode);
	return true;
}


void SCurve::Fit()
{
        double chiLimit = 2., chi;
	fit->SetParameters(25., graph->GetMean(), 167., 25.);         // half amplitude, threshold (50% point), width, offset
	graph->Fit("Fit", "Q", "", 0.0, 0.3);
	chi = fit->GetChisquare()/fit->GetNDF();

	bool notConverged = (!strcmp(gMinuit->fCstatu.Data(), "FAILED    ")) && (gMinuit->fEDM > 1.e-4);   //if fEDM very small, convergence failed only due to limited machine accuracy
        if (!notConverged  && (chi < chiLimit))  // if not failed
        {
          thr = fit->GetParameter(1) * ePerVcal / slope;  // conversion Vcal voltage -> Vcal DACs -> electrons
          sig = 1. / (TMath::Sqrt(2.) * fit->GetParameter(2)) * ePerVcal / slope;  // conversion Vcal voltage -> Vcal DACs -> electrons
        }
	else
	{
		if (debug)
		{
			if (chi > chiLimit) printf("Chi %e\n", chi);
			else printf("not converged\n");
		}		
	}
}


bool SCurve::ReadData(int &n, double x[], double y[], double xErr[], double yErr[])
{
	const double nReadouts = 50.;
	double eff;
        int start, value;

	bool badPixel = false;
	bool deadPixel = true;
	bool plateau = false;
	bool zeroLevel = false;
	
	fscanf(inputFile, "%i %i", &n, &start);
	for (int i = 0; i < n; i++) 
	{
		fscanf(inputFile, "%i", &value);
		if (value > 0) deadPixel = false;
		if ((value < 0) || (value > nReadouts)) badPixel = true;
		if (plateau && value == 0) value = (int)nReadouts; //bad readout
		if (value == (int)nReadouts) plateau = true;
		if (value == 0) zeroLevel = true;
		x[i] = gCalibrationTable->VcalDAC(0, start + i);
		xErr[i] = 0.;
		eff =  ((double)value + 1.)/(nReadouts + 2.);
		y[i] = nReadouts*eff;
     		yErr[i] = nReadouts*TMath::Sqrt((eff*(1.-eff))/(nReadouts + 3.));
	}
	if (!plateau || !zeroLevel) badPixel = true; //no full SCurve
			
	if (debug)
	{
		if (deadPixel) printf("dead pixel\n");
		if (badPixel) printf("bad pixel\n");
	}
	if (badPixel || deadPixel) return false;
	return true;
}


void SCurve::FitSCurves(char *dirName, int nRocs)
{
	printf("Fitting SCurves %s\n", dirName);
	double x[255], y[255], xErr[255], yErr[255];
	int n;
	char fname[1000];
	
	for (int iroc = 0; iroc < nRocs; iroc++)
	{
		printf("Fitting SCurves for chip %i\n", iroc);

		if (!OpenInputFile(dirName, iroc)) continue;
	
		// outputFile
		sprintf(fname, "%s/SCurve_C%i.dat", dirName, iroc);
		outputFile = fopen(fname, "w");
		if (!outputFile)
		{
			printf("!!!!!!!!!  ----> SCurve: Could not open file %s to write the fit results\n", fname);
			return;
		}
		fprintf(outputFile, "Threshold Sigma\n\n");
		
		for (int icol = 0; icol < 52; ++icol)
		{
			for (int irow = 0; irow < 80; ++irow)
			{
                                thr = -1.;
                                sig = -1.;
				
                                if (ReadData(n, x, y, xErr, yErr))
                                {
                                  graph = new TGraphErrors(n, x, y, xErr, yErr);
                                  Fit();
				  delete graph;
                                }
                                fprintf(outputFile, "%+.3e %+.3e   Pix %2i %2i\n", thr, sig, icol, irow);
				
				if ((thr == -1.) && debug) printf("roc %i, col %i, row %i\n", iroc, icol, irow);
			}
		}
		fclose(outputFile);
		fclose(inputFile);
	}
	delete fit;
}



void SCurve::FitSCurve(char *dirName, int roc, int col, int row)
{
        bool result = false;
	double x[255], y[255], xErr[255], yErr[255];
	int nPoints;

	thr = -1.;
	sig = -1.;

	OpenInputFile(dirName, roc);
	
	for (int icol = 0; icol < 52; ++icol)
	{
		for (int irow = 0; irow < 80; ++irow)
		{
			if (ReadData(nPoints, x, y, xErr, yErr))
			{
				if (icol == col && irow == row)
				{
					graph = new TGraphErrors(nPoints, x, y, xErr, yErr);
					Fit();
					result = true;
				}
			}
			else
			{
			       if (icol == col && irow == row)
			       {
			                printf(">>>>>>>> No valid SCurve <<<<<<<<<\n");
			       }
			}
		}
	}
	fclose(inputFile);

	printf("thr %e sigma %e \n", thr, sig);
	if (result) graph->Draw("A*");
}
