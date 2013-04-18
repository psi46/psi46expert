#include <stdio.h>
#include <string.h>
#include <string>

#include "BasePixel/Roc.h"
#include "TBAnalogInterface.h"
#include "interface/Log.h"

Roc::Roc(TBInterface* const aTBInterface, const int aChipId, const int aHubId, const int aPortId, const int anAoutChipPosition) :
	tbInterface(aTBInterface), chipId(aChipId), hubId(aHubId), portId(aPortId), aoutChipPosition(anAoutChipPosition)
{
	for (int i = 0; i < ROCNUMDCOLS; i++)
	{
		doubleColumn[i] = new DoubleColumn(this, i);
	}
	dacParameters = new DACParameters(this);

	/* Chip properties */
	analog_readout = false;
	row_address_inverted = false;
}


Roc::~Roc()
{
	for (int i = 0; i < ROCNUMDCOLS; i++)
	{
		delete doubleColumn[i];
	}
	delete dacParameters;
}


void Roc::Initialize()
{
	ConfigParameters *configParameters = ConfigParameters::Singleton();
	ReadDACParameterFile(configParameters->GetDacParametersFileName());
	ReadTrimConfiguration(configParameters->GetTrimParametersFileName());

	ClrCal();
	Mask();
	tbInterface->Flush();

}

void Roc::Initialize(ConfigParameters *pcfg)
{
	ConfigParameters *configParameters = pcfg;
	ReadDACParameterFile(configParameters->GetDacParametersFileName());
	ReadTrimConfiguration(configParameters->GetTrimParametersFileName());

	ClrCal();
	Mask();
	tbInterface->Flush();

}


bool Roc::Execute(SysCommand &command, int warning)
{

	if( (command.carg[0]==NULL) || command.narg==0) return false;

	int buf[2];
	int *col=&buf[0];
	int *row=&buf[1];

	if (strcmp(command.carg[0],"cole") == 0)
	{
		for(int* j=command.iarg[1]; (*j)>=0; j++)
		{
			EnableDoubleColumn(*j);
		}
		return true;
	}
	else if (strcmp(command.carg[0],"cold") == 0)
	{
		for(int* j=command.iarg[1]; (*j)>=0; j++)
		{
			DisableDoubleColumn(*j);
		}
		return true;
	}
	else if (strcmp(command.carg[0],"pixe") == 0)
	{
		for(int* j=command.iarg[1]; (*j)>=0; j++)
		{
			for(int* k=command.iarg[2]; (*k)>=0; k++)
			{
				psi::LogDebug() << "[Roc] pixel " << *j << ' ' << *k << psi::endl;
				EnablePixel(*j, *k);
			}
		}
		return true;
	}
	else if (strcmp(command.carg[0],"trim") == 0)
	{
		for(int* j=command.iarg[1]; (*j)>=0; j++)
		{
			for(int* k=command.iarg[2]; (*k)>=0; k++)
			{
				GetPixel(*j, *k)->SetTrim(*command.iarg[3]);
			}
		}
		return true;
	}
	else if (strcmp(command.carg[0],"pixd") == 0)
	{
		for(int* j=command.iarg[1]; (*j)>=0; j++)
		{
			for(int* k=command.iarg[2]; (*k)>=0; k++)
			{
				DisablePixel(*j, *k);
			}
		}
		return true;
	}
	else if (strcmp(command.carg[0],"cal") == 0)
	{
		for(int* j=command.iarg[1]; (*j)>=0; j++)
		{
			for(int* k=command.iarg[2]; (*k)>=0; k++)
			{
				Cal(*j, *k);
			}
		}
		return true;
	}
	else if (strcmp(command.carg[0],"cals") == 0)
	{
		for(int* j=command.iarg[1]; (*j)>=0; j++)
		{
			for(int* k=command.iarg[2]; (*k)>=0; k++)
			{
				Cals(*j, *k);
			}
		}
		return true;
	}
	else if (strcmp(command.carg[0],"mask") == 0)
	{
		Mask();return true;
	}
	else if (strcmp(command.carg[0],"cald") == 0)
	{
		ClrCal(); return true;
	}
	else if (strcmp(command.carg[0],"init") == 0)
	{
		/* Initialize("defaultDACParameters.dat");
		tbInterface->Initialize("defaultTBParameters.dat");*/
		return true;
	}
	else if (strcmp(command.carg[0],"range") == 0)
	{
		SetDAC("CtrlReg",*command.iarg[1]);
		return true;
	}
	else if (command.Keyword("arm",&col,&row))
	{
		for(int* j=col; (*j)>=0; j++)
		{
			for(int* k=row; (*k)>=0; k++)
			{
				ArmPixel(*j, *k);
			}
		}
		return true;
	}
	else if (strcmp(command.carg[0],"writeDAC") == 0)
	{
		WriteDACParameterFile(command.carg[1]);
		return true;
	}
	else if (strcmp(command.carg[0],"readDAC") == 0)
	{
		ReadDACParameterFile(command.carg[1]);
		return true;
	}
	else
	{
		if (!dacParameters->Execute(command))
		{
			if (warning) {cerr << "unknown command " << command.carg[0] << endl;}
			return false;
		}
		return true;
	}
}

TBInterface* Roc::GetTBInterface()
{
	return tbInterface;
}


int Roc::GetChipId()
{
	return chipId;
}


int Roc::GetAoutChipPosition()
{
	return aoutChipPosition;
}


void Roc::SetTrim(int iCol, int iRow, int trimBit)
{
	GetPixel(iCol, iRow)->SetTrim(trimBit);
}


void Roc::GetTrimValues(int buffer[])
{
	for (int i = 0; i < ROCNUMCOLS; i++)
	{
		for (int k = 0; k < ROCNUMROWS; k++)
		{
			buffer[i*ROCNUMROWS + k] = GetPixel(i,k)->GetTrim();
		}
	}
}

// == Parameters =============================================================

DACParameters* Roc::GetDACParameters()
{
	return dacParameters;
}


void Roc::SetDAC(int reg, int value)
{
	dacParameters->SetParameter(reg,value);
}


void Roc::SetDAC(const char* dacName, int value)
{
	dacParameters->SetParameter(dacName,value);
}


int Roc::GetDAC(const char* dacName)
{
	return dacParameters->GetDAC(dacName);
}


int Roc::GetDAC(int dacReg)
{
	return dacParameters->GetDAC(dacReg);
}


// -- Saves the current DAC parameters for later use
DACParameters *Roc::SaveDacParameters()
{
	DACParameters *copy = dacParameters->Copy();
	savedDacParameters = copy;
	return copy;
}


// -- Restores the saved DAC parameters
void Roc::RestoreDacParameters(DACParameters *aDacParameters)
{
	if (aDacParameters) dacParameters = aDacParameters;
	else dacParameters = savedDacParameters;
	dacParameters->Restore();
	Flush();
}


bool Roc::ReadDACParameterFile( const char *filename)
{
	bool result;
	char fname[1000];
	if (strstr(filename, ".dat"))
	{
		result = dacParameters->ReadDACParameterFile(filename);
	}
	else
	{
		sprintf(fname, "%s_C%i.dat", filename, chipId);
		result = dacParameters->ReadDACParameterFile(fname);
	}
	Flush();
	return result;
}


bool Roc::WriteDACParameterFile(const char* filename)
{
	char fname[1000];
	if (strstr(filename, ".dat"))
	{
		return dacParameters->WriteDACParameterFile(filename);
	}
	else
	{
		sprintf(fname, "%s_C%i.dat", filename, chipId);
		return dacParameters->WriteDACParameterFile(fname);
	}
}


// == Roc actions ==================================================================

void Roc::ClrCal()
{
	SetChip();
	GetTBAnalogInterface()->RocClrCal();
	tbInterface->CDelay(50);
}


void Roc::SendCal(int nTrig)
{
	GetTBAnalogInterface()->SendCal(nTrig);
}


void Roc::SingleCal()
{
	GetTBAnalogInterface()->SingleCal();
}


// -- Sends a calibrate signal to the chip, but does not read back the result immediately
void Roc::SendRoCnt()
{
	((TBAnalogInterface*)tbInterface)->SendRoCnt();
}


// -- Reads back the result of an earlier sent calibrate signal
int Roc::RecvRoCnt()
{
	return ((TBAnalogInterface*)tbInterface)->RecvRoCnt();
}


// -- Disables all double columns and pixels
void Roc::Mask()
{
	for (int i = 0; i < ROCNUMDCOLS; i++)
	{
		doubleColumn[i]->Mask();
	}
}


int Roc::GetRoCnt()
{
	return tbInterface->GetRoCnt();
}


// == Pixel actions ===============================================================

// -- Enables a pixels and sends a calibrate signal
void Roc::ArmPixel(int column, int row)
{
	GetDoubleColumn(column)->ArmPixel(column, row);
}


void Roc::DisarmPixel(int column, int row)
{
	GetDoubleColumn(column)->DisarmPixel(column, row);
}


void Roc::SetTrim(int trim)
{
	for (int i = 0; i < ROCNUMCOLS; i++)
	{
		for (int k = 0; k < ROCNUMROWS; k++)
		{
			GetPixel(i,k)->SetTrim(trim);
		}
	}
}

Pixel* Roc::GetPixel(int col, int row)
{
	return GetDoubleColumn(col)->GetPixel(col,row);
}


void Roc::EnablePixel(int col, int row)
{
	GetDoubleColumn(col)->EnablePixel(col, row);
}


void Roc::EnableAllPixels()
{
	for (int i = 0; i < ROCNUMCOLS; i++)
	{
		for (int k = 0; k < ROCNUMROWS; k++)
		{
			EnablePixel(i,k);
		}
	}
}


void Roc::DisablePixel(int col, int row)
{
	GetDoubleColumn(col)->DisablePixel(col, row);
}


void Roc::Cal(int col, int row)
{
	GetDoubleColumn(col)->Cal(col, row);
}


void Roc::Cals(int col, int row)
{
	GetDoubleColumn(col)->Cals(col, row);
}


// -- sends n calibrate signals and gives back the resulting ADC readout
bool Roc::ADCData(short buffer[], unsigned short &wordsread)
{
	return GetTBAnalogInterface()->ADCData(buffer, wordsread);
}


// -- sends n calibrate signals and gives back the resulting ADC readout
void Roc::SendADCTrigs(int nTrig)
{
	GetTBAnalogInterface()->SendADCTrigs(nTrig);
}


bool Roc::GetADC(short buffer[], unsigned short buffersize, unsigned short &wordsread, int nTrig, int startBuffer[], int &nReadouts)
{
	return GetTBAnalogInterface()->GetADC(buffer, buffersize, wordsread, nTrig, startBuffer, nReadouts);
}


// == Private low level Roc actions ==========================================================


void Roc::SetChip()
{
	GetTBAnalogInterface()->SetChip(chipId, hubId, portId, aoutChipPosition);
}


void Roc::PixTrim(int col, int row, int value)
{
	SetChip();
	GetTBAnalogInterface()->RocPixTrim(col, row, value);
	tbInterface->CDelay(50);
}


void Roc::PixMask(int col, int row)
{
	SetChip();
	GetTBAnalogInterface()->RocPixMask(col, row);
	tbInterface->CDelay(50);
}


void Roc::PixCal(int col, int row, int sensorcal)
{
	SetChip();
	GetTBAnalogInterface()->RocPixCal(col, row, sensorcal);
	tbInterface->CDelay(50);
}


void Roc::ColEnable(int col, int on)
{
	SetChip();
	GetTBAnalogInterface()->RocColEnable(col, on);
	tbInterface->CDelay(50);
}


void Roc::RocSetDAC(int reg, int value)
{
	SetChip();
	GetTBAnalogInterface()->RocSetDAC(reg, value);
}


void Roc::DoubleColumnADCData(int doubleColumn, short data[], int readoutStop[])
{
	SetChip();
	Flush();
	GetTBAnalogInterface()->DoubleColumnADCData(doubleColumn, data, readoutStop);
}


int Roc::ChipThreshold(int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int data[])
{
	SetChip();
	Flush();
	int trim[ROCNUMROWS*ROCNUMCOLS];
	GetTrimValues(trim);
	return GetTBAnalogInterface()->ChipThreshold(start, step, thrLevel, nTrig, dacReg, xtalk, cals, trim, data);
}



int Roc::PixelThreshold(int col, int row, int start, int step, int thrLevel, int nTrig, int dacReg, int xtalk, int cals, int trim)
{
	SetChip();
	Flush();
	return GetTBAnalogInterface()->PixelThreshold(col, row, start, step, thrLevel, nTrig, dacReg, xtalk, cals, trim);
}


int Roc::MaskTest(short nTriggers, short res[])
{
	SetChip();
	Flush();
	return GetTBAnalogInterface()->MaskTest(nTriggers, res);
}


int Roc::ChipEfficiency(int nTriggers, double res[])
{
	SetChip();
	Flush();
	int trim[ROCNUMROWS*ROCNUMCOLS];
	GetTrimValues(trim);
	return GetTBAnalogInterface()->ChipEfficiency(nTriggers, trim, res);
}


int Roc::AoutLevelChip(int position, int nTriggers, int res[])
{
	SetChip();
	Flush();
	int trim[ROCNUMROWS*ROCNUMCOLS];
	GetTrimValues(trim);
	return GetTBAnalogInterface()->AoutLevelChip(position, nTriggers, trim, res);
}


int Roc::AoutLevelPartOfChip(int position, int nTriggers, int res[], bool pxlFlags[])
{
	int trim[ROCNUMROWS*ROCNUMCOLS];
	GetTrimValues(trim);
	return GetTBAnalogInterface()->AoutLevelPartOfChip(position, nTriggers, trim, res, pxlFlags);
}



void Roc::DacDac(int dac1, int dacRange1, int dac2, int dacRange2, int nTrig, int result[])
{
	SetChip();
	Flush();
	GetTBAnalogInterface()->DacDac(dac1, dacRange1, dac2, dacRange2, nTrig, result);
}


void Roc::AddressLevelsTest(int result[])
{
	SetChip();
	Flush();
	int position = aoutChipPosition*3;
	if (tbInterface->TBMIsPresent()) position+=8;
	GetTBAnalogInterface()->AddressLevels(position, result);
}

void Roc::TrimAboveNoise(short nTrigs, short thr, short mode, short result[])
{
	SetChip();
	Flush();
	GetTBAnalogInterface()->TrimAboveNoise(nTrigs, thr, mode, result);
}


// == DoubleColumn actions ===============================================


DoubleColumn* Roc::GetDoubleColumn(int column)
{
	return doubleColumn[column/2];
}


void Roc::EnableDoubleColumn(int col)
{
	GetDoubleColumn(col)->EnableDoubleColumn();
}


void Roc::DisableDoubleColumn(int col)
{
	GetDoubleColumn(col)->DisableDoubleColumn();
}


// -- sends the commands to the testboard, only meaningful for an analog testboard
void Roc::Flush()
{
	tbInterface->Flush();
}


// -- sends a delay command to the testboard, only meaningful for an analog testboard
void Roc::CDelay(int clocks)
{
	tbInterface->CDelay(clocks);
}


void Roc::WriteTrimConfiguration(const char* filename)
{
	char fname[1000];
	if (strstr(filename, ".dat")) sprintf(fname, "%s", filename);
	else sprintf(fname, "%s_C%i.dat", filename, chipId);

	FILE *file = fopen(fname, "w");
	if (!file)
	{
		psi::LogInfo() << "[Roc] Can not open file '" << fname
				<< "' to write trim configuration." << psi::endl;
		return;
	}
	psi::LogInfo() << "[Roc] Roc" << chipId
			<< ": Writing trim configuration to '" << filename
			<< "'." << psi::endl;

	for (int iCol = 0; iCol < ROCNUMCOLS; iCol++)
	{
		for (int iRow = 0; iRow < ROCNUMROWS; iRow++)
		{
			fprintf(file, "%2i   Pix %2i %2i\n", GetPixel(iCol, iRow)->GetTrim(), iCol, iRow);
		}
	}
	fclose(file);
}


void Roc::ReadTrimConfiguration(const char * filename)
{
	if (!filename)
		return;

	/* Add a filename extension if necessary */
	char * fname;
	int fname_len = strlen(filename);

	char * extension = (char *) strstr(filename, ".dat");
	if (extension && (extension - filename == fname_len - 4)) {
		fname = new char [fname_len + 1];
		strcpy(fname, filename);
	} else {
		fname = new char [fname_len + 8 + 1];
		sprintf(fname, "%s_C%i.dat", filename, chipId);
	}

	/* Open the file */
	FILE * file = fopen(fname, "r");
	if (!file) {
		psi::LogInfo() << "[Roc] Can not open file '" << fname << "' to read trim configuration." << psi::endl;
		return;
	}

	psi::LogInfo() << "[Roc] Reading Trim configuration from '" << fname << "'." << psi::endl;

	/* Set default trim values (trimming off = 15) */
	int col, row;
	for (int col = 0; col < ROCNUMCOLS; col++) {
		for (int row = 0; row < ROCNUMROWS; row++) {
			GetPixel(col, row)->SetTrim(15);
		}
	}

	/* Read the trim values from the file */
	int trim, retval;
	while ((retval = fscanf(file, "%2d Pix %2d %2d", &trim, &col, &row)) != EOF) {
		if (retval != 3) {
			/* There were less than 3 integers read */
			psi::LogInfo() << "[Roc] Error reading from file '" << fname << "': Invalid syntax." << psi::endl;
			break;
		}

		if (col < 0 || col >= ROCNUMCOLS || row < 0 || row >= ROCNUMROWS) {
			psi::LogInfo() << "[Roc] Skipping trim bits for invalid pixel " << col << ":" << row << psi::endl;
			continue;
		}

		if (trim >= 0 && trim <= 15)
			GetPixel(col, row)->SetTrim(trim);
		else
			GetPixel(col, row)->MaskCompletely();
	}

	/* Clean up */
	fclose(file);
	delete [] fname;
}

/**
	\details
	Because there are many chip types that have to be tested this
	function takes a predefined indentifier and sets flags depending
	on the features of the chip.
	\param chip_identifier A string that is one of the predefined chip types
 */
int Roc::set_chip_type(string chip_identifier)
{
	analog_readout = false;
	row_address_inverted = false;
	if (chip_identifier == "psi46v2") {
		analog_readout = true;
	} else if (chip_identifier == "psi46xdb") {
		analog_readout = true;
	} else if (chip_identifier == "psi46dig") {
		row_address_inverted = true;
	} else if (chip_identifier == "psi46digv2") {
	} else if (chip_identifier == "psi46digv2_b") {
	} else {
		psi::LogError() << "[Roc] Unknown chip type " << chip_identifier << "!" << psi::endl;
		return 0;
	}
	psi::LogDebug() << "[Roc] Setting ROC " << chipId << " as type " << chip_identifier << psi::endl;
	return 1;
}

/**
	\details
	The readout of the ROC was analog until psi46v2. This function
	indicates, whether this is the case with this chip.
 */
bool Roc::has_analog_readout()
{
	return analog_readout;
}

/**
	\details
	The readout of the ROC was analog until psi46v2. This function
	indicates, whether it is digital with this chip.
 */
bool Roc::has_digital_readout()
{
	return !analog_readout;
}

/**
	\details
	The psi46dig chip has the row address (bitwise) inverted in the
	readout by mistake. This function indicates, whether this is is the
	case for the chip under test.
 */
bool Roc::has_row_address_inverted()
{
	return row_address_inverted;
}
