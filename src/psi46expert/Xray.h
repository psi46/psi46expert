// Vcal calibration with xray

#ifndef XRAY
#define XRAY

#include "Test.h"

class Xray : public Test
{

public:
    Xray(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void ModuleAction();
    virtual void RocAction();

protected:

    int nTrig, vthrCompMin, vthrCompMax;
    int clockStretchFactor;                 ///< Factor for the stretch of the clock to increase statistics
    int clockStretchDelay;                  ///< Delay after the calibration signal after which to stretch the clock
    double maxEff;
    TH1F * histo[MODULENUMROCS];
};


#endif

