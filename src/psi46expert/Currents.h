#include <TH1D.h>
#include "Test.h"
#include "BasePixel/TBAnalogInterface.h"
#include "TestParameters.h"
#include "TestRange.h"

/// Class to Scan through a specified DAC and record the analog and digital currents.
/**
    The specified DAC is scanned through in a given number of steps and then the analog and digital currents on the ROC measured.
 */
class Currents : public Test
{
public:
    Currents(TestRange * testRange, TestParameters * testParameters,  TBInterface * aTBInterface);
    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void RocAction();
    void DoCurrentScan();   ///< Perform the Scan through the DAC

protected:
    int NumberOfSteps;      ///< Number of steps in the Scan. Must be a divisor of 256 or 16 for 4- or 8-bit DACs, respectively.
    int DacRegister;        ///< DAC register of the DAC to be varied in the Scan.
};
