#include "BasePixel/TBAnalogParameters.h"
#include "BasePixel/TBAnalogInterface.h"
#include <string.h>
#include <stdio.h>


TBAnalogParameters::TBAnalogParameters(TBInterface *aTBInterface) {

	tbInterface = aTBInterface;

	for (int i = 0; i < NTBParameters; i++) {
		parameters[i] = -1;
		names[i] = "";
	}

	names[8]="clk";
	names[9]="sda";
	names[10]="ctr";
	names[11]="tin";
	names[12]="rda";

	names[17]="trc";
	names[18]="tcc";
	names[19]="tct";
	names[20]="ttk";
	names[21]="trep";
	names[22]="cc";
	
	names[77]="spd";  // dummy register for clock frequency

}


// -- sets a parameter
void TBAnalogParameters::SetParameter(int reg, int value)
{
	parameters[reg] = value;
	TBAnalogInterface* analogInterface = (TBAnalogInterface *)tbInterface;
	if (reg == 77) analogInterface->SetClock(value);
	else if (reg > 15) 
	{
		tbInterface->Set(reg,value);
	}
	else 
	{
		((TBAnalogInterface *)tbInterface)->SetDelay(reg, value);
	}
// 	gLog->printf("set tb parameter %i to %i\n", reg, value);
}



TBAnalogParameters* TBAnalogParameters::Copy()
{
	TBAnalogParameters* newParameters;
	newParameters = new TBAnalogParameters(tbInterface);
	for (int i = 0; i < NTBParameters; i++)
	{
		newParameters->_SetParameter(i,parameters[i]);
	}
	return newParameters;
}

