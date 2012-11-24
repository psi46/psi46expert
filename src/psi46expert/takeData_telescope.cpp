
#include <cstdlib>
#include <stdio.h>
#include "psi46expert/TestParameters.h"
#include "psi46expert/UsbDaq.h"
#include "psi46expert/daqLoggingManager.hh"
#include "psi46expert/histogrammer.h"
#include "BasePixel/ConfigParameters.h"
#include "../BasePixel/psi46_tbts.h"
#include <TString.h>
#include <TApplication.h>
#include "telescopeFrame.h"

using namespace std;

// ----------------------------------------------------------------------
int main(int argc, char* argv[]) 
{ 
  int mode(7), runnumber(0), localtrigger(0);
  bool batchMode = false, trimArg = false, dacArg = false, maskArg = false;	
  char rootFile[1000], logFile[1000], dacFile[1000], trimFile[1000], directory[1000], tbName[1000], maskFile[1000];


  ConfigParameters *mtbConfigParameters = new ConfigParameters();
  //  sprintf(mtbConfigParameters->directory, "testModule");
  sprintf(mtbConfigParameters->directory, "./setup/mtb");
  int V=0;



  // -- command line arguments
  for (int i = 0; i < argc; i++)  {
    if (!strcmp(argv[i],"-b")) batchMode = true;
    if (!strcmp(argv[i],"-V")) V=atoi(argv[++i]);
    if (!strcmp(argv[i],"-l")) localtrigger = 1;
    if (!strcmp(argv[i],"-m")) mode = atoi(argv[++i]);
    if (!strcmp(argv[i],"-dir")) sprintf(mtbConfigParameters->directory, argv[++i]);		
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
      sprintf(maskFile, "%s", "pixelMask.dat");
    }
  } 

  mtbConfigParameters->ReadConfigParameterFile(Form("%s/configParameters.dat", mtbConfigParameters->directory));
  if (dacArg) mtbConfigParameters->SetDacParameterFileName(dacFile);
  if (trimArg) mtbConfigParameters->SetTrimParameterFileName(trimFile);
  if (maskArg) mtbConfigParameters->SetMaskFileName(maskFile);
  
  //logging manager
  //daqLoggingManager *lm = new daqLoggingManager("/tmp");
  //lm->setRunMode(mode);
  //lm->setMTBConfigParameters(mtbConfigParameters);

  //UsbDaq *daq = new UsbDaq(mode);
  //if (runnumber > 0) lm->setRunNumber(runnumber);

  //histogrammer *h = new histogrammer();
  //daq->setHistogrammer(h);


  CTestboard_TS* tb = new CTestboard_TS();
  
  if (!tb->Open(mtbConfigParameters->testboardName))
     {
       cout << "Open failed." << endl;
       //return 0;
     }
  else
     cout << "Open succeeded!" << endl;

  
  TApplication *application = new TApplication("App", 0, 0, 0, -1);
  CTelescopeFrame* frame = new CTelescopeFrame(gClient->GetRoot(), mtbConfigParameters->GetDataDirName());

  frame->SetTB(tb);

 //tb->StartRun(2000); 
 
  //cout << "Ready: " << tb->ReadyForReadout() << endl;
 // tb->GetData();

  //cout << "Status=" << tb->GetStatus() << endl;
  

 // for(int x = 0 ; x < 1000 ; x++)
	{
//	   cout << "Status=" << tb->GetStatus() << endl;
	}
  //sleep(10);


  //cout << "Board ID: " <<tb->GetBoardId() << endl;

  application->Run();

  cout << "Closing TB" << endl;
  tb->Close();  
  delete tb;
  return 0;

}
