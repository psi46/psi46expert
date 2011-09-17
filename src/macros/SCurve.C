void FitSCurves(char *dirName)
{
        gSystem->Load("libMinuit.so");
	gSystem->Load("../lib/libana.so");
	
	SCurve sCurve;
	sCurve.FitSCurves(dirName);
}


void FitSCurve(char *dirName, int roc, int col, int row)
{
	gSystem->Load("../lib/libana.so");
	
	SCurve sCurve;
	sCurve.FitSCurve(dirName, roc, col, row);
}
