// Implementation of the tests at DoubleColumn level

#ifndef DOUBLECOLUMNTEST
#define DOUBLECOLUMNTEST

#include "BasePixel/DoubleColumn.h"
#include "TestPixel.h"

#include<TH1D.h>
#include<TH2D.h>

class TestDoubleColumn : public DoubleColumn {

public:
    TestDoubleColumn(Roc * roc, int dColumn, TestParameters * parameters);
    TestPixel * GetPixel(int column, int row);
    TestPixel * GetPixel(int iPixel);

    // == Tests =====================================================
    int FindGoodPixels(int count, TestPixel * pix[]);
    void TestWBCSBC();
    void TestTimeStampBuffer();
    void TestDataBuffer();
    void DoubleColumnTest();

protected:

};

#endif

