#ifndef __HIGH_RATE_TRIM_LOW_H__
#define __HIGH_RATE_TRIM_LOW_H__

#include "Test.h"
#include "BasePixel/TBInterface.h"
#include "TestParameters.h"
#include "TestRange.h"

class HRTrimLow : public Test {
public:
    HRTrimLow(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);
    ~HRTrimLow();
    virtual void RocAction();
    virtual void PixelAction();

private:
    void MakeMap();
    TH2I * map;
    int triggers;
};

#endif
