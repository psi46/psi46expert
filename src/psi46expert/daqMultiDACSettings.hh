#ifndef DAQMULTIDACSETTINGS
#define DAQMULTIDACSETTINGS

#include <stdio.h>

#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGTab.h>
#include <TGTextEdit.h>
#include <TGResourcePool.h>
#include <TString.h>

#define MAXDACSETTINGS 13


class daqMultiDACSettings {

public:
  daqMultiDACSettings(const TGWindow *p, const TGWindow *main);
  virtual ~daqMultiDACSettings();
  
  // slots
  void CloseWindow();
  void Set();
  void DoOK();
  
  
private:

  TGTab                *fMain;

  //  TGCompositeFrame     *fF[16];

  TGLayoutHints        *fL1;
  TGLayoutHints        *fL2;
  TGLayoutHints        *fL3;
  TGTextEntry          *fTxt1, *fTxt2;
  TGButton             *fSetButton;
  TGButton             *fExitButton;


  static const int     fNDACParameters = 256;

  TGCompositeFrame    *fF[16];
  TGLabel              *fLabel[16][fNDACParameters];
  TGNumberEntry        *fNumericEntries[16][fNDACParameters];
  int                  fParIndex[16][fNDACParameters];
  int                  fParValues[16][fNDACParameters];
  TString              fParNames[16][fNDACParameters];

  TGTextBuffer         *fFileNameBuffer[16];
  FILE                 *fFile[16];

};


#endif

