// Basic test functionalities

#ifndef TEST
#define TEST

#include "TestRange.h"
#include "BasePixel/TBInterface.h"
#include "interface/Delay.h"
#include "TestParameters.h"
#include "BasePixel/DACParameters.h"
#include <TList.h>
#include <TH2D.h>
#include <TH1D.h>

class TestControlNetwork;
class TestModule;
class TestRoc;
class TestDoubleColumn;
class TestPixel;

class Test {

public:
    Test();
    virtual ~Test() {}

    TList * GetHistos();
    TH2D * GetMap(const char * mapName);
    TH1D * GetHisto(const char * histoName);
    virtual void ReadTestParameters(TestParameters * testParameters);
    virtual void ControlNetworkAction(TestControlNetwork * controlNetwork);
    virtual void ModuleAction(TestModule * testModule);
    virtual void RocAction(TestRoc * testRoc);
    virtual void DoubleColumnAction(TestDoubleColumn * testDoubleColumn);
    virtual void PixelAction(TestPixel * textPixel);
    virtual void ModuleAction();
    virtual void RocAction();
    virtual void DoubleColumnAction();
    virtual void PixelAction();

    //  == tbInterface actions =====================================================

    void Flush();
    int GetRoCnt();
    void SendRoCnt();
    int RecvRoCnt();
    void SendCal(int nTrig);


    // == roc actions ==============================================================

    void SetModule(TestModule * module);
    void SetRoc(TestRoc * roc);
    void SetPixel(TestPixel * pixel);
    void SetDAC(const char * dacName, int value);
    void SetDAC(int dacReg, int value);
    int GetDAC(const char * dacName);
    int GetDAC(int dacReg);
    TestPixel * GetPixel(int col, int row);
    void EnableDoubleColumn(int column);
    void DisableDoubleColumn(int column);
    void ClrCal();
    void SaveDacParameters();
    void RestoreDacParameters();
    void Mask();
    void EnableAllPixels();
    void SendADCTrigs(int nTrig);
    bool GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts);
    bool ADCData(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig);
    int AoutLevel(int position, int nTriggers);
    int SCurve(int nTrig, int dacReg, int threshold, int res[]);
    // == pixel actions ============================================================

    void EnablePixel();
    void Cals();
    void DisablePixel();
    void ArmPixel();
    void DisarmPixel();
    void Cal();

    // == test range ===============================================================

    bool IncludesPixel();
    bool IncludesDoubleColumn();

protected:

    ConfigParameters * configParameters;
    TestRange   *   testRange;
    TBInterface  *  tbInterface;
    TList     *     histograms;
    TestParameters * testParameters;

    TestControlNetwork * controlNetwork;
    TestModule * module;
    TestRoc * roc;
    TestDoubleColumn * doubleColumn;
    TestPixel * pixel;
    int chipId, column, row, dColumn, aoutChipPosition;
    DACParameters * savedDacParameters;

    bool debug;
};


#endif

