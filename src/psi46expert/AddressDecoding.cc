#include "interface/Log.h"

#include "AddressDecoding.h"
#include "TestRoc.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/DigitalReadoutDecoder.h"

bool AddressDecoding::fPrintDebug = false;

using namespace RawPacketDecoderConstants;
using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;

AddressDecoding::AddressDecoding(TestRange * aTestRange, TestParameters * aTestParameters, TBInterface * aTBInterface, bool debug)
{
    psi::LogDebug() << "[AddressDecoding] Initialization." << psi::endl;

    testRange = aTestRange;
    tbInterface = aTBInterface;
    testParameters = aTestParameters;
    ReadTestParameters(testParameters);
    gDecoder = RawPacketDecoder::Singleton();
    fdebug = debug;
}


void AddressDecoding::RocAction()
{
    map = GetMap("AddressDecoding");
    map->SetMaximum(1);
    map->SetMinimum(0);
    Test::RocAction();
    histograms->Add(map);
}


void AddressDecoding::DoubleColumnAction()
{
    if (!fdebug)
    {
        if (IncludesDoubleColumn())
        {
            roc->DoubleColumnADCData(doubleColumn->DoubleColumnNumber(), data, readoutStop);

            for (int k = 0; k < 2 * ROCNUMROWS; k++)
            {
                SetPixel(doubleColumn->GetPixel(k));
                if (IncludesPixel())
                {
                    AnalyseResult(k);
                }
            }
        }
    } else if (true) {
        int twait = 1000;
        short data[FIFOSIZE * 2];
        unsigned short nword;
        doubleColumn->EnableDoubleColumn(); usleep(twait);
        Flush(); usleep(twait);

        for (int i = 0; i < ROCNUMROWS * 2; i++)
        {

            SetPixel(doubleColumn->GetPixel(i));
            if (testRange->IncludesPixel(chipId, column, row)) {
                ArmPixel();
                ((TBAnalogInterface *)tbInterface)->ADCData(data, nword);
                DisarmPixel();
                Flush();
                if (nword < 25) {
                    cout << "pixel " << column << "," << row << "  \033[31;49mNOT found\033[0m " << nword << endl;
                } else {
                    map->Fill(column, row);
                }
            }
        }
        doubleColumn->DisableDoubleColumn(); usleep(twait);
        Flush();
    }
}

void AddressDecoding::AnalyseResult(int pixel)
{
    int readoutStart = 0, nDecodedPixels;
    DecodedReadoutModule decodedModuleReadout;
    if (pixel > 0) readoutStart = readoutStop[pixel - 1];
    int error;

    ConfigParameters * configParameters = ConfigParameters::Singleton();
    int nRocs = configParameters->nRocs;

    if (readoutStop[pixel] - readoutStart == ((TBAnalogInterface *)tbInterface)->GetEmptyReadoutLengthADC() + 6)
    {
        if (roc->has_analog_readout()) {
            error = nDecodedPixels = gDecoder->decode(readoutStop[pixel] - readoutStart,
                                     &data[readoutStart],
                                     decodedModuleReadout,
                                     nRocs);
        } else {
            int flags = 0;
            flags |= roc->has_row_address_inverted() ? DRO_INVERT_ROW_ADDRESS : 0;
            error = decode_digital_readout(&decodedModuleReadout, data + readoutStart, readoutStop[pixel] - readoutStart, nRocs, flags);
            if (error < 0)
                nDecodedPixels = error;
            else
                nDecodedPixels = decodedModuleReadout.roc[roc->GetAoutChipPosition()].numPixelHits;
        }
    }
    else
    {
        if (readoutStop[pixel] - readoutStart != ((TBAnalogInterface *)tbInterface)->GetEmptyReadoutLengthADC())
            psi::LogInfo() << "[AddressDecoding] Warning: Invalid readout length (" << readoutStop[pixel] - readoutStart << ")" << psi::endl;
        else
            psi::LogInfo() << "[AddressDecoding] Warning: Empty readout for pixel " << column << ":" << row << " on ROC " << roc->GetChipId() << "." << psi::endl;
        if (fPrintDebug)
        {
            cout << "ADC values = { ";
            for (int ivalue = readoutStart; ivalue < readoutStop[pixel]; ivalue++)
            {
                cout << data[ivalue] << " ";
            }
            cout << "}" << endl;
        }

        nDecodedPixels = -1;
    }

    if (nDecodedPixels < 0)
    {
        psi::LogInfo() << "[AddressDecoding] Warning: Decoding Problem for Pixel( "
                       << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << psi::endl;

        if ((readoutStop[pixel] - readoutStart) ==
                dynamic_cast<TBAnalogInterface *>(tbInterface)->GetEmptyReadoutLengthADC())
        {
            psi::LogDebug() << "[AddressDecoding] Pixel seems to be dead."
                            << psi::endl;
        }

        else if ((readoutStop[pixel] - readoutStart) !=
                 (dynamic_cast<TBAnalogInterface *>(tbInterface)->GetEmptyReadoutLengthADC() + 6))
        {
            psi::LogDebug() << "[AddressDecoding] Pixel has a wrong length ("
                            << readoutStop[pixel] - readoutStart
                            << ") of read-out signal. Expected length is "
                            << (dynamic_cast<TBAnalogInterface *>(tbInterface)->GetEmptyReadoutLengthADC() + 6)
                            << '.' << psi::endl;
        }
    }
    else if (nDecodedPixels == 0 || decodedModuleReadout.roc[roc->GetAoutChipPosition()].numPixelHits == 0)
    {
        psi::LogInfo() << "[AddressDecoding] Warning: No address levels were found "
                       << "for Pixel( " << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << psi::endl;
    }
    else if (nDecodedPixels > 1)
    {
        psi::LogInfo() << "[AddressDecoding] Warning: Too many address levels were "
                       << "found for Pixel( " << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << psi::endl;
    }
    else if (row != decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].rowROC)
    {
        psi::LogInfo() << "[AddressDecoding] Warning: wrong row "
                       << decodedModuleReadout.roc[roc->GetChipId()].pixelHit[0].rowROC
                       << " for Pixel( " << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << psi::endl;
    }
    else if (column != decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].columnROC)
    {
        psi::LogInfo() << "[AddressDecoding] Warning: wrong column "
                       << decodedModuleReadout.roc[roc->GetAoutChipPosition()].pixelHit[0].columnROC
                       << " for Pixel( " << column << ", " << row
                       << ") on ROC" << roc->GetChipId() << '.' << psi::endl;
    }
    else map->Fill(column, row);
}
