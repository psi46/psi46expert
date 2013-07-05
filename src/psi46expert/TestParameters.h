// Central definition point of the parameters which control the test bahaviour

#ifndef TESTPARAMETERS
#define TESTPARAMETERS

class TestParameters
{
public:
    TestParameters(const char * testParametersFileName);

    bool ReadTestParameterFile(const char * filename);

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
    int PHScanDac;
    int PHScanVcalRange;

    int TrimVcal;
    int TrimNTrig;
    int TrimDoubleWbc;
    int TrimVtrim;
    int TrimVthrcomp;
    int TrimAdjustCalDel;

    int Dac1;
    int Dac2;
    int DacRange1;
    int DacRange2;
    int DacNTrig;

    int PHCalibrationNTrig;
    int PHCalibrationMode;
    int PHCalibrationNPixels;
    int PHCalibrationAdjustVthrComp;
    int PHCalibrationAdjustCalDel;

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

    int XrayNTrig;                      ///< Number of triggers per scan point
    int XrayVthrCompMin;                ///< Minimum VthrComp value in the scan
    int XrayVthrCompMax;                ///< Maximum VthrComp value in the scan (plus one)
    int XrayClockStretchFactor;         ///< Factor for the stretch of the clock to increase statistics
    int XrayClockStretchDelay;          ///< Delay after the calibration signal after which to stretch the clock

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

    int CurrentScanDac;
    int CurrentScanNumberOfSteps;
};

#endif
