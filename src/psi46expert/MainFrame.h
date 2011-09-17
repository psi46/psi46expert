// The GUI

#ifndef MAINFRAME
#define MAINFRAME

#include <TGButton.h>
#include <TGButtonGroup.h>
#include <TGFrame.h>
#include <TGMenu.h>
#include <TGLayout.h>
#include <TGTextEntry.h>
#include <TGTextView.h>
#include <TGTextBuffer.h>
#include <TGProgressBar.h>
#include <TGComboBox.h>
#include <TRootEmbeddedCanvas.h>
#include <TGLabel.h>
#include <TObject.h>
#include <TObjArray.h>
#include <TList.h>
#include <TF1.h>
#include <TGraph.h>

#include "TestControlNetwork.h"
#include "CommandLineInterpreter.h"
#include "BasePixel/ConfigParameters.h"
#include "BasePixel/TBInterface.h"
#include "Analysis.h"
#include "BasePixel/SysCommand.h"
#include "TestRange.h"
#include "Test.h"

class MainFrame: public TGMainFrame
{
  public:
    MainFrame( const TGWindow *p, UInt_t w,UInt_t h,
               TBInterface *aTbInterface, TestControlNetwork *aCN, ConfigParameters  *configParameters, bool openWindow = true);
    virtual ~MainFrame();

    void Execute(SysCommand command);

    // == General Actions =============================================

    void SetParameter();
    void Pon();
    void Poff();
    void HVon();
    void HVoff();
    void Exit();
    void Save();
    
    TestRange *GetRange(bool completeRocs = false);

    // == Tests ================================================

    void DoTest();
    void DoTest(Test *aTest);
    void PreTest();
    void IV();
    void DoFullTest();
    void DoTrim();
    void DoPhCal();
    void DoUbCheck();
    void DoFom();
    void DoADC();
    void TestN();

    // == Histograms ===========================================

    void Draw();
    void DrawUpdate();
    void DrawAuto();
    void DrawLines();
    void DrawPoints();
    void DrawColz();
    void DrawStatistics();
    void DrawOverlay();

    void Clear();
    void ClearCanvas();
    void AddLast(TH1* histo, int test);

    void NextHistogram();
    void PreviousHistogram();
    
    void MapFullRange();
    void ClearOldHistos();
    
    void DoTextField();

  private:
    TRootEmbeddedCanvas *fEcanvas;
    TGButtonGroup       *fFctButtons;
    TGRadioButton       *fR[4];
    TGCheckButton       *fOverlayButton;

    TGTextBuffer        *moduleTextBuffer;
    TGTextBuffer        *rocTextBuffer;
    TGTextBuffer        *colTextBuffer;
    TGTextBuffer        *rowTextBuffer;
    TGTextBuffer        *parameterTextBuffer;

    TGTextView          *transcript;
    TGTextEntry         *commandLine;

    TGHProgressBar      *progressBar;

    TGComboBox          *parametersComboBox;
    TGComboBox          *histogramsComboBox;
    TGComboBox          *dac1ComboBox;
    TGComboBox          *dac2ComboBox;
    TGComboBox          *criteriaComboBox;

    CommandLineInterpreter *fInterpreter;
    SysCommand sysCommand;

    // == Tests ===========================================================================

    static const Int_t nTests = 21;

    TList *histograms[nTests + 1];
    Int_t currentTest;
    TObject *currentHistogram[nTests + 1];

    int  fOverlay;
    int  drawOption;
    bool drawStatistics;
    bool clearOldHistos;
    bool mapFullRange;

    bool test[nTests];

    TBInterface        *tbInterface;
    TestControlNetwork *controlNetwork;
    Analysis           *fAnalysis;
    ConfigParameters   *configParameters;

    ClassDef(MainFrame,1)
};

#endif
