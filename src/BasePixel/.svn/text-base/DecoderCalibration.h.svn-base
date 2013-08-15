#ifndef DECODERCALIBRATION_H
#define DECODERCALIBRATION_H

//////////////////////////////////////////////////////////////////////////
//
// Classes to define the address levels for decoding of raw pixel data packets
// (Modules, [Plaquettes, to be implemented ?], TBMs and ROCs)
//
// Christian Veelken (UC Davis), 05/11/06
//
/////////////////////////////////////////////////////////////////////////

#include <iostream>

#include "RawPacketDecoder.h"

namespace DecoderCalibrationConstants
{
const int NUM_LEVELSROC =     6; // number of different ROC address levels
const int NUM_LEVELSTBM =     4; // number of different TBM bit levels
}

class DecoderCalibrationTBM
{
public:
    DecoderCalibrationTBM();
    DecoderCalibrationTBM(ADCword levels[]);

    void SetUltraBlackLevel(ADCword level) { fUltraBlackLevel = level; }
    void SetBlackLevel(ADCword level) { fBlackLevel = level; }
    void SetStatusLevel(int levelIndex, ADCword level);

    ADCword GetUltraBlackLevel() const { return fUltraBlackLevel; }
    ADCword GetBlackLevel() const { return fBlackLevel; }
    ADCword GetStatusLevel(int levelIndex) const;
    const ADCword * GetStatusLevel() const { return fStatusLevel; }

protected:
    ADCword fUltraBlackLevel;
    ADCword fBlackLevel;
    ADCword fStatusLevel[DecoderCalibrationConstants::NUM_LEVELSTBM + 1];
};

class DecoderCalibrationROC
{
public:
    DecoderCalibrationROC();
    DecoderCalibrationROC(ADCword levels[]);

    void SetUltraBlackLevel(ADCword level) { fUltraBlackLevel = level; }
    void SetBlackLevel(ADCword level) { fBlackLevel = level; }
    void SetAddressLevel(int levelIndex, ADCword level);

    ADCword GetUltraBlackLevel() const { return fUltraBlackLevel; }
    ADCword GetBlackLevel() const { return fBlackLevel; }
    ADCword GetAddressLevel(int levelIndex) const;
    const ADCword * GetAddressLevel() const { return fAddressLevel; }

protected:
    ADCword fUltraBlackLevel;
    ADCword fBlackLevel;
    ADCword fAddressLevel[DecoderCalibrationConstants::NUM_LEVELSROC + 1];
};

class DecoderCalibrationModule
{
public:
    DecoderCalibrationModule();
    DecoderCalibrationModule(ADCword ultraBlack, ADCword black,
                             ADCword levelROC_Address0, ADCword levelROC_Address1, ADCword levelROC_Address2, ADCword levelROC_Address3,
                             ADCword levelROC_Address4, ADCword levelROC_Address5, ADCword levelROC_Address6,
                             ADCword levelTBM_Address0 = 2000, ADCword levelTBM_Address1 = 2000, ADCword levelTBM_Address2 = 2000,
                             ADCword levelTBM_Address3 = 2000, ADCword levelTBM_Address4 = 2000);
    DecoderCalibrationModule(ADCword levelsTBM[], ADCword levelsROC[][DecoderCalibrationConstants::NUM_LEVELSROC + 1], int numROCs);
    DecoderCalibrationModule(const char * fileName, int fileType, int mode, int numROCs);

    ADCword GetPedestalADC() const { return fPedestalADC; }
    const struct DecoderCalibrationTBM &GetCalibrationTBM() const { return fCalibrationTBM; }
    const struct DecoderCalibrationROC &GetCalibrationROC(int rocId) const;
    int GetNumROCs() const { return fNumROCs; }

    void Print(std::ostream * outputStream) const;

protected:
    void SetCalibration(ADCword ultraBlack, ADCword black,
                        ADCword levelROC_Address0, ADCword levelROC_Address1, ADCword levelROC_Address2, ADCword levelROC_Address3,
                        ADCword levelROC_Address4, ADCword levelROC_Address5, ADCword levelROC_Address6,
                        ADCword levelTBM_Address0 = 2000, ADCword levelTBM_Address1 = 2000, ADCword levelTBM_Address2 = 2000,
                        ADCword levelTBM_Address3 = 2000, ADCword levelTBM_Address4 = 2000);
    void SetCalibration(ADCword levelsTBM[], ADCword levelsROC[][DecoderCalibrationConstants::NUM_LEVELSROC + 1], int numROCs);

    int ReadCalibrationFile(const char * fileName, int fileType, int mode, int numROCs);
    int ReadCalibrationFile1(const char * fileName, int mode, int numROCs);
    int ReadCalibrationFile2(const char * fileName, int mode, int numROCs);
    int ReadCalibrationFile3(const char * fileName, int mode, int numROCs);

    static bool fPrintDebug;
    static bool fPrintWarning;
    static bool fPrintError;

    ADCword fPedestalADC;
    DecoderCalibrationTBM fCalibrationTBM;
    DecoderCalibrationROC fCalibrationROC[DecodedReadoutConstants::NUM_ROCSMODULE];
    int fNumROCs;
};

#endif // DECODERCALIBRATION_H

