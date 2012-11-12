
#include <iostream>
#include <string>

#include "TROOT.h"
#include "TApplication.h"
#include "TGProgressBar.h"
#include "TGComboBox.h"
#include "TFile.h"
#include "TTree.h"
#include "TBranch.h"

#include <TSystem.h>
#include "TCanvas.h"
#include "TRootEmbeddedCanvas.h"
#include "TRandom.h"
#include "TH1.h"
#include "TF1.h"
#include "TLatex.h"

#include <TGButton.h>
#include <TGButtonGroup.h>
#include <TGFrame.h>
#include <TGTextBuffer.h>
#include <TGTextEntry.h>
#include <TGComboBox.h>
#include <TGMenu.h>
#include <TGLabel.h>

/* Workaround to avoid an error with rootcint */
#ifndef __CINT__
#include "BasePixel/psi46_tbts.h"
#else
class CTestboard_TS;
#endif

struct t_event{
    int run;
    int event;
    int Thresh;
    float VA;
    float VD;
    float IA;
    float ID;
    UShort_t RawData[4096];
};

class CTelescopeFrame: public TGMainFrame 
{

public:
	CTelescopeFrame(const TGWindow *p);


  void doGetRaw();
  void doStart();
  void doStop();
  void doExit();
  void doClear();
  void setPath();
  void setPedestal();
  void setMemSize();
  void setVA();
  void setVD();
  void OpenData(const char* path);
  void GetData(unsigned short* buffer);
  int  ParseData(unsigned short* buffer);
  void SetTB(CTestboard_TS* pTB){ts = pTB;std::cout << "ts = " << ts << std::endl;}
  void WriteRecord();
  void InitTree();
  void FillHistogram(unsigned short *buffer,int iStart);
  void doPon();
  void setClock();
  void UpdateStatus();
  int  getCurrentRun();
  void SetRunNumber(int iRun);
  void ClearHistograms();
  void doCalPulse();
  void doIntTrig();
  void doCalStart();
  void setLatchDelay();
  void SelectCanvas(Int_t event,Int_t i1 ,Int_t i2 ,TObject* gTQSender);
  short ConvertTwos(unsigned short value);
  void LoadPedestals();
private:

  TGTransientFrame    *fTempMain;

  TGComboBox          *fParametersComboBox, *fTbParametersComboBox,
                      *fManualControlComboBox, *fActionComboBox;

  TGTextBuffer        *fModuleTextBuffer, *fRocTextBuffer, *fVABuffer, *fIABuffer,*fVDBuffer,*fIDBuffer,
                      *fColTextBuffer, *fRowTextBuffer,*fDelayBuffer,*fTimeDelayBuffer,
                      *fParameterTextBuffer, *fTbParameterTextBuffer,
                      *fManualControlTextBuffer,
                      *fRunTextBuffer, *fwOutputDirBuffer, *fwPedestalFileBuffer,
                      *fwMemSizeBuffer, *fwOrbitBuffer, *fwScalerBuffer, *fwRateBuffer, *fLogText, *fTemperatureBuffer, *fwSetClockBuffer,
                      *fwSysCommand1Text, *fwSysCommand2Text,
                      *fwMemMtbBuffer, *fwMemRtbBuffer, *fEvtTextBuffer;
  TGLabel             *fwSysCommand1Target, *fwSysCommand2Target;

  TGTextEntry         *fwRunNumber, *fwOutputDir, *fwMemSize, *fwMemMtb, *fwEvtNumber,
			 *fwSetClock, *fwVA, *fwIA, *fwVD,*fwID,*fwSetDelay,*fwSetTimeDelay, *fwPedestalFile;
  TCanvas             *fCanvas1;	

  CTestboard_TS       *ts; //! 

  TFile 	      *fp;

  int		      fMemSize,fRun;

  t_event 	      event;

  TTree		      *tree;
  bool 		      bRunning;

  TGButton	      *fwStatus7,*fwStatus6, *fwInternalTrigger, *fwCalPulse;

  TRootEmbeddedCanvas *canvas0,*canvas1,*canvas2,*canvas3,*canvas4,*canvas5,*canvas6,*canvas7;

  TH1F		      *hSensor0,*hSensor1,*hSensor2,*hSensor3,*hSensor4,
		      *hSensor5, *hSensor6, *hSensor7,*hPed0,*hPed1,*hPed2,*hPed3,*hPed4,*hPed5,*hPed6,*hPed7;

  std::string	      fPath,fPedestals;

  Pixel_t colWhite;
  Pixel_t colGreen;
  Pixel_t colBlue;
  Pixel_t colRed;
  Pixel_t colCyan;
  Pixel_t colLightBlue;
  Pixel_t colBlack;
  Pixel_t colPink;
  Pixel_t colDarkSeaGreen;
  Pixel_t colDarkOliveGreen;

  ClassDef(CTelescopeFrame,1)
};

