#include "TestControlNetwork.h"
#include "MainFrame.h"
#include "IVCurve.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/TBInterface.h"
#include <TApplication.h>
#include <TSystem.h>
#include <iostream>

using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;

// Initializes the TestControlNetwork to a give configuration
TestControlNetwork::TestControlNetwork(TBInterface * aTBInterface, ConfigParameters * aConfigParameters)
{
    RawPacketDecoder * gDecoder = RawPacketDecoder::Singleton();
    configParameters = aConfigParameters;
    testParameters = new TestParameters(configParameters->GetTestParametersFileName());
    tbInterface = aTBInterface;
    nModules = configParameters->nModules;

    for (int i = 0; i < nModules; i++)
    {
        module[i] = new TestModule(configParameters, 0, tbInterface, testParameters);
    }

    TString fileName = TString(configParameters->directory).Append("/addressParameters.dat");
    cout << "Reading Address Level-Parameters from " << fileName << endl;
    //DecoderCalibrationModule* decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, NUM_ROCSMODULE);
    DecoderCalibrationModule * decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, configParameters->nRocs);
    decoderCalibrationModule->Print(&cout);
    gDecoder->SetCalibration(decoderCalibrationModule);

    Initialize();
}


void TestControlNetwork::DoIV()
{
    // dirty solution for a not understood problem
    // from gui: IV Test works well, steps take 5s
    // from commandline: IV Test slow, steps take 30s
    // code is identical, gui framework somehow influences the read command in Keithley.cc

    new TApplication("App", 0, 0, 0, -1);
    MainFrame * mf = new MainFrame(gClient->GetRoot(), 400, 400, tbInterface, this, configParameters, false);
    mf->Connect("IV()", "MainFrame", mf, "IV()");
    mf->Emit("IV()");
}


void TestControlNetwork::FullTestAndCalibration()
{
    for (int i = 0; i < nModules; i++) GetModule(i)->FullTestAndCalibration();
}


void TestControlNetwork::ShortTestAndCalibration()
{
    for (int i = 0; i < nModules; i++) GetModule(i)->ShortTestAndCalibration();
}


void TestControlNetwork::ShortCalibration()
{
    for (int i = 0; i < nModules; i++) GetModule(i)->ShortCalibration();
}


TestParameters * TestControlNetwork::GetTestParameters()
{
    return testParameters;
}


TestModule * TestControlNetwork::GetModule(int iModule)
{
    return (TestModule *)module[iModule];
}


void TestControlNetwork::Execute(SysCommand &command)
{
    if (command.Keyword("IV")) {DoIV();}
    else GetModule(command.module)->Execute(command);
}


// Tries to automatically adjust Vana
void TestControlNetwork::AdjustVana()
{
    for (int i = 0; i < nModules; i++)
    {
        GetModule(i)->AdjustVana();
    }
}


void TestControlNetwork::AdjustDACParameters()
{
    for (int i = 0; i < nModules; i++)
    {
        GetModule(i)->AdjustDACParameters();
    }
}
