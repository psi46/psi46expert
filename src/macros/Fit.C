void Fit(){
  gSystem->Load("../psi46expert/.libs/libpsi46ana.so");
	
  SCurve sCurve;
  sCurve.FitSCurves("../psi46expert/currentModule/");
  
  }
