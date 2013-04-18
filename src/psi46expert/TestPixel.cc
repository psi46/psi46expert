#include "TestPixel.h"
#include "TestRoc.h"
#include "BasePixel/CalibrationTable.h"
#include "BasePixel/TBAnalogInterface.h"
#include "interface/Delay.h"
#include "ThresholdMap.h"
#include "TestRange.h"


TestPixel::TestPixel(Roc * aRoc, int columnNumber, int rowNumber, TestParameters * parameters): Pixel(aRoc, columnNumber, rowNumber)
{
    testParameters = parameters;
}


TestRoc * TestPixel::GetRoc()
{
    return (TestRoc *)roc;
}


// == General functions ===================================================


// -- Find the threshold (50% point of the SCurve)
double TestPixel::FindThreshold(const char * mapName, int nTrig, bool doubleWbc)
{
    TestRange * range = new TestRange();
    range->AddPixel(roc->GetChipId(), column, row);

    ThresholdMap * thresholdMap = new ThresholdMap();
    if (doubleWbc) thresholdMap->SetDoubleWbc();
    TH2D * map = thresholdMap->GetMap(mapName, GetRoc(), range, nTrig);

    double result = map->GetBinContent(column + 1, row + 1);

    delete range;
    delete thresholdMap;
    delete map;

    return result;
}
