// Central definition point of the parameters which control the test bahaviour

#ifndef TESTPARAMETERS
#define TESTPARAMETERS

class TestParameters
{
  public:
    TestParameters( const char *testParametersFileName);

    bool ReadTestParameterFile( const char *filename);

    int PixelMapReadouts;
    int PixelMapEfficiency;

    int SCurveMode;
    int SCurveNTrig;
    int SCurveVcal;
    int SCurveVthr;
    int SCurveBeamNTrig;

    int ThresholdMode;
    int ThresholdNTrig;
    int ThresholdVcal;
    int ThresholdVthr;

    int BumpBondingNTrig;

    int TrimBitsVtrim14;
    int TrimBitsVtrim13;
    int TrimBitsVtrim11;
    int TrimBitsVtrim7;
    int TrimBitsNTrig;

    int PHNTrig;
    int PHMode;
    int PHdac1Start;
    int PHdac1Stop;
    int PHdac1Step;
    int PHdac2Start;
    int PHdac2Stop;
    int PHdac2Step;
    int PHNumberOfSteps;
    int PHtestVcal;
    int PHDacType;
    int PHSafety;

    int TrimVcal;
    int TrimNTrig;
    int TrimDoubleWbc;

    int Dac1;
    int Dac2;
    int DacRange1;
    int DacRange2;
    int DacNTrig;

    int PHCalibrationNTrig;
    int PHCalibrationMode;
    int PHCalibrationNPixels;
    int PHCalibrationCalDelVthrComp;

    int IVStep;
    int IVStart;
    int IVStop;
    int IVDelay;

    int TempNTrig;

    int TBMUbLevel;

    int TWMeanShift;

    int vsfStart;
    int vsfStop;
    int vsfSteps;
    int vhldDelStart;
    int vhldDelStop;
    int vhldDelSteps;

    float goalPar1;
    float goalCurrent;

    int XrayNTrig;
    int XrayVthrCompMin;
    int XrayVthrCompMax;

    double XrayMaxEff;
    
    int HRPixelMapTriggerRate;
    int HRPixelMapRepetitions;
    float HRPixelMapAquisitionTime;
    int HRPixelMapClockStretch;
    int HRPixelMapStretchDelay;

    int HREfficiencyTriggers;
    
    int HRSCurveThrStart;
    int HRSCurveThrEnd;
    int HRSCurveTriggers;
};

#endif
