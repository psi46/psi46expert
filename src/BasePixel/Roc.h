// Class provides basic readout chip functionalities, contains the double columns of a ROC

#ifndef ROC
#define ROC

#include <string>

#include "DoubleColumn.h"
#include "TBInterface.h"
#include "DACParameters.h"
#include "GlobalConstants.h"

class TBAnalogInterface;
class Module;
class ConfigParameters;


class Roc
{

public:
    Roc(TBInterface * const aTBInterface, const int chipId, const int hubId, const int portId, const int aoutChipPosition);
    virtual ~Roc();

    void Initialize();
    void Initialize(ConfigParameters *);
    bool Execute(SysCommand &command, int warning = 1);
    TBInterface * GetTBInterface();
    TBAnalogInterface * GetTBAnalogInterface() {return (TBAnalogInterface *)tbInterface;};
    int GetChipId();
    int GetAoutChipPosition();
    void SetTrim(int iCol, int iRow, int trimBit);

    void WriteTrimConfiguration(const char * filename);
    void ReadTrimConfiguration(const char * filename);
    void GetTrimValues(int buffer[]);

    // == DAC Parameters ====================================

    void SetDAC(int reg, int value);
    void SetDAC(const char * dacName, int value);
    int GetDAC(const char * dacName);
    int GetDAC(int dacReg);
    bool ReadDACParameterFile(const char * filename);
    bool WriteDACParameterFile(const char * filename);
    DACParameters * SaveDacParameters();
    void RestoreDacParameters(DACParameters * dacParameters = 0);
    DACParameters * GetDACParameters();

    // == ROC actions =======================================

    void ClrCal();
    void SingleCal();
    void SendCal(int nTrig);
    void Mask();
    int GetRoCnt();
    void SendRoCnt();
    int RecvRoCnt();
    bool ADCData(short buffer[], unsigned short &wordsread);
    void SendADCTrigs(int nTrig);
    bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts);

    // == Pixel actions =====================================

    Pixel * GetPixel(int col, int row);
    void EnablePixel(int col, int row);
    void EnableAllPixels();
    void DisablePixel(int col, int row);
    void Cal(int col, int row);
    void Cals(int col, int row);
    void ArmPixel(int column, int row);
    void DisarmPixel(int column, int row);
    void SetTrim(int trim);
    void DoubleColumnADCData(int doubleColumn, short data[], int readoutStop[]);
    int ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int data[]);
    int ChipEfficiency(int nTriggers, double res[]);
    int MaskTest(short nTriggers, short res[]);
    int PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim);
    int AoutLevelChip(int position, int nTriggers, int res[]);
    int AoutLevelPartOfChip(int position, int nTriggers, int res[], bool pxlFlags[]);
    void DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[]);
    void AddressLevelsTest(int result[]);
    void TrimAboveNoise(short nTrigs, short thr, short mode, short result[]);

    // == Low level ROC actions, do not use them, use those in the pixel or dacParameters object ================================

    void SetChip();
    void PixTrim(int col, int row, int value);
    void PixMask(int col, int row);
    void PixCal(int col, int row, int sensorcal);
    void ColEnable(int col, int on);
    void RocSetDAC(int reg, int value);

    // == DoubleColumn actions ==============================

    DoubleColumn * GetDoubleColumn(int column);
    void EnableDoubleColumn(int col);
    void DisableDoubleColumn(int col);

    void Flush();
    void CDelay(int clocks);

    // == Chip properties ===================================
    int set_chip_type(std::string type_specifier);  /**< Sets the chip type from a predefined identifier. */
    bool has_analog_readout();          /**< Check whether the chip under test has analog readout or not */
    bool has_digital_readout();         /**< Check whether the chip under test has digital readout or not */
    bool has_row_address_inverted();    /**< Check whether this chip has the row address inverted or not */
    int get_threshold_autoset_value();  /**< Get the value used for auto set the threshold, e.g. in the pretest */
protected:

    TBInterface * tbInterface;

    const int chipId, hubId, portId, aoutChipPosition;
    DoubleColumn * doubleColumn[ROCNUMDCOLS];
    DACParameters * dacParameters, *savedDacParameters;

    //  static const int RES = 0x08, CAL = 0x04, TRG = 0x02, TOK = 0x01;

    /* Chip properties */
    bool analog_readout;            /**< Flag that specifies whether the chip has analog readout or not */
    bool row_address_inverted;      /**< Flag that specifies whether the row address is inverted in the readout or not */
    int threshold_autoset_value;    /**< Value used for auto set the threshold, e.g. in the pretest */

};


#endif

