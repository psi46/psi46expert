#include <time.h>
#include <stdio.h>

#include <TFile.h>
#include <TString.h>
#include <TApplication.h>
#include <TStyle.h>

#include "interface/Delay.h"
#include "psi46expert/TestParameters.h"
#include "psi46expert/TestControlNetwork.h"
#include "psi46expert/MainFrame.h"
#include "psi46expert/Xray.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/SysCommand.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/Getline.c"
#include "BasePixel/Keithley.h"
#include "interface/Log.h"

TBAnalogInterface* tbInterface;
TestControlNetwork *controlNetwork;
ConfigParameters *configParameters;
SysCommand sysCommand;

Keithley *Power_supply;

char* testMode(""), cmdFile[1000] = "";
bool guiMode(false);
int V=0;

const char *fullTest = "full";
const char *shortTest = "short";
const char *shortCalTest = "shortCal";
const char *calTest = "cal";
const char *phCalTest = "phCal";
const char *dtlTest = "dtlScan";
const char *xrayTest = "xray";

const char *guiTest = "GUI";
const char *scurveTest = "scurves";
const char *preTest = "preTest";
const char *TrimTest = "trimTest";
const char *ThrMaps ="ThrMaps";

void runGUI()
{
  TApplication *application = new TApplication("App",0,0, 0, -1);
  MainFrame MainFrame(gClient->GetRoot(), 400, 400, tbInterface, controlNetwork, configParameters);
  application->Run();
}


void execute(SysCommand &command)
{
  do
  {
    if (command.Keyword("gui"))
    {
      runGUI();
    }
    else if (command.TargetIsTB()) {tbInterface -> Execute(command);}
    else  {controlNetwork->Execute(command);}
  }
  while (command.Next());
  tbInterface->Flush();
}



void runTest()
{
  if (tbInterface->IsPresent() < 1)
  {
    cout << "Error!! Testboard not present. Aborting" << endl;
    return;
  }
  gDelay->Timestamp();
  if (strcmp(testMode, fullTest) == 0)
  {
    psi::LogInfo() << "[psi46expert] SvFullTest and Calibration: start." << psi::endl; 

    controlNetwork->FullTestAndCalibration();

    psi::LogInfo() << "[psi46expert] SvFullTest and Calibration: end." << psi::endl; 
  }
  if (strcmp(testMode, shortTest) == 0)
  {
    psi::LogInfo() << "[psi46expert] SvShortTest: start." << psi::endl; 

    controlNetwork->ShortCalibration();

    psi::LogInfo() << "[psi46expert] SvShortTest: end." << psi::endl; 
  }
  if (strcmp(testMode, shortCalTest) == 0)
  {
    psi::LogInfo() << "[psi46expert] SvShortTest and Calibration: start." << psi::endl; 

    controlNetwork->ShortTestAndCalibration();

    psi::LogInfo() << "[psi46expert] SvShortTest and Calibration: end." << psi::endl; 
  } 
  if (strcmp(testMode, xrayTest) == 0)
  {
    TestRange *testRange = new TestRange();
    testRange->CompleteRange();
    Test *test = new Xray(testRange, controlNetwork->GetTestParameters(), tbInterface);
    test->ControlNetworkAction(controlNetwork);
  } 
  if (strcmp(testMode, calTest) == 0)
  {
    sysCommand.Read("cal.sys");
    execute(sysCommand);
  }
  if (strcmp(testMode, phCalTest) == 0)
  {
    sysCommand.Read("phCal.sys");
    execute(sysCommand);
  }
  if (strcmp(testMode, dtlTest) == 0)
  {
    sysCommand.Read("dtlTest.sys");
    execute(sysCommand);
  }
        
        if (strcmp(testMode, guiTest) == 0)
        {
          sysCommand.Read("gui.sys");
          execute(sysCommand);
        }
        
        if (strcmp(testMode, ThrMaps) == 0)
        {
          sysCommand.Read("ThrMaps.sys");
          execute(sysCommand);
        }
 	if (strcmp(testMode,scurveTest ) == 0)
        {
          sysCommand.Read("scurve.sys");
          execute(sysCommand);
        }

  gDelay->Timestamp();
}


void runFile()
{
  if (tbInterface->IsPresent() < 1)
  {
    psi::LogInfo() << "[psi46expert] Error: Testboard is not present. Abort.";

    return;
  }
  
  gDelay->Timestamp();
  
  psi::LogInfo() << "[psi46expert] Executing file '" << cmdFile
                 << "'." << psi::endl; 

  sysCommand.Read(cmdFile);
  execute(sysCommand);
  
  gDelay->Timestamp();
}


void parameters(int argc, char* argv[], ConfigParameters *configParameters)
{
  int hubId;
  char rootFile[1000], logFile[1000], dacFile[1000], trimFile[1000], directory[1000], tbName[1000], maskFile[1000];
        sprintf(directory, "testModule");
  bool rootFileArg(false), dacArg(false), trimArg(false), tbArg(false), logFileArg(false), cmdFileArg(false), hubIdArg(false),
	     maskArg(false);

  // == command line arguments ======================================================
  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i],"-dir")) 
    {
      sprintf(directory, argv[++i]);
    }
    if (!strcmp(argv[i],"-c")) 
    {
      rootFileArg = true;
      sprintf(rootFile, Form("test-%s.root", argv[++i]));
    }
    if (!strcmp(argv[i],"-d"))
    {
      dacArg = true;
      sprintf(dacFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-r"))
    {
      rootFileArg = true;
      sprintf(rootFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-f"))
    {
      cmdFileArg = true;
      sprintf(cmdFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-log"))
    {
      logFileArg = true;
      sprintf(logFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-trim")) 
    {
      trimArg = true;
      sprintf(trimFile, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-trimVcal")) 
    {
      trimArg = true;
      dacArg = true;
      int vcal = atoi(argv[++i]);
      sprintf(trimFile, "%s%i", "trimParameters", vcal);
      sprintf(dacFile, "%s%i", "dacParameters", vcal);
    }
		if (!strcmp(argv[i],"-mask")) 
		{
			maskArg = true;
			sprintf(maskFile, "%s","pixelMask.dat" );//argv[++i]);
		}		
    if (!strcmp(argv[i],"-tb")) 
    {
      tbArg = true;
      sprintf(tbName, "%s", argv[++i]);
    }
    if (!strcmp(argv[i],"-t")) 
    {
      testMode = argv[++i];
      if (strcmp(testMode, dtlTest) == 0)
      {
        hubIdArg = true;
        hubId = -1;
      }
    }
    if (!strcmp(argv[i],"-g")) guiMode = true;

  } 
  sprintf(configParameters->directory, directory);
  
  if (strcmp(testMode, fullTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "FullTest.log");
    rootFileArg = true;
    sprintf(rootFile, "FullTest.root");   
  }
  if (strcmp(testMode, shortTest) == 0 || strcmp(testMode, shortCalTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "ShortTest.log");
    rootFileArg = true;
    sprintf(rootFile, "ShortTest.root");    
  }
  else if (strcmp(testMode, calTest) == 0)
  {
    logFileArg = true;
    sprintf(logFile, "Calibration.log");
    rootFileArg = true;
    sprintf(rootFile, "Calibration.root");    
  }
  
  if (logFileArg) configParameters->SetLogFileName(logFile);
  else configParameters->SetLogFileName( "log.txt");

  configParameters->SetDebugFileName( "debug.log");

  psi::LogInfo ().setOutput( configParameters->GetLogFileName() );
  psi::LogDebug().setOutput( configParameters->GetDebugFileName() );

  psi::LogInfo() << "[psi46expert] --------- psi46expert ---------" 
                 << psi::endl;
  psi::LogInfo() << "[psi46expert] " << TDatime().AsString() << psi::endl;
  
  configParameters->ReadConfigParameterFile(Form("%s/configParameters.dat", directory));
  if (rootFileArg) configParameters->SetRootFileName(rootFile);
  if (dacArg) configParameters->SetDacParameterFileName(dacFile);
  if (tbArg) sprintf(configParameters->testboardName, tbName);
  if (trimArg) configParameters->SetTrimParameterFileName(trimFile);
	if (maskArg) configParameters->SetMaskFileName(maskFile);
  if (hubIdArg) configParameters->hubId = hubId;
}


int main(int argc, char* argv[])
{
  for (int i = 0; i < argc; i++)
  {
    if (!strcmp(argv[i],"-V")) 
    {
      V=atoi(argv[++i]);
    }}

  configParameters = ConfigParameters::Singleton();
  parameters(argc, argv, configParameters);
  // == Initialization =====================================================================

  TFile* histoFile = new TFile(configParameters->GetRootFileName(), "RECREATE");
  gStyle->SetPalette(1,0);
  
  tbInterface = new TBAnalogInterface(configParameters);
  if (!tbInterface->IsPresent()) return -1;
  controlNetwork = new TestControlNetwork(tbInterface, configParameters);
  
//  sysCommand.Read("start.sys");
//  execute(sysCommand);

        Power_supply=new Keithley();
        if(V>0){

          Power_supply->Open();
          Power_supply->Init();
          int volt=25,step=25;
          while (volt<V-25){
            Power_supply->SetVoltage(volt,1);
            volt=volt+step;
            if(volt>400){step=10;}
            if(volt>600){step=5;}
          } 
          Power_supply->SetVoltage(V,4);
        }
        

  if (guiMode) runGUI();
  else if (strcmp(testMode, "") != 0) runTest();
  else if (strcmp(cmdFile, "") != 0) runFile();
  else
  {
    // == CommandLine ================================================================

    char *p;
    Gl_histinit("../.hist");
    do
    {
      p = Getline("psi46expert> ");
      Gl_histadd(p);

      psi::LogDebug() << "psi46expert> " << p << psi::endl;

      if (sysCommand.Parse(p)) execute(sysCommand);
    }
    while ((strcmp(p,"exit\n") != 0) && (strcmp(p,"q\n") != 0));
  }

  // == Exit ========================================================================

        if (!strcmp(testMode, phCalTest) == 0)
        {
          tbInterface->HVoff();
          tbInterface->Poff();
          tbInterface->Cleanup();
        }

        if(V>0)
        {
        Power_supply->ShutDown();}        

  delete controlNetwork;
  delete tbInterface;

  histoFile->Write();
  histoFile->Close();
  delete histoFile;
        delete Power_supply;
  
  return 0;
}
