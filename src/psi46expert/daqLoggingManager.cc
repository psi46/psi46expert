#include "psi46expert/daqLoggingManager.hh"

#include <cstdlib>

#include "TSystem.h"
#include "TUnixSystem.h"

#include "psi46expert/daqFrame.hh"
#include "BasePixel/TBInterface.h"
#include "psi46expert/TestParameters.h"
#include "psi46expert/TestControlNetwork.h"

// ----------------------------------------------------------------------
daqLoggingManager::daqLoggingManager(const char * d) {

    fRunMode   = 7;

    system("/bin/rm -f daqLM.123456789");
    fOUT = new ofstream("daqLM.123456789");

    init();
    setBaseDir(d);

}

// ----------------------------------------------------------------------
int daqLoggingManager::initialRunNumber() {
    float run, maxRun(-1);
    const char * file;
    void * pDir = gSystem->OpenDirectory("/tmp");
    while (file = gSystem->GetDirEntry(pDir)) {
        if (sscanf(file, "bt05r%f", &run)) {
            if (run > maxRun) maxRun = run;
        }
    }

    int nextrun = int(maxRun) + 1;
    log(Form("==>LM: Next runnumber: %i", nextrun));
    return nextrun;
}


// ----------------------------------------------------------------------
void daqLoggingManager::init() {
    log("==>LM: daqLoggingManager::init()");
    fRunNumber = initialRunNumber();
}


// ----------------------------------------------------------------------
void daqLoggingManager::close() {
    log("==>LM: daqLoggingManager::close()");

}


// ----------------------------------------------------------------------
void daqLoggingManager::setBaseDir(const char * s) {
    fBaseDir   = TString(s);
    //  fOutputDir = fBaseDir + TString("/rtb_run_") + TString(Form("%06i", getRunNumber())); // FIXME is this a feature??
    fOutputDir = fBaseDir + TString("/bt05r") + TString(Form("%06i", getRunNumber())); // FIXME is this a feature??
}


// ----------------------------------------------------------------------
void daqLoggingManager::setupRun() {

    log(Form("==>LM: setup Run %i", getRunNumber()));

    // -- check whether directory exists
    void * dirp = gSystem->OpenDirectory(getOutputDir());
    if (dirp) {
        log(Form("==>LM: Warning directory %s", getOutputDir()));
    } else {
        sprintf(fLine, "/bin/mkdir -p %s", getOutputDir());
        log(Form("==>LM: %s", fLine));
        system(fLine);
    }

    // -- Migrate log file to new place
    log(Form("==>LM: Migrate logfile to %s", getOutputDir()));
    fOUT->close();

    //   system(Form("/bin/mv daqLM.123456789 %s/rtb_run_%06i.log", getOutputDir(), getRunNumber()));
    //   fOUT = new ofstream(Form("%s/rtb_run_%06i.log", getOutputDir(), getRunNumber()),  ios::app);

    system(Form("/bin/mv daqLM.123456789 %s/bt05r%06i.log", getOutputDir(), getRunNumber()));
    fOUT = new ofstream(Form("%s/bt05r%06i.log", getOutputDir(), getRunNumber()),  ios::app);

}


// ----------------------------------------------------------------------
void daqLoggingManager::dumpHardwareConfiguration(int imod, TestControlNetwork * cn, TBInterface * tb) {

    //   system(Form("/bin/cp -r %s %s/settings", fpMTB->directory, getOutputDir()));

    //   char line[1000];
    //
    //   if (!cn) {
    //     log("==>LM: vCN dumpHardwareConfiguration");
    //   }
    //   else {
    //     sprintf(line, "%s/logdac-cn%i", getOutputDir(), imod);
    //     cn->WriteDACParameterFile(line);
    //     sprintf(line, "%s/logtrim-cn%i", getOutputDir(), imod);
    //     cn->WriteTrimConfiguration(line);
    //   }
    //
    //   if (!tb) {
    //     log("==>LM: vTB dumpHardwareConfiguration");
    //   }
    //   else {
    //     sprintf(line, "%s/logtestboard-%i", getOutputDir(), imod);
    //     tb->WriteTBParameterFile(line);
    //   }

}


// ----------------------------------------------------------------------
void daqLoggingManager::copyHardwareInitialization() {


}


// ----------------------------------------------------------------------
int daqLoggingManager::incrementRunNumber() {
    ++fRunNumber;
    setBaseDir(getBaseDir());
    return fRunNumber;
}


// ----------------------------------------------------------------------
void daqLoggingManager::log(const char * l) {
    sprintf(fLine2, "%s: %s", timeStamp(), l);
    cout << fLine2 << endl;
    (*fOUT) << fLine2 << endl;
}


// ----------------------------------------------------------------------
const char * daqLoggingManager::timeStamp() {
    // get and log time
    time_t rawtime;
    struct tm * timeinfo;
    time(&rawtime);
    timeinfo = localtime(&rawtime);
    //  fpLM->log(Form("Current date and time are: %s", asctime (timeinfo) ));
    strcpy(fLine2, asctime(timeinfo));
    fLine2[24] = '\0';
    return fLine2;
}
