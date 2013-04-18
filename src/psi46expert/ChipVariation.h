// pulse height - Vcal overview for different chips / pixels

#ifndef CHIPVARIATION
#define CHIPVARIATION

#include "Test.h"
#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

class ChipVariation : public PhDacScan
{

public:

    ChipVariation(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();
    virtual void PixelAction();
    virtual void ModuleAction();

    void Scan();
    TH1D * linRange;

protected:

    int NumberOfSteps;

};


#endif
