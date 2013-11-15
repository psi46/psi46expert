#include <iostream>
#include <time.h>
#include <stdlib.h>
#include <BasePixel/Keithley.h>

#include "BasePixel/TBInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "psi46expert/TestControlNetwork.h"

#include "TROOT.h"
#include "TApplication.h"
#include "TGProgressBar.h"
#include "TGComboBox.h"

#include <TSystem.h>
#include "TCanvas.h"
#include "TRandom.h"
#include "TH1.h"
#include "TLatex.h"
#include <fstream>

#include "interface/Log.h"

#include "daqFrame.hh"

ClassImp(daqFrame)


// ----------------------------------------------------------------------
daqFrame::daqFrame(const TGWindow * p, UInt_t w, UInt_t h, daqLoggingManager * pLM, bool batchMode) : TGMainFrame(p, w, h) {
    Power_supply = new  Keithley();
    fpLM     = pLM;
    fInterpreter  = new CommandLineInterpreter();
    fpSysCommand1 = new SysCommand();

    fLocalTrigger    = 0; // run in local CTR mode
    fExternalTrigger = 1; //
    fMtbLogging      = 1;
    fFillMem         = 0;
    fTemperature     = 0;
    fRunDuration     = 3600;
    fRunning         = 0;



    SetLayoutBroken(kTRUE);

    // -- Colors
    Pixel_t colWhite; gClient->GetColorByName("white", colWhite);
    Pixel_t colPink;  gClient->GetColorByName("pink", colPink);
    Pixel_t colBlack; gClient->GetColorByName("black", colBlack);
    Pixel_t colCyan;  gClient->GetColorByName("cyan", colCyan);
    Pixel_t colGreen; gClient->GetColorByName("green", colGreen);
    Pixel_t colRed;   gClient->GetColorByName("red", colRed);
    Pixel_t colBlue;  gClient->GetColorByName("blue", colBlue);
    Pixel_t colLightBlue;       gClient->GetColorByName("LightBlue", colLightBlue);
    Pixel_t colDarkOliveGreen;  gClient->GetColorByName("DarkOliveGreen", colDarkOliveGreen);
    Pixel_t colDarkSeaGreen;    gClient->GetColorByName("DarkSeaGreen", colDarkSeaGreen);

    SetBackgroundColor(colDarkSeaGreen);
  doRefreshWindowTitle();

    // ----------------------------------------------------------------------
    // -- Frame for Run Control
    TGHorizontalFrame * wRunCtrl = new TGHorizontalFrame(this, 500, 210, kHorizontalFrame);
    AddFrame(wRunCtrl);
    wRunCtrl->SetBackgroundColor(colDarkOliveGreen);
    wRunCtrl->DrawBorder();

    fwRunNumber = new TGTextEntry(wRunCtrl, fRunTextBuffer = new TGTextBuffer(30));
    TGLabel * wRunNumberLabel = new TGLabel(wRunCtrl, "Run: ");
    wRunNumberLabel->MoveResize(5, 20, 50, fwRunNumber->GetDefaultHeight());
    wRunCtrl->AddFrame(wRunNumberLabel);

    fwRunNumber->SetText(Form("%i", fpLM->getRunNumber()));
    fwRunNumber->MoveResize(70, 20, 50, fwRunNumber->GetDefaultHeight());
    fwRunNumber->Connect("ReturnPressed()", "daqFrame", this, "doRunNumberUpdate()");
    wRunCtrl->AddFrame(fwRunNumber);

    // -- Run duration
    fwDuration = new TGTextEntry(wRunCtrl, fwDurationBuffer = new TGTextBuffer(100));
    TGLabel * wDurationLabel = new TGLabel(wRunCtrl, "Duration:");
    wDurationLabel->MoveResize(5, 60, 55, fwDuration->GetDefaultHeight());
    wRunCtrl->AddFrame(wDurationLabel);

    fwDuration->SetText(Form("%i", fRunDuration));
    fwDuration->MoveResize(70, 60, 50, fwDuration->GetDefaultHeight());
    fwDuration->Connect("ReturnPressed()", "daqFrame", this, "doDuration()");
    wRunCtrl->AddFrame(fwDuration);

    // -- Directory for output
    fwOutputDir = new TGTextEntry(wRunCtrl, fwOutputDirBuffer = new TGTextBuffer(100));
    TGLabel * wOutputDirLabel = new TGLabel(wRunCtrl, "Directory:");
    wOutputDirLabel->MoveResize(130, 60, 55, fwOutputDir->GetDefaultHeight());
    wRunCtrl->AddFrame(wOutputDirLabel);

    fwOutputDir->SetText(fpLM->getOutputDir());
    fwOutputDir->MoveResize(200, 60, 200, fwOutputDir->GetDefaultHeight());
    fwOutputDir->Connect("ReturnPressed()", "daqFrame", this, "doOutputDirUpdate()");
    wRunCtrl->AddFrame(fwOutputDir);


    // -- Check boxes instead of command line arguments
    TGCheckButton * wExternal = new TGCheckButton(wRunCtrl, "External", 70);
    wExternal->MoveResize(5, 100, 90, 15);
    wExternal->Connect("Clicked()", "daqFrame", this, "doSetExternal()");
    //  wExternal->SetOn();
    if( fExternalTrigger ) wExternal->SetOn();
    wRunCtrl->AddFrame(wExternal);

    TGCheckButton * wLocal = new TGCheckButton(wRunCtrl, "Local", 70);
    wLocal->MoveResize(5, 130, 90, 15);
    wLocal->Connect("Clicked()", "daqFrame", this, "doSetLocal()");
    if( fLocalTrigger ) wLocal->SetOn();
    wRunCtrl->AddFrame(wLocal);

    TGCheckButton * wTemperature = new TGCheckButton(wRunCtrl, "Temperature", 70);
    wTemperature->MoveResize(185, 100, 90, 15);
    wTemperature->Connect("Clicked()", "daqFrame", this, "doMeasureTemperature()");
    //   wTemperature->SetOff();
    wRunCtrl->AddFrame(wTemperature);

    TGCheckButton * wLogMtb = new TGCheckButton(wRunCtrl, "MTB Logging", 70);
    wLogMtb->MoveResize(185, 130, 95, 15);
    wLogMtb->Connect("Clicked()", "daqFrame", this, "doMtbLogging()");
    wLogMtb->SetOn();
    wRunCtrl->AddFrame(wLogMtb);

    TGCheckButton * wFillMem = new TGCheckButton(wRunCtrl, "FillMem", 70);
    wFillMem->MoveResize(355, 100, 95, 15);
    wFillMem->Connect("Clicked()", "daqFrame", this, "doFillMem()");
    //   wFillMem->SetOn();
    wRunCtrl->AddFrame(wFillMem);


    // -- Run Control Buttons
    TGTextButton * wStart = new TGTextButton(wRunCtrl, "Start");
    wStart->Connect("Clicked()", "daqFrame", this, "doStart()");
    wRunCtrl->AddFrame(wStart);
    wStart->MoveResize(130, 20, 50, 20);
    wStart->ChangeBackground(colGreen);

    TGTextButton * wStop = new TGTextButton(wRunCtrl, "Stop");
    wStop->MoveResize(200, 20, 50, 20);
    wStop->Connect("Clicked()", "daqFrame", this, "doStop()");
    wStop->ChangeBackground(colRed);
    wRunCtrl->AddFrame(wStop);

    TGTextButton * wDraw = new TGTextButton(wRunCtrl, "Draw");
    wDraw->Connect("Clicked()", "daqFrame", this, "doDraw()");
    wRunCtrl->AddFrame(wDraw);
    wDraw->MoveResize(300, 20, 50, 20);



    TGLabel * wlabel = new TGLabel(wRunCtrl, "Comments:");
    wlabel->MoveResize(5, 180, 80, fwOutputDir->GetDefaultHeight());
    wRunCtrl->AddFrame(wlabel);

    TGTextEntry * wLogFile = new TGTextEntry(wRunCtrl, fLogText = new TGTextBuffer(100));
    wLogFile->MoveResize(100, 180, 330, wLogFile->GetDefaultHeight());
    wLogFile->SetText("");
    wLogFile->Connect("ReturnPressed()", "daqFrame", this, "doSetLogText()");
    wRunCtrl->AddFrame(wLogFile);

    TGTextButton * setLogText = new TGTextButton(wRunCtrl, "OK");
    setLogText->Connect("Clicked()", "daqFrame", this, "doSetLogText()");
    setLogText->MoveResize(440, 180, 50, setLogText->GetDefaultHeight());
    wRunCtrl->AddFrame(setLogText);

    TGTextButton * wExit = new TGTextButton(wRunCtrl, "Exit");
    wExit->Connect("Clicked()", "daqFrame", this, "doExit()");
    wExit->ChangeBackground(colPink);
    wRunCtrl->AddFrame(wExit);
    wExit->MoveResize(370, 20, 50, 20);


    // ----------------------------------------------------------------------
    // -- Frame with embedded canvas
    TRootEmbeddedCanvas * wCanvas1 = new TRootEmbeddedCanvas(0, this, int(0.5 * w), int(0.5 * h));
    wCanvas1->Move(0.45 * w, 0.45 * h);
    wCanvas1->SetBackgroundColor(colDarkOliveGreen);
    AddFrame(wCanvas1, new TGLayoutHints(kLHintsLeft | kLHintsTop, 1, 1, 1, 1));
    Int_t wwCanvas1 = wCanvas1->GetCanvasWindowId();
    fCanvas1 = new TCanvas("C1", 10, 10, wwCanvas1);
    wCanvas1->AdoptCanvas(fCanvas1);


    // ----------------------------------------------------------------------
    // -- Frame for output
    TGHorizontalFrame * wOutput = new TGHorizontalFrame(this, 500, 210, kHorizontalFrame);
    AddFrame(wOutput);
    wOutput->SetBackgroundColor(colLightBlue);
    wOutput->Move(500, 0);
    wOutput->DrawBorder();


    // -- Display of memory counters
    fwMemMtb = new TGTextEntry(wOutput, fwMemMtbBuffer = new TGTextBuffer(100));
    TGLabel * wMemMtbLabel = new TGLabel(wOutput, "MTB Memory:");
    wMemMtbLabel->MoveResize(10, 10, 100, fwMemMtb->GetDefaultHeight());
    wOutput->AddFrame(wMemMtbLabel);

    fwMemMtb->SetText("0");
    fwMemMtb->MoveResize(120, 10, 100, fwMemMtb->GetDefaultHeight());
    wOutput->AddFrame(fwMemMtb);

    // ----------------------------------------------------------------------
    // -- Frame for setup
    TGHorizontalFrame * wSetup = new TGHorizontalFrame(this, 300, 200, kHorizontalFrame);
    AddFrame(wSetup);
    wSetup->SetBackgroundColor(colDarkOliveGreen);
    wSetup->Move(0, 220);
    wSetup->DrawBorder();

    TGLabel * wTitle = new TGLabel(wSetup, "THE COMMAND LINE");
    wTitle->MoveResize(20, 20, 220, wTitle->GetDefaultHeight());
    wSetup->AddFrame(wTitle);

    // -- MTB SysCommand
    TGTextEntry * wSysCommand1 = new TGTextEntry(wSetup, fwSysCommand1Text = new TGTextBuffer(100));
    wSysCommand1->MoveResize(50, 40, 190, wSysCommand1->GetDefaultHeight());
    //wSysCommand1->SetText("exec module.ini");
    wSysCommand1->Connect("ReturnPressed()", "daqFrame", this, "doSetSysCommand1Text()");
    wSetup->AddFrame(wSysCommand1);

    TGTextButton * setSysCommand1Text = new TGTextButton(wSetup, "OK");
    setSysCommand1Text->Connect("Clicked()", "daqFrame", this, "doSetSysCommand1Text()");
    setSysCommand1Text->MoveResize(250, 40, 50, setSysCommand1Text->GetDefaultHeight());
    wSetup->AddFrame(setSysCommand1Text);

    fwSysCommand1Target = new TGLabel(wSetup, "        ");
    fwSysCommand1Target->SetText("MTB");
    fwSysCommand1Target->MoveResize(0, 40, 40, setSysCommand1Text->GetDefaultHeight());
    wSetup->AddFrame(fwSysCommand1Target);

    // ----------------------------------------------------------------------
    // -- Frame for overall control
    TGHorizontalFrame * wControl = new TGHorizontalFrame(this, 300, 255, kHorizontalFrame, colDarkOliveGreen);
    AddFrame(wControl);
    wControl->Move(0, 440);
    wControl->SetBackgroundColor(colDarkOliveGreen);

    fParametersComboBox = new TGComboBox(wControl, 100);
    fParametersComboBox->Move(40, 20);
    DACParameters * dacParameters = new DACParameters();
    char * name;
    for (Int_t reg = 0; reg < 256; reg++) {
        name = dacParameters->GetName(reg);
        if (strcmp(name, "") != 0)  {
        //    fpLM->log(Form("==>daqf: %s", name));
            fParametersComboBox->AddEntry(name, reg);
        }
    }
    fParametersComboBox->MoveResize(20, 20, 100, 20);
    fParametersComboBox->Select(254);
    wControl->AddFrame(fParametersComboBox);

    TGTextEntry * parameterValue = new TGTextEntry(wControl, fParameterTextBuffer = new TGTextBuffer(30));
    parameterValue->Resize(50, parameterValue->GetDefaultHeight());
    parameterValue->SetText("100");
    parameterValue->Move(130, 20);
    wControl->AddFrame(parameterValue);

    TGTextButton * setParameter = new TGTextButton(wControl, "Set");
    setParameter->Connect("Clicked()", "daqFrame", this, "doSetParameter()");
    setParameter->MoveResize(190, 20, 50, parameterValue->GetDefaultHeight());
    wControl->AddFrame(setParameter);

    TGLabel * wLabel = new TGLabel(wControl, "DACs");
    wLabel->MoveResize(250, 20, 45, parameterValue->GetDefaultHeight());
    wControl->AddFrame(wLabel);

    // -- Module/Roc Selection
    TGHorizontalFrame * inputFrame = new TGHorizontalFrame(wControl, 100, 100);
    inputFrame->MoveResize(20, 50, 220, 80);

    TGLabel * rocLabel = new TGLabel(inputFrame, "ROC");
    inputFrame->AddFrame(rocLabel);
    TGTextEntry * roc = new TGTextEntry(inputFrame, fRocTextBuffer = new TGTextBuffer(50));
    rocLabel->MoveResize(40, 20, 30, roc->GetDefaultHeight());
    roc->MoveResize(80, 20, 50, roc->GetDefaultHeight());
    roc->SetText("0:4");
    inputFrame->AddFrame(roc);

    TGLabel * colLabel = new TGLabel(inputFrame, "COL");
    inputFrame->AddFrame(colLabel);
    TGTextEntry * col = new TGTextEntry(inputFrame, fColTextBuffer = new TGTextBuffer(50));
    colLabel->MoveResize(120, 20, 30, col->GetDefaultHeight());
    col->MoveResize(155, 20, 50, col->GetDefaultHeight());
    col->SetText("0:51");
    inputFrame->AddFrame(col);

    TGLabel * rowLabel = new TGLabel(inputFrame, "ROW");
    inputFrame->AddFrame(rowLabel);
    TGTextEntry * row = new TGTextEntry(inputFrame, fRowTextBuffer = new TGTextBuffer(50));
    rowLabel->MoveResize(120, 50, 30, row->GetDefaultHeight());
    row->MoveResize(155, 50, 50, row->GetDefaultHeight());
    row->SetText("0:79");
    inputFrame->AddFrame(row);

    wControl->AddFrame(inputFrame);


    // -- Global PON/POFF/HV ON/HV OFF
    TGTextButton * wPON = new TGTextButton(wControl, "PON");
    wPON->Connect("Clicked()", "daqFrame", this, "doPON()");
    wControl->AddFrame(wPON);
    wPON->MoveResize(20, 138, 50, 20);

    TGTextButton * wPOFF = new TGTextButton(wControl, "POFF");
    wPOFF->Connect("Clicked()", "daqFrame", this, "doPOFF()");
    wControl->AddFrame(wPOFF);
    wPOFF->MoveResize(20, 163, 50, 20);

    TGTextButton * wHVON = new TGTextButton(wControl, "HV ON");
    wHVON->Connect("Clicked()", "daqFrame", this, "doHVON()");
    wControl->AddFrame(wHVON);
    wHVON->MoveResize(76, 138, 50, 20);

    TGTextButton * wHVOFF = new TGTextButton(wControl, "HV OFF");
    wHVOFF->Connect("Clicked()", "daqFrame", this, "doHVOFF()");
    wControl->AddFrame(wHVOFF);
    wHVOFF->MoveResize(76, 163, 50, 20);

    // -- Actions: Hook to add special functions to the GUI
    fActionComboBox = new TGComboBox(wControl, 100);
    fActionComboBox->Move(40, 20);
    fActionComboBox->AddEntry("WBC scan", 0);
    fActionComboBox->AddEntry("Vcal scan", 1);

    fActionComboBox->MoveResize(130, 163, 130, 20);
    fActionComboBox->Select(0);
    wControl->AddFrame(fActionComboBox);

    setParameter = new TGTextButton(wControl, "Go!");
    setParameter->Connect("Clicked()", "daqFrame", this, "doAction()");
    setParameter->Move(270, 163);
    wControl->AddFrame(setParameter);


    // -- Manual control over hardware
    fManualControlComboBox = new TGComboBox(wControl, 100);
    fManualControlComboBox->AddEntry("Enable",  0);
    fManualControlComboBox->AddEntry("Disable", 1);
    fManualControlComboBox->AddEntry("Mask",    2);
    fManualControlComboBox->AddEntry("Arm",     3);
    fManualControlComboBox->AddEntry("Trim",    4);

    fManualControlComboBox->MoveResize(20, 190, 90, 20);
    fManualControlComboBox->Select(2);

    wControl->AddFrame(fManualControlComboBox);

    parameterValue = new TGTextEntry(wControl, fManualControlTextBuffer = new TGTextBuffer(30));
    parameterValue->Resize(50, parameterValue->GetDefaultHeight());
    parameterValue->SetText("1");
    parameterValue->Move(120, 190);
    wControl->AddFrame(parameterValue);

    setParameter = new TGTextButton(wControl, "Set");
    setParameter->Connect("Clicked()", "daqFrame", this, "doSetManualControlParameter()");
    setParameter->Move(180, 190);
    wControl->AddFrame(setParameter);

    wLabel = new TGLabel(wControl, "CTRL");
    wLabel->MoveResize(250, 190, 45, parameterValue->GetDefaultHeight());
    wControl->AddFrame(wLabel);


    // -- TB setup
    for (int i = 0; i < 256; i++) {
        fTbParNames[i] = TString("");
    }
    fTbParNames[8] = TString("clk");
    fTbParNames[9] = TString("sda");
    fTbParNames[10] = TString("ctr");
    fTbParNames[11] = TString("tin");
    fTbParNames[12] = TString("rda");

    fTbParNames[17] = TString("trc");
    fTbParNames[18] = TString("tcc");
    fTbParNames[19] = TString("tct");
    fTbParNames[20] = TString("ttk");
    fTbParNames[21] = TString("trep");
    fTbParNames[22] = TString("cc");

    fTbParNames[77] = TString("spd");


    fTbParametersComboBox = new TGComboBox(wControl, 100);

    for (int i = 0; i < 256; ++i) {
        if (strcmp(fTbParNames[i].Data(), "")) {
            fTbParametersComboBox->AddEntry(fTbParNames[i].Data(), i);
        }
    }

    fTbParametersComboBox->MoveResize(20, 230, 90, 20);
    fTbParametersComboBox->Select(8);

    wControl->AddFrame(fTbParametersComboBox);

    parameterValue = new TGTextEntry(wControl, fTbParameterTextBuffer = new TGTextBuffer(30));
    parameterValue->Resize(50, parameterValue->GetDefaultHeight());
    parameterValue->SetText("");
    parameterValue->Move(120, 230);
    wControl->AddFrame(parameterValue);

    setParameter = new TGTextButton(wControl, "Set");
    setParameter->Connect("Clicked()", "daqFrame", this, "doSetTbParameter()");
    setParameter->Move(180, 230);
    wControl->AddFrame(setParameter);

    wLabel = new TGLabel(wControl, "TB");
    wLabel->MoveResize(250, 230, 45, parameterValue->GetDefaultHeight());
    wControl->AddFrame(wLabel);


    // ======================================================================
    // -- Draw it

  if( !batchMode ) {
        MapSubwindows();
        MapWindow();
    }

    initializeHardware();
}


// ----------------------------------------------------------------------
void daqFrame::initializeHardware() {

    fpLM->log(Form("=========================="));
    fpLM->log(Form("==>daqf: INSTANTIATING MTB"));
    fpLM->log(Form("=========================="));
    fTB = new TBDigitalInterface(fpLM->getMTBConfigParameters());
    fTB->Flush();
    fCN = new TestControlNetwork(fTB, fpLM->getMTBConfigParameters());
    fTB->Flush();

    //if (fExternalTrigger) {
     //   fTB->SetReg(21, 200);  //t_periode
     //   fTB->SetReg(26, 85);  //trigger delay on testboard in units of 25ns
    //    fTB->Intern(RES);
    //    fTB->Flush();
    //}

    fTB->ProbeSelect(0, 1);

  fTB->SetTriggerMode( TRIGGER_ROC ); fpLM->log( Form( "TRIGGER_ROC" ) ); // our standard
    fTB->getCTestboard()->DataBlockSize(100);

    /*  fpLM->log("Enable all pixels");
      fpLM->log(">>>>>>>> Skip chip 3 <<<<<<<<<<<<");
      TestModule *module = fCN->GetModule(0);*/

    //fTB->SetClockStretch(STRETCH_AFTER_CAL, 5, 1000);

    for (int i = 0; i < fCN->GetModule(0)->NRocs(); i++)
    {
        fCN->GetModule(0)->GetRoc(i)->EnableAllPixels();
     //   fCN->GetModule(0)->GetRoc(i)->SetDAC("WBC", 106);
    }

    for (int iRoc = 0; iRoc < fCN->GetModule(0)->NRocs(); ++iRoc)
    {
        vtrim[iRoc] = fCN->GetModule(0)->GetRoc(iRoc)->GetDAC("Vtrim");
     //   fCN->GetModule(0)->GetRoc(iRoc)->SetDAC("Vtrim", 0);
        vthrcomp[iRoc] = fCN->GetModule(0)->GetRoc(iRoc)->GetDAC("VthrComp");
     //   fCN->GetModule(0)->GetRoc(iRoc)->SetDAC("VthrComp", 0);
    }

    sleep(1);
    double tmp1 = fTB->getCTestboard()->GetIA();
    double tmp2 = fTB->getCTestboard()->GetID();
    fpLM->log(Form("==>daqf: Analog/Digital current for the MTB %f  %f ", tmp1, tmp2));
}


// ----------------------------------------------------------------------
void daqFrame::getTemperature() {

    float temp(0.);
    if (fCN->GetModule(0)) {
        temp = fCN->GetModule(0)->GetTemperature();
        fpLM->log(Form("==>daqf: Mod: Temperature on MTB %5.3f:  ", temp));
    }
}



// ----------------------------------------------------------------------
void daqFrame::runStart() {

    fpLM->log("==>daqf: reading from h/w");

    // -- Flush and start from scratch!
    fTB->Flush();
    fReg21 = 0x0;
    fReg41 = 0x0;
    fReg43 = 0x0;
    fReg47 = 0x0;

    // -- set up RTB and MTB
    fpLM->log("==>daqf: --> MTB setup ");

  if( fExternalTrigger ) {

    //fTB->SetReg(21, 200);  //t_periode (obsolete says Tilman 2011)
    fTB->SetReg( 26, 85 );  //trigger delay on testboard in units of 25ns
    //fTB->SetReg( 26, 135 );  //trigger delay on testboard test for xdb2 27/06/2012 (WBC)

    //fTB->Intern(0); // 10.6.2013. Massensterben of DCs
    fTB->Intern(RES); // was active all the time to 10.6.2013
    //fTB->Intern(RES|TOK);//gives corrupt data (10.8.2011)
    //fTB->Intern(RES|CAL); // DCF test summer 2013
    //fTB->Intern(RES|CAL|TOK);//gives corrupt data (10.8.2011)
    //fTB->Intern(1001);//RES|cal|trg|TOK. gives corrupt data (10.8.2011)

    fTB->Flush();
    gDelay->Mdelay(50);//milli secs

  }
    if (fLocalTrigger) {
    fpLM->log("==>daqf: ........ using local TRG of MTB.........");
        //fReg41 = 0x22; // tbm present and intern ctr
        // -- BEAT MEINT: Hier ist noch das Aequivalent zu "tb loop" zu programmieren...
    fReg41 = 0x20; // DP: tbm not present and intern trg
    if(
       strcmp( fpLM->getMTBConfigParameters()->directory, "chip202" ) == 0 ||
       strcmp( fpLM->getMTBConfigParameters()->directory, "chip203" ) == 0 ||
       strcmp( fpLM->getMTBConfigParameters()->directory, "chip205" ) == 0 ||
       strcmp( fpLM->getMTBConfigParameters()->directory, "chip308" ) == 0 ) {
	  
      fReg41 = 0x21; // DP: no TBM, intern trg, ADC2 for dig
    }

  }

  else { // ext trig

    if( strcmp( fpLM->getMTBConfigParameters()->directory, "chip11" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip10" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip16" ) == 0 ||

	strcmp( fpLM->getMTBConfigParameters()->directory, "chip21" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip22" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip23" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip24" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip25" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip26" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip27" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip28" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip29" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip30" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip31" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip32" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip33" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip34" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip35" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip36" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip37" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip38" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip39" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip40" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip41" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip42" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip43" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip44" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip45" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip46" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip47" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip48" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip49" ) == 0 ||

	strcmp( fpLM->getMTBConfigParameters()->directory, "chip51" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip52" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip53" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip54" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip55" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip56" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip57" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip58" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip59" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip60" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip61" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip62" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip63" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip64" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip65" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip66" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip67" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip68" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip69" ) == 0 ||

	strcmp( fpLM->getMTBConfigParameters()->directory, "chip70" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip71" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip72" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip73" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip74" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip75" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip76" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip77" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip78" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip79" ) == 0 ||

	// DESY analog 2013

	strcmp( fpLM->getMTBConfigParameters()->directory, "chip110" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip111" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip112" ) == 0 ||
	strcmp( fpLM->getMTBConfigParameters()->directory, "chip113" ) == 0 

	)
        fReg41 = 0x42; // tbm present and extern ctr
    else { // dig

      //fReg41 = 0x41; // extern trg and ADC2 for digital ROC (Aug 2012)
      fReg41 = 0x43; // extern trg and TBM and ADC2 for digital ROC (Aug 2012)
    }

    // -- Set up MTB
  } // ext trigg

    // -- Set up MTB
  cout << " Setting DAC registers 41 to '" << fReg41 << "' and 43 to '" << 2 << "'" << endl;
    fTB->SetReg(41, fReg41);
    fTB->SetReg(43, 2);
    if (fLocalTrigger) {
    // int istretch = 10000;//10000*25 = 0.25 ms. 4 kHz trig rate max. 
    // int istretch = 1000;//1000*25 = 0.025 ms. 40 kHz trig rate max. 
    // int istretch = 100;//100*25 = 0.0025 ms. 400 kHz trig rate max. 
    // int istretch = 65000;//max 2^16-1 = 65535
    // int istretch = 40000;//40000*25ns = 1 ms, 1 kHz trig rate
    // int istretch = 20000;//20000*25ns = 0.5 ms
    int istretch = 0; // test beam

    fTB->SetClockStretch( STRETCH_AFTER_CAL, 5, istretch );

    fpLM->log( Form( "runStart: SetClockStretch = %i", istretch ) );

    //fTB->Intern(15);  // FPGA sends RES|CAL|TRG|TOK sequence
    fTB->Intern(CAL|TRG|TOK);  // FPGA sequence, no reset for DCF study

  } else {
    fTB->SetClockStretch( STRETCH_AFTER_CAL, 5, 0 );//reset clock stretch
    }
    //fTB->Intern(RES);
    fTB->Flush();

    // -- start ADC of RTB and MTB
    fpLM->log("==>daqf: --> MTB DataCtrl ADC ");

    int tbmc = fpLM->getMTBConfigParameters()->tbmChannel;
    fTB->Flush();
    dataBuffer_fpga1 = fTB->getCTestboard()->Daq_Init(dataBuffer_numWords);
    fTB->getCTestboard()->Daq_Enable();
    fTB->getCTestboard()->DataCtrl(tbmc, false, false, true); // go
    fTB->Flush();

    // -- run data aquisition
    fpLM->log("==>daqf: --> MTB start data aquisition ");
    fReg41 |= 0x8;
    fTB->SetReg(41, fReg41);
    fTB->Flush();
}


// ----------------------------------------------------------------------
void daqFrame::doStart() {

   // for (int iRoc = 0; iRoc < fCN->GetModule(0)->NRocs(); ++iRoc)
   // {
   //     fCN->GetModule(0)->GetRoc(iRoc)->SetDAC("Vtrim", vtrim[iRoc]);
   //     fCN->GetModule(0)->GetRoc(iRoc)->SetDAC("VthrComp", vthrcomp[iRoc]);
   // }

    uint32_t filledMem1 = 0;

    fRunning = 1;

    fpLM->log("==>daqf: START! ");
    fpLM->setupRun();
  if(!(strcmp(fpLM->getMTBConfigParameters()->GetMaskFileName(),"default")==0)){
    ApplyMaskFile(fpLM->getMTBConfigParameters()->GetMaskFileName()); // AP: 200612, works for individual ROCs so far
    cout << "\033[34mMask file is \033[0m " << fpLM->getMTBConfigParameters()->GetMaskFileName() << endl;
  }

    fpLM->log("==>daqf: Dumping h/w configuration");
    fpLM->dumpHardwareConfiguration(0, fCN, fTB);

    fpLM->log(Form("==>daqf: Open ROOT outputfile %s/takeDataHist.root", fpLM->getOutputDir()));
    fpDAQ->getHistogrammer()->openRootFile(Form("%s/takeDataHist.root", fpLM->getOutputDir()));
    fpDAQ->getHistogrammer()->init("daq");
    fpDAQ->getHistogrammer()->reset();

    fpLM->log(Form("==>daqf: MTB analog    current = %f   voltage = %f ",
                   fTB->getCTestboard()->GetIA(), fTB->getCTestboard()->GetVA()
                  ));
    fpLM->log(Form("==>daqf: MTB digital   current = %f   voltage = %f ",
                   fTB->getCTestboard()->GetID(), fTB->getCTestboard()->GetVD()
                  ));

    if (fTemperature) getTemperature();
    for (int i = 0; i < fCN->GetModule(0)->NRocs(); i++) fCN->SetDAC(0, i, 27, 2); //Set TempReg DAC
    fTB->Flush();

    FILE * f;

    if (!fFillMem) {
        f = fopen(Form("%s/mtb.bin", fpLM->getOutputDir()), "wb");
        if (f == NULL) {
            psi::LogInfo() << psi::endl
                           << "Could not open file " << fpLM->getOutputDir()
                           << "/mtb.bin" << psi::endl;
            return;
        }
    }

    //??  fRunDuration = atoi(fwDurationBuffer->GetString());

    int nRuns = 1;
    if (fFillMem) {
        nRuns = fRunDuration;
        cout << " recording " << fRunDuration << " runs " << endl;
    }

    for (int k = 0; k < nRuns; k++) {
        if (fFillMem) {
            fpLM->setupRun();
            fpLM->dumpHardwareConfiguration(0, fCN, fTB);
            f = fopen(Form("%s/mtb.bin", fpLM->getOutputDir()), "wb");
            if (f == NULL) {
                psi::LogInfo() << psi::endl
                               << "Could not open file " << fpLM->getOutputDir()
                               << "/mtb.bin" << psi::endl;
                return;
            }
        }

        runStart();

        int seconds = 0, stepSize = 0;
	uint32_t filledSize = fTB->getCTestboard()->Daq_GetSize(); //DP
	//unsigned int filledSize = fTB->getCTestboard()->Daq_GetSize();

	// prepare warning flags/variables (HP 20120427)
	bool bufferFullWarned = false; // remeber if we warned about full buffer already
	bool bufferBlockageWarned = false; // remeber if we warned about the buffer being jammed
	// i.e. not being filled anymore
	uint32_t lastFilledMem = 0; // remember previous state of filledMem to recognize a blocked state
	uint32_t oldFilledMem = 0; // remember previous state of filledMem to recognize a blocked state
        while (1) {
            if (fRunning == 0) break;
            sleep(1);
            seconds++;

	  filledMem1 = fTB->getCTestboard()->Daq_GetPointer() - dataBuffer_fpga1;
	  filledSize = fTB->getCTestboard()->Daq_GetSize();
	  fpLM->log( Form( "==>daqf: %4i: mem: %8lu  left %8lu  last sec %6lu",
			   seconds,(unsigned long ) filledMem1, (unsigned long )filledSize, (unsigned long )(filledMem1-lastFilledMem) ) );
	  lastFilledMem = filledMem1; // remember value

	  // check for possible problems: buffer blockage i.e. buffer not being filled (HP 20120427)
	  // check every 20 seconds for changed values

	  if( (seconds-1)%20 == 0 ) {
	    if( oldFilledMem == 0 ) oldFilledMem = filledMem1; // init
	    else if( filledMem1 != oldFilledMem ) {
	      oldFilledMem = filledMem1; // remember value
	      if( bufferBlockageWarned ) {
		bufferBlockageWarned = false; // reset flag
		// gui: reset warning
		fCanvas1->cd();
		fCanvas1->Clear();
		fCanvas1->Modified(); 
		fCanvas1->Update();
	      }
	    }
	    else if( !bufferBlockageWarned ) {
	      // we might have a problem: let the user know
	      if( filledMem1 == 0 ) {
		// text warning:
		fpLM->log(" Warning: no data coming in!");
		// gui warning:
		fCanvas1->cd();
		fCanvas1->Clear();
		TLatex *tl = new TLatex(); tl->SetNDC(kTRUE); tl->SetTextSize(0.15);
		tl->SetTextColor(kRed);
		tl->DrawLatex(0.15, 0.65, Form("Warning:"));
		tl->DrawLatex(0.15, 0.35, Form("no data?"));
		fCanvas1->Modified(); 
		fCanvas1->Update();
		bufferBlockageWarned=true;
	      }
	      else {
		// got stuck while taking data
		// text warning:
		fpLM->log(" WARNING: no more data coming in!");
		cout
		  << " _            __  __             _     _            _            _  " << endl
		  << "| |__  _   _ / _|/ _| ___ _ __  | |__ | | ___   ___| | _____  __| | " << endl
		  << "| '_ \\| | | | |_| |_ / _ \\ '__| | '_ \\| |/ _ \\ / __| |/ / _ \\/ _` | " << endl
		  << "| |_) | |_| |  _|  _|  __/ |    | |_) | | (_) | (__|   <  __/ (_| | " << endl
		  << "|_.__/ \\__,_|_| |_|  \\___|_|    |_.__/|_|\\___/ \\___|_|\\_\\___|\\__,_| " << endl
		  << "                                                                    " << endl;
		//beep
		std::cout << '\a';
		// gui warning:
		fCanvas1->cd();
		fCanvas1->Clear();
		TLatex *tl = new TLatex(); tl->SetNDC(kTRUE); tl->SetTextSize(0.15);
		tl->SetTextColor(kRed);
		tl->DrawLatex(0.15, 0.65, Form("WARNING:"));
		tl->DrawLatex(0.15, 0.35, Form("no more data!"));
		fCanvas1->Modified(); 
		fCanvas1->Update();
		bufferBlockageWarned=true;
	      }
	    } // if buffer blockage found and !warned about it
	  }
	  else if (bufferBlockageWarned && (filledMem1 != oldFilledMem)) {
	    // revert flag
	    bufferBlockageWarned=false; // reset flag
	    fpLM->log(" Buffer filling (again)..  :-)");
	    // gui: reset warning
	    fCanvas1->cd();
	    fCanvas1->Clear();
	    fCanvas1->Modified(); 
	    fCanvas1->Update();
	  } // check for buffer blockage
     
	  // check for possible problems: buffer filled completely (HP 20120427)

	  if( filledSize == 0 && !bufferFullWarned ) {
	    // text warning:
	    cout
	      << " _____                        " << endl
	      << "| ____|_ __ _ __ ___  _ __ _  " << endl
	      << "|  _| | '__| '__/ _ \\| '__(_) " << endl
	      << "| |___| |  | | | (_) | |   _  " << endl
	      << "|_____|_|  |_|  \\___/|_|  (_) " << endl
	      << "                              " << endl;
	    cout
	      << " ____         __  __             _____      _ _  " << endl
	      << "| __ ) _   _ / _|/ _| ___ _ __  |  ___|   _| | | " << endl
	      << "|  _ \\| | | | |_| |_ / _ \\ '__| | |_ | | | | | | " << endl
	      << "| |_) | |_| |  _|  _|  __/ |    |  _|| |_| | | | " << endl
	      << "|____/ \\__,_|_| |_|  \\___|_|    |_|   \\__,_|_|_| " << endl
	      << "                                                 " << endl;
	    fpLM->log(" ERROR: BOARD REPORTS NO SPACE LEFT IN BUFFER!");
	    // beep
	    std::cout << '\a';
	    // gui warning:
	    fCanvas1->cd();
	    fCanvas1->Clear();
	    TLatex *tl = new TLatex(); tl->SetNDC(kTRUE); tl->SetTextSize(0.15);
	    tl->SetTextColor(kRed);
	    tl->DrawLatex(0.15, 0.65, Form("ERROR:"));
	    tl->DrawLatex(0.15, 0.35, Form("BUFFER FULL"));
	    fCanvas1->Modified(); 
	    fCanvas1->Update();
	    bufferFullWarned=true;
	  } // buffer full
            if (stepSize == 0) stepSize = filledMem1;
            fwMemMtb->SetText(Form("%8i", filledMem1));
            gSystem->ProcessEvents();

            if ((fFillMem) && (filledMem1 > dataBuffer_numWords - 2.*stepSize)) break;
            else if ((!fFillMem) && (seconds == fRunDuration)) break;
        }

        doBreak();
        readout(f, filledMem1);

	fpLM->log( Form( "==>daqf: read out Run %i", fpLM->getRunNumber() ) );
        if (fFillMem) {
            fclose(f);
            seconds = fpLM->incrementRunNumber();
	}
	else {

	  int nextrunnumber = fpLM->incrementRunNumber();
	  fRunTextBuffer->Clear();
	  fRunTextBuffer->AddText(0, Form("%i", nextrunnumber));
	  fwRunNumber->TextChanged();
	  gClient->NeedRedraw(fwRunNumber);

	  fwOutputDirBuffer->Clear();
	  fwOutputDirBuffer->AddText(0, fpLM->getOutputDir());
	  fwOutputDir->TextChanged();
	  gClient->NeedRedraw(fwOutputDir);
  
	  doRefreshWindowTitle();
        }

    }

    if (!fFillMem) {
        fclose(f);
    }

    stopTriggers();  // Disable triggers

    fCanvas1->cd();
    fCanvas1->Clear();
    TH1 * h = fpDAQ->getHistogrammer()->getHistogram("h0");
    if (h) {
        h->Draw("e");
        TLatex * tl = new TLatex(); tl->SetNDC(kTRUE); tl->SetTextSize(0.15);
        double eff = h->GetBinContent(2) / h->GetBinContent(4);
        h->Draw("");
        tl->DrawLatex(0.15, 0.5, Form("RUN FINISHED!"));
    }
    fCanvas1->Modified();
    fCanvas1->Update();

    // --  for oscilloscope
    //startTriggers();
    doStop();
}


// ----------------------------------------------------------------------
void daqFrame::startTriggers() {

    fpLM->log("==>daqf: Enable triggers");
  if( fLocalTrigger) 
    //DP fReg41 = fReg41 | 0x22; // tbm present and intern trg
    fReg41 = fReg41 | 0x20; // tbm not present and intern trg
  else 
    fReg41 = fReg41 | 0x42; // tbm present and extern trg

    fpLM->log(Form("==>daqf: startTriggers MTB Enable triggers; writing reg41: %02x, unset data_aqu", fReg41));
    if (fTB) fTB->SetReg(41, fReg41);
    if (fTB) fTB->Flush();
  gDelay->Mdelay(50);//milli secs
}


// ----------------------------------------------------------------------
void daqFrame::stopTriggers()
{
    fReg41 &= ~0x8;
    fpLM->log(Form("==>daqf: stopTriggers MTB disable; writing reg41: %02x, unset data_aqu", fReg41));
    fTB->SetReg(41, fReg41);
    if (fLocalTrigger) fTB->Single(0);       // stop
    fTB->Flush();
}


// ----------------------------------------------------------------------
void daqFrame::doBreak()
{
    fReg41 &= ~0x8;

    fpLM->log(Form("==>daqf: MTB disable; writing reg41: %02x, unset data_aqu", fReg41));
    fTB->SetReg(41, fReg41);
    fTB->Flush();

    fTB->getCTestboard()->Daq_Done();
    fTB->getCTestboard()->Daq_Disable();
    fTB->getCTestboard()->DataCtrl(0, false, false, false); // stop
    fTB->Flush();

    fpLM->log(Form("==>daqf: Run %i breaked. OK", fpLM->getRunNumber()));
}



// ----------------------------------------------------------------------
void daqFrame::doStop() {

    if (fRunning == 0) {
        fpLM->log("==>daqf: STOP pressing STOP! ");
        return;
    }
    fRunning = 0;

    fpLM->log("==>daqf: Run stop");
    fpLM->log(Form("==>daqf: Run %i stopped. OK", fpLM->getRunNumber()));
    fpDAQ->stop();

}


// ----------------------------------------------------------------------
void daqFrame::doDraw() {

    fCanvas1->cd();
    TH1 * h = fpDAQ->getHistogrammer()->getNextHistogram();

  if( h ) {
        h->Print();
        h->Draw();
        fCanvas1->Update();
    }
}


// ----------------------------------------------------------------------
void daqFrame::doExit() {

    fpLM->log(Form("==>daqf: Shutting down MTB"));
    fTB->HVoff();
    fTB->Flush();
    fTB->Poff();
    fTB->Close();
    gApplication->Terminate(0);
}



// ----------------------------------------------------------------------
void daqFrame::readout(FILE * file, uint32_t filledMem1)
{

    fpLM->log(Form("==>daqf: read mtb, words = %d", filledMem1));
    fTB->Mem_ReadOut(file, dataBuffer_fpga1, filledMem1);
  fTB->getCTestboard()->DataCtrl( 0, true, false, false ); // clear FIFO
  fTB->Flush();
  fTB->getCTestboard()->DataCtrl( 0, false, false, false ); // clear FIFO
  fTB->Flush();
}


// ----------------------------------------------------------------------
void daqFrame::doSetLogText() {
    fpLM->log(Form("==>daqf: LOG: %s", fLogText->GetString()));
}
//

// ----------------------------------------------------------------------
void daqFrame::doDuration() {
    fRunDuration = atoi(fwDurationBuffer->GetString());
    fpLM->log(Form("==>daqf: Run length set to %i", fRunDuration));
}


// ----------------------------------------------------------------------
void daqFrame::doRunNumberUpdate() {
    fpLM->setRunNumber(atoi(fRunTextBuffer->GetString()));
  fwOutputDir->SetText(fpLM->getOutputDir());//show new outputdir
  doRefreshWindowTitle();
}

// ----------------------------------------------------------------------
void daqFrame::doOutputDirUpdate() {
    fpLM->setOutputDir(fRunTextBuffer->GetString());
}

// ----------------------------------------------------------------------
void daqFrame::doSetExternal() {
    if (fExternalTrigger == 0) fExternalTrigger = 1;
    else fExternalTrigger = 0;
    fpLM->log(Form("==>daqf: fExternalTrigger set to  %i", fExternalTrigger));
}

// ----------------------------------------------------------------------
void daqFrame::doSetLocal() {
    if (fLocalTrigger == 0) fLocalTrigger = 1;
    else fLocalTrigger = 0;
    fpLM->log(Form("==>daqf: fLocalTrigger set to  %i", fLocalTrigger));
}


// ----------------------------------------------------------------------
void daqFrame::doMtbLogging() {
    if (fMtbLogging == 0) fMtbLogging = 1;
    else fMtbLogging = 0;
    fpLM->log(Form("==>daqf: fMtbLogging set to  %i", fMtbLogging));
}


// ----------------------------------------------------------------------
void daqFrame::doFillMem() {
    if (fFillMem == 0) fFillMem = 1;
    else fFillMem = 0;
    fpLM->log(Form("==>daqf: fFillMem set to  %i", fMtbLogging));
}


// ----------------------------------------------------------------------
void daqFrame::doMeasureTemperature() {
    if (fTemperature == 0) fTemperature = 1;
    else fTemperature = 0;
    fpLM->log(Form("==>daqf: fTemperature set to  %i", fTemperature));
}


// ----------------------------------------------------------------------
void daqFrame::doPON()
{
    fTB->Pon();
    fTB->Flush();
    fpLM->log("==>daqf: PON");
}


// ----------------------------------------------------------------------
void daqFrame::doPOFF()
{
    fTB->Poff();
    fTB->Flush();
    fpLM->log("==>daqf: POFF");
}


// ----------------------------------------------------------------------
void daqFrame::doHVON() {
    fTB->HVon();
    fTB->Flush();
    fpLM->log("==>daqf: HV ON");
}


// ----------------------------------------------------------------------
void daqFrame::doHVOFF() {
    fTB->HVoff();
    fTB->Flush();
    fpLM->log("==>daqf: HV OFF");
}


// ----------------------------------------------------------------------
void daqFrame::doSetParameter() {
    bool inputOk = true;
    int dacValue, reg, rocMin, rocMax;
    char * name;

    fInterpreter->SetString(fParameterTextBuffer->GetString());
    if (!fInterpreter->GetInt(dacValue, 0, 255)) {inputOk = false;}

    fInterpreter->SetString(fRocTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(rocMin, rocMax, 0, 15)) {inputOk = false;}

    reg = fParametersComboBox->GetSelected();
    if (reg < 0 || reg > 255) {inputOk = false;}

    if (inputOk) {
        name = (new DACParameters())->GetName(reg); // ?? FIXME: Memory leak ??
        for (int iRoc = rocMin; iRoc <= rocMax; iRoc++) {
            if (fCN->GetModule(0) && (iRoc < fCN->GetModule(0)->NRocs())) {
                fpLM->log(Form("==>daqf: ROC %i: Parameter %s set to %i", iRoc, name, dacValue));
	  if( reg == 254 ){
	    fCN->GetModule(0)->GetRoc(iRoc)->SetDAC( "WBC", dacValue );//AG 26.8.2011
	  }
	  else{
                fCN->SetDAC(0, iRoc, reg, dacValue);
	  }
                fTB->Flush(); // FIXME: Too often flush??
            }
        }
    } else {
        fpLM->log(Form("==>daqf: Invalid Value in doSetParameter: %s", name));
    }
}

// ----------------------------------------------------------------------
// -- Manual labor so as not to interfere hopefully with pixel testing
void daqFrame::doSetManualControlParameter() {

    bool inputOk = true;
    int settingValue, reg, colMin, colMax, rowMin, rowMax, rocMin, rocMax;

    fInterpreter->SetString(fManualControlTextBuffer->GetString());
    if (!fInterpreter->GetInt(settingValue, 0, 255)) {inputOk = false;}

    fInterpreter->SetString(fRowTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(rowMin, rowMax, 0, 79)) {inputOk = false;}

    fInterpreter->SetString(fColTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(colMin, colMax, 0, 51)) {inputOk = false;}

    fInterpreter->SetString(fRocTextBuffer->GetString());
    if (!fInterpreter->GetIntRange(rocMin, rocMax, 0, 15)) {inputOk = false;}

    reg = fManualControlComboBox->GetSelected();
    if (reg < 0 || reg > 255) {inputOk = false;}

    if (inputOk) {
        Roc * r;

        for (int iRoc = rocMin; iRoc <= rocMax; iRoc++) {
            if (fCN) {
                if (fCN->GetModule(0) && (iRoc < fCN->GetModule(0)->NRocs())) {
                    r = fCN->GetModule(0)->GetRoc(iRoc);
                    for (int icol = colMin; icol <= colMax; ++icol) {
                        if (reg == 0) {
                            r->EnableDoubleColumn(icol);
                            cout << Form("r%i->EnableDoubleColumn(%i)", iRoc, icol) << endl;
                        }
                        // FIXME: DisableDoubleColumns for Pixel Disable?
                        for (int irow = rowMin; irow <= rowMax; ++irow) {
                            if (reg == 0) {
                                r->EnablePixel(icol, irow);
                            } else if (reg == 1) {
                                r->DisablePixel(icol, irow);
                            } else if (reg == 2) {
                                r->PixMask(icol, irow); // FIXME: "do not use them, use those in the pixel or dacParameters object"
                            } else if (reg == 3) {
                                r->ArmPixel(icol, irow);
                                cout << Form("armpix(%i, %i)", icol, irow) << endl;
                            } else if (reg == 4) {
                                r->PixTrim(icol, irow, settingValue);
                                cout << Form("trimpix(%i, %i, %i)", icol, irow, settingValue) << endl;
                            }
                        }
                    }
                    fpLM->log(Form("==>daqf: ROC %i, col %i:%i, row %i:%i: Action %i",
                                   iRoc, colMin, colMax, rowMin, rowMax, reg));
                }
            } else {
                fpLM->log(Form("==>daqf: ROC %i, col %i:%i, row %i:%i: Action %i",
                               iRoc, colMin, colMax, rowMin, rowMax, reg));
            }
        }
    } else {
        fpLM->log(Form("==>daqf: Invalid Value in doSetManualControlParameter: %i", reg));
    }

}


// ----------------------------------------------------------------------
void daqFrame::doSetTbParameter() {
    bool inputOk = true;
    int tbValue, reg;
    const char * name;

    fInterpreter->SetString(fTbParameterTextBuffer->GetString());
    if (!fInterpreter->GetInt(tbValue, 0, 255)) {inputOk = false;}

    reg = fTbParametersComboBox->GetSelected();
    if (reg < 0 || reg > 255) {inputOk = false;}

    if (inputOk) {
        name = fTbParNames[reg].Data();
        fpLM->log(Form("==>daqf: TB: Parameter %s set to %i", name, tbValue));
        fTB->SetTBParameter(name, tbValue);
        fTB->Flush();  // FIXME: Too often flush??
    } else {
        fpLM->log("==>daqf: Invalid Value");
    }
}



// ----------------------------------------------------------------------
void daqFrame::doAction() {

    int selected = fActionComboBox->GetSelected();

    if (selected == 0) {
        cout << "WBC scan" << endl;
        wbcScan();
    }
    else if (selected == 1) {
        cout << "DAC scan" << endl;
        dacScan();
    }

    else {
        cout << "still to be defined" << endl;
    }

}


// ----------------------------------------------------------------------
void daqFrame::doAskTemperature() {
    fpLM->log(Form("==>daqf: Temperature reading: %5.2f ", atof(fTemperatureBuffer->GetString())));
    //  fTempMain->CloseWindow();
    fTempMain->SendCloseMessage();
}


// ----------------------------------------------------------------------
void daqFrame::doSetSysCommand1Text() {

    char * p = (char *)fwSysCommand1Text->GetString();
    if (fpSysCommand1->Parse(p)) {
        do {
            if (fpSysCommand1->TargetIsTB()) {
                fTB->Execute(*fpSysCommand1);
                cout << Form(" fTB->Execute: ");
            } else  {
                fCN->Execute(*fpSysCommand1);
                cout << Form(" fCN->Execute: ");
            }
            char * s = fpSysCommand1->toString();
            cout << Form(" %s", s) << endl;
            fpLM->log(s);
            delete s;
        } while (fpSysCommand1->Next());
    }
    //  fwSysCommand1Text->AddText(0, "");
}



// ----------------------------------------------------------------------
void daqFrame::wbcScan() {


  const int WBCStart = 95; // local
  const int WBCStop = 106; // local

  int wbcscannumber=0;
  while ( (WBCStart+wbcscannumber)<WBCStop){
    int thiswbc = WBCStart+wbcscannumber;
    wbcscannumber++; // increment to the next WBC value
    fpLM->log(Form("--> set WBC = %d ", thiswbc));        
    for( int i = 0; i < fCN->GetModule(0)->NRocs(); i++)
      {
	fCN->GetModule(0)->GetRoc(i)->EnableAllPixels();
	fCN->GetModule(0)->GetRoc(i)->SetDAC("WBC", thiswbc);
      }

    unsigned long filledMem1 = 0;

    //  for (int iwbc = 98; iwbc < 103; ++iwbc) {
    fRunning = 1;

    fpLM->log("==>daqf: START! ");
    fpLM->setupRun();

    fpLM->log("==>daqf: Dumping h/w configuration");
    fpLM->dumpHardwareConfiguration(0, fCN, fTB);
                // -- Force WBC setting even when no periodic resets are generated:
    fpLM->log(Form("==>daqf: MTB analog    current = %f   voltage = %f ",
		   fTB->getCTestboard()->GetIA(), fTB->getCTestboard()->GetVA() 
		   ));
    fpLM->log(Form("==>daqf: MTB digital   current = %f   voltage = %f ",
		   fTB->getCTestboard()->GetID(), fTB->getCTestboard()->GetVD() 
		   ));
                //    Need to enable LOCAL CTR first, then do RESET. runStart() will disable the local CTR.
    if( fTemperature) getTemperature();
    for( int i = 0; i < fCN->GetModule(0)->NRocs(); i++) fCN->SetDAC(0, i, 27, 2); //Set TempReg DAC
                fTB->Flush();
    gDelay->Mdelay(50);//milli secs
    FILE *f;

    if( !fFillMem) {
      	char fileName[200];
	sprintf(fileName, "%s/scan_wbc_%03d.bin", fpLM->getOutputDir(), thiswbc);
	psi::LogInfo() << "Opening File " << fpLM->getOutputDir()
		       << "/" << fileName << psi::endl;
	
	f = fopen(fileName, "wb");
      if( f == NULL) { 
	psi::LogInfo() << psi::endl
		       << "Could not open file " << fpLM->getOutputDir()
		       << "/" << fileName << psi::endl;
	return;
            }
        }

    int nRuns = 1;

    // if( fFillMem) {
    //   nRuns = fRunDuration;
    //   cout << " recording " << fRunDuration << " runs " << endl;
    // }

    for( int k = 0; k < nRuns; k++) {

      if( fFillMem) {
	fpLM->setupRun();
	fpLM->dumpHardwareConfiguration(0, fCN, fTB);
	char fileName[200];
	sprintf(fileName, "%s/scan_wbc_%03d.bin", fpLM->getOutputDir(), thiswbc);
	psi::LogInfo() << "Opening " << fpLM->getOutputDir()
		       << "/" << fileName << psi::endl;
	f = fopen(fileName, "wb");
	if( f == NULL) { 
	  psi::LogInfo() << psi::endl
			 << "Could not open file " << fpLM->getOutputDir()
			 << "/" << fileName << psi::endl;
	  return;
	}
      }
        runStart();  // This also starts triggers

        // -- Run loop and data watching
        //    fRunDuration = atoi(fwDurationBuffer->GetString());
      int seconds = 0, stepSize = 0;
      uint32_t filledSize = fTB->getCTestboard()->Daq_GetSize(); //DP
      //unsigned int filledSize = fTB->getCTestboard()->Daq_GetSize();

      while(1) {

	if( fRunning == 0) break;
            sleep(1);
	seconds++;
            // -- memory address counter
	filledMem1 = fTB->getCTestboard()->Daq_GetPointer() - dataBuffer_fpga1;
	filledSize = fTB->getCTestboard()->Daq_GetSize();
	fpLM->log(Form("==>daqf: %4i: mem: %8lu  left %8lu",  seconds, (unsigned long )filledMem1, (unsigned long )filledSize ) );

	//fpLM->log(Form("==>daqf: %6i", seconds ) );

	if( stepSize == 0) stepSize = filledMem1;

	fwMemMtb->SetText(Form("%8lu", filledMem1));//for GUI

	gSystem->ProcessEvents();// handle GUI events: ROOT?
  
	if( (fFillMem) && (filledMem1 > dataBuffer_numWords - 2.*stepSize)) break;
	else if( (!fFillMem) && (seconds == fRunDuration)) break;
        }

        //stopTriggers();  // stop triggers during readout

        // -- readout TB memory
        /* get number of WORDs written to RAM by the DMA controller */
        //filledMem1 = (fTB->getCTestboard()->Daq_GetPointer() - dataBuffer_fpga1) / 2;
        //fpLM->log(Form("... read out memory till address = %d", filledMem1));

      doBreak();
      readout( f, filledMem1 );

      if( fFillMem ) {
	fclose(f);
            }
        }
        //fTB->getCTestboard()->Daq_Done();
        //fTB->getCTestboard()->Daq_Disable();

    if( !fFillMem) {
        fclose(f);
    }

    stopTriggers();  // Disable triggers

        //double nero = 0, nero1 = 0;

        //nero = h->Integral(19 + nRocs * 3, 201); // non empty readouts
        //nero1 = h->Integral(15 + nRocs * 3, 201); // all readouts

    if( fRunning == 0) {
      fpLM->log("==>daqf: STOP pressing STOP! ");
      return;
    }
    fRunning = 0; 
    
    fpLM->log("==>daqf: Run stop");
    fpLM->log(Form("==>daqf: Run %i, WBC = %i stopped. OK", fpLM->getRunNumber(), thiswbc));
        gSystem->ProcessEvents();
    }


  int nextrunnumber = fpLM->incrementRunNumber();
  fRunTextBuffer->Clear();
  fRunTextBuffer->AddText(0, Form("%i", nextrunnumber));
  fwRunNumber->TextChanged();
  gClient->NeedRedraw(fwRunNumber);

  fwOutputDirBuffer->Clear();
  fwOutputDirBuffer->AddText(0, fpLM->getOutputDir());
  fwOutputDir->TextChanged();
  gClient->NeedRedraw(fwOutputDir);
  
  doRefreshWindowTitle();

  fCanvas1->cd();
  fCanvas1->Clear();
  TLatex *tl = new TLatex(); tl->SetNDC(kTRUE); tl->SetTextSize(0.15);
  tl->DrawLatex(0.15, 0.5, Form("SCAN FINISHED!"));
  fCanvas1->Modified(); 
  fCanvas1->Update();

}



// ----------------------------------------------------------------------
void daqFrame::dacScan()
{
    fpLM->log("==>daqf: DAC SCAN! ");
    fpLM->setupRun();
    fpLM->log(Form("==>daqf: Open ROOT outputfile %s/takeDataHist.root", fpLM->getOutputDir()));
    fpDAQ->getHistogrammer()->openRootFile(Form("%s/takeDataHist.root", fpLM->getOutputDir()));
    fpDAQ->getHistogrammer()->init("daq");
    fpDAQ->getHistogrammer()->reset();

    TLatex * tl = new TLatex(); tl->SetNDC(kTRUE);

    uint32_t filledMem1(99);
    unsigned short BLOCKSIZE = 32767;
    unsigned char bbuffer[BLOCKSIZE];

    //  Scan loop from to
    int VcalMin = 10;
    int VcalMax = 100;
    for (int idac = VcalMin; idac < VcalMax; idac += 10) {
        char * filename = new char[1000];
        sprintf(filename, "scan%d.bin", idac);
        FILE * f = fopen(filename, "wb");

        fpDAQ->getHistogrammer()->reset();
        cout << "----------------------------------------------------------------------" << endl;
        cout << "--> set Vcal = " << idac << endl;
        for (int iRoc = 0; iRoc < fCN->GetModule(0)->NRocs(); ++iRoc)
        {
            if (fCN->GetModule(0) && (iRoc < fCN->GetModule(0)->NRocs()))
            {
                fCN->SetDAC(0, iRoc, 25, idac);  // change Vcal
                fTB->Flush();
            }
        }

        runStart();
        // -- Run loop and data watching
        fRunDuration = atoi(fwDurationBuffer->GetString());
        for (int i = 0; i < fRunDuration; ++i) {
            sleep(1);

            // -- memory address counter
            Clear();
            /* get number of WORDs written to RAM by the DMA controller */
            filledMem1 = (fTB->getCTestboard()->Daq_GetPointer() - dataBuffer_fpga1);
            fpLM->log(Form("memory fill: %8d", filledMem1));
        }


        for (int k = 0; k < filledMem1 / BLOCKSIZE; ++k)  {
            fTB->getCTestboard()->MemRead((uint32_t)(dataBuffer_fpga1 + k * BLOCKSIZE), BLOCKSIZE, bbuffer);
            fwrite(bbuffer, BLOCKSIZE, 1, f);               // Write to disk
            fpDAQ->setBinaryBuffer(BLOCKSIZE, &bbuffer[0]); // setup analysis
            fpDAQ->singleStep();                            // histogramm
        }
        cout << "---------------->>>>> done" << endl;
        fclose(f);

        fCanvas1->cd();
        TH1 * h  = fpDAQ->getHistogrammer()->getHistogram("h200");
        h->Draw("");

        double nero = h->Integral(67, 201); // non empty readouts
        double nero1 = h->Integral(63, 201); // all readouts

        tl->DrawLatex(0.7, 0.7, Form("nero: %5.3f", nero));
        double tmp = nero / nero1;
        cout << " non empty readouts " << nero << " all readouts " << nero1 << " eff " << tmp << endl;
        fpLM->log(Form("=>> Vcal = %i, Eff: %f, Readouts: %f ", idac, tmp, nero1));
        fCanvas1->Modified(); fCanvas1->Update();
        gSystem->ProcessEvents();
    }

    fCanvas1->Clear(); fCanvas1->Divide(1, 2);
    fCanvas1->Modified(); fCanvas1->Update();

    cout << "end of scan" << endl;

}

void daqFrame::doRefreshWindowTitle(){
  TString thisWindowName = "takeData: ";
  thisWindowName+= fpLM->getMTBConfigParameters()->directory;
  thisWindowName+= ", run ";
  thisWindowName+= fpLM->getRunNumber();
  SetWindowName(thisWindowName);
}


// ----------------------------------------------------------------------
daqFrame::~daqFrame() {

    doHVOFF();
    doPOFF();

    Cleanup();
}

void daqFrame::doVup(int V) {

    Power_supply->Open();
    Power_supply->Init();
    int volt = 25, step = 25;
    while (volt < V - 25) {
        Power_supply->SetVoltage(volt, 1);
        volt = volt + step;
        if (volt > 400) {step = 10;}
        if (volt > 600) {step = 5;}
    }
    Power_supply->SetVoltage(V, 4);

    float v, c;
    Power_supply->ReadCurrent(v, c);
    ofstream current_file;
    current_file.open("../output/CCEStudy/currentModule/Current.txt");
    c = c * -1000000;
    current_file << c;
    current_file.close();
}

void daqFrame::doVdown(int V) {
    int step = 25;
    while (V > 25) {
        Power_supply->SetVoltage(V, 1);
        V = V - step;
        if (V < 600) {step = 10;}
        if (V < 400) {step = 25;}
        if (V > 600) {step = 5;}
    }

    Power_supply->ShutDown();
    delete Power_supply;

}
void daqFrame::setRunDuration(int duration){
  fRunDuration = duration;
  // update text field in gui
  fwDuration->SetText(Form("%i", fRunDuration));
  fwDuration->TextChanged();
  gClient->NeedRedraw(fwDuration);
}

void daqFrame::ApplyMaskFile(const char *fileName){ // AP: added 200612 

  using namespace std;//DP 8.6.2011

  char fname[1000];
  sprintf(fname, "%s", fileName);
  
  int roc, col, row;	
  char keyWord[100], line[1000];

  ifstream maskFile;
  maskFile.open(fname);
  
  if (maskFile.bad()) 
  {
      cout << "!!!!!!!!!  ----> Could not open file "<<fname<<" to read pixel mask\n";
      return;
  }  
  
  cout << "Reading pixel mask from "<< fname << endl;
  
  while(maskFile.good()){
    maskFile>>keyWord;
    if (strcmp(keyWord,"#")==0){ 
       maskFile.getline(line,60, '\n');
       cout << "# "<<line << endl;// ignore rows starting with "#" = comment
    }
    else if(strcmp(keyWord,"pix")==0){
       maskFile>>roc>>col>>row;
       cout << "Exclude "<<keyWord<<" "<<roc<<" "<<col<<" "<<row<<endl; 
       if ((roc >= 0)&&(roc < MODULENUMROCS)&&(col >= 0)&&(col < ROCNUMCOLS)&&(row >= 0)&&(row < ROCNUMROWS)){
	 RemovePix(roc,col,row);
       }else{
         cout << "!!!!!!!!!  ----> Pixel number out of range: "<<keyWord<<" "<<roc<<" "<<col<<" "<<row<<endl;
       }
    }else if(strcmp(keyWord,"col")==0){
       maskFile>>roc>>col;
       cout << "Exclude "<<keyWord<<" "<<roc<<" "<<col<<endl;
       if ((roc >= 0)&&(roc < MODULENUMROCS)&&(col >= 0)&&(col < ROCNUMCOLS)){
	 ExcludeColumn(roc,col); 
       }else{
         cout << "!!!!!!!!!  ----> Pixel number out of range: "<<keyWord<<" "<<roc<<" "<<col<<endl; 
       }
    }else if(strcmp(keyWord,"row")==0){
       maskFile>>roc>>row;
       cout << "Exclude "<<keyWord<<" "<<roc<<" "<<row<<endl;
       if ((roc >= 0)&&(roc < MODULENUMROCS)&&(row >= 0)&&(row < ROCNUMROWS)){
	 ExcludeRow(roc,row);   
       }else{
         cout << "!!!!!!!!!  ----> Pixel number out of range: "<<keyWord<<" "<<roc<<" "<<row<<endl;
       }
    }else if(strcmp(keyWord,"roc")==0){
       maskFile>>roc;
       cout << "Exclude "<<keyWord<<" "<<roc<<endl;
       if ((roc >= 0)&&(roc < MODULENUMROCS)){
	 ExcludeRoc(roc);         
       }else{
         cout << "!!!!!!!!!  ----> Pixel number out of range: "<<keyWord<<" "<<roc<<" "<<col<<" "<<row<<endl;
       }       
    } 
    sprintf(keyWord,"\0");   
  
  }
  
  maskFile.close();
  
  return;
  
}


void daqFrame::RemovePix(int iRoc, int col, int row) // AP: added 200612
{
  Roc *r;
  r = fCN->GetModule(0)->GetRoc(0);
  r->PixMask(col, row);
}

bool daqFrame::ExcludeColumn(int iRoc, int column) // AP: added 200612
{
  bool result = false;
  for (int l = 0; l < ROCNUMROWS; l++) 
    {
      Roc *r;
      r = fCN->GetModule(0)->GetRoc(0);  
      r->PixMask(column, l);
      result = true ;
    }
  return result;
}

bool daqFrame::ExcludeRow(int iRoc, int row) // AP: added 200612
{
  bool result = false;
  for (int l = 0; l < ROCNUMCOLS; l++) 
    {
      Roc *r;
      r = fCN->GetModule(0)->GetRoc(0);  
      r->PixMask(l, row);
      result = true ;
    }
  return result;
}

bool daqFrame::ExcludeRoc(int iRoc) // AP: added 200612
{
  bool result = false;
  for (int l = 0; l < ROCNUMROWS; l++) {
    for (int m = 0; m < ROCNUMCOLS; m++){
      Roc *r;
      r = fCN->GetModule(0)->GetRoc(0);  
      r->PixMask(m, l);
      result = true ;
    }
  }  
  return result;	
}
