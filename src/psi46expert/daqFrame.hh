#ifndef DAQFRAME
#define DAQFRAME

#include <TBuffer.h>
#include <TCanvas.h>
#include <TGButton.h>
#include <TGButtonGroup.h>
#include <TGFrame.h> 
#include <TGTextBuffer.h> 
#include <TGTextEntry.h> 
#include <TGComboBox.h>
#include <TGMenu.h>
#include <TGLabel.h>

#include <TRootEmbeddedCanvas.h> 

#include <TF1.h>
#include <TString.h>

#include "UsbDaq.h"
#include "daqLoggingManager.hh"
#include "CommandLineInterpreter.h"
#include "BasePixel/SysCommand.h"
#include "BasePixel/ConfigParameters.h"
#include <BasePixel/Keithley.h>


class TBAnalogInterface;
class TestControlNetwork;

class daqFrame: public TGMainFrame { 

public: 
  daqFrame(const TGWindow *p, UInt_t w = 600, UInt_t h = 600, daqLoggingManager *q =0, bool batchMode = false); 
  virtual ~daqFrame(); 

  Keithley *Power_supply; 
  
  const TGWindow * getWindow() {return fpWindow;}

  void initializeHardware();
  void runStart();
  void stopTriggers();
  void startTriggers();
  void getTemperature();
  void wbcScan();
  void dacScan();
  void setRunDuration(int duration) {fRunDuration = duration;};
  void setFillMem(int i) {fFillMem = i;};
  
  void setUsbDAQ(UsbDaq *p) { fpDAQ = p;}
  void setLoggingManager(daqLoggingManager *p) { fpLM = p;}
  void setTbInterface(TBAnalogInterface *tb) {fTB = tb;}
  void setControlNetwork(TestControlNetwork *tcn) {fCN = tcn;}

  UsbDaq*             getDAQ() {return fpDAQ;}
  TBAnalogInterface*  getTbInterface(int n) {return fTB;}
  TestControlNetwork* getControlNetwork(int n) {return fCN;}

  // -- Slots 
  void doStart();
  void doStop();
  void doBreak();
  void doExit();

  void doDraw();
  void readout(FILE *file, unsigned int filledMem);

   void doVdown(int V);
    void doVup(int V); 
   
  void doDuration();
  void doAskTemperature();
  void doSetLogText();
  void doRunNumberUpdate();
  void doOutputDirUpdate();
  void doSetExternal();
  void doSetLocal();
  void doMtbLogging();
  void doMeasureTemperature(); 
  void doSetParameter();
  void doSetTbParameter();
  void doSetManualControlParameter();
  void doSetSysCommand1Text();
  void doFillMem();
  void doPON();
  void doPOFF();
  void doHVON();
  void doHVOFF();
  void doAction();

  int                  fLocalTrigger;
  int                  fExternalTrigger;

private: 

  int fRunning;

  UsbDaq              *fpDAQ;  //! do not save to file else there are 
  daqLoggingManager   *fpLM;   //! do not save to file else there are 
  TBAnalogInterface   *fTB; //! problems with dictionary creation ...
  TestControlNetwork  *fCN; //! (note that //! is a magic comment)

  static const int dataBuffer_numWords = 30000000; 
  unsigned int dataBuffer_fpga1;
  
  SysCommand          *fpSysCommand1;

  CommandLineInterpreter *fInterpreter;

  TGTransientFrame    *fTempMain;

  TGComboBox          *fParametersComboBox, *fTbParametersComboBox, 
                      *fManualControlComboBox, *fActionComboBox;

  TGTextBuffer        *fModuleTextBuffer, *fRocTextBuffer,  
                      *fColTextBuffer, *fRowTextBuffer,  
                      *fParameterTextBuffer, *fTbParameterTextBuffer,
                      *fManualControlTextBuffer,
                      *fRunTextBuffer, *fwOutputDirBuffer,
                      *fwDurationBuffer, *fwOrbitBuffer, *fwScalerBuffer, *fwRateBuffer, *fLogText, *fTemperatureBuffer,
                      *fwSysCommand1Text, *fwSysCommand2Text,
                      *fwMemMtbBuffer, *fwMemRtbBuffer;
  TGLabel             *fwSysCommand1Target, *fwSysCommand2Target;

  TGTextEntry         *fwRunNumber, *fwOutputDir, *fwDuration, *fwMemMtb;
  TCanvas             *fCanvas1;

  TString              fTbParNames[256];
  
  int                  fRunDuration, fFillMem;
  int                  fMtbLogging, fTemperature;

  int fReg21;
  int fReg41;
  int fReg43;
  int fReg47;
  
  int vtrim[16], vthrcomp[16];

  const TGWindow *fpWindow;

  ClassDef(daqFrame,1)
}; 

#endif

