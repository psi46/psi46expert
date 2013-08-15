void FitSCurves(char *dirName)
{
        gSystem->Load("libMinuit.so");
	gSystem->Load("../psi46expert/.libs/libpsi46ana.so");
	
	SCurve sCurve;
	sCurve.FitSCurves(dirName);
}


void FitSCurve(char *dirName, int roc, int col, int row)
{
	gSystem->Load("../psi46expert/.libs/libpsi46ana.so");
	
	SCurve sCurve;
	sCurve.FitSCurve(dirName, roc, col, row);
}
