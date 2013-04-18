#include "RawPacketDecoder.h"

#include <cstdlib>
#include <iostream>
#include <iomanip>

//--- include header file that defines global constants
//    (currently, GlobalConstants.h as used with the PSI46 testboards is included;
//     need to find a more flexible solution when using the RawPacketDecoder class with spy-data
//     of the Front-End-Driver of the final CMS Pixel system)
#include "BasePixel/GlobalConstants.h"

#include "DecoderCalibration.h"
#include "DecodedReadout.h"

using namespace std;
using namespace DecoderCalibrationConstants;
using namespace RawPacketDecoderConstants;
using namespace DecodedReadoutConstants;

RawPacketDecoder * RawPacketDecoder::fInstance = 0;

bool RawPacketDecoder::fPrintDebug   = false;
//bool RawPacketDecoder::fPrintDebug   = true;
bool RawPacketDecoder::fPrintWarning = true;
bool RawPacketDecoder::fPrintError   = true;

//-------------------------------------------------------------------------------
RawPacketDecoder::RawPacketDecoder()
{
    Initialize();
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void RawPacketDecoder::Initialize()
{
    if (fPrintDebug) {
        cout << "<RawPacketDecoder::Initialize>: Creating pixel packet decoder Singleton instance" << endl;
    }

    fCalibration = 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
RawPacketDecoder * RawPacketDecoder::Singleton()
{
    if (fInstance == 0) {
        fInstance = new RawPacketDecoder();
    }

    return fInstance;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
RawPacketDecoder::~RawPacketDecoder(void)
{
    // *** nothing to be done yet ***
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::decode(int dataLength, ADCword dataBuffer[], DecodedReadoutModule &module, int numROCs)
/*
  Interprete the raw data in the dataBuffer to extract pixel hit information;
  the extracted information is stored in the pixelHits array

  Return value of function is the number of pixels hits that have been decoded

  Error code: -1 TBM header not found
              -2 TBM trailer not found
              -3 ROC label error
              -4 pixel data not equal to n*6
              -5 no ROC labels, no token pass
          -6 no Calibration object set
*/
{
    if (fPrintDebug) {
        cout << "ADC values = { ";
        for (int ivalue = 0; ivalue < dataLength; ivalue++) {
            cout << dataBuffer[ivalue] << " ";
        }
        cout << "}" << endl;
    }

    //--- check that UltraBlack, Black and Status Levels are set for TBM and
    //               UltraBlack, Black and Address Levels are set for ROCs
    if (fCalibration == 0) {
        cerr << "Error in <RawPacketDecoder::decode>: no Calibration object set !" << endl;
        return -6;
    }

    //--- reset number of pixel hits
    int numPixelHitsModule = 0;

    //--- correct ADC values for pedestal of ADC
    for (int ivalue = 0; ivalue < dataLength; ivalue++) {
        dataBuffer[ivalue] -= fCalibration->GetPedestalADC();
    }

    //--- find TBM header
    //    (function returns index of first ADC value in TBM header)
    //    exit with error code if TBM header cannot be found
    int indexTBMheader = findTBMheader(0, dataLength, dataBuffer);
    if (indexTBMheader < 0) {
        if (fPrintError) cerr << "Error in <RawPacketDecoder::Decode>: could not find TBM header !" << endl;
        return -1;
    }

    //--- decode TBM header
    decodeTBMheader(indexTBMheader, dataLength - indexTBMheader, dataBuffer, module);

    //--- find TBM trailer
    //    (function returns index of first ADC value in TBM trailer)
    //    exit with error code if TBM trailer cannot be found
    //    (start searching for TBM trailer after TBM header)
    int indexTBMtrailer = findTBMtrailer(indexTBMheader + fNumClocksTBMheader, dataLength, dataBuffer);
    if (indexTBMtrailer < 0) {
        if (fPrintError) cerr << "Error in <RawPacketDecoder::Decode>: could not find TBM trailer !" << endl;
        return -2;
    }

    //--- decode TBM trailer
    decodeTBMtrailer(indexTBMtrailer, dataLength - indexTBMtrailer, dataBuffer, module);

    //--- check if there is any ROC data contained between TBM header and trailer
    //    (otherwise the read-out token probably did not pass through all ROCs)
    if (indexTBMtrailer == (indexTBMheader + fNumClocksTBMheader)) {
        if (fPrintError) cerr << "Error in <RawPacketDecoder::Decode>: data packet contains no ROC data, only TBM header and trailer !" << endl;
        return -5;
    }

    //--- find ROC headers
    //    (inbetween TBM header and TBM trailer)
    int index = indexTBMheader + fNumClocksTBMheader;
    int indexROCheader[MAX_ROCS];
    int numROCheaders = 0;
    while (index >= 0 && index < (indexTBMtrailer - 1)) {
        index = findROCheader(numROCheaders, index, dataLength - fNumClocksTBMtrailer, dataBuffer);

        if (index >= 0) {
            indexROCheader[numROCheaders] = index;
            numROCheaders++;
            index += fNumClocksROCheader;
        }
    }

    //--- check that headers have been found for all ROCs
    if (numROCheaders != numROCs) {
        if (fPrintError) cerr << "Error in <RawPacketDecoder::Decode>: could not find headers of all " << numROCs << " ROCs !" << endl;
        return -3;
    }

    //--- decode ROC sequence
    for (int iroc = 0; iroc < numROCheaders; iroc++) {
        int indexStart = indexROCheader[iroc];
        int indexStop  = (iroc < (numROCheaders - 1)) ? indexROCheader[iroc + 1] - 1 : indexTBMtrailer - 1;

        int numPixelHitsROC = decodeROCsequence(iroc, indexStart, indexStop, dataBuffer, module, numROCs);
        if (numPixelHitsROC < 0)
            return -4;
        else
            numPixelHitsModule += numPixelHitsROC;
    }

    return numPixelHitsModule;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::transformROCaddress2ModuleAddress(int rocId, int columnROC, int rowROC, int &columnModule, int &rowModule) const
/*
  transform coordinates columnROC and rowROC on a single ROC
  to row and column coordinates in the module frame

 row 1  +-------+-------+-   -+--------+--------+
        |       |       |     |        |        |            Kapton
        | ROC 7 | ROC 6 | ... | ROC 1  | ROC 0  |            Cable
        |       |       |     |        |        |###############
        +-------+-------+-   -+--------+--------+###############
        |       |       |     |        |        |###############
        | ROC 8 | ROC 9 | ... | ROC 14 | ROC 15 |
        |       |       |     |        |        |
row 160 +-------+-------+-   -+--------+--------+
     column 1                               column 416

     Return value is 0 if the conversion to module coordinates has been succesfull

     Error code: -1 ROC Id outside range 0-15
*/
{
    //--- formula from Wolfram assumes rows/columns start from index 0
    int columnROC0 = columnROC - 1;
    int rowROC0 = rowROC - 1;

    if (rocId >= 0 && rocId < 8) {
        rowModule = 159 - rowROC0;
        columnModule = rocId * 52 + columnROC0;
    } else if (rocId >= 8 && rocId < 16) {
        rowModule = rowROC0;
        columnModule = (16 - rocId) * 52 - columnROC0 - 1;
    } else {
        cerr << "Error in <RawPacketDecoder::transformROCaddress2ModuleAddress>: ROC Id outside range of module !" << endl;
        return -1;
    }

    //--- convert back to Danek's convention
    rowModule += 1;
    columnModule += 1;

    return 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::decodeROCaddressLevel(int rocId, ADCword adcValue) const
/*
  Decode row or column address from adcValue for specified ROC

  Return value is address level

  Error code: -1 adcValue out of address level range
*/
{
    const ADCword * addressLevel = fCalibration->GetCalibrationROC(rocId).GetAddressLevel();
    if (adcValue < addressLevel[6]) {
        if (adcValue > addressLevel[5]) return 5;
        if (adcValue > addressLevel[4]) return 4;
        if (adcValue > addressLevel[3]) return 3;
        if (adcValue > addressLevel[2]) return 2;
        if (adcValue > addressLevel[1]) return 1;
        if (adcValue > addressLevel[0]) return 0;
    }

    if (fPrintError) cerr << "Error in <RawPacketDecoder::decodeROCaddressLevel>: ADC value = " << adcValue << " outside address level range !" << endl;
    return -1;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::decodeTBMstatusLevel(ADCword adcValue) const
/*
  Decode event counter and error status information
  contained in TBM header and trailer from adcValue

  Return value is bit value

  Error codes: -1 adcValue out of address level range
*/
{
    const ADCword * statusLevel = fCalibration->GetCalibrationTBM().GetStatusLevel();
    if (adcValue < statusLevel[4]) {
        if (adcValue > statusLevel[3]) return 3;
        if (adcValue > statusLevel[2]) return 2;
        if (adcValue > statusLevel[1]) return 1;
        if (adcValue > statusLevel[0]) return 0;
    }

    if (fPrintError) cerr << "Error in <RawPacketDecoder::decodeTBMstatusLevel>: ADC value = " << adcValue << " outside range !" << endl;
    return -1;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::findTBMheader(int indexStart, int dataLength, ADCword dataBuffer[]) const
/*
  Find the location of the TBM header (UltraBlack, UltraBlack, Black, Black)
  starting after position index

  Return value is the start position of the TBM header found

  Error codes: -1 TBM header not found
               -2 header starting sequence found, but header is not a full header
                  (first UltraBlack exist, but subsequent UltraBlack, UltraBlack, Black are missing)
*/
{
    //--- check that UltraBlack, Black and Status Levels are set for TBM and
    //               UltraBlack, Black and Address Levels are set for ROCs
    if (fCalibration == 0) {
        cerr << "Error in <RawPacketDecoder::findTBMheader>: no Calibration object set !" << endl;
        return -3;
    }

    int index = indexStart;
    while ((index + 8) <= dataLength) {
        if (isUltraBlackTBM(dataBuffer[index])) {
            if (isUltraBlackTBM(dataBuffer[index + 1]) &&
                    isUltraBlackTBM(dataBuffer[index + 2]) &&
                    isBlackTBM(dataBuffer[index + 3])) {
                if (fPrintDebug) cout << " TBM header found at position " << index << endl;
                return index;
            } else {
                if (fPrintError) {
                    cerr << " Error in <RawPacketDecoder::findTBMheader>: found UltraBlack at position " << index << ", but no subsequent UltraBlack, UltraBlack, Black sequence !" << endl;
                    cerr << "  packet = " << dataBuffer[index] << " " << dataBuffer[index + 1] << " " << dataBuffer[index + 2] << " " << dataBuffer[index + 3] << endl;
                }
                return -2;
            }
        }

        index++;
    }

    if (index >= dataLength) {
        if (fPrintError) cerr << " Error in <RawPacketDecoder::findTBMheader>: no TBM header found after position " << indexStart << endl;
        return -1;
    }

    if (fPrintDebug) cout << "TBM header found at position " << index << endl;

    return index;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::findTBMtrailer(int indexStart, int dataLength, ADCword dataBuffer[]) const
/*
  Find the location of the TBM header (UltraBlack, UltraBlack, Black, Black)
  starting after position index

  Return value is the start position of the TBM trailer found

  Error codes: -1 TBM header not found
               -2 no Calibration object set
*/
{
    //--- check that UltraBlack, Black and Status Levels are set for TBM and
    //               UltraBlack, Black and Address Levels are set for ROCs
    if (fCalibration == 0) {
        cerr << "Error in <RawPacketDecoder::findTBMtrailer>: no Calibration object set !" << endl;
        return -2;
    }

    int index = indexStart;
    while ((index + 8) <= dataLength) {
        if (isUltraBlackTBM(dataBuffer[index])) {
            if (isUltraBlackTBM(dataBuffer[index + 1]) &&
                    isBlackTBM(dataBuffer[index + 2]) &&
                    isBlackTBM(dataBuffer[index + 3])) {
                if (fPrintDebug) cout << " TBM trailer found at position " << index << endl;
                return index;
            }
        }

        index++;
    }

    if (index >= dataLength) {
        if (fPrintError) cerr << " Error in <RawPacketDecoder::findTBMtrailer>: no TBM trailer found after position " << indexStart << endl;
        return -1;
    }

    if (fPrintDebug) cout << "TBM trailer found at position " << index << endl;

    return index;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::findROCheader(int rocId, int indexStart, int dataLength, ADCword dataBuffer[]) const
/*
  Find the location of the next ROC header (UltraBlack, Black)
  starting after position index

  Return value is the start position of the first ROC header found

  Error codes: -1 ROC header not found
               -2 header starting sequence found, but header is not a full header
                  (UltraBlack exist, but subsequent Black is missing)
           -3 address levels not defined for ROC
           -4 no Calibration object set
*/
{
    //--- check that UltraBlack, Black and Status Levels are set for TBM and
    //               UltraBlack, Black and Address Levels are set for ROCs
    if (fCalibration == 0) {
        cerr << "Error in <RawPacketDecoder::findROCheader>: no Calibration object set !" << endl;
        return -4;
    }

    //--- do not try to find anymore ROC headers if the address levels (in particular the UltraBlack level) are not defined
    if (rocId >= fCalibration->GetNumROCs()) {
        //if ( fPrintWarning ) cerr << "Warning in <RawPacketDecoder::findROCheader>: address levels not defined for ROC " << rocId << " !" << endl;
        return -3;
    }

    int index = indexStart;
    while ((index + 3) <= dataLength) {
        if (isUltraBlackROC(rocId, dataBuffer[index])) {
            if (isBlackROC(rocId, dataBuffer[index + 1])) {
                if (fPrintDebug) cout << " ROC header found at position " << index << ", last DAC = " << dataBuffer[index + 2] << endl;
                return index;
            } else {
                if (fPrintError) {
                    cerr << "Error in <RawPacketDecoder::findROCheader>: found UltraBlack at position " << index << ", but no subsequent Black !" << endl;
                    cerr << " packet = " << dataBuffer[index] << " " << dataBuffer[index + 1] << " " << dataBuffer[index + 2] << endl;
                }
                return -2;
            }
        }

        index += fNumClocksPixelHit;
    }

    if (index >= dataLength) {
        if (fPrintError) cerr << " Error in <RawPacketDecoder::findROCheader>: no ROC header found after position " << indexStart << endl;
        return -1;
    }

    return index;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::decodeTBMheader(int indexStart, int dataLength, ADCword dataBuffer[], DecodedReadoutModule &module)
/*
  Decode the TBM header information

  Return value is decoded event counter

  Error codes: -1 level outside range
*/
{
    int tbmHeaderValue = 0;
    for (int index = indexStart + 4; index < (indexStart + 8); index++) {
        int bitValue = decodeTBMstatusLevel(dataBuffer[index]);

        if (bitValue < 0 || bitValue > (NUM_LEVELSTBM - 1)) {
            if (fPrintError) cerr << "Error in <decodeTBMheader>: bit level = " << dataBuffer[index] << " outside range at position " << index << " !" << endl;
            return -1;
        }

        tbmHeaderValue *= 4;
        tbmHeaderValue += bitValue;
    }

    module.tbm.tbmEventCounter = tbmHeaderValue;

    return tbmHeaderValue;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::decodeROCsequence(int rocId, int indexStart, int indexStop, ADCword dataBuffer[], DecodedReadoutModule &module, int numROCs)
/*
  Decode the pixel hit information for a ROC

  Return value is the number of decoded hits

  Error codes: -1 ROC header too short
               -2 length of ROC hit information not equal to n*6
               -3 invalid ROC address
*/
{
    int dataLength   = (indexStop + 1 - indexStart);
    int numPixelHits = 0;

    if (dataLength < fNumClocksROCheader) {
        cerr << "Error in <RawPacketDecoder::decodeROCsequence>: ROC header too short !" << endl;
        return -1;
    }

    //--- store last DAC value
    module.roc[rocId].lastDac = dataBuffer[indexStart + 2];

    int numHits = (dataLength - fNumClocksROCheader) / fNumClocksPixelHit;
    if (fPrintDebug) cout << " number of pixel hits = " << numHits << endl;

    bool corruptBuffer = (((dataLength - fNumClocksROCheader) % fNumClocksPixelHit) != 0) ? true : false;
    if (corruptBuffer) {
        cerr << " Error in <RawPacketDecoder::decodeROCsequence>: dataBuffer length = " << dataLength << ", expect n*6 + " << fNumClocksROCheader << " !" << endl;
        return -2;
    }

    int index = indexStart + fNumClocksROCheader;
    for (int ihit = 0; ihit < numHits; ihit++) {
        ADCword rawADC[fNumClocksPixelHit];
        rawADC[0] = dataBuffer[index + ihit * fNumClocksPixelHit];      // high valued part of column address
        rawADC[1] = dataBuffer[index + ihit * fNumClocksPixelHit + 1];  // low -""-
        rawADC[2] = dataBuffer[index + ihit * fNumClocksPixelHit + 2];  // high valued part of row address
        rawADC[3] = dataBuffer[index + ihit * fNumClocksPixelHit + 3];  // medium -""-
        rawADC[4] = dataBuffer[index + ihit * fNumClocksPixelHit + 4];  // low -""-
        rawADC[5] = dataBuffer[index + ihit * fNumClocksPixelHit + 5];  // analog pulse height

        if (fPrintDebug) cout << "raw hit information (" << ihit << ") = "
                                  << rawADC[0] << " " << rawADC[1] << " " << rawADC[2] << " "
                                  << rawADC[3] << " " << rawADC[4] << " " << rawADC[5] << endl;

        //--- decode row and column addresses
        int columnROC, rowROC, rawColumn, rawPixel;
        int errorFlag = decodeROCaddress(rocId, rawADC, columnROC, rowROC, rawColumn, rawPixel);
        if (errorFlag < 0) return -3;

        //rawADC[5] -= fCalibration->GetPedestalADC();

        if (numPixelHits < MAX_PIXELSROC) {
            module.roc[rocId].pixelHit[numPixelHits].rocId = rocId;

            module.roc[rocId].pixelHit[numPixelHits].columnROC = columnROC - 1;
            module.roc[rocId].pixelHit[numPixelHits].rowROC = rowROC - 1;

            if (numROCs == 1) {
                //--- use ROC coordinates
                module.roc[rocId].pixelHit[numPixelHits].columnModule = columnROC - 1;
                module.roc[rocId].pixelHit[numPixelHits].rowModule = rowROC - 1;
            } else {
                //--- use module coordinates
                //    (WARNING: this section has to be extended for the Forward Pixel detector !!!)
                int columnModule, rowModule;
                transformROCaddress2ModuleAddress(rocId, columnROC, rowROC, columnModule, rowModule);

                if (fPrintDebug) cout << "row in module coordinates = " << rowModule << ", column in module coordinates = " << columnModule << endl;

                module.roc[rocId].pixelHit[numPixelHits].columnModule = columnModule - 1;
                module.roc[rocId].pixelHit[numPixelHits].rowModule = rowModule - 1;
            }

            module.roc[rocId].pixelHit[numPixelHits].analogPulseHeight = rawADC[5];

            for (int ivalue = 0; ivalue < fNumClocksPixelHit; ivalue++) {
                module.roc[rocId].pixelHit[numPixelHits].rawADC[ivalue] = rawADC[ivalue];
            }

            numPixelHits++;
        } else {
            if (fPrintError) cerr << "Error in <RawPacketDecoder::decodeROCsequence>: pixel buffer too small !" << endl;
        }
    }

    module.roc[rocId].numPixelHits = numPixelHits;

    return numPixelHits;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::decodeROCaddress(int rocId, ADCword rawADC[], int &columnROC, int &rowROC, int &rawColumn, int &rawPixel) const
/*
  Decode the ROC row and column address

  Return value is 0 if the decoding has been succesfull

  Error codes: -1 wrong pixel address
               -2 wrong double column address
               -3 wrong row address
           -4 wrong column address
*/
{
    //--- decode raw column address
    //    (grey coded double column address 0-25)
    int columnLevel1 = decodeROCaddressLevel(rocId, rawADC[0]);
    int columnLevel2 = decodeROCaddressLevel(rocId, rawADC[1]);

    rawColumn = NUM_LEVELSROC * columnLevel1 + columnLevel2;
    if (rawColumn < 0 || rawColumn > 25) {
        if (fPrintError) cerr << "Error in <RawPacketDecoder::decodeROCaddress>: double column address outside range, address levels = { "
                                  << columnLevel1 << " " << columnLevel2 << " } !" << endl;
        rawColumn = -1;
        rawPixel = 0;

        return -2;
    }

    //--- decode raw row address
    //    (grey coded "Weber zig-zag pattern" pixel address 2-161)
    int rowLevel1 = decodeROCaddressLevel(rocId, rawADC[2]);
    int rowLevel2 = decodeROCaddressLevel(rocId, rawADC[3]);
    int rowLevel3 = decodeROCaddressLevel(rocId, rawADC[4]);

    rawPixel = NUM_LEVELSROC * NUM_LEVELSROC * rowLevel1 + NUM_LEVELSROC * rowLevel2 + rowLevel3;
    if (rawPixel < 2 || rawPixel > 161) {
        if (fPrintWarning) cerr << "Warning in <RawPacketDecoder::decodeROCaddress>: pixel address outside range, address levels = { "
                                    << rowLevel1 << " " << rowLevel2 << " " << rowLevel3 << " } !" << endl;
        rawColumn = 0;
        rawPixel = -1;

        return -1;
    }

    if (fPrintDebug) {
        cout << " columnLevel1 = " << columnLevel1 << endl;
        cout << " columnLevel2 = " << columnLevel2 << endl;
        cout << " rowLevel1 = " << rowLevel1 << endl;
        cout << " rowLevel2 = " << rowLevel2 << endl;
        cout << " rowLevel3 = " << rowLevel3 << endl;
    }

    //--- decode grey coded row and column addresses
    //    (tricky code from Danek)
    int columnEvenOdd = rawPixel % 2;  // 0 = first column, 1 = second column
    columnROC = rawColumn * 2 + columnEvenOdd + 1; // column address (starting from index 1)
    if (columnROC < 1 || columnROC > ROCNUMCOLS) {
        if (fPrintWarning) cerr << "Warning in <RawPacketDecoder::decodeROCaddress>: column address outside range, address levels = { "
                                    << columnLevel1 << " " << columnLevel2 << " } !" << endl;
        columnROC = -1;
        rowROC = 0;

        return -4;
    }

    rowROC = abs((rawPixel / 2) - 80) + 1; // row address (starting from index 1)
    if (columnROC < 1 || columnROC > ROCNUMCOLS) {
        if (fPrintWarning) cerr << "Warning in <RawPacketDecoder::decodeROCaddress>: row address outside range, address levels = { "
                                    << rowLevel1 << " " << rowLevel2 << " " << rowLevel3 << " } !" << endl;
        columnROC = 0;
        rowROC = -1;

        return -3;
    }

    if (fPrintDebug) cout << "column = " << columnROC << ", row = " << rowROC << endl;

    //--- return success
    return 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int RawPacketDecoder::decodeTBMtrailer(int indexStart, int dataLength, ADCword dataBuffer[], DecodedReadoutModule &module)
/*
  Decode the TBM trailer information

  Return value is the bit-coded TBM status information

  Bit values:
     0 - status OK, no message
     1 - stack full
     2 - pre-cal issued
     4 - event counter cleared
     8 - sync trigger
    16 - sync trigger error
    32 - ROC reset
    64 - TBM reset
   128 - No token pass

  Error codes: -1 level outside range
*/
{
    int tbmTrailerValue = 0;
    for (int index = indexStart + 4; index < (indexStart + 8); index++) {
        int bitValue = decodeTBMstatusLevel(dataBuffer[index]);

        if (bitValue < 0 || bitValue > (NUM_LEVELSTBM - 1)) {
            if (fPrintError) cerr << "Error in <decodeTBMtrailer>: bit level = " << dataBuffer[index] << " outside range at position " << index << " !" << endl;
            return -1;
        }

        module.tbm.tbmErrorStatus[2 * index]     = bitValue % 2;
        module.tbm.tbmErrorStatus[2 * index + 1] = bitValue / 2;

        tbmTrailerValue *= 4;
        tbmTrailerValue += bitValue;
    }

    return tbmTrailerValue;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
bool RawPacketDecoder::isBlackTBM(ADCword adcValue) const
{
    if (fCalibration == 0) {
        cerr << "Error in <RawPacketDecoder::isBlackTBM>: no Calibration object set !" << endl;
        return false;
    }

    return (adcValue >= fCalibration->GetCalibrationTBM().GetUltraBlackLevel() && adcValue < fCalibration->GetCalibrationTBM().GetBlackLevel());
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
bool RawPacketDecoder::isUltraBlackTBM(ADCword adcValue) const
{
    if (fCalibration == 0) {
        cerr << "Error in <RawPacketDecoder::isUltraBlackTBM>: no Calibration object set !" << endl;
        return false;
    }

    return (adcValue < fCalibration->GetCalibrationTBM().GetUltraBlackLevel());
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
bool RawPacketDecoder::isBlackROC(int rocId, ADCword adcValue) const
{
    if (fCalibration == 0) {
        cerr << "Error in <RawPacketDecoder::isBlackROC>: no Calibration object set !" << endl;
        return false;
    }

    return (adcValue >= fCalibration->GetCalibrationROC(rocId).GetUltraBlackLevel() && adcValue < fCalibration->GetCalibrationROC(rocId).GetBlackLevel());
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
bool RawPacketDecoder::isUltraBlackROC(int rocId, ADCword adcValue) const
{
    if (fCalibration == 0) {
        cerr << "Error in <RawPacketDecoder::isUltraBlackROC>: no Calibration object set !" << endl;
        return false;
    }

    return (adcValue < fCalibration->GetCalibrationROC(rocId).GetUltraBlackLevel());
}
//-------------------------------------------------------------------------------
