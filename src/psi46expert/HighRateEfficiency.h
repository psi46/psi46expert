#ifndef __HR_EFFICIENCY_H__
#define __HR_EFFICIENCY_H__

#include "Test.h"
#include "BasePixel/TBInterface.h"
#include "TestParameters.h"
#include "TestRange.h"

class HREfficiency : public Test {
public:
    HREfficiency(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);
    ~HREfficiency();
    virtual void ModuleAction();
};

#endif
