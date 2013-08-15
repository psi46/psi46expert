// Sets the VIbias_roc and the VOffsetOp DAC so that PH is within the range -tbmublevel+safety ... +tbmublevel

#ifndef PHRANGE_H
#define PHRANGE_H

#include "Test.h"


class PHRange : public Test
{

public:

    PHRange();
    PHRange(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);
    void Init();
    int PHMin();
    int PHMax();
    int PH(int ctrlReg, int vcal, int calDel, int vthrComp, int vtrim, int pixel);

    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();
    void ValidationPlot();

private:

    int phSafety, tbmUbLevel, minPixel, maxPixel, phPosition;
    int calDelMin, vthrCompMin, calDelMax, vthrCompMax, vcalMin, vcalMax, ctrlRegMin, ctrlRegMax, vtrimMin, vtrimMax;
};

#endif
