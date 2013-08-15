#ifndef DAQDACSETTINGS
#define DAQDACSETTINGS

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

#define MAXDACSETTINGS 13


class daqFrame;

class daqDACSettings {

public:
    daqDACSettings(const TGWindow * p, const TGWindow * main, int tbNumber);
    virtual ~daqDACSettings();

    // slots
    void CloseWindow();
    void Set();
    void DoOK();
    int  ReadFile();


private:

    void readInteger(int &reg, int &value, FILE * file);
    void setParameter(int reg, int value);
    void setParameter(const char * dacName, int value);

    int                  fTB;
    daqFrame      *      fDAQF;

    TGTransientFrame   *  fMain;
    TGVerticalFrame   *   fF1, *fF2;
    TGLayoutHints    *    fL1;
    TGLayoutHints    *    fL2;
    TGLayoutHints    *    fL3;
    TGButton       *      fSetButton;
    TGButton       *      fExitButton;

    static const int     fNDACParameters = 256;
    TGHorizontalFrame  *  fF[fNDACParameters];
    TGLabel       *       fLabel[fNDACParameters];
    TGNumberEntry    *    fNumericEntries[fNDACParameters];
    int                  fParIndex[fNDACParameters];
    int                  fParValues[fNDACParameters];
    TString              fParNames[fNDACParameters];
    int                  fParMax;

    TGTextBuffer     *    fFileNameBuffer;
    FILE         *        fFile;


};


#endif

