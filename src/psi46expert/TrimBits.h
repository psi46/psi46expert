// Trim bit test

#ifndef TRIMBITTEST
#define TRIMBITTEST

#include "Test.h"

class TrimBits : public Test
{

public:
    TrimBits(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();

protected:

    int nTrig, vtrim14, vtrim13, vtrim11, vtrim7;
};


#endif

