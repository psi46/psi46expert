// Implementation of the tests at pixel level

#ifndef PIXELTEST
#define PIXELTEST

#include "BasePixel/Pixel.h"
#include "TestParameters.h"
#include "SCurve.h"
#include <TH1D.h>
#include <TH2D.h>
#include <TGraph.h>

class TestRoc;

class TestPixel: public Pixel {

public:
    TestPixel(Roc * roc, int columnNumber, int rowNumber, TestParameters * parameters);
    TestRoc * GetRoc();

    // == General functions ===================================================

    double FindThreshold(const char * mapName, int nTrig, bool doubleWbc = false);


protected:

    TestParameters * testParameters;

};

#endif

