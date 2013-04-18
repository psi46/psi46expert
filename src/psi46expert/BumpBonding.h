// Bumpbonding test

#ifndef BUMPBONDING
#define BUMPBONDING

#include "Test.h"

class BumpBonding : public Test
{

public:
    BumpBonding(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();

protected:

    int nTrig, vthrComp, ctrlReg;
};


#endif

