// Tests address levels and both tbms

#ifndef TBMTEST
#define TBMTEST

#include "Test.h"


class TBMTest : public Test
{

public:
    TBMTest(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void ModuleAction();

    void DualModeTest();
    void ReadoutTest();

protected:

    int result[2];

};


#endif

