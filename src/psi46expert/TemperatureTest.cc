#include <TGraph.h>
#include <TF1.h>

#include "TemperatureTest.h"
#include "interface/Log.h"
#include "BasePixel/TBAnalogInterface.h"

TemperatureTest::TemperatureTest(TestRange * aTestRange, TestParameters * testParameters, TBInterface * aTBInterface)
{
    psi::LogDebug() << "[TemperatureTest] Initialization." << psi::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    ReadTestParameters(testParameters);
}


void TemperatureTest::ReadTestParameters(TestParameters * testParameters)
{
    nTrig = testParameters->TempNTrig;
}


Double_t Fitfcn(Double_t * x, Double_t * par)
{
    return par[1] * x[0] + par[0];
}


void TemperatureTest::RocAction()
{
    TBAnalogInterface * anaInterface = (TBAnalogInterface *)tbInterface;

    // get black level
    unsigned short count;
    short data[FIFOSIZE], blackLevel;

    anaInterface->ADCRead(data, count, nTrig);
    blackLevel = data[9 + aoutChipPosition * 3];

    // Calibrate
    TGraph * calib = new TGraph();
    calib->SetName(Form("TempCalibration_C%i", chipId));
    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++)
    {
        psi::LogInfo() << "[TemperatureTest] Measuring calibration point " << rangeTemp << " ..." << psi::endl;
        SetDAC("RangeTemp", rangeTemp + 8);
        Flush();
        calib->SetPoint(rangeTemp, rangeTemp, anaInterface->LastDAC(nTrig, aoutChipPosition));
    }
    histograms->Add(calib);
    calib->Write();

    // Measure temperature
    TGraph * meas = new TGraph();
    meas->SetName(Form("TempMeasurement_C%i", chipId));

    for (int rangeTemp = 0; rangeTemp < 8; rangeTemp++)
    {
        psi::LogInfo() << "[TemperatureTest] Measuring temperature point " << rangeTemp << " ..." << psi::endl;
        SetDAC("RangeTemp", rangeTemp);
        Flush();
        meas->SetPoint(rangeTemp, rangeTemp, anaInterface->LastDAC(nTrig, aoutChipPosition));
    }

    histograms->Add(meas);
    meas->Write();
}


