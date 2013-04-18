// Optimization of the Dynamic Range for VoffsetOp and VOffsetR0

#ifndef OFFSETOPTIMIZATION
#define OFFSETOPTIMIZATION

#include "Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

class OffsetOptimization : public PhDacScan
{

public:

    OffsetOptimization(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();
    virtual void PixelAction();

    void DoDacDacScan();

protected:

    int dac1Start, dac1Stop, dac1Step, dac2Start, dac2Stop, dac2Step;

};


#endif
