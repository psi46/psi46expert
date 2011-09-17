#include "DecoderCalibration.h"

#include <iostream>
#include <iomanip>
#include <fstream>

#include "RawPacketDecoder.h"

using namespace std;
using namespace DecoderCalibrationConstants;
using namespace RawPacketDecoderConstants;

bool DecoderCalibrationModule::fPrintDebug   = false;
//bool DecoderCalibrationModule::fPrintDebug   = true;
bool DecoderCalibrationModule::fPrintWarning = true;
bool DecoderCalibrationModule::fPrintError   = true;

//-------------------------------------------------------------------------------
DecoderCalibrationModule::DecoderCalibrationModule()
{
  fCalibrationTBM.SetUltraBlackLevel(-300);
  fCalibrationTBM.SetBlackLevel(300);

  fPedestalADC = 0;
}

DecoderCalibrationModule::DecoderCalibrationModule(ADCword ultraBlack, ADCword black,
						   ADCword levelROC_Address0, ADCword levelROC_Address1, ADCword levelROC_Address2, ADCword levelROC_Address3, 
						   ADCword levelROC_Address4, ADCword levelROC_Address5, ADCword levelROC_Address6,
						   ADCword levelTBM_Address0, ADCword levelTBM_Address1, ADCword levelTBM_Address2, 
						   ADCword levelTBM_Address3, ADCword levelTBM_Address4)
{
  SetCalibration(ultraBlack, black,
		 levelROC_Address0, levelROC_Address1, levelROC_Address2, levelROC_Address3, 
		 levelROC_Address4, levelROC_Address5, levelROC_Address6,
		 levelTBM_Address0, levelTBM_Address1, levelTBM_Address2, 
		 levelTBM_Address3, levelTBM_Address4);

  fPedestalADC = 0;
}

DecoderCalibrationModule::DecoderCalibrationModule(ADCword levelsTBM[], ADCword levelsROC[][NUM_LEVELSROC + 1], int numROCs)
{
  SetCalibration(levelsTBM, levelsROC, numROCs);

  fPedestalADC = 0;
}

DecoderCalibrationModule::DecoderCalibrationModule(const char* fileName, int fileType, int mode, int numROCs)
{
  ReadCalibrationFile(fileName, fileType, mode, numROCs);

  fPedestalADC = 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationModule::SetCalibration(ADCword ultraBlack, ADCword black,
					      ADCword levelROC_Address0, ADCword levelROC_Address1, ADCword levelROC_Address2, ADCword levelROC_Address3, 
					      ADCword levelROC_Address4, ADCword levelROC_Address5, ADCword levelROC_Address6,
					      ADCword levelTBM_Status0, ADCword levelTBM_Status1, ADCword levelTBM_Status2,
					      ADCword levelTBM_Status3, ADCword levelTBM_Status4)
  
{
  if ( fPrintWarning ) cerr << "Warning in <DecoderCalibrationModule::SetCalibration>: this function is deprecated, please use readCalibration instead !" << endl;

  fCalibrationTBM.SetUltraBlackLevel(ultraBlack);
  fCalibrationTBM.SetBlackLevel(black);
  fCalibrationTBM.SetStatusLevel(0, levelTBM_Status0);
  fCalibrationTBM.SetStatusLevel(1, levelTBM_Status1);
  fCalibrationTBM.SetStatusLevel(2, levelTBM_Status2);
  fCalibrationTBM.SetStatusLevel(3, levelTBM_Status3);
  fCalibrationTBM.SetStatusLevel(4, levelTBM_Status4);
  
  for ( int iroc = 0; iroc < MAX_ROCS; iroc++ ){
    fCalibrationROC[iroc].SetUltraBlackLevel(ultraBlack);
    fCalibrationROC[iroc].SetBlackLevel(black);
    
    fCalibrationROC[iroc].SetAddressLevel(0, levelROC_Address0);
    fCalibrationROC[iroc].SetAddressLevel(1, levelROC_Address1);
    fCalibrationROC[iroc].SetAddressLevel(2, levelROC_Address2);
    fCalibrationROC[iroc].SetAddressLevel(3, levelROC_Address3);
    fCalibrationROC[iroc].SetAddressLevel(4, levelROC_Address4);
    fCalibrationROC[iroc].SetAddressLevel(5, levelROC_Address5);
    fCalibrationROC[iroc].SetAddressLevel(6, levelROC_Address6);

    if ( fPrintDebug ){
      if ( iroc == 0 ){
	cout << "defining adress levels = { ";
	for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
	  cout << fCalibrationROC[iroc].GetAddressLevel(ilevel) << " ";
	}
	cout << "}" << endl;
      }
    }
  }

  fNumROCs = MAX_ROCS;
  
  Print(&cout);
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationModule::SetCalibration(ADCword levelsTBM[], ADCword levelsROC[][NUM_LEVELSROC + 1], int numROCs)
{
  fCalibrationTBM.SetUltraBlackLevel(levelsTBM[0]);
  fCalibrationTBM.SetBlackLevel(300);
  for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
    fCalibrationTBM.SetStatusLevel(ilevel, levelsTBM[ilevel]);
  }
  
  for ( int iroc = 0; iroc < numROCs; iroc++ ){
    fCalibrationROC[iroc].SetUltraBlackLevel(levelsROC[iroc][0]);
    fCalibrationROC[iroc].SetBlackLevel(300);
    for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
      fCalibrationROC[iroc].SetAddressLevel(ilevel, levelsROC[iroc][ilevel]);
    }
  }
  
  fNumROCs = numROCs;

  Print(&cout);
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int DecoderCalibrationModule::ReadCalibrationFile(const char* fileName, int fileType, int mode, int numROCs)
{
  switch ( fileType ){
  case 1: 
    return ReadCalibrationFile1(fileName, mode, numROCs);
  case 2: 
    return ReadCalibrationFile2(fileName, mode, numROCs);
  case 3: 
    return ReadCalibrationFile3(fileName, mode, numROCs);
  default: 
    cerr << "Error in <DecoderCalibration::ReadCalibrationFile>: file type " << fileType << " not defined !" << endl;
    return -1;
  }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int DecoderCalibrationModule::ReadCalibrationFile1(const char* fileName, int mode, int numROCs)
/*
  Read the UltraBlack, Black and address levels of ROCs 
  and the UltraBlack, Black and bit levels of the TBM
  from a calibration file

  Return value is 0 if the reading of the calibration file has been succesfull

  Error codes: 1 file not found
               2 invalid file format
*/
{
  int status = 0;
  FILE* calibrateFile = 0;
  
//--- open file
  if ( mode < 10 ){ // open file for module
    calibrateFile = fopen("levels-module.dat", "r");
    if ( fPrintDebug ) cout << "Read in calibration from file: levels-module.dat" << endl;
  } else { // open file for ROC
    calibrateFile = fopen("levels-roc.dat", "r");
    if ( fPrintDebug ) cout << "Read in calibration from file: levels-roc.cal" << endl;
  }

  if ( calibrateFile == 0 ){
    if ( fPrintError ) cerr << " Error in <DecodeRawPacket::readCalibration>: cannot open the calibration file !" << endl;
    return 1;
  }

//--- read TBM ultra black, black and bit levels
//    (4 levels --> 5 limits)
  ADCword level = 0;
  status = fscanf(calibrateFile, "%d", &level); // read UltraBlack
  if ( status == EOF ){
    if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" <<endl;
    return 2;
  }
  fCalibrationTBM.SetUltraBlackLevel(level);
  

  //status = fscanf(calibrateFile, "%d", &level); // skip reading TBM black level, keep it constant
  //if ( status == EOF ){
  //  if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" <<endl;
  //  return 2;
  //}
  //fCalibrationTBM.SetBlackLevel(level);

  for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
    status = fscanf(calibrateFile, "%d", &level);
    if ( status == EOF ){
      if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" <<endl;
      return 2;
    }
    fCalibrationTBM.SetStatusLevel(ilevel, level);
  }

  if ( fPrintDebug ){
    cout << " TBM bit levels = { " 
	 << fCalibrationTBM.GetUltraBlackLevel() << " " << fCalibrationTBM.GetBlackLevel();
    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
      cout << fCalibrationTBM.GetStatusLevel(ilevel) << " ";
    }
    cout << "}" << endl;
  }
  
//--- read ROC ultra black, black and address levels
  for ( int iroc = 0; iroc < numROCs; iroc++ ){
    status = fscanf(calibrateFile, "%d", &level); // read ultraBlack for ROC
    if ( status == EOF ){
      if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" <<endl;
      return 2;
    }
    fCalibrationROC[iroc].SetUltraBlackLevel(level);

    //status = fscanf(calibrateFile, "%d", &fLevelROC_Black[iroc]); // skip reading Black for ROC, keep it constant
    //if ( status == EOF ){
    //  if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" <<endl;
    //  return 2;
    //}
    //fCalibrationROC[iroc].SetBlackLevel(level);

    for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
      status = fscanf(calibrateFile, "%d", &level);
      if ( status == EOF ){
	if ( fPrintError) cerr << "Error in <DecodeRawPacket::readCalibration>: invalid format of calibration file !" <<endl;
	return 2;
      }
      fCalibrationROC[iroc].SetAddressLevel(ilevel, level);
    }

    if ( fPrintDebug ){
      cout << " ROC (" << iroc << ") address levels = { " 
	   << fCalibrationROC[iroc].GetUltraBlackLevel() << " " << fCalibrationROC[iroc].GetBlackLevel();
      for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
	cout << fCalibrationROC[iroc].GetAddressLevel(ilevel) << " ";
      }
      cout << "}" << endl;
    }
  }
  
//--- close the calibration file.
  fclose(calibrateFile);

  if ( fPrintDebug ) Print(&cout);

  fNumROCs = numROCs;

//--- return success
  return 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int DecoderCalibrationModule::ReadCalibrationFile2(const char* fileName, int mode, int numROCs)
/*
  Read the UltraBlack, Black and address levels of ROCs 
  and the UltraBlack, Black and bit levels of the TBM
  from a calibration file

  The order from the lowest level is :
   <= UltraBlack              --> it is an ultra black
   >  UltraBlack and <= Black --> it is black
   > table[0] and <= table[1] --> it is 0 (table[0] and black can be the same)
   > table[1] and <= table[2] --> it is 1
   > table[2] and <= table[3] --> it is 2
   > table[3] and <= table[4] --> it is 3
   > table[4] and <= table[5] --> it is 4
   > table[5] and <= table[6] --> it is 5
   > table[6]                 --> invalid (can be infinte, e.g. 99999)

  Return value is 0 if the reading of the calibration file has been succesfull

  Error codes: 1 file not found
               2 invalid file format
*/
{
  int status = 0;
  FILE* calibrateFile = 0;

//--- open file
  if ( mode < 10 ){ // open file for module
    calibrateFile = fopen("module.cal", "r");
    if ( fPrintDebug ) cout << "Read in calibration from file: module.cal" << endl;
  } else { // open file for ROC
    calibrateFile = fopen("singleROC.cal", "r");
    if ( fPrintDebug ) cout << "Read in calibration from file: singleROC.cal" << endl;
  }

  if ( calibrateFile == 0 ){
    if ( fPrintError ) cerr << " Error in <DecodeRawPacket::readCalibration2>: cannot open the calibration file !" << endl;
    return 1;
  }

  ADCword level;
  status = fscanf(calibrateFile, "%d", &level); // read UltraBlack
  if ( status == EOF ){
    if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration2>: invalid format of calibration file !" <<endl;
    return 2;
  }
  fCalibrationTBM.SetUltraBlackLevel(level);

  status = fscanf(calibrateFile, "%d", &level); // read Black
  if ( status == EOF ){
    if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration2>: invalid format of calibration file !" <<endl;
    return 2;
  }
  fCalibrationTBM.SetBlackLevel(level);

//--- read levels of all ROCs
  bool end = false;
  for ( int iroc = 0; iroc < numROCs; iroc++ ){
//--- set UltraBlack and Black levels of ROCs to the TBM values
    fCalibrationROC[iroc].SetUltraBlackLevel(fCalibrationTBM.GetUltraBlackLevel());
    fCalibrationROC[iroc].SetBlackLevel(fCalibrationTBM.GetBlackLevel());

    if ( !end ){
      for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
	status = fscanf(calibrateFile, "%d", &level);
	if ( status==EOF || level == -1 ){
	  if ( iroc == 0 ){
	    if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration2>: invalid format of calibration file !" <<endl;
	    return 2;
	  } else {
	    if ( fPrintDebug ) cout << "ROC address level information not complete, use information of first ROC for all ROCs" << endl;
	    for ( int jroc = 1; jroc < numROCs; jroc++ ){
	      for ( int jlevel = 0; jlevel < (NUM_LEVELSROC + 1); jlevel++ ){
		fCalibrationROC[jroc].SetAddressLevel(jlevel, fCalibrationROC[0].GetAddressLevel(jlevel));
	      }
	      end = true;
	      break;
	    }
	  }
	} else {
	  fCalibrationROC[iroc].SetAddressLevel(ilevel, level);
	}
      }
    }

    if ( fPrintDebug ){
      cout << " ROC (" << iroc << ") address levels = { " 
	   << fCalibrationROC[iroc].GetUltraBlackLevel() << " " << fCalibrationROC[iroc].GetBlackLevel();
      for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
	cout << fCalibrationROC[iroc].GetAddressLevel(ilevel) << " ";
      }
      cout << "}" << endl;
    }
  }

//--- read TBM bit levels
//    (4 levels --> 5 limits)
  for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
    status = fscanf(calibrateFile, "%d", &level);
    if ( status == EOF || fCalibrationTBM.GetStatusLevel(ilevel) == -1 ){
      if ( fPrintDebug ) cout << "no bit levels for TBM specified, use default" << endl;
      break;
    }
    fCalibrationTBM.SetStatusLevel(ilevel, level);
  }

  if ( fPrintDebug ){
    cout << " TBM bit levels = { " 
	 << fCalibrationTBM.GetUltraBlackLevel() << " " << fCalibrationTBM.GetBlackLevel();
    for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
      cout << fCalibrationTBM.GetStatusLevel(ilevel) << " ";
    }
    cout << "}" << endl;
  }

//--- close the calibration file.
  fclose(calibrateFile);

  if ( fPrintDebug ) Print(&cout);

  fNumROCs = numROCs;

//--- return success
  return 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
int DecoderCalibrationModule::ReadCalibrationFile3(const char* fileName, int mode, int numROCs)
/*
  Read the UltraBlack, Black and address levels of ROCs 
  and the UltraBlack, Black and bit levels of the TBM
  from a calibration file

  The file format is that defined by DecodeRawPacket::Print

  Return value is 0 if the reading of the calibration file has been succesfull

  Error codes: 1 file not found
               2 invalid file format
*/
{
  ifstream* file = new ifstream(fileName);
  if ( file == 0 ){
    if ( fPrintError ) cerr << " Error in <DecodeRawPacket::readCalibration3>: cannot open the calibration file " << fileName << " !" << endl;
    return 1;
  }

//--- skip reading labels and separating lines
  char dummyString[100];
  for ( int iskip = 0; iskip < NUM_LEVELSTBM + 8; iskip++ ){
    *file >> dummyString;
    if ( fPrintDebug ) cout << "READ (dummyString): " << dummyString << endl;
  }

//--- skip reading first "-2000" number 
//    (not needed for address decoding)
  int dummyNumber;
  *file >> dummyNumber;
  if ( fPrintDebug ) cout << "READ (dummyNumber): " << dummyNumber << endl;

//--- read TBM UltraBlack and address levels
//    (skip reading TBM black level, keep it constant)
  ADCword level;
  *file >> level;
  if ( fPrintDebug ) cout << "READ (TBM UB): " << level << endl;
  fCalibrationTBM.SetUltraBlackLevel(level);
  *file >> level;
  if ( fPrintDebug ) cout << "READ (TBM B): " << level << endl;
  fCalibrationTBM.SetBlackLevel(level);
 
  if ( file->eof() || file->bad() ){
    if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" <<endl;
    return 2;
  }

  for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
    *file >> level;
    if ( fPrintDebug ) cout << "READ (TBM Lev" << ilevel << "): " << level << endl;
    fCalibrationTBM.SetStatusLevel(ilevel, level);

    if ( file->eof() || file->bad() ){
      if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" <<endl;
      return 2;
    }
  }

//--- skip reading labels and separating lines
  for ( int iskip = 0; iskip < NUM_LEVELSROC + 3; iskip++ ){
    *file >> dummyString;
    if ( fPrintDebug ) cout << "READ (dummyString): " << dummyString << endl;
  }

//--- read UltraBlack and address levels for each ROC
//    (skip reading black levels, keep them constant)
  for ( int iroc = 0; iroc < numROCs; iroc++ ){
    *file >> dummyString; // skip reading ROC label
    if ( fPrintDebug ) cout << "READ (dummyString): " << dummyString << endl;

    *file >> dummyNumber;
    if ( fPrintDebug ) cout << "READ (dummyNumber): " << dummyNumber << endl;

    *file >> level;
    if ( fPrintDebug ) cout << "READ (ROC" << iroc << " UB): " << level << endl;
    fCalibrationROC[iroc].SetUltraBlackLevel(level);
    *file >> level;
    if ( fPrintDebug ) cout << "READ (ROC" << iroc << " B): " << level << endl;
    fCalibrationROC[iroc].SetBlackLevel(level);
    
    if ( file->eof() || file->bad() ){
      if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" <<endl;
      return 2;
    }

    for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
      *file >> level;
      if ( fPrintDebug ) cout << "READ (ROC" << iroc << " Lev" << ilevel << "): " << level << endl;
      fCalibrationROC[iroc].SetAddressLevel(ilevel, level);
      
      if ( file->eof() || file->bad() ){
	if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" <<endl;
	return 2;
      }
    }
  }

//--- skip reading last separating lines
  *file >> dummyString;
  if ( fPrintDebug ) cout << "READ (dummyString): " << dummyString << endl;

  if ( file->eof() || file->bad() ){
    if ( fPrintError ) cerr << "Error in <DecodeRawPacket::readCalibration3>: invalid format of calibration file !" <<endl;
    return 2;
  }

  delete file;

  if ( fPrintDebug ) Print(&cout);

  fNumROCs = numROCs;

//--- return success
  return 0;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
const struct DecoderCalibrationROC& DecoderCalibrationModule::GetCalibrationROC(int rocId) const
{
  if ( rocId >= 0 && rocId < fNumROCs ){
    return fCalibrationROC[rocId];
  } else {
    cerr << "Error in <DecoderCalibrationModule::GetCalibrationROC>: no Calibration defined for ROC " << rocId << " !" << endl;
    return 0;
  }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationModule::Print(ostream* outputStream) const
{
  *outputStream << "Module Address Level Table:" << endl;
  *outputStream << "====================================================================================================" << endl;
  *outputStream << "         " << setw(9) << "UB" << setw(9) << "B" << endl;
  *outputStream << "                                      ";
  for ( int ilevel = 0; ilevel < NUM_LEVELSTBM; ilevel++ ){
    char label[6];
    sprintf(label, "Lev%d", ilevel);
    *outputStream << setw(9) << label;
  }
  *outputStream << endl;
  *outputStream << setw(5) << "TBM" << setw(9) << -2000 << setw(9) << fCalibrationTBM.GetUltraBlackLevel() << setw(9) << fCalibrationTBM.GetBlackLevel() << endl;
  *outputStream << "                                 ";
  for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
    *outputStream << setw(9) << fCalibrationTBM.GetStatusLevel(ilevel);
  }
  *outputStream << endl;
  *outputStream << "----------------------------------------------------------------------------------------------------" << endl;
  *outputStream << "         " << setw(9) << "UB" << setw(9) << "B" << endl;
  *outputStream << "                                      ";
  for ( int ilevel = 0; ilevel < NUM_LEVELSROC; ilevel++ ){
    char label[6];
    sprintf(label, "Lev%d", ilevel);
    *outputStream << setw(9) << label;
  }
  *outputStream << endl;
  for ( int iroc = 0; iroc < fNumROCs; iroc++ ){
    char label[6];
    sprintf(label, "ROC%d", iroc);
    *outputStream << setw(5) << label << setw(9) << -2000 << setw(9) << fCalibrationROC[iroc].GetUltraBlackLevel() << setw(9) << fCalibrationROC[iroc].GetBlackLevel() << endl;
    *outputStream << "                                 ";
    for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
      *outputStream << setw(9) << fCalibrationROC[iroc].GetAddressLevel(ilevel);
    }
    *outputStream << endl;
  }
  *outputStream << "====================================================================================================" << endl;
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
DecoderCalibrationTBM::DecoderCalibrationTBM()
{
  fUltraBlackLevel =   0;
  fBlackLevel      = 300;
  
  for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
    fStatusLevel[ilevel] = 2000;
  }
}

DecoderCalibrationTBM::DecoderCalibrationTBM(ADCword levels[])
{
  fUltraBlackLevel = levels[0];
  fBlackLevel      = 300;
  
  for ( int ilevel = 0; ilevel < (NUM_LEVELSTBM + 1); ilevel++ ){
    fStatusLevel[ilevel] = levels[ilevel + 1];
  }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationTBM::SetStatusLevel(int levelIndex, ADCword level)
{
  if ( levelIndex >= 0 && levelIndex <= NUM_LEVELSTBM ){
    fStatusLevel[levelIndex] = level;
  } else {
    cerr << "Error in <DecoderCalibrationTBM::SetStatusLevel>: index " << levelIndex << " out of range !" << endl;
  }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
ADCword DecoderCalibrationTBM::GetStatusLevel(int levelIndex) const
{
  if ( levelIndex >= 0 && levelIndex <= NUM_LEVELSTBM ){
    return fStatusLevel[levelIndex];
  } else {
    cerr << "Error in <DecoderCalibrationTBM::GetStatusLevel>: index " << levelIndex << " out of range !" << endl;
    return 0;
  }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
DecoderCalibrationROC::DecoderCalibrationROC()
{
  fUltraBlackLevel =   0;
  fBlackLevel      = 300;
  
  for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
    fAddressLevel[ilevel] = 2000;
  }
}

DecoderCalibrationROC::DecoderCalibrationROC(ADCword levels[])
{
  fUltraBlackLevel = levels[0];
  fBlackLevel      = 300;
  
  for ( int ilevel = 0; ilevel < (NUM_LEVELSROC + 1); ilevel++ ){
    fAddressLevel[ilevel] = levels[ilevel + 1];
  }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
void DecoderCalibrationROC::SetAddressLevel(int levelIndex, ADCword level)
{
  if ( levelIndex >= 0 && levelIndex <= NUM_LEVELSROC ){
    fAddressLevel[levelIndex] = level;
  } else {
    cerr << "Error in <DecoderCalibrationROC::SetAddressLevel>: index " << levelIndex << " out of range !" << endl;
  }
}
//-------------------------------------------------------------------------------


//-------------------------------------------------------------------------------
ADCword DecoderCalibrationROC::GetAddressLevel(int levelIndex) const
{
  if ( levelIndex >= 0 && levelIndex <= NUM_LEVELSROC ){
    return fAddressLevel[levelIndex];
  } else {
    cerr << "Error in <DecoderCalibrationROC::GetAddressLevel>: index " << levelIndex << " out of range !" << endl;
    return 0;
  }
}
//-------------------------------------------------------------------------------
