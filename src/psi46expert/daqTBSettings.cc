#include <iostream.h>
#include "daqTBSettings.hh"

#include "BasePixel/TBAnalogInterface.h"
#include "psi46expert/daqFrame.hh"

daqTBSettings::daqTBSettings(const TGWindow *p, const TGWindow *daq, int tbNumber) {

  fTB   = tbNumber;
  fDAQF = (daqFrame*)daq;

  // build widgets
  fMain = new TGTransientFrame(p, daq, 10, 10, kHorizontalFrame);
  fMain->Connect("CloseWindow()", "daqTBSettings", this, "CloseWindow()");
  
  fF1 = new TGVerticalFrame(fMain, 200, 300);
  fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 2, 2, 2, 2);
  fMain->AddFrame(fF1, fL1);
  fL2 = new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 1, 1, 1, 1);

  for (int i = 0; i < fNTBParameters; i++) {
    fParValues[i] = -1;
    fParNames[i] = "";
    fParIndex[i] = 0;
    fNumericEntries[i] = 0;
  }

  fParNames[17] = TString("trc");
  fParNames[18] = TString("tcc");
  fParNames[19] = TString("tct");
  fParNames[20] = TString("ttk");
  fParNames[21] = TString("trep");
  fParNames[22] = TString("cc");
  fParNames[8] = TString("clk");
  fParNames[9] = TString("sda");
  fParNames[10] = TString("ctr");
  fParNames[11] = TString("tin");
  fParNames[77] = TString("spd");

  TGTextButton *wReadFile = new TGTextButton(fMain, "Read");
  wReadFile->Connect("Clicked()", "daqTBSettings", this, "ReadFile()");
  fMain->AddFrame(wReadFile, new TGLayoutHints(kLHintsLeft,2, 2, 2, 2));
  
  TGTextEntry *wFileName = new TGTextEntry(fMain, fFileNameBuffer = new TGTextBuffer(80));
  wFileName->Resize(200, wFileName->GetDefaultHeight());
  wFileName->SetText("../BasePixel/defaultTBParameters-BT40Mhz.dat");
  fMain->AddFrame(wFileName, new TGLayoutHints(kLHintsExpandX,2, 2, 2, 2)); 

  int bla = ReadFile(); 
  fParMax = 0;
  for (int i = 0; i < fNTBParameters; i++) {
    if (!strcmp(fParNames[i].Data(), "")) continue;
    fParIndex[fParMax] = i;
    //    cout << " new TGNumberEntry(fF[fParIndex[fParMax]], fParIndex[fParMax] = " << fParIndex[fParMax] << endl;
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
  fSetButton->Connect("Clicked()", "daqTBSettings", this, "Set()");
  fF2->AddFrame(fSetButton, fL3);
  fExitButton = new TGTextButton(fF2, " Close ", 1);
  fExitButton->Connect("Clicked()", "daqTBSettings", this, "DoOK()");
  fF2->AddFrame(fExitButton, fL3);

  fPonButton = new TGTextButton(fF2, " Pon ", 1);
  fPonButton->Connect("Clicked()", "daqTBSettings", this, "DoPon()");
  fF2->AddFrame(fPonButton, fL3);

  fPoffButton = new TGTextButton(fF2, " Poff ", 1);
  fPoffButton->Connect("Clicked()", "daqTBSettings", this, "DoPoff()");
  fF2->AddFrame(fPoffButton, fL3);

  fHVonButton = new TGTextButton(fF2, " HV on ", 1);
  fHVonButton->Connect("Clicked()", "daqTBSettings", this, "DoHVon()");
  fF2->AddFrame(fHVonButton, fL3);

  fHVoffButton = new TGTextButton(fF2, " HV off ", 1);
  fHVoffButton->Connect("Clicked()", "daqTBSettings", this, "DoHVoff()");
  fF2->AddFrame(fHVoffButton, fL3);
  
  // set dialog box title
  fMain->SetWindowName("TB Parameter Settings");

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

daqTBSettings::~daqTBSettings() {
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

  delete fPonButton;
  delete fPoffButton;
  delete fHVonButton;
  delete fHVoffButton;

  delete fMain;
}


// ----------------------------------------------------------------------
void daqTBSettings::CloseWindow() {
  delete this;
}


// ----------------------------------------------------------------------
void daqTBSettings::DoOK() {
  // Handle ok button.
  fMain->SendCloseMessage();
}


// ----------------------------------------------------------------------
void daqTBSettings::Set() {
  for (int i = 0; i < fParMax; i++) {
    cout << "daqTBSettings::Set(): " << fNumericEntries[fParIndex[i]]->GetIntNumber() << endl;
    fDAQF->getTbInterface(fTB)->SetTBParameter(fParIndex[i], fNumericEntries[fParIndex[i]]->GetIntNumber()); 
  }
  fDAQF->getTbInterface(fTB)->Flush();
}


// ----------------------------------------------------------------------
int daqTBSettings::ReadFile() {
  char filename[1000];
  sprintf(filename, fFileNameBuffer->GetString());

  cout << "daqTBSettings::ReadFile(): " << filename << endl;

  fFile = fopen(filename, "r");
  if (!fFile) {printf(" File not found\n"); return -1;}
  
  int reg(0), value(0);
  printf(" Reading TB-Parameters from %s\n", filename);
  
  do {
    cout << Form(" reading tb-param %s %i %i from file", fParNames[reg].Data(), reg, value) << endl;
    readInteger(reg,value, fFile);
    cout << Form(" read, reg = %i, value = %i", reg, value) << endl;
    if (reg > fNTBParameters) {
      fclose(fFile);
      return 0;
    }
    setParameter(reg, value);
    //printf("tb-param %i %i\n",reg,value);
  }
  while (!feof(fFile));
  fclose(fFile);
  //  tbInterface->Flush();
  return 0;
  
}

// ----------------------------------------------------------------------
void daqTBSettings::readInteger(int &reg, int &value, FILE *file) {
  char s[200];
  char string[10];
  fgets(s, 198, file);
  if (s == NULL) return;
  
  int v,i;
  sscanf(s,"%i %s %i ",&i, &string, &v);
  value = v;
  reg = i;
  //  printf("readInteger reg: %i value: %i\n",reg, value);
}


// ----------------------------------------------------------------------
void daqTBSettings::setParameter(const char* tbName, int value) {
  bool parameterSet = false;
  for (int i = 0; i < fNTBParameters; i++) {
    if (strcmp(fParNames[i].Data(), tbName) == 0) {
      setParameter(i, value);
      parameterSet = true;
      i = fNTBParameters;
    }
  }
  if (!parameterSet) {printf(" >>>>>>>>>>> Error: TB Parameter %s not found\n",tbName);}
}


// ----------------------------------------------------------------------
void daqTBSettings::setParameter(int reg, int value) {
  int correctValue;
  if (reg == 25) {
    correctValue = value; //calibrationTable->CorrectedVcalTB(value);
  }
  else {
    correctValue = value;
  }
  fParValues[reg] = correctValue;
  //  roc->RocSetTB(reg,correctValue);

  cout << "daqTBSettings::setParameter()" << fParIndex[reg] << endl;
  if (fNumericEntries[fParIndex[reg]]) fNumericEntries[fParIndex[reg]]->SetIntNumber(correctValue);

  // printf(" parameter %i set to %i\n", reg, value);
}


// ----------------------------------------------------------------------
void daqTBSettings::DoPon() {
  fDAQF->getTbInterface(fTB)->Pon(); 
  fDAQF->getTbInterface(fTB)->Flush(); 

}


// ----------------------------------------------------------------------
void daqTBSettings::DoPoff() {
  fDAQF->getTbInterface(fTB)->Poff(); 
  fDAQF->getTbInterface(fTB)->Flush(); 
}

// ----------------------------------------------------------------------
void daqTBSettings::DoHVon() {
  fDAQF->getTbInterface(fTB)->HVon(); 
  fDAQF->getTbInterface(fTB)->Flush(); 
}


// ----------------------------------------------------------------------
void daqTBSettings::DoHVoff() {
  fDAQF->getTbInterface(fTB)->HVoff(); 
  fDAQF->getTbInterface(fTB)->Flush(); 
}
