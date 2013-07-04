#ifndef PHTEST
#define PHTEST

#include "Test.h"
#include <TH2D.h>
#include <TH1D.h>

class PHTest : public Test
{

public:
    PHTest(TestRange * testRange, TestParameters * testParameters, TBInterface * aTBInterface);

    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();
    virtual void RocAction(TestRoc * testRoc);
    virtual void PixelAction();

    void PhDac(char * dacName);
    void PulseHeightRocDigital(int data []);

    void SetMode(int mode);                     ///< Sets the test mode: 0: Pulse height map, >0: DAC scan
    void SetNTrig(int ntrig);                   ///< Sets the number of triggers per DAC setting or pixel (mode 0)


protected:

    int nTrig, mode;
    TH2D * map;
    int nRocs;                                  ///< Number of ROCs in the module

};


#endif

