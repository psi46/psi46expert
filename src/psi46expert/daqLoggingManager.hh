#ifndef DAQLOGGINGMANAGER_H
#define DAQLOGGINGMANAGER_H

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <time.h>

class TestControlNetwork;
class TBDigitalInterface;
class ConfigParameters;

#include "TString.h"

class daqLoggingManager {
public:
    daqLoggingManager(const char * basedir = "/tmp");
    ~daqLoggingManager();

    void        close();
    void        init();

    int         initialRunNumber();
    const char * timeStamp();

    void        log(const char *);
    //  ofstream*   LOGStream() {return fOUT;}

    const char * getOutputDir() {return fOutputDir.Data();}
    void        setOutputDir(const char * s) {fOutputDir = TString(s);}

    const char * getBaseDir() {return fBaseDir.Data();}
    void        setBaseDir(const char * s); // modifies the output directory as well

    void        setRunMode(int iRunMode) {fRunMode = iRunMode;}
    int         getRunMode() {return fRunMode;}

    void               setCTBConfigParameters(ConfigParameters * cfg) {fpCTB = cfg;}
    ConfigParameters * getCTBConfigParameters() {return fpCTB;}
    void               setRTBConfigParameters(ConfigParameters * cfg) {fpRTB = cfg;}
    ConfigParameters * getRTBConfigParameters() {return fpRTB;}
    void               setMTBConfigParameters(ConfigParameters * cfg) {fpMTB = cfg;}
    ConfigParameters * getMTBConfigParameters() {return fpMTB;}

    void        setRunNumber(int irun) {fRunNumber = irun; std::cout << "LM: runnumber set to " << irun << std::endl;}
    int         getRunNumber() {return fRunNumber;}
    int         incrementRunNumber();

    void        setupRun();
    void        dumpHardwareConfiguration(int i, TestControlNetwork *, TBDigitalInterface *);
    void        copyHardwareInitialization();

private:
    struct tm  * fTime;
    char         fLine[2000], fLine2[2000];
    TString      fBaseDir, fOutputDir;

    int          fRunMode;
    int          fRunNumber;

    ConfigParameters * fpCTB, *fpRTB, *fpMTB;

    std::ofstream   *  fOUT;

};
#endif
