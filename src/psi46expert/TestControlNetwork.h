// This class provides support for the tests on the ControlNetwork level

#ifndef TESTCONTROLNETWORK
#define TESTCONTROLNETWORK

#include "BasePixel/ControlNetwork.h"
#include "TestModule.h"
#include "TestParameters.h"

class TestControlNetwork: public ControlNetwork
{


public:
    TestControlNetwork(TBInterface * aTbInterface, ConfigParameters * configParameters);
    void Execute(SysCommand &command);

    TestParameters * GetTestParameters();
    TestModule * GetModule(int iModule);

    void AdjustDACParameters();
    void AdjustVana();
    void DoIV();
    void FullTestAndCalibration();
    void ShortTestAndCalibration();
    void ShortCalibration();


protected:

    TestParameters * testParameters;

};


#endif


