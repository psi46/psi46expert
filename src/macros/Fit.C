void Fit(){
  gSystem->Load("../lib/libana.so");
	
  SCurve sCurve;
  sCurve.FitSCurves("../psi46expert/currentModule/");
  
  }