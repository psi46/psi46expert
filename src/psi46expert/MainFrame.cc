#include <string>

#include <TROOT.h>
#include <TApplication.h>
#include <TCanvas.h>
#include <TRandom.h>
#include <TFile.h>
#include <TStyle.h>
#include <TGMsgBox.h>

#include "BasePixel/TBAnalogInterface.h"
#include "interface/Delay.h"
#include "interface/Log.h"
#include "MainFrame.h"
#include "Test.h"
#include "PixelAlive.h"
#include "BumpBonding.h"
#include "AddressLevels.h"
#include "AddressDecoding.h"
#include "Trim.h"
#include "TrimLow.h"
#include "TrimVcal.h"
#include "FullTest.h"
#include "PHCalibration.h"
#include "UbCheck.h"
#include "TestRange.h"
#include "SCurveTest.h"
#include "SCurveTestBeam.h"
#include "DacDependency.h"
#include "PHTest.h"
#include "PhDacScan.h"
#include "OffsetOptimization.h"
#include "VhldDelOptimization.h"
#include "ChipVariation.h"
#include "FigureOfMerit.h"
#include "TimeWalkStudy.h"
#include "PhDacOverview.h"
#include "DacOverview.h"
#include "TrimBits.h"
#include "ThresholdTest.h"
#include "IVCurve.h"
#include "TBMTest.h"
#include "CalDelay.h"
#include "ThrComp.h"
#include "TemperatureTest.h"
#include "TemperatureCalibration.h"
#include "VsfOptimization.h"
#include "VsfScan.h"
#include "Xray.h"
#include "HighRateEfficiency.h"
#include "HighRatePixelMap.h"
#include "HighRateTrimLow.h"
#include "HighRateSCurve.h"

ClassImp(MainFrame)

// --- Building the GUI -------------------------------------------------------------------
MainFrame::MainFrame(const TGWindow * p, UInt_t w, UInt_t h,
                     TBInterface * aTBInterface, TestControlNetwork * aControlNetwork,
                     ConfigParameters * aConfigParameters, bool openWindow)
    : TGMainFrame(p, w, h)
{
    // == Initialization ===========================================================================
    tbInterface = aTBInterface;
    controlNetwork = aControlNetwork;
    configParameters = aConfigParameters;
    configParameters->guiMode = true;

    fInterpreter = new CommandLineInterpreter;
    fAnalysis = new Analysis;

    gStyle->SetPalette(1, 0);
    fOverlay = 0;
    drawOption = -1;
    drawStatistics = false;

    clearOldHistos = true;
    mapFullRange = true;

    for (int i = 0; i < nTests + 1; i++)
    {
        histograms[i] = new TList;
        currentHistogram[i] = 0;
    }
    for (int i = 0; i < nTests; i++) {test[i] = false;}
    currentTest = nTests;

    //  -- Create function display canvas

    fEcanvas = new TRootEmbeddedCanvas("Ecanvas", this, w, h);
    fEcanvas->GetCanvas()->ToggleEventStatus();
    AddFrame(fEcanvas, new TGLayoutHints(kLHintsExpandX | kLHintsExpandY, 10, 10, 10, 1));

    // == Navigation frame ========================================================================

    TGHorizontalFrame * histoNavigationFrame = new TGHorizontalFrame(this, w, 40);

    histogramsComboBox = new TGComboBox(histoNavigationFrame, 100);
    histogramsComboBox->Resize(100, 20);
    histoNavigationFrame->AddFrame(histogramsComboBox, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    TGTextButton * previous = new TGTextButton(histoNavigationFrame, "&Previous");
    previous->Connect("Clicked()", "MainFrame", this, "PreviousHistogram()");
    histoNavigationFrame->AddFrame(previous, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    TGTextButton * next = new TGTextButton(histoNavigationFrame, "&Next");
    next->Connect("Clicked()", "MainFrame", this, "NextHistogram()");
    histoNavigationFrame->AddFrame(next, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    TGTextButton * update = new TGTextButton(histoNavigationFrame, "&Update");
    update->Connect("Clicked()", "MainFrame", this, "DrawUpdate()");
    histoNavigationFrame->AddFrame(update, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    TGTextButton * clear = new TGTextButton(histoNavigationFrame, "&Clear");
    clear->Connect("Clicked()", "MainFrame", this, "Clear()");
    histoNavigationFrame->AddFrame(clear, new TGLayoutHints(kLHintsCenterX | kLHintsCenterY, 5, 5, 3, 4));

    AddFrame(histoNavigationFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    // == Draw Options ==============================================================================

    TGHorizontalFrame * drawOptionsFrame = new TGHorizontalFrame(this, w, 40);

    fFctButtons = new TGButtonGroup(drawOptionsFrame, "Draw Options", kHorizontalFrame);
    fR[0] = new TGRadioButton(fFctButtons, new TGHotString("&Auto"));
    fR[0]->Connect("Clicked()", "MainFrame", this, "DrawAuto()");
    fR[1] = new TGRadioButton(fFctButtons, new TGHotString("&Lines"));
    fR[1]->Connect("Clicked()", "MainFrame", this, "DrawLines()");
    fR[2] = new TGRadioButton(fFctButtons, new TGHotString("&Points"));
    fR[2]->Connect("Clicked()", "MainFrame", this, "DrawPoints()");
    fR[3] = new TGRadioButton(fFctButtons, new TGHotString("&Colz"));
    fR[3]->Connect("Clicked()", "MainFrame", this, "DrawColz()");

    drawOptionsFrame->AddFrame(fFctButtons, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    //  TGCheckButton *overlayButton = new TGCheckButton(drawOptionsFrame, "Overlay", 100);
    //  overlayButton->Connect("Clicked()", "MainFrame", this, "DrawOverlay()");
    //  drawOptionsFrame->AddFrame(overlayButton, new TGLayoutHints(kLHintsRight | kLHintsCenterY , 5, 5, 3, 4));

    TGCheckButton * statisticsButton = new TGCheckButton(drawOptionsFrame, "Stats", 101);
    statisticsButton->Connect("Clicked()", "MainFrame", this, "DrawStatistics()");
    drawOptionsFrame->AddFrame(statisticsButton, new TGLayoutHints(kLHintsRight | kLHintsCenterY , 5, 5, 3, 4));

    TGCheckButton * clearOldHistosButton = new TGCheckButton(drawOptionsFrame, "Clear", 100);
    clearOldHistosButton->Connect("Clicked()", "MainFrame", this, "ClearOldHistos()");
    clearOldHistosButton->SetOn();
    drawOptionsFrame->AddFrame(clearOldHistosButton, new TGLayoutHints(kLHintsRight | kLHintsCenterY , 5, 5, 3, 4));

    TGCheckButton * mapFullRangeButton = new TGCheckButton(drawOptionsFrame, "fullRange", 100);
    mapFullRangeButton->Connect("Clicked()", "MainFrame", this, "MapFullRange()");
    mapFullRangeButton->SetOn();
    drawOptionsFrame->AddFrame(mapFullRangeButton, new TGLayoutHints(kLHintsRight | kLHintsCenterY , 5, 5, 3, 4));


    AddFrame(drawOptionsFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    // == Actions ==========================================================================================

    TGHorizontalFrame * hframe = new TGHorizontalFrame(this, w, 40);

    TGTextButton * iv = new TGTextButton(hframe, "&IV");
    iv->Connect("Clicked()", "MainFrame", this, "IV()");
    hframe->AddFrame(iv, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * pretest = new TGTextButton(hframe, "&PreTest");
    pretest->Connect("Clicked()", "MainFrame", this, "PreTest()");
    hframe->AddFrame(pretest, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * write = new TGTextButton(hframe, "&FullTest");
    write->Connect("Clicked()", "MainFrame", this, "DoFullTest()");
    hframe->AddFrame(write, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * trim = new TGTextButton(hframe, "&Trim");
    trim->Connect("Clicked()", "MainFrame", this, "DoTrim()");
    hframe->AddFrame(trim, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * phCal = new TGTextButton(hframe, "&PHCal");
    phCal->Connect("Clicked()", "MainFrame", this, "DoPhCal()");
    hframe->AddFrame(phCal, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * adc = new TGTextButton(hframe, "&ADC");
    adc->Connect("Clicked()", "MainFrame", this, "DoADC()");
    hframe->AddFrame(adc, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    AddFrame(hframe, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    TGHorizontalFrame * hframe2 = new TGHorizontalFrame(this, w, 40);

    TGTextButton * test = new TGTextButton(hframe2, "&Test");
    test->Connect("Clicked()", "MainFrame", this, "DoTest()");
    hframe2->AddFrame(test, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * save = new TGTextButton(hframe2, "&Save");
    save->Connect("Clicked()", "MainFrame", this, "Save()");
    hframe2->AddFrame(save, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * exit = new TGTextButton(hframe2, "&Exit");
    exit->Connect("Clicked()", "MainFrame", this, "Exit()");
    hframe2->AddFrame(exit, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    // Connect Window Close button (or key combo) with Exit slot to properly
    // close application.
    this->Connect("CloseWindow()", "MainFrame", this, "Exit()");

    TGTextButton * pon = new TGTextButton(hframe2, "&PON");
    pon->Connect("Clicked()", "MainFrame", this, "Pon()");
    hframe2->AddFrame(pon, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * poff = new TGTextButton(hframe2, "&POFF");
    poff->Connect("Clicked()", "MainFrame", this, "Poff()");
    hframe2->AddFrame(poff, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * hvon = new TGTextButton(hframe2, "&HVON");
    hvon->Connect("Clicked()", "MainFrame", this, "HVon()");
    hframe2->AddFrame(hvon, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    TGTextButton * hvoff = new TGTextButton(hframe2, "&HVOFF");
    hvoff->Connect("Clicked()", "MainFrame", this, "HVoff()");
    hframe2->AddFrame(hvoff, new TGLayoutHints(kLHintsCenterX, 5, 5, 3, 4));

    AddFrame(hframe2, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));


    // == Tests frame ========================================================================

    TGHorizontalFrame * testFrame = new TGHorizontalFrame(this, w, 40);
    TGHorizontalFrame * testFrame2 = new TGHorizontalFrame(this, w, 40);
    TGHorizontalFrame * testFrame3 = new TGHorizontalFrame(this, w, 40);
    TGHorizontalFrame * testFrame4 = new TGHorizontalFrame(this, w, 40);
    TGHorizontalFrame * frame;

    TGCheckButton * testButton;
    const char * testName;

    const char testNames [nTests][14] = {"PixMap", "DacDac", "SCurve", "BondMap", "TrimBits",
                                         "AdrLev", "PhScan", "PH", "Thr", "AdrDec", "CalDel",
                                         "ThrComp", "Temp", "TempCal", "VsfOpt", "LinRange",
                                         "HldDelOpti", "TimeWalk", "Xray", "VsfScan", "AdrDec2",
                                         "HR PixelMap", "HR Efficiency", "HR SCurve"};

    for (int i = nTests - 1; i >= 0; i--)
    {
        testName = testNames[i];

        if (i < 6) frame = testFrame;
        else if (i < 13) frame = testFrame2;
        else if (i < 21) frame = testFrame3;
        else frame = testFrame4;

        testButton = new TGCheckButton(frame, testName, i);
        testButton->Connect("Clicked()", "MainFrame", this, "TestN()");
        testButton->SetState(kButtonUp);
        frame->AddFrame(testButton, new TGLayoutHints(kLHintsRight, 5, 5, 3, 4));

        histogramsComboBox->AddEntry(testName, i);
    }
    histogramsComboBox->AddEntry("All", nTests);
    histogramsComboBox->Select(nTests);

    AddFrame(testFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
    AddFrame(testFrame2, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
    AddFrame(testFrame3, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));
    AddFrame(testFrame4, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    // == Parameters ========================================================================================

    TGHorizontalFrame * parametersFrame = new TGHorizontalFrame(this, w, 80);

    parametersComboBox = new TGComboBox(parametersFrame, 100);
    DACParameters * dacParameters = new DACParameters();
    char * name;
    for (Int_t reg = 0; reg < 256; reg++)
    {
        name = dacParameters->GetName(reg);
        if (strcmp(name, "") != 0)
        {
            parametersComboBox->AddEntry(name, reg);
        }
    }
    parametersComboBox->AddEntry("Trim" , 256);
    parametersComboBox->AddEntry("Inputbias" , 257);
    parametersComboBox->AddEntry("Outputbias" , 258);
    parametersComboBox->AddEntry("Dacgain" , 259);

    parametersComboBox->Resize(100, 20);
    parametersComboBox->Select(2);

    parametersFrame->AddFrame(parametersComboBox, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

    TGTextEntry * parameterValue = new TGTextEntry(parametersFrame, parameterTextBuffer = new TGTextBuffer(100));
    parameterValue->Resize(150, parameterValue->GetDefaultHeight());
    parameterValue->SetText("");
    parametersFrame->AddFrame(parameterValue, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    TGTextButton * setParameter = new TGTextButton(parametersFrame, "&Set");
    setParameter->Connect("Clicked()", "MainFrame", this, "SetParameter()");
    parametersFrame->AddFrame(setParameter, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

    AddFrame(parametersFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));


    // == figure of merit  ========================================================================================

    TGHorizontalFrame * fomFrame = new TGHorizontalFrame(this, w, 20);

    dac1ComboBox = new TGComboBox(fomFrame, 120);
    for (Int_t reg = 0; reg < 256; reg++)
    {
        name = dacParameters->GetName(reg);
        if (strcmp(name, "") != 0) dac1ComboBox->AddEntry(name, reg);
    }
    dac1ComboBox->Resize(100, 20);
    dac1ComboBox->Select(17);
    fomFrame->AddFrame(dac1ComboBox, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

    dac2ComboBox = new TGComboBox(fomFrame, 120);
    for (Int_t reg = 0; reg < 256; reg++)
    {
        name = dacParameters->GetName(reg);
        if (strcmp(name, "") != 0) dac2ComboBox->AddEntry(name, reg);
    }
    dac2ComboBox->Resize(100, 20);
    dac2ComboBox->Select(15);
    fomFrame->AddFrame(dac2ComboBox, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

    criteriaComboBox = new TGComboBox(fomFrame, 100);
    criteriaComboBox->AddEntry("Timewalk" , 0);
    criteriaComboBox->AddEntry("Linearity (high range)" , 1);
    criteriaComboBox->AddEntry("Pulse Height" , 2);
    criteriaComboBox->AddEntry("Curvature (low range)" , 3);
    criteriaComboBox->AddEntry("Threshold (Vcal)" , 4);
    criteriaComboBox->Resize(180, 20);
    criteriaComboBox->Select(3);
    fomFrame->AddFrame(criteriaComboBox, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

    TGTextButton * startFom = new TGTextButton(fomFrame, "&FOM");
    startFom->Connect("Clicked()", "MainFrame", this, "DoFom()");
    fomFrame->AddFrame(startFom, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));


    AddFrame(fomFrame, new TGLayoutHints(kLHintsCenterX, 2, 2, 2, 2));

    // == Module / Roc / Column / Row Input =================================================================================

    TGHorizontalFrame * inputFrame = new TGHorizontalFrame(this, w, 60);

    TGLabel * moduleLabel = new TGLabel(inputFrame, "Mod ");
    inputFrame->AddFrame(moduleLabel, new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 2, 6, 2));

    TGTextEntry * module = new TGTextEntry(inputFrame, moduleTextBuffer = new TGTextBuffer(80));
    module->Resize(100, module->GetDefaultHeight());
    module->SetText("0");
    inputFrame->AddFrame(module, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    TGLabel * rocLabel = new TGLabel(inputFrame, "Roc ");
    inputFrame->AddFrame(rocLabel, new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 2, 6, 2));

    TGTextEntry * roc = new TGTextEntry(inputFrame, rocTextBuffer = new TGTextBuffer(80));
    roc->Resize(100, roc->GetDefaultHeight());
    roc->SetText("0");
    inputFrame->AddFrame(roc, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    TGLabel * colLabel = new TGLabel(inputFrame, "Col ");
    inputFrame->AddFrame(colLabel, new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 2, 6, 2));

    TGTextEntry * col = new TGTextEntry(inputFrame, colTextBuffer = new TGTextBuffer(80));
    col->Resize(100, col->GetDefaultHeight());
    col->SetText("5");
    inputFrame->AddFrame(col, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    TGLabel * rowLabel = new TGLabel(inputFrame, "Row");
    inputFrame->AddFrame(rowLabel, new TGLayoutHints(kLHintsTop | kLHintsLeft, 10, 2, 6, 2));

    TGTextEntry * row = new TGTextEntry(inputFrame, rowTextBuffer = new TGTextBuffer(80));
    row->Resize(100, row->GetDefaultHeight());
    row->SetText("5");
    inputFrame->AddFrame(row, new TGLayoutHints(kLHintsExpandX, 2, 10, 2, 2));

    AddFrame(inputFrame, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    // == Transcript ============================================================================================

    TGHorizontalFrame * textOutputFrame = new TGHorizontalFrame(this, w, 100);
    transcript = new TGTextView(textOutputFrame, w, 100);
    textOutputFrame->AddFrame(transcript, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    AddFrame(textOutputFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 2, 2));


    // == Command Line =============================================================================================

    TGHorizontalFrame * cmdLineFrame = new TGHorizontalFrame(this, w, 100);
    commandLine = new TGTextEntry(cmdLineFrame, new TGTextBuffer(30), 100);
    commandLine->Connect("ReturnPressed()", "MainFrame", this, "DoTextField()");
    cmdLineFrame->AddFrame(commandLine, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    AddFrame(cmdLineFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 2, 2));

    // == Progress Bar ==========================================================================================

    TGHorizontalFrame * progressFrame = new TGHorizontalFrame(this, w, 60);
    progressBar = new TGHProgressBar(progressFrame, TGProgressBar::kStandard, 100);
    progressBar->SetBarColor("yellow");
    progressBar->SetRange(0., 1.);
    progressFrame->AddFrame(progressBar, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));
    AddFrame(progressFrame, new TGLayoutHints(kLHintsExpandX, 10, 10, 2, 2));

    //  == Main frame =============================================================================================

    if (openWindow)
    {
        SetWindowName("psi46expert");
        transcript->AddLine("Welcome to psi46expert!");
        MapSubwindows();
        Resize(GetDefaultSize());
        MapWindow();
    }
}


MainFrame::~MainFrame()
{}


// == Actions ==============================================================================


// Sets DAC parameters
void MainFrame::SetParameter()
{
    bool inputOk = true;
    int dacValue, reg, rocMin, rocMax, moduleMin, moduleMax, colMin, colMax, rowMin, rowMax;
    const char * name;

    fInterpreter->SetString(parameterTextBuffer->GetString());
    if (!fInterpreter->GetInt(dacValue, 0, 255)) {inputOk = false;}

    fInterpreter->SetString(colTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(colMin, colMax, 0, ROCNUMCOLS)) {inputOk = false;}

    fInterpreter->SetString(rowTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(rowMin, rowMax, 0, ROCNUMROWS)) {inputOk = false;}

    fInterpreter->SetString(rocTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(rocMin, rocMax, 0, 15)) {inputOk = false;}

    fInterpreter->SetString(moduleTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(moduleMin, moduleMax, 0, 1)) {inputOk = false;}

    reg = parametersComboBox->GetSelected();
    if (reg < 0 || reg > 259) {inputOk = false;}

    if (inputOk)
    {
        if (reg != 256) name = (new DACParameters())->GetName(reg);
        for (int iModule = moduleMin; iModule <= moduleMax; iModule++)
        {
            for (int iRoc = rocMin; iRoc <= rocMax; iRoc++)
            {
                if (reg < 256)  controlNetwork->SetDAC(iModule, iRoc, reg, dacValue);
                else if (reg == 256)
                {
                    name = "trim";
                    for (int iCol = colMin; iCol <= colMax; iCol++)
                    {
                        for (int iRow = rowMin; iRow <= rowMax; iRow++) controlNetwork->SetTrim(iModule, iRoc, iCol, iRow, dacValue);
                    }

                }
                else if (reg == 257)
                {
                    name = "inputbias";
                    reg = 2;
                    controlNetwork->SetTBM(iModule, iRoc, reg, dacValue);
                }
                else if (reg == 258)
                {
                    name = "outputbias";
                    reg = 3;
                    controlNetwork->SetTBM(iModule, iRoc, reg, dacValue);
                }
                else if (reg == 259)
                {
                    name = "dacgain";
                    reg = 4;
                    controlNetwork->SetTBM(iModule, iRoc, reg, dacValue);
                }
            }
        }
        transcript->AddLine(Form("Parameter %s set to %i", name, dacValue));
    }
    else {transcript->AddLine("Invalid Value");}
}


void MainFrame::Pon()
{
    tbInterface->Pon();
}


void MainFrame::Poff()
{
    tbInterface->Poff();
}


// -- Turn high voltage on
void MainFrame::HVon()
{
    if (tbInterface->IsAnalogTB())
    {
        ((TBAnalogInterface *)tbInterface)->HVon();
        tbInterface->Flush();
    }
}


// -- Turn high voltage off
void MainFrame::HVoff()
{
    if (tbInterface->IsAnalogTB())
    {
        ((TBAnalogInterface *)tbInterface)->HVoff();
        tbInterface->Flush();
    }
}


void MainFrame::Exit()
{
    ((TBAnalogInterface *)tbInterface)->HVoff();
    tbInterface->Flush();
    tbInterface->Poff();
    tbInterface->Cleanup();

    gFile->Write();
    gFile->Close();

    gApplication->Terminate(0);
}

void MainFrame::Save()
{
    // std::cout << "Folder: " << ConfigParameters::Singleton()->directory << std::endl;
    controlNetwork->WriteDACParameterFile(std::string(ConfigParameters::Singleton()->directory).append("module").c_str());
    //  controlNetwork->WriteTrimConfiguration("trim");
}


void MainFrame::Draw()
{
    TString * option = new TString("");

    if ((currentHistogram[currentTest] == 0) && (histograms[currentTest]->GetSize() == 0)) {return;}
    if (currentHistogram[currentTest] == 0) {currentHistogram[currentTest] = histograms[currentTest]->First();}

    ClearCanvas();
    TCanvas * canvas = fEcanvas->GetCanvas();

    if (strcmp(currentHistogram[currentTest]->ClassName(), "TGraph") == 0)
    {
        option->Append("A*");
        //option->Append("L");
    }

    if (currentHistogram[currentTest]->InheritsFrom("TH1"))
    {
        if (drawStatistics) {((TH1 *)currentHistogram[currentTest])->SetStats(kTRUE);}
        else {((TH1 *)currentHistogram[currentTest])->SetStats(kFALSE);}
    }

    if (drawOption == -1)
    {
        if (currentHistogram[currentTest]->InheritsFrom("TH2")) {option->Append("colz");}
    }

    if (fOverlay) {option->Append("same");}
    if (drawOption == 1) {option->Append("P");}
    if (drawOption == 2) {option->Append("colz");}

    currentHistogram[currentTest]->DrawClone(option->Data());

    canvas->cd();
    canvas->Update();
}


// == Tests =======================================================================================


// -- A check box was clicked
void MainFrame::TestN()
{
    TGCheckButton * button = (TGCheckButton *)gTQSender;
    Int_t id = button->WidgetId();
    test[id] = !test[id];
}


void MainFrame::PreTest()
{
    controlNetwork->AdjustDACParameters();
}


void MainFrame::IV()
{
    TestRange * range = new TestRange();
    Test * aTest = new IVCurve(range, controlNetwork->GetTestParameters(), tbInterface);
    DoTest(aTest);
}


void MainFrame::DoFullTest()
{
    TestRange * range = GetRange(mapFullRange);
    Test * aTest = new FullTest(configParameters, range, controlNetwork->GetTestParameters(), tbInterface, 1);
    DoTest(aTest);
}


void MainFrame::DoTest(Test * aTest)
{
    psi::LogInfo() << psi::endl << "[MainFrame] Starting test ..." << psi::endl;
    controlNetwork->GetTestParameters()->ReadTestParameterFile(configParameters->GetTestParametersFileName());

    if (clearOldHistos) Clear();

    gDelay->Timestamp();
    aTest->ControlNetworkAction(controlNetwork);
    gDelay->Timestamp();

    TIter next(aTest->GetHistos());
    while (TH1 * histo = (TH1 *)next()) histograms[nTests]->AddLast(histo);

    DrawUpdate();
    psi::LogInfo() << "[MainFrame] Test finished." << psi::endl;
}




// -- Perform the selected tests
void MainFrame::DoTest()
{
    psi::LogInfo() << psi::endl << "[MainFrame] Starting tests ..." << psi::endl;
    controlNetwork->GetTestParameters()->ReadTestParameterFile(configParameters->GetTestParametersFileName());
    gDelay->Timestamp();

    if (clearOldHistos) Clear();
    progressBar->Reset();
    progressBar->ShowPosition(true);
    TCanvas * canvas = fEcanvas->GetCanvas();
    canvas->cd();
    canvas->Update();

    TestRange * testRange = GetRange();
    TestRange * testRangeFull = testRange;
    if (mapFullRange) testRangeFull = GetRange(true);

    if (!(strcmp(configParameters->GetMaskFileName(), "default") == 0)) {
        testRangeFull->ApplyMaskFile(configParameters->GetMaskFileName());
    }

    Test * testToDo;

    for (int iTest = 0; iTest < nTests; iTest++) {
        if (!test[iTest])
            continue;

        switch (iTest) {
        case 0 : testToDo = new PixelAlive(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 1 : testToDo = new DacDependency(testRange, controlNetwork->GetTestParameters(), tbInterface); break;
        case 2 : testToDo = new SCurveTest(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 3 : testToDo = new BumpBonding(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 4 : testToDo = new TrimBits(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 5 : testToDo = new AddressLevels(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 6 : testToDo = new PhDacOverview(testRange, controlNetwork->GetTestParameters(), tbInterface); break;
        case 7 : testToDo = new PHTest(testRange, controlNetwork->GetTestParameters(), tbInterface); break;
        case 8 : testToDo = new ThresholdTest(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 9 : testToDo = new AddressDecoding(testRangeFull, controlNetwork->GetTestParameters(), tbInterface, false); break;
        case 10: testToDo = new CalDelay(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 11: testToDo = new ThrComp(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 12: testToDo = new TemperatureTest(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 13: testToDo = new TemperatureCalibration(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 14: testToDo = new VsfOptimization(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 15: testToDo = new ChipVariation(testRange, controlNetwork->GetTestParameters(), tbInterface); break;
        case 16: testToDo = new VhldDelOptimization(testRange, controlNetwork->GetTestParameters(), tbInterface); break;
        case 17: testToDo = new TimeWalkStudy(testRange, controlNetwork->GetTestParameters(), tbInterface); break;
        case 18: testToDo = new Xray(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 19: testToDo = new VsfScan(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 20: testToDo = new AddressDecoding(testRangeFull, controlNetwork->GetTestParameters(), tbInterface, true); break;
        case 21: testToDo = new HRPixelMap(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 22: testToDo = new HREfficiency(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        case 23: testToDo = new HRSCurve(testRange, controlNetwork->GetTestParameters(), tbInterface); break;
            //case 24: testToDo = new HRTrimLow(testRangeFull, controlNetwork->GetTestParameters(), tbInterface); break;
        }

        testToDo->ControlNetworkAction(controlNetwork);
        TIter next(testToDo->GetHistos());
        while (TH1 * histo = (TH1 *)next()) AddLast(histo, iTest);
    }
    DrawUpdate();
    gDelay->Timestamp();
    psi::LogInfo() << "[MainFrame] Tests finished." << psi::endl;
}


// == Histograms =====================================================================


// -- Enable auto draw option
void MainFrame::DrawAuto()
{
    drawOption = -1;
}


// -- Draw histograms in the line mode
void MainFrame::DrawLines()
{
    drawOption = 0;
}


// -- Draw histograms in the point mode
void MainFrame::DrawPoints()
{
    drawOption = 1;
}


// -- Draw (2D) histograms in the colz mode
void MainFrame::DrawColz()
{
    drawOption = 2;
}


// -- Draw the statistic
void MainFrame::DrawStatistics()
{
    drawStatistics = !drawStatistics;
}


// -- Overlay the histograms
void MainFrame::DrawOverlay()
{
    fOverlay = !fOverlay;
}


void MainFrame::MapFullRange()
{
    mapFullRange = !mapFullRange;
}


void MainFrame::ClearOldHistos()
{
    clearOldHistos = !clearOldHistos;
}


// -- Clear all histograms
void MainFrame::Clear()
{
    ClearCanvas();
    for (int i = 0; i < nTests + 1 ; i++)
    {
        delete histograms[i];
        histograms[i] = new TList;
        currentHistogram[i] = 0;
    }
}


// -- Clear the canvas
void MainFrame::ClearCanvas()
{
    TCanvas * c = fEcanvas->GetCanvas();
    c->Clear();
    c->cd();
    c->Update();
}


// -- Update the main canvas
void MainFrame::DrawUpdate()
{
    currentTest = histogramsComboBox->GetSelected();
    if (histograms[currentTest]->GetSize() == 0) {return;}
    if (currentHistogram[currentTest] == 0)
    {
        //      if ((TH1*)histograms[currentTest]->Last() == currentHistogram[currentTest])
        //      {
        currentHistogram[currentTest] = (TH1 *)histograms[currentTest]->First();
        //      }
        //      else
        //      {
        //          currentHistogram[currentTest] = (TH1*)histograms[currentTest]->After(currentHistogram[currentTest]);
        //      }
    }

    Draw();

}


// -- Display the next histogram
void MainFrame::NextHistogram()
{
    currentTest = histogramsComboBox->GetSelected();
    if (histograms[currentTest]->GetSize() == 0) {return;}
    if ((TH1 *)histograms[currentTest]->Last() == currentHistogram[currentTest])
    {
        currentHistogram[currentTest] = (TH1 *)histograms[currentTest]->First();
    }
    else
    {
        currentHistogram[currentTest] = (TH1 *)histograms[currentTest]->After(currentHistogram[currentTest]);
    }

    Draw();

}


// -- Display the previous histogram
void MainFrame::PreviousHistogram()
{
    currentTest = histogramsComboBox->GetSelected();
    if (histograms[currentTest]->GetSize() == 0) {return;}
    if ((TH1 *)histograms[currentTest]->First() == currentHistogram[currentTest])
    {
        currentHistogram[currentTest] = (TH1 *)histograms[currentTest]->Last();
    }
    else
    {
        currentHistogram[currentTest] = (TH1 *)histograms[currentTest]->Before(currentHistogram[currentTest]);
    }

    Draw();

}


// -- Add a histogram in the last position
void MainFrame::AddLast(TH1 * histo, int test)
{
    histograms[test]->AddLast(histo);
    histograms[nTests]->AddLast(histo);
}


// -- A command was entered at the command line
void MainFrame::DoTextField()
{
    char line[100];
    strncpy(line, commandLine->GetText(), 80);
    if (sysCommand.Parse(line))
    {
        Execute(sysCommand);
    }
    commandLine->SetText("");
}


// -- Execute the command line command
void MainFrame::Execute(SysCommand command)
{
    do
    {
        if (command.TargetIsTB()) {tbInterface -> Execute(command);}
        else  {controlNetwork->Execute(command);}
    }
    while (command.Next());
}


void MainFrame::DoTrim()
{
    TestRange * range = GetRange(mapFullRange);

    if (!(strcmp(configParameters->GetMaskFileName(), "default") == 0)) {
        range->ApplyMaskFile(configParameters->GetMaskFileName());
    }

    Test * aTest = new TrimLow(range, controlNetwork->GetTestParameters(), tbInterface);
    DoTest(aTest);

}


void MainFrame::DoPhCal()
{
    TestRange * range = GetRange(true);
    Test * aTest = new PHCalibration(range, controlNetwork->GetTestParameters(), tbInterface);
    DoTest(aTest);

}

void MainFrame::DoUbCheck()
{
    TestRange * range = GetRange(true);
    Test * aTest = new UbCheck(range, controlNetwork->GetTestParameters(), tbInterface);
    DoTest(aTest);

}


void MainFrame::DoFom()
{
    int dac1, dac2, crit;
    dac1 = dac1ComboBox->GetSelected();
    dac2 = dac2ComboBox->GetSelected();
    crit = criteriaComboBox->GetSelected();

    transcript->AddLine("FOM is running.");
    TestRange * range = GetRange();
    Test * aTest = new FigureOfMerit(range, controlNetwork->GetTestParameters(), tbInterface, dac1, dac2, crit);
    DoTest(aTest);
    transcript->AddLine("FOM is finished.");
}


void  MainFrame::DoADC()
{
    unsigned short count;
    short data[FIFOSIZE];
    bool is_analog = controlNetwork->GetModule(0)->GetRoc(0)->has_analog_readout();
    if (is_analog)
        ((TBAnalogInterface *)tbInterface)->ADCRead(data, count);
    else
        ((TBAnalogInterface *)tbInterface)->ADCRead_digital(data, count);

    TH1D * hist = new TH1D("ADC", "ADC", count, 0, count);

    for (unsigned int n = 0; n < count; n++) {
        if (is_analog)
            hist->SetBinContent(n + 1, data[n]);
        else {
            hist->SetBinContent(n + 1, (data[n / 16] & (1 << (16 - n % 16 - 1))) ? 1 : 0);
        }
    }

    if (is_analog) {
        hist->SetMinimum(-2048);
        hist->SetMaximum(+2048);
        hist->GetYaxis()->SetTitle("ADC value");
        hist->GetXaxis()->SetTitle("40 MHz clock cycle");
    } else {
        hist->SetMinimum(-1);
        hist->SetMaximum(+2);
        hist->GetYaxis()->SetTitle("Bit");
        hist->GetXaxis()->SetTitle("160 MHz clock cycle");
    }

    if (clearOldHistos) Clear();
    AddLast(hist, 0);
    DrawUpdate();
}



TestRange * MainFrame::GetRange(bool completeRocs)
{
    bool inputOk = true;
    int moduleMin, moduleMax, rocMin, rocMax, colMin, colMax , rowMin, rowMax;

    // Getting the range for which the tests should be performed
    fInterpreter->SetString(rowTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(rowMin, rowMax, 0, ROCNUMROWS - 1)) {inputOk = false;}

    fInterpreter->SetString(colTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(colMin, colMax, 0, ROCNUMROWS - 1)) {inputOk = false;}

    fInterpreter->SetString(rocTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(rocMin, rocMax, 0, 15)) {inputOk = false;}

    fInterpreter->SetString(moduleTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(moduleMin, moduleMax, 0, 1)) {inputOk = false;}

    TestRange * testRange = new TestRange();

    if (inputOk)
    {
        for (int iModule = moduleMin; iModule <= moduleMax; iModule++)
        {
            for (int iRoc = rocMin; iRoc <= rocMax; iRoc++)
            {
                if (completeRocs) testRange->CompleteRoc(iRoc);
                else
                {
                    for (int iCol = colMin; iCol <= colMax; iCol++)
                    {
                        for (int iRow = rowMin; iRow <= rowMax; iRow++)
                        {
                            testRange->AddPixel(iRoc, iCol, iRow);
                        }
                    }
                }
            }
        }
    }
    return testRange;
}
