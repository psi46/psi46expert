// Adjust VhldDel by optimizing the Linearity in the low range

#ifndef VHLDDELOPTIMIZATION_H
#define VHLDDELOPTIMIZATION_H

#include "Test.h"


class VhldDelOptimization : public Test
{

public:

    VhldDelOptimization();
    VhldDelOptimization(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    void Initialize();
    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();
    int AdjustVhldDel(TestRange * pixelRange);
    void PixelLoop();

private:

    int hldDelValue;

};

#endif
