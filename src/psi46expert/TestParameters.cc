#include "math.h"

#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>

#include "TestParameters.h"
#include "interface/Log.h"

TestParameters::TestParameters(const char * _file)
    :   PixelMapReadouts(20),
        PixelMapEfficiency(60),

        SCurveMode(1),
        SCurveNTrig(0),
        SCurveVcal(150),
        SCurveVthr(80),
        SCurveBeamNTrig(5),

        ThresholdMode(1),
        ThresholdNTrig(0),
        ThresholdVcal(150),
        ThresholdVthr(80),

        BumpBondingNTrig(10),

        TrimBitsVtrim14(250),
        TrimBitsVtrim13(200),
        TrimBitsVtrim11(150),
        TrimBitsVtrim7(100),
        TrimBitsNTrig(10),

        PHNTrig(5),
        PHMode(25),
        PHdac1Start(100),
        PHdac1Stop(100),
        PHdac1Step(10),
        PHdac2Start(40),
        PHdac2Stop(40),
        PHdac2Step(10),
        PHNumberOfSteps(4),
        PHtestVcal(60),
        PHDacType(1),
        PHSafety(50),
        PHScanDac(0),
        PHScanVcalRange(0),

        TrimVcal(50),
        TrimNTrig(10),
        TrimDoubleWbc(1),
        TrimVtrim(-1),
        TrimVthrcomp(-1),
        TrimAdjustCalDel(0),

        Dac1(12),
        Dac2(25),
        DacRange1(256),
        DacRange2(256),
        DacNTrig(10),

        PHCalibrationNTrig(1),
        PHCalibrationMode(0),
        PHCalibrationNPixels(4160),
        PHCalibrationAdjustVthrComp(1),
        PHCalibrationAdjustCalDel(1),

        IVStep(5),
        IVStart(0),
        IVStop(600),
        IVDelay(3),

        TempNTrig(1),
        TBMUbLevel(-700),
        TWMeanShift(20),

        vsfStart(120),
        vsfStop(180),
        vsfSteps(10),
        vhldDelStart(10),
        vhldDelStop(250),
        vhldDelSteps(10),

        goalPar1(1.4),
        goalCurrent(0.005),

        XrayNTrig(10000),
        XrayVthrCompMin(40),
        XrayVthrCompMax(110),

        XrayMaxEff(0.01),

        HRPixelMapTriggerRate(5),
        HRPixelMapRepetitions(1),
        HRPixelMapAquisitionTime(10),
        HRPixelMapClockStretch(0),
        HRPixelMapStretchDelay(6),

        HREfficiencyTriggers(100),

        HRSCurveThrStart(80),
        HRSCurveThrEnd(130),
        HRSCurveTriggers(50)
{
    ReadTestParameterFile(_file);
}


bool TestParameters::ReadTestParameterFile(const char * _file)
{
    std::ifstream _input(_file);
    if (!_input.is_open())
    {
        psi::LogInfo() << "[TestParameters] Could not open file '" << _file
                       << "' to read Test parameters." << psi::endl;

        return false;
    }

    psi::LogInfo() << "[TestParameters] Reading Test-Parameters from '"
                   << _file << "'." << psi::endl;

    // Read file by lines
    for (std::string _line; _input.good();)
    {
        getline(_input, _line);

        // Skip Empty Lines and Comments (starting from # or - )
        if (!_line.length()
                || '#' == _line[0]
                || '-' == _line[0]) continue;

        std::istringstream _istring(_line);
        std::string _name;
        double      _value;

        _istring >> _name >> _value;

        // Skip line in case any errors occured while reading parameters
        if (_istring.fail() || !_name.length()) continue;

        if (0 == _name.compare("PixelMapReadouts")) { PixelMapReadouts   = static_cast<int>(_value); }
        else if (0 == _name.compare("PixelMapEfficiency")) { PixelMapEfficiency = static_cast<int>(_value); }

        else if (0 == _name.compare("SCurveVcal")) { SCurveVcal      = static_cast<int>(_value); }
        else if (0 == _name.compare("SCurveVthr")) { SCurveVthr      = static_cast<int>(_value); }
        else if (0 == _name.compare("SCurveMode")) { SCurveMode      = static_cast<int>(_value); }
        else if (0 == _name.compare("SCurveNTrig")) { SCurveNTrig     = static_cast<int>(_value); }
        else if (0 == _name.compare("SCurveBeamNTrig")) { SCurveBeamNTrig = static_cast<int>(_value); }

        else if (0 == _name.compare("ThresholdVcal")) { ThresholdVcal  = static_cast<int>(_value); }
        else if (0 == _name.compare("ThresholdVthr")) { ThresholdVthr  = static_cast<int>(_value); }
        else if (0 == _name.compare("ThresholdMode")) { ThresholdMode  = static_cast<int>(_value); }
        else if (0 == _name.compare("ThresholdNTrig")) { ThresholdNTrig = static_cast<int>(_value); }

        else if (0 == _name.compare("BumpBondingNTrig")) { BumpBondingNTrig = static_cast<int>(_value); }

        else if (0 == _name.compare("TrimBitsNTrig")) { TrimBitsNTrig   = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimBitsVtrim14")) { TrimBitsVtrim14 = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimBitsVtrim13")) { TrimBitsVtrim13 = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimBitsVtrim11")) { TrimBitsVtrim11 = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimBitsVtrim7")) { TrimBitsVtrim7  = static_cast<int>(_value); }

        else if (0 == _name.compare("PHNTrig")) { PHNTrig         = static_cast<int>(_value); }
        else if (0 == _name.compare("PHMode")) { PHMode          = static_cast<int>(_value); }
        else if (0 == _name.compare("PHdac1Start")) { PHdac1Start     = static_cast<int>(_value); }
        else if (0 == _name.compare("PHdac1Stop")) { PHdac1Stop      = static_cast<int>(_value); }
        else if (0 == _name.compare("PHdac1Step")) { PHdac1Step      = static_cast<int>(_value); }
        else if (0 == _name.compare("PHdac2Start")) { PHdac2Start     = static_cast<int>(_value); }
        else if (0 == _name.compare("PHdac2Stop")) { PHdac2Stop      = static_cast<int>(_value); }
        else if (0 == _name.compare("PHdac2Step")) { PHdac2Step      = static_cast<int>(_value); }
        else if (0 == _name.compare("PHNumberOfSteps")) { PHNumberOfSteps = static_cast<int>(_value); }
        else if (0 == _name.compare("PHtestVcal")) { PHtestVcal      = static_cast<int>(_value); }
        else if (0 == _name.compare("PHDacType")) { PHDacType       = static_cast<int>(_value); }
        else if (0 == _name.compare("PHSafety")) { PHSafety        = static_cast<int>(_value); }
        else if (0 == _name.compare("PHScanDac"))       { PHScanDac       = static_cast<int>(_value); }
        else if (0 == _name.compare("PHScanVcalRange")) { PHScanVcalRange = static_cast<int>(_value); }

        else if (0 == _name.compare("TrimVcal")) { TrimVcal         = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimNTrig")) { TrimNTrig        = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimDoubleWbc")) { TrimDoubleWbc    = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimVtrim")) { TrimVtrim        = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimVthrcomp")) { TrimVthrcomp     = static_cast<int>(_value); }
        else if (0 == _name.compare("TrimAdjustCalDel")) { TrimAdjustCalDel = static_cast<int>(_value); }

        else if (0 == _name.compare("Dac1")) { Dac1      = static_cast<int>(_value); }
        else if (0 == _name.compare("Dac2")) { Dac2      = static_cast<int>(_value); }
        else if (0 == _name.compare("DacNTrig")) { DacNTrig  = static_cast<int>(_value); }
        else if (0 == _name.compare("DacRange1")) { DacRange1 = static_cast<int>(_value); }
        else if (0 == _name.compare("DacRange2")) { DacRange2 = static_cast<int>(_value); }

        else if (0 == _name.compare("IVStep")) { IVStep  = static_cast<int>(_value); }
        else if (0 == _name.compare("IVStart")) { IVStart = static_cast<int>(_value); }
        else if (0 == _name.compare("IVStop")) { IVStop  = static_cast<int>(_value); }
        else if (0 == _name.compare("IVDelay")) { IVDelay = static_cast<int>(_value); }

        else if (0 == _name.compare("PHCalibrationNTrig")) { PHCalibrationNTrig          = static_cast<int>(_value); }
        else if (0 == _name.compare("PHCalibrationMode")) { PHCalibrationMode           = static_cast<int>(_value); }
        else if (0 == _name.compare("PHCalibrationNPixels")) { PHCalibrationNPixels        = static_cast<int>(_value); }
        else if (0 == _name.compare("PHCalibrationAdjustVthrComp")) { PHCalibrationAdjustVthrComp = static_cast<int>(_value); }
        else if (0 == _name.compare("PHCalibrationAdjustCalDel")) { PHCalibrationAdjustCalDel = static_cast<int>(_value); }

        else if (0 == _name.compare("TempNTrig")) { TempNTrig  = static_cast<int>(_value); }
        else if (0 == _name.compare("TBMUbLevel")) { TBMUbLevel = abs(static_cast<int>(_value)); }

        else if (0 == _name.compare("vsfStart")) { vsfStart     = static_cast<int>(_value); }
        else if (0 == _name.compare("vsfStop")) { vsfStop      = static_cast<int>(_value); }
        else if (0 == _name.compare("vsfSteps")) { vsfSteps     = static_cast<int>(_value); }
        else if (0 == _name.compare("vhldDelStart")) { vhldDelStart = static_cast<int>(_value); }
        else if (0 == _name.compare("vhldDelStop")) { vhldDelStop  = static_cast<int>(_value); }
        else if (0 == _name.compare("vhldDelSteps")) { vhldDelSteps = static_cast<int>(_value); }
        else if (0 == _name.compare("goalPar1")) { goalPar1     = _value; }
        else if (0 == _name.compare("goalCurrent")) { goalCurrent  = _value; }

        else if (0 == _name.compare("TWMeanShift")) { TWMeanShift                 = static_cast<int>(_value); }

        else if (0 == _name.compare("XrayNTrig")) { XrayNTrig       = static_cast<int>(_value); }
        else if (0 == _name.compare("XrayVthrCompMin")) { XrayVthrCompMin = static_cast<int>(_value); }
        else if (0 == _name.compare("XrayVthrCompMax")) { XrayVthrCompMax = static_cast<int>(_value); }
        else if (0 == _name.compare("XrayMaxEff")) { XrayMaxEff      = _value; }

        else if (0 == _name.compare("HRPixelMapTriggerRate")) { HRPixelMapTriggerRate = static_cast<int>(_value); }
        else if (0 == _name.compare("HRPixelMapRepetitions")) { HRPixelMapRepetitions = static_cast<int>(_value); }
        else if (0 == _name.compare("HRPixelMapAquisitionTime")) { HRPixelMapAquisitionTime = static_cast<float>(_value); }
        else if (0 == _name.compare("HRPixelMapClockStretch")) { HRPixelMapClockStretch = static_cast<int>(_value); }
        else if (0 == _name.compare("HRPixelMapStretchDelay")) { HRPixelMapStretchDelay = static_cast<int>(_value); }

        else if (0 == _name.compare("HREfficiencyTriggers")) { HREfficiencyTriggers = static_cast<int>(_value); }

        else if (0 == _name.compare("HRSCurveThrStart")) { HRSCurveThrStart = static_cast<int>(_value); }
        else if (0 == _name.compare("HRSCurveThrEnd")) { HRSCurveThrEnd = static_cast<int>(_value); }
        else if (0 == _name.compare("HRSCurveTriggers")) { HRSCurveTriggers = static_cast<int>(_value); }

        else {
            psi::LogInfo() << "[TestParameter] Did not understand '" << _name << "'."
                           << psi::endl;
        }
    }

    _input.close();

    return true;
}
