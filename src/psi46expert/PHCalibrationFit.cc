// fit modes: 1 = Peter's 5 parameter function
//            0 = linear in the main region
//            3 = tanh fit

#include <TSystem.h>
#include <TStyle.h>
#include "TF1.h"
#include "TH1D.h"
#include "TFile.h"
#include "PHCalibrationFit.h"
#include "TGraphErrors.h"
#include "TCanvas.h"
#include <TMath.h>
#include <TROOT.h>

ClassImp(PHCalibrationFit)

const double xCut = TMath::Pi() / 2. - 0.0005;
const double tanXCut = TMath::Tan(xCut);


Double_t Fitfcn(Double_t * x, Double_t * par)
{
    if (par[0]*x[0] - par[4] > xCut) return tanXCut + (x[0] - (xCut + par[4]) / par[0]) * 1e8;
    return TMath::Tan(par[0] * x[0] - par[4]) + par[1] * x[0] * x[0] * x[0] + par[5] * x[0] * x[0] + par[2] * x[0] + par[3];
}


Double_t FitfcnTan(Double_t * x, Double_t * par)
{
    return par[3] + par[2] * TMath::TanH(par[0] * x[0] - par[1]);
}


const char * Fitfcn()
{
    return "TMath::Tan(par[0]*x[0] - par[4]) + par[1]*x[0]*x[0]*x[0] + par[5]*x[0]*x[0] + par[2]*x[0] + par[3]";
}

const char * FitfcnTan()
{
    return "par[3] + par[2] * TMath::TanH(par[0]*x[0] - par[1])";
}


PHCalibrationFit::PHCalibrationFit(int aNumber)
{
    fitMode = aNumber;

    if (3 == fitMode) nFitParams = 4;
    else nFitParams = 6;
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1, 0);

    histoFit[0] = new TH1D("histoFit1", "histoFit1", 200, 0.0001, 0.0003);
    histoFit[1] = new TH1D("histoFit2", "histoFit2", 400, 0.0000001, .0000009);
    histoFit[2] = new TH1D("histoFit3", "histoFit3", 300, 0.4, 0.7);
    histoFit[3] = new TH1D("histoFit4", "histoFit4", 160, 180., 340.);
    histoFit[4] = new TH1D("histoFit5", "histoFit5", 200, -1.5, -1.3);
    histoFit[5] = new TH1D("histoFit6", "histoFit6", 400, -4.e-4, 0.);
    histoChi = new TH1D("histoChi", "histoChi", 1000, 0., 10.);

    if (3 == fitMode) phFit = new TF1("phFit", FitfcnTan, 50., 1500., 4);
    else phFit = new TF1("phFit", Fitfcn, -400., 1000., nFitParams);
    phFit->SetNpx(1000);

    for (int i = 0; i < 2 * vcalSteps; i++)
    {
        yErr[i] = 2.;
    }

    xErr[0] = 8.94;
    xErr[1] = 8.89;
    xErr[2] = 8.55;
    xErr[3] = 8.55;
    xErr[4] = 9.16;
    xErr[5] = 8.68;
    xErr[6] = 8.90;
    xErr[7] = 7.85;
    xErr[8] = 7.29;
    xErr[9] = 4.37;

    vcal[0] = 50.;
    vcal[1] = 100.;
    vcal[2] = 150.;
    vcal[3] = 200.;
    vcal[4] = 250.;
    vcal[5] = 30.;
    vcal[6] = 50.;
    vcal[7] = 70.;
    vcal[8] = 90.;
    vcal[9] = 200.;

    for (int i = 0; i < 2 * vcalSteps; i++)
    {
        vcalLow[i] = vcal[i];
        if (i > (vcalSteps - 1)) vcalLow[i] *= rangeConversion;
    }
}


void PHCalibrationFit::FitTanPol()
{
    bool verbose = false;

    graph = new TGraphErrors(n, x, y, xErr, yErr);

    double xmax = 0., xmin = 9999.;
    for (int i = 0; i < n; i++)
    {
        if (x[i] < xmin) xmin = x[i];
        if (x[i] > xmax) xmax = x[i];
    }
    phFit->SetRange(xmin, xmax);

    int upperPoint = vcalSteps + 2 - 1;
    int lowerPoint = vcalSteps / 3 - 1;
    double slope;

    if ((upperPoint < n) && (lowerPoint < n) && ((x[upperPoint] - x[lowerPoint]) != 0)) slope = (y[upperPoint] - y[lowerPoint]) / (x[upperPoint] - x[lowerPoint]);
    else slope = 0.5;

    phFit->SetParameter(2, slope);
    phFit->SetParameter(3, y[upperPoint] - slope * x[upperPoint]);


    double par0 = (TMath::Pi() / 2. - 1.4) / x[n - 1];
    phFit->SetParameter(0, par0);
    phFit->SetParameter(1, 5.e-7);
    phFit->SetParameter(4, -1.4);
    if (x[upperPoint] != 0.) phFit->SetParameter(5, (y[upperPoint] - (TMath::Tan(phFit->GetParameter(0)*x[upperPoint] - phFit->GetParameter(4)) + phFit->GetParameter(1)*x[upperPoint]*x[upperPoint]*x[upperPoint] + slope * x[upperPoint] + phFit->GetParameter(3))) / (x[upperPoint]*x[upperPoint]));
    else phFit->SetParameter(5, 0.);

    if (verbose) graph->Fit("phFit", "R", "");
    else graph->Fit("phFit", "RQ", "");

    for (int i = 0; i < nFitParams; i++) {histoFit[i]->Fill(phFit->GetParameter(i));}

}


void PHCalibrationFit::FitLin()
{
    bool verbose = false;

    graph = new TGraphErrors(n, y, x, yErr, xErr);

    phFit->SetRange(vcal[2], vcal[8]*rangeConversion);

    int upperPoint = vcalSteps + 2 - 1;
    int lowerPoint = vcalSteps / 3 - 1;
    double slope;

    if ((upperPoint < n) && (lowerPoint < n) && ((x[upperPoint] - x[lowerPoint]) != 0)) slope = (y[upperPoint] - y[lowerPoint]) / (x[upperPoint] - x[lowerPoint]);
    else slope = 0.5;

    phFit->SetParameter(2, slope);
    phFit->SetParameter(3, y[upperPoint] - slope * x[upperPoint]);

    phFit->FixParameter(0, 0.);
    phFit->FixParameter(1, 0.);
    phFit->FixParameter(4, 0.);
    phFit->FixParameter(5, 0.);

    if (verbose) graph->Fit("phFit", "R", "");
    else graph->Fit("phFit", "RQ", "");

    for (int i = 0; i < nFitParams; i++) {histoFit[i]->Fill(phFit->GetParameter(i));}

}



void PHCalibrationFit::FitTanh()
{
    bool verbose = false;

    graph = new TGraphErrors(n, y, x, yErr, xErr);

    phFit->SetParameter(0, 0.004);
    phFit->SetParameter(1, 1.4);
    phFit->SetParameter(2, 1000);
    phFit->SetParameter(3, 0);
    phFit->SetRange(50, 1500);


    if (verbose) graph->Fit("phFit", "R", "");
    else graph->Fit("phFit", "RQ");

    for (int i = 0; i < nFitParams; i++) {histoFit[i]->Fill(phFit->GetParameter(i));}

}

void PHCalibrationFit::WriteOutputFile(FILE * outputFile)
{
    if (0 == fitMode)
    {
        fprintf(outputFile, "%+e ", phFit->GetParameter(0));
        fprintf(outputFile, "%+e ", phFit->GetParameter(1));
        fprintf(outputFile, "%+e ", 1 / phFit->GetParameter(2));
        fprintf(outputFile, "%+e ", -phFit->GetParameter(3) / phFit->GetParameter(2));
        fprintf(outputFile, "%+e ", phFit->GetParameter(4));
        fprintf(outputFile, "%+e ", phFit->GetParameter(5));
    }
    else
    {
        for (int i = 0; i < nFitParams; i++)
        {
            fprintf(outputFile, "%+e ", phFit->GetParameter(i));
        }
    }
}


void PHCalibrationFit::FitAllCurves(char * dirName, int nRocs)
{
    FILE * inputFile, *outputFile;
    char fname[1000], string[500];
    int ph[2 * vcalSteps], a, b, maxRoc, maxCol, maxRow;
    double chiSquare, maxChiSquare = 0.;

    printf("Fitting digital PH Curves %s\n", dirName);

    for (int chip = 0; chip < nRocs; chip++)
    {
        maxChiSquare = 0.;
        printf("Fitting pulse height curves for chip %i\n", chip);

        sprintf(fname, "%s/phCalibration_C%i.dat", dirName, chip);
        inputFile = fopen(fname, "r");
        if (!inputFile)
        {
            printf("!!!!!!!!!  ----> PHCalibration: Could not open file %s to read pulse height calibration\n", fname);
            continue;
        }

        for (int i = 0; i < 4; i++) fgets(string, 500, inputFile);

        if (3 == fitMode) sprintf(fname, "%s/phCalibrationFitTan_C%i.dat", dirName, chip);
        else sprintf(fname, "%s/phCalibrationFit_C%i.dat", dirName, chip);
        outputFile = fopen(fname, "w");
        if (!outputFile)
        {
            printf("!!!!!!!!!  ----> PHCalibration: Could not open file %s to write the fit results\n", fname);
            continue;
        }
        fprintf(outputFile, "Parameters of the vcal vs. pulse height fits\n");
        if (3 == fitMode) fprintf(outputFile, "%s\n", FitfcnTan());
        else fprintf(outputFile, "%s\n", Fitfcn());
        fprintf(outputFile, "\n");

        for (int iCol = 0; iCol < 52; iCol++)
        {
            for (int iRow = 0; iRow < 80; iRow++)
            {
                n = 0;
                for (int i = 0; i < 2 * vcalSteps; i++)
                {
                    fscanf(inputFile, "%s", string);

                    if (1 == fitMode || 3 == fitMode)
                    {
                        if (atoi(string)<0);
                        else {
                            ph[i] = atoi(string);
                            x[n] = (double)ph[i];
                            y[n] = vcalLow[i];
                            n++;
                        }
                    }
                    if (0 == fitMode)
                    {
                        if (atoi(string)<0 || (i < 2) || (i > 2 * vcalSteps - 2));
                        else {
                            ph[i] = atoi(string);
                            x[n] = (double)ph[i];
                            y[n] = vcalLow[i];
                            n++;
                        }
                    }
                }
                fscanf(inputFile, "%s %2i %2i", string, &a, &b);  //comment

                if (n != 0)
                {
                    if (3 == fitMode) FitTanh();
                    else if (1 == fitMode) FitTanPol();
                    else if (0 == fitMode) FitLin();

                    delete graph;
                    chiSquare = phFit->GetChisquare() / phFit->GetNDF();
                    if (chiSquare > maxChiSquare)
                    {
                        maxChiSquare = chiSquare;
                        maxRoc = chip;
                        maxCol = iCol;
                        maxRow = iRow;
                    }
                    histoChi->Fill(chiSquare);
                    WriteOutputFile(outputFile);
                }
                else  // there may be dead pixels
                {
                    for (int i = 0; i < nFitParams; i++) {fprintf(outputFile, "%+e ", 0.);}
                }
                fprintf(outputFile, "    Pix %2i %2i\n", iCol, iRow);
            }
        }
        printf(" %i %i Max ChiSquare/NDF %e\n", maxCol, maxRow, maxChiSquare);
        fclose(inputFile);
        fclose(outputFile);
    }
    printf(" %i %i %i Max ChiSquare/NDF %e\n", maxRoc, maxCol, maxRow, maxChiSquare);
}


void PHCalibrationFit::FitCurve(char * dirName, int chip, int col, int row)
{
    FILE * inputFile;
    char fname[1000], string[1000];
    int ph[2 * vcalSteps], a, b;
    double chiSquare;

    sprintf(fname, "%s/phCalibration_C%i.dat", dirName, chip);
    inputFile = fopen(fname, "r");
    if (!inputFile)
    {
        printf("!!!!!!!!!  ----> PHCalibration: Could not open file %s to read pulse height calibration\n", fname);
        return;
    }

    for (int i = 0; i < 4; i++) fgets(string, 1000, inputFile);
    for (int i = 0; i < col * 80 + row; i++) fgets(string, 1000, inputFile);

    n = 0;
    for (int i = 0; i < 2 * vcalSteps; i++)
    {
        fscanf(inputFile, "%s", string);
        if (1 == fitMode || 3 == fitMode)
        {
            if (atoi(string)<0);
            else {
                ph[i] = atoi(string);
                printf("ph %i vcal %.0f\n", ph[i], vcalLow[i]);
                x[n] = (double)ph[i];
                y[n] = vcalLow[i];
                n++;
            }
        }
        if (0 == fitMode)
        {
            if (atoi(string)<0 || (i < 2) || (i > 2 * vcalSteps - 2));
            else {
                ph[i] = atoi(string);
                //              printf("ph %i vcal %.0f\n", ph[i], vcalLow[i]);
                x[n] = (double)ph[i];
                y[n] = vcalLow[i];
                n++;
            }
        }

    }
    fscanf(inputFile, "%s %2i %2i", string, &a, &b);  //comment

    if (n != 0)
    {
        if (3 == fitMode) FitTanh();
        else if (1 == fitMode) FitTanPol();
        else if (0 == fitMode) FitLin();

        chiSquare = phFit->GetChisquare() / phFit->GetNDF();
        printf("chiSquare/NDF %e\n", chiSquare);
        graph->SetTitle("");
        if (1 == fitMode)
        {
            graph->GetXaxis()->SetTitle("Pulse height (ADC units)");
            graph->GetXaxis()->SetRangeUser(-600., 1300.);
            graph->GetYaxis()->SetTitle("Vcal (DAC units)");
            graph->GetYaxis()->SetRangeUser(0., 1700.);
        }
        else
        {
            graph->GetYaxis()->SetTitle("Pulse height (ADC units)");
            graph->GetYaxis()->SetRangeUser(-600., 2000.);
            graph->GetXaxis()->SetTitle("Vcal (DAC units)");
            graph->GetXaxis()->SetRangeUser(0., 1700.);
        }

        graph->GetYaxis()->SetTitleOffset(1.2);
        graph->Draw("A*");
    }
    else printf("Error: No measured pulse height values for this pixel\n");

}


void PHCalibrationFit::ShowFitPlots()
{

    TCanvas * c1 = new TCanvas();
    c1->Divide(3, 2);

    c1->cd(1);
    histoFit[1]->Draw();

    c1->cd(2);
    histoFit[2]->Draw();

    c1->cd(3);
    histoFit[3]->Draw();

    c1->cd(4);
    histoFit[4]->Draw();

    c1->cd(5);
    histoFit[5]->Draw();

    c1->cd(6);
    histoChi->Draw();
}
