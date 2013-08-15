// Measures an empty analog readout

#ifndef ANALOGREADOUT
#define ANALOGREADOUT

#include "Test.h"


class AnalogReadout : public Test
{

public:
    AnalogReadout(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    virtual void ModuleAction();

    void AddressLevelsTest();
    void DualModeTest();

protected:

};


#endif

