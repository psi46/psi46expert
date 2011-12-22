#ifndef __PIXEL_MAP_H__
#define __PIXEL_MAP_H__

#include "Test.h"
#include "BasePixel/TBAnalogInterface.h"
#include "TestParameters.h"
#include "TestRange.h"

class HRPixelMap : public Test {
	public:
		HRPixelMap(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);
		~HRPixelMap();
		virtual void ModuleAction();
};

#endif
