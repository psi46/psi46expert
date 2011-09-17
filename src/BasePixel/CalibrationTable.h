//
// Author: Peter Trüb
// Vcal calibration data provided and measured by Wolfram Erdmann
//
// Class provides calibration data for DACs
// 


#ifndef CALIBRATIONTABLE
#define CALIBRATIONTABLE


class CalibrationTable
{

public:

// == Vcal Calibration ===========================================================

	int CorrectedVcalDAC(int value);
	double VcalDAC(int range, int value);
	double VcalDAC(int range, double value);
	int VcalDACInv(int range, double value);
	void VcalDACOrder(int range);
	double Slope(int range);

// == Vthr Calibration ===========================================================

	void SetVcalVthr(int vthr, int vcal, int range);
	double VthrDAC(int range, int value);
	int VthrDACInv(int range, double value);

protected:

	int vcalVthrTable[1][256];  /*Vcal as a function of Vthr*/

};

static CalibrationTable *gCalibrationTable = new CalibrationTable();

#endif

