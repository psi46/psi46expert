#include <TH1D.h>
#include "Test.h"
#include "BasePixel/TBAnalogInterface.h"
#include "TestParameters.h"
#include "TestRange.h"

class Currents : public Test
{
public:
    Currents(TestRange * testRange, TestParameters * testParameters,  TBInterface * aTBInterface);
    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();
    void DoCurrentScan();

protected:
    int NumberOfSteps;
};
