#include "BasePixel/TBInterface.h"
#include "psi46expert/TestParameters.h"
#include "psi46expert/TestControlNetwork.h"
#include "psi46expert/daqFrame.hh"
#include "psi46expert/UsbDaq.h"
#include "psi46expert/daqLoggingManager.hh"
#include "psi46expert/histogrammer.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include "BasePixel/DecodedReadout.h"
#include "BasePixel/ConfigParameters.h"
#include <stdlib.h>
#include <TString.h>
#include <TApplication.h>
#include <signal.h> // for handling ctrl-c (sigint) events

daqFrame *dF; // need global daqFrame object so that we can refer to it from signal handling function
int caught_sigint;

void my_sigint_handler(int s){
  if (s != 2) {
    printf("Caught signal %d, not handled by this routine\n",s);
    return;
  }
  if (dF == 0 || caught_sigint>4){
    // object not ready yet or already caught sigint: just exit
    printf("Caught signal %d, going to exit hard\n",s);
    exit(1); 
  }
  else {
    printf("Caught signal %d, going to stop run\n",s);
    caught_sigint++;
    dF->doStop();
  }
}


// ----------------------------------------------------------------------
int main(int argc, char * argv[])
{
  dF = 0;
  caught_sigint = false;
  // setup signal handling
  struct sigaction act;
  act.sa_handler = my_sigint_handler;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  sigaction(SIGINT, &act, 0);
    int mode(7), runnumber(0), localtrigger(0);
  int secondBoard = 0;
  int duration = -1;
    bool batchMode = false, trimArg = false, dacArg = false, maskArg = false;
    char rootFile[1000], logFile[1000], dacFile[1000], trimFile[1000], directory[1000], tbName[1000], maskFile[1000];

  int numROCs = 1;

    ConfigParameters * mtbConfigParameters = new ConfigParameters();
    //  sprintf(mtbConfigParameters->directory, "testModule");
    sprintf(mtbConfigParameters->directory, "./setup/mtb");
    int V = 0;



    // -- command line arguments
    for (int i = 0; i < argc; i++)  {
        if (!strcmp(argv[i], "-b")) batchMode = true;
        if (!strcmp(argv[i], "-V")) V = atoi(argv[++i]);
        if (!strcmp(argv[i], "-numrocs")) numROCs = atoi(argv[++i]);
        if (!strcmp(argv[i], "-l")) localtrigger = 1;
	if (!strcmp(argv[i],"-s")) secondBoard = 1;
        if (!strcmp(argv[i], "-m")) mode = atoi(argv[++i]);
	if (!strcmp(argv[i],"-r")) runnumber = atoi(argv[++i]);
	if (!strcmp(argv[i],"-duration")) duration = atoi(argv[++i]);
        if (!strcmp(argv[i], "-dir")) strcpy(mtbConfigParameters->directory, argv[++i]);
        if (!strcmp(argv[i], "-trimVcal"))
        {
            trimArg = true;
            dacArg = true;
            int vcal = atoi(argv[++i]);
            sprintf(trimFile, "%s%i", "trimParameters", vcal);
            sprintf(dacFile, "%s%i", "dacParameters", vcal);
        }
        if (!strcmp(argv[i], "-mask"))
        {
            maskArg = true;
	    sprintf(maskFile, "%s/%s", mtbConfigParameters->directory, "pixelMask.dat");
        }
    }

    mtbConfigParameters->ReadConfigParameterFile(Form("%s/configParameters.dat", mtbConfigParameters->directory));
    if (dacArg) mtbConfigParameters->SetDacParameterFileName(dacFile);
    if (trimArg) mtbConfigParameters->SetTrimParameterFileName(trimFile);
    if (maskArg) mtbConfigParameters->SetMaskFileName(maskFile);

    //logging manager
    daqLoggingManager * lm = new daqLoggingManager("../data");
    lm->setRunMode(mode);
    lm->setMTBConfigParameters(mtbConfigParameters);
    if (runnumber > 0) lm->setRunNumber(runnumber);
    else if( secondBoard ) runnumber = lm->incrementRunNumber();

    //decoder
    RawPacketDecoder * gDecoder = RawPacketDecoder::Singleton();
    TString fileName = TString(mtbConfigParameters->directory).Append("/addressParameters.dat");
    cout << "Reading Address Level-Parameters from " << fileName << endl;
    DecoderCalibrationModule * decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, numROCs);
    decoderCalibrationModule->Print(&cout);
    gDecoder->SetCalibration(decoderCalibrationModule);

    UsbDaq * daq = new UsbDaq(mode);

    histogrammer * h = new histogrammer();
    daq->setHistogrammer(h);
    daq->setDecoder(gDecoder);

    // -- GUI
    TApplication * application = new TApplication("App", 0, 0, 0, -1);
  dF = new daqFrame(gClient->GetRoot(), 1000, 700, lm, batchMode);
    dF->setUsbDAQ(daq);
    dF->setLoggingManager(lm);
    if (localtrigger) dF->fLocalTrigger = 1;
    //  if(V>0)dF->doVup(V);
    if( duration > 0 ) dF->setRunDuration(duration);

    if (batchMode) {
        dF->setFillMem(0);
        dF->doSetSysCommand1Text(); //exec module.ini
        if (V > 0)dF->doVup(V);
        dF->doStart();
        if (V > 0)dF->doVdown(V);
        dF->doExit();
    } else  {
        dF->doSetSysCommand1Text(); //exec module.ini
        application->Run();
        // if(V>0)dF->doVdown(V);
        dF->doExit();
    }
    return 0;
}
