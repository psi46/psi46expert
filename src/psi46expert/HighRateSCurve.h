#ifndef __HR_SCURVE_H__
#define __HR_SCURVE_H__

#include "Test.h"
#include "BasePixel/TBInterface.h"
#include "TestParameters.h"
#include "TestRange.h"

class HRSCurve : public Test {
public:
    HRSCurve(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);
    ~HRSCurve();
    virtual void ModuleAction();
private:
    void TakeEfficiencyMap(int ntrig, bool set_vcal, int vcal_offset);
    TH2I ** efficiency_map;
    TH2I ** rough_threshold;
};

#endif
