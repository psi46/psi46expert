//-----------------------------------------------------//
// Modified: Samvel Khalatyan (UIC) 2009 for SLC5
// Modified by Jose Lazo-Flores(UNL), Beat Meier (PSI) and Valeria Radicci (KU) 2010
// to be compatible with the last version of the Firmware and the TBM Emulator  
//-----------------------------------------------------//

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include <string.h>
#include <stdio.h>

#include "ConfigParameters.h"
#include "interface/Log.h"

ConfigParameters* ConfigParameters::instance = 0;

ConfigParameters::ConfigParameters()
{
	Initialize();
}

ConfigParameters::ConfigParameters(char *configParametersFileName)
{
	Initialize();
	ReadConfigParameterFile(configParametersFileName);
}

void ConfigParameters::Initialize()
{
	sprintf(testboardName, "\n");

	nRocs = 16;
	nModules = 1;
	hubId = 31;

	customModule = 0;
	
	hvOn = true;
	tbmEnable = true;
	tbmEmulator = false;
	keithleyRemote = false;
	guiMode = false;
	tbmChannel = 0;
	halfModule = 0;
	
	dataTriggerLevel = -500;
	emptyReadoutLength = 54;
	emptyReadoutLengthADC = 64;
	emptyReadoutLengthADCDual = 40;
	
	dacParametersFileName  = "defaultDACParameters.dat";
	tbmParametersFileName  = "defaultTBMParameters.dat";
	tbParametersFileName   = "defaultTBParameters.dat";
	trimParametersFileName = "defaultTrimParameters.dat";
	testParametersFileName = "defaultTestParameters.dat";
	maskFileName           = "defaultMaskFile.dat";	
	logFileName            = "log.txt";
  debugFileName          = "debug.log";
	rootFileName           = "expert.root";
		
	ia = 1.2;
	id = 1.;
	va = 1.7;
	vd = 2.5;
}

ConfigParameters* ConfigParameters::Singleton()
{
 	if (instance == 0) {instance = new ConfigParameters();}
 	return instance;
}



bool ConfigParameters::ReadConfigParameterFile( const char *_file)
{
  std::ifstream _input( _file);
  if( !_input.is_open())
  {
    psi::LogInfo() << "[ConfigParameters] Can not open file '"
                   << _file << "' to read Config Parameters." << psi::endl;

    return false;
  }

	psi::LogInfo() << "[ConfigParameters] Reading Config-Parameters from '"
                 << _file << "'." << psi::endl;

  // Read file by lines
  for( std::string _line; _input.good(); )
  {
    getline( _input, _line);

    // Skip Empty Lines and Comments (starting from # or - )
    if( !_line.length()
        || '#' == _line[0]
        || '-' == _line[0] ) continue;

    std::istringstream _istring( _line);
    std::string _name;
    std::string _value;

    _istring >> _name >> _value;

    // Skip line in case any errors occured while reading parameters
    if( _istring.fail() || !_name.length() ) continue;

    int _ivalue = atoi( _value.c_str() );

         if( 0 == _name.compare( "testboardName"             ) ) { sprintf( testboardName, _value.c_str()); }
    else if( 0 == _name.compare( "directory"                 ) ) { sprintf( directory    , _value.c_str()); }

    else if( 0 == _name.compare( "tbParameters"              ) ) { SetTbParameterFileName  ( _value); }
    else if( 0 == _name.compare( "tbmParameters"             ) ) { SetTbmParameterFileName ( _value); }
    else if( 0 == _name.compare( "testParameters"            ) ) { SetTestParameterFileName( _value); }
    else if( 0 == _name.compare( "dacParameters"             ) ) { SetDacParameterFileName ( _value); }
    else if( 0 == _name.compare( "rootFileName"              ) ) { SetRootFileName         ( _value); }
    else if( 0 == _name.compare( "trimParameters"            ) ) { SetTrimParameterFileName( _value); }

    else if( 0 == _name.compare( "nModules"                  ) ) { nModules                  = _ivalue; }
    else if( 0 == _name.compare( "nRocs"                     ) ) { nRocs                     = _ivalue; }
    else if( 0 == _name.compare( "hubId"                     ) ) { hubId                     = _ivalue; }
    else if( 0 == _name.compare( "customModule"              ) ) { customModule              = _ivalue; }
    else if( 0 == _name.compare( "halfModule"                ) ) { halfModule                = _ivalue; }
    else if( 0 == _name.compare( "dataTriggerLevel"          ) ) { dataTriggerLevel          = _ivalue; }
    else if( 0 == _name.compare( "emptyReadoutLength"        ) ) { emptyReadoutLength        = _ivalue; }
    else if( 0 == _name.compare( "emptyReadoutLengthADC"     ) ) { emptyReadoutLengthADC     = _ivalue; }
    else if( 0 == _name.compare( "emptyReadoutLengthADCDual" ) ) { emptyReadoutLengthADCDual = _ivalue; }
    else if( 0 == _name.compare( "hvOn"                      ) ) { hvOn                      = _ivalue; }
    else if( 0 == _name.compare( "keithleyRemote"            ) ) { keithleyRemote            = _ivalue; }
    else if( 0 == _name.compare( "tbmEnable"                 ) ) { tbmEnable                 = _ivalue; }
    else if( 0 == _name.compare( "tbmEmulator"                 ) ) { tbmEmulator             = _ivalue; }
    else if( 0 == _name.compare( "tbmChannel"                ) ) { tbmChannel                = _ivalue; }

    else if( 0 == _name.compare( "ia") ) { ia = .001 * _ivalue; }
    else if( 0 == _name.compare( "id") ) { id = .001 * _ivalue; }
    else if( 0 == _name.compare( "va") ) { va = .001 * _ivalue; }
    else if( 0 == _name.compare( "vd") ) { vd = .001 * _ivalue; }

    else { psi::LogInfo() << "[ConfigParameters] Did not understand '" 
                          << _name << "'." << psi::endl; }
  }

  _input.close();

  return true;
}


void ConfigParameters::SetTbParameterFileName( const std::string &_file)
{
  tbParametersFileName.assign( directory).append(  "/").append( _file);
}


void ConfigParameters::SetTbmParameterFileName( const std::string &_file)
{
  tbmParametersFileName.assign( directory).append(  "/").append( _file);
}


void ConfigParameters::SetDacParameterFileName( const std::string &_file)
{
  dacParametersFileName.assign( directory).append(  "/").append( _file);
}


void ConfigParameters::SetTrimParameterFileName( const std::string &_file)
{
  trimParametersFileName.assign( directory).append(  "/").append( _file);
}


void ConfigParameters::SetTestParameterFileName( const std::string &_file)
{
  testParametersFileName.assign( directory).append(  "/").append( _file);
}


void ConfigParameters::SetRootFileName( const std::string &_file)
{
  rootFileName.assign( directory).append(  "/").append( _file);
}


void ConfigParameters::SetLogFileName( const std::string &_file)
{
  logFileName.assign( directory).append(  "/").append( _file);
}

void ConfigParameters::SetDebugFileName( const std::string &_file)
{
  debugFileName.assign( directory).append( "/").append( _file);
}

void ConfigParameters::SetMaskFileName( const std::string &_file)
{
  maskFileName.assign( directory).append(  "/").append( _file);
}

bool ConfigParameters::WriteConfigParameterFile()
{
	char filename[1000];
	sprintf(filename, "%s/configParameters.dat", directory);
	FILE *file = fopen(filename, "w");
	if (!file) 
	{
		psi::LogInfo() << "[ConfigParameters] Can not open file '" << filename
                   << "' to write configParameters." << psi::endl;
		return false;
	}

	psi::LogInfo() << "[ConfigParameters] Writing Config-Parameters to '"
                 << filename << "'." << psi::endl;

	fprintf(file, "testboardName %s\n\n", testboardName);
	
	fprintf(file, "-- parameter files\n\n");
	
	fprintf(file, "tbParameters %s\n", &tbParametersFileName[strlen(directory)+1]);
	fprintf(file, "dacParameters %s\n", &dacParametersFileName[strlen(directory)+1]);
	fprintf(file, "tbmParameters %s\n", &tbmParametersFileName[strlen(directory)+1]);
	fprintf(file, "trimParameters %s\n", &trimParametersFileName[strlen(directory)+1]);
	fprintf(file, "testParameters %s\n", &testParametersFileName[strlen(directory)+1]);
	fprintf(file, "rootFileName %s\n\n", &rootFileName[strlen(directory)+1]);

	fprintf(file, "-- configuration\n\n");

	if (customModule) fprintf(file, "customModule %i\n", customModule);
		
	fprintf(file, "nModules %i\n", nModules);
	fprintf(file, "nRocs %i\n", nRocs);
	fprintf(file, "hubId %i\n", hubId);
	fprintf(file, "tbmEnable %i\n", tbmEnable);
	fprintf(file, "tbmEmulator %i\n", tbmEmulator);
	fprintf(file, "hvOn %i\n", hvOn);
	fprintf(file, "tbmChannel %i\n\n", tbmChannel);
	fprintf(file, "halfModule %i\n\n", halfModule);
	
	fprintf(file, "-- voltages and current limits\n\n");

	fprintf(file, "ia %i\n"  , static_cast<int>( ia * 1000));
	fprintf(file, "id %i\n"  , static_cast<int>( id * 1000));
	fprintf(file, "va %i\n"  , static_cast<int>( va * 1000));
	fprintf(file, "vd %i\n\n", static_cast<int>( vd * 1000));
	
	fprintf(file, "-- adc parameters\n\n");
	
	fprintf(file, "dataTriggerLevel %i\n", dataTriggerLevel);
	fprintf(file, "emptyReadoutLength %i\n", emptyReadoutLength);
	fprintf(file, "emptyReadoutLengthADC %i\n", emptyReadoutLengthADC);
	fprintf(file, "emptyReadoutLengthADCDual %i\n", emptyReadoutLengthADCDual);
	
	fclose(file);
	return true;
}
