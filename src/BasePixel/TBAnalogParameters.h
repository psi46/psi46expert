// The class represents the settings of an analog testboard

#ifndef TBANALOGPARAMETERS
#define TBANALOGPARAMETERS

#include "TBParameters.h"


class TBInterface;

class TBAnalogParameters : TBParameters {

 public:
	TBAnalogParameters(TBInterface *aTBInterface);
	void SetParameter(int reg, int value);
	TBAnalogParameters *Copy();
	
};

#endif
