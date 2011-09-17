#include <iostream>
#include "daqDACSettings.hh"
#include "daqFrame.hh"
#include "psi46expert/TestControlNetwork.h"
#include "BasePixel/TBAnalogInterface.h"

daqDACSettings::daqDACSettings(const TGWindow *p, const TGWindow *daq, int tbNumber) {
  // build widgets
  
  fTB   = tbNumber;
  fDAQF = (daqFrame*)daq;

  fMain = new TGTransientFrame(p, daq, 10, 10, kHorizontalFrame);
  fMain->Connect("CloseWindow()", "daqDACSettings", this, "CloseWindow()");
  
  fF1 = new TGVerticalFrame(fMain, 200, 300);
  fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2);
  fMain->AddFrame(fF1, fL1);
  fL2 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 1, 1, 1, 1);

  for (int i = 0; i < fNDACParameters; i++) {
    fParValues[i] = -1;
    fParNames[i] = "";
    fParIndex[i] = 0;
    fNumericEntries[i] = 0;
  }

  fParNames[1] = TString("Vdig");
  fParNames[2] = TString("Vana");
  fParNames[3] = TString("Vsf");
  fParNames[4] = TString("Vcomp");
  fParNames[5] = TString("Vleak_comp");
  fParNames[6] = TString("VrgPr");
  fParNames[7] = TString("VwllPr");
  fParNames[8] = TString("VrgSh");
  fParNames[9] = TString("VwllSh");
  fParNames[10] = TString("VhldDel");
  fParNames[11] = TString("Vtrim");
  fParNames[12] = TString("VthrComp");
  fParNames[13] = TString("VIBias_Bus");
  fParNames[14] = TString("Vbias_sf");
  fParNames[15] = TString("VoffsetOp");
  fParNames[16] = TString("VIbiasOp");
  fParNames[17] = TString("VOffsetR0");
  fParNames[18] = TString("VIon");
  fParNames[19] = TString("VIbias_PH");
  fParNames[20] = TString("Ibias_DAC");
  fParNames[21] = TString("VIbias_roc");
  fParNames[22] = TString("VIColOr");
  fParNames[23] = TString("Vnpix");
  fParNames[24] = TString("VSumCol");
  fParNames[25] = TString("Vcal");
  fParNames[26] = TString("CalDel");
  fParNames[27] = TString("RangeTemp");
  fParNames[253] = TString("CtrlReg");
  fParNames[254] = TString("WBC");
  

  TGTextButton *wReadFile = new TGTextButton(fMain, "Read");
  wReadFile->Connect("Clicked()", "daqDACSettings", this, "ReadFile()");
  fMain->AddFrame(wReadFile, new TGLayoutHints(kLHintsLeft,2, 2, 2, 2));
  
  TGTextEntry *wFileName = new TGTextEntry(fMain, fFileNameBuffer = new TGTextBuffer(80));
  wFileName->Resize(200, wFileName->GetDefaultHeight());
  wFileName->SetText("../BasePixel/defaultDACParameters.dat");
  fMain->AddFrame(wFileName, new TGLayoutHints(kLHintsExpandX,2, 2, 2, 2)); 

  int bla = ReadFile(); 
  fParMax = 0;
  for (int i = 0; i < fNDACParameters; i++) {
    if (!strcmp(fParNames[i].Data(), "")) continue;
    fParIndex[fParMax] = i;
    cout << " new TGNumberEntry(fF[fParIndex[fParMax]], fParIndex[fParMax] = " << fParIndex[fParMax] << endl;
    fF[fParIndex[fParMax]] = new TGHorizontalFrame(fF1, 200, 30);
    fF1->AddFrame(fF[fParIndex[fParMax]], fL2);
    fNumericEntries[fParIndex[fParMax]] = new TGNumberEntry(fF[fParIndex[fParMax]], 
							 fParValues[fParIndex[fParMax]], 
							 4, i, TGNumberFormat::kNESInteger);
    fF[fParIndex[fParMax]]->AddFrame(fNumericEntries[fParIndex[fParMax]], fL2);
    //    fLabel[i] = new TGLabel(fF[i], numlabel[i], myGC(), myfont->GetFontStruct());
    fLabel[fParIndex[fParMax]] = new TGLabel(fF[fParIndex[fParMax]], fParNames[fParIndex[fParMax]].Data());
    fF[fParIndex[fParMax]]->AddFrame(fLabel[fParIndex[fParMax]], fL2);
    ++fParMax;
  }
  fF2 = new TGVerticalFrame(fMain, 100, 500);
  fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2);
  fMain->AddFrame(fF2, fL3);

  fSetButton = new TGTextButton(fF2, " Set ", 2);
  fSetButton->Connect("Clicked()", "daqDACSettings", this, "Set()");
  fF2->AddFrame(fSetButton, fL3);
  fExitButton = new TGTextButton(fF2, " Close ", 1);
  fExitButton->Connect("Clicked()", "daqDACSettings", this, "DoOK()");
  fF2->AddFrame(fExitButton, fL3);
  
  // set dialog box title
  fMain->SetWindowName("DAC Parameter Settings");

  // resize & move to center
  fMain->MapSubwindows();
  UInt_t width = fMain->GetDefaultWidth();
  UInt_t height = fMain->GetDefaultHeight();
  fMain->Resize(width, height);
  //  fMain->CenterOnParent();
  // make the message box non-resizable
  fMain->SetWMSize(width, height);
  fMain->SetWMSizeHints(width, height, width, height, 0, 0);
  fMain->SetMWMHints(kMWMDecorAll | kMWMDecorResizeH | kMWMDecorMaximize |
		     kMWMDecorMinimize | kMWMDecorMenu,
		     kMWMFuncAll | kMWMFuncResize | kMWMFuncMaximize |
		     kMWMFuncMinimize, kMWMInputModeless);
  
  fMain->MapWindow();
  gClient->WaitFor(fMain);
}

daqDACSettings::~daqDACSettings() {
  for (int i = 0; i < fParMax; i++) {
    delete fNumericEntries[fParIndex[i]];
    delete fLabel[fParIndex[i]];
    delete fF[fParIndex[i]];
  }
  delete fSetButton;
  delete fExitButton;
  delete fF1;
  delete fF2;
  delete fL1;
  delete fL2;
  delete fL3;
  delete fMain;
}


// ----------------------------------------------------------------------
void daqDACSettings::CloseWindow() {
  delete this;
}


// ----------------------------------------------------------------------
void daqDACSettings::DoOK() {
  // Handle ok button.
  fMain->SendCloseMessage();
}


// ----------------------------------------------------------------------
void daqDACSettings::Set() {
  for (int i = 0; i < fParMax; i++) {
    cout << fNumericEntries[fParIndex[i]]->GetIntNumber() << endl;
    fDAQF->getControlNetwork(fTB)->SetDAC(0, 0, fParIndex[i], fNumericEntries[fParIndex[i]]->GetIntNumber());
  }
  
  fDAQF->getTbInterface(fTB)->Flush();
}


// ----------------------------------------------------------------------
int daqDACSettings::ReadFile() {
  char filename[1000];
  sprintf(filename, fFileNameBuffer->GetString());

  cout << "Read File " << filename << endl;

  fFile = fopen(filename, "r");
  if (!fFile) return -1;
  
  int reg(0), value(0);
  printf("Reading DAC-Parameters from %s\n", filename);
  
  do {
    cout << Form("reading dac-param %s %i %i from file", fParNames[reg].Data(), reg, value) << endl;
    readInteger(reg, value, fFile);
    cout << Form("read, reg = %i, value = %i", reg, value) << endl;
    if (reg > fNDACParameters) {
      fclose(fFile);
      return 0;
    }
    setParameter(reg, value);
  }
  while (!feof(fFile));
  fclose(fFile);
  return 0;
}

// ----------------------------------------------------------------------
void daqDACSettings::readInteger(int &reg, int &value, FILE *file) {
  char s[200];
  char string[10];
  fgets(s, 198, file);
  if (s == NULL) return;
  
  int v,i;
  sscanf(s,"%i %s %i ",&i, &string, &v);
  value = v;
  reg = i;
  printf("readInteger reg: %i value: %i\n",reg, value);
}


// ----------------------------------------------------------------------
void daqDACSettings::setParameter(const char* dacName, int value) {
  bool parameterSet = false;
  for (int i = 0; i < fNDACParameters; i++) {
    if (strcmp(fParNames[i].Data(), dacName) == 0) {
      setParameter(i, value);
      parameterSet = true;
      i = fNDACParameters;
    }
  }
  if (!parameterSet) {printf(" >>>>>>>>>>> Error: DAC Parameter %s not found\n",dacName);}
}


// ----------------------------------------------------------------------
void daqDACSettings::setParameter(int reg, int value) {
  int correctValue;
  if (reg == 25) {
    correctValue = value; //calibrationTable->CorrectedVcalDAC(value);
  }
  else {
    correctValue = value;
  }
  fParValues[reg] = correctValue;
  //  roc->RocSetDAC(reg,correctValue);

  cout << fParIndex[reg] << endl;
  if (fNumericEntries[fParIndex[reg]]) fNumericEntries[fParIndex[reg]]->SetIntNumber(correctValue);

  printf("parameters %i set to %i\n", reg, value);
}

