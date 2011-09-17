#ifndef DAQTBSETTINGS
#define DAQTBSETTINGS

#include <stdio.h>

#include <TGFrame.h>
#include <TGIcon.h>
#include <TGLabel.h>
#include <TGButton.h>
#include <TGTextEntry.h>
#include <TGNumberEntry.h>
#include <TGMsgBox.h>
#include <TGMenu.h>
#include <TGTextEdit.h>
#include <TGResourcePool.h>
#include <TString.h>


class daqFrame;


#define MAXTBSETTINGS 11

class daqTBSettings {

public:
  daqTBSettings(const TGWindow *p, const TGWindow *main, int tbNumber);
  virtual ~daqTBSettings();
  
  // slots
  void CloseWindow();
  void Set();
  void DoOK();
  int  ReadFile();

  void DoPon();
  void DoPoff();

  void DoHVon();
  void DoHVoff();
  
  
private:

  void readInteger(int &reg, int &value, FILE *file);
  void setParameter(int reg, int value);
  void setParameter(const char* tbName, int value);


  int                  fTB;
  daqFrame*            fDAQF;

  TGTransientFrame     *fMain;
  TGVerticalFrame      *fF1, *fF2;
  TGLayoutHints        *fL1;
  TGLayoutHints        *fL2;
  TGLayoutHints        *fL3;
  TGButton             *fSetButton;
  TGButton             *fExitButton;
  TGButton             *fPonButton,  *fPoffButton;
  TGButton             *fHVonButton, *fHVoffButton;

  static const int     fNTBParameters = 256;
  TGHorizontalFrame    *fF[fNTBParameters];
  TGLabel              *fLabel[fNTBParameters];
  TGNumberEntry        *fNumericEntries[fNTBParameters];
  int                  fParIndex[fNTBParameters];
  int                  fParValues[fNTBParameters];
  TString              fParNames[fNTBParameters];
  int                  fParMax;

  TGTextBuffer         *fFileNameBuffer;
  FILE                 *fFile;

};


#endif

