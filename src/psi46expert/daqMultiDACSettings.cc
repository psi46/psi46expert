#include <iostream>
#include "daqMultiDACSettings.hh"

daqMultiDACSettings::daqMultiDACSettings(const TGWindow * p, const TGWindow * main) {

    for (int ic = 0; ic < 16; ++ic) {
        for (int i = 0; i < fNDACParameters; i++) {
            fParValues[ic][i] = -1;
            fParNames[ic][i] = "";
            fParIndex[ic][i] = 0;
        }

        fParNames[ic][1] = TString("Vdig");
        fParNames[ic][2] = TString("Vana");
        fParNames[ic][3] = TString("Vsf");
        fParNames[ic][4] = TString("Vcomp");
        fParNames[ic][5] = TString("Vleak_comp");
        fParNames[ic][6] = TString("VrgPr");
        fParNames[ic][7] = TString("VwllPr");
        fParNames[ic][8] = TString("VrgSh");
        fParNames[ic][9] = TString("VwllSh");
        fParNames[ic][10] = TString("VhldDel");
        fParNames[ic][11] = TString("Vtrim");
        fParNames[ic][12] = TString("VthrComp");
        fParNames[ic][13] = TString("VIBias_Bus");
        fParNames[ic][14] = TString("Vbias_sf");
        fParNames[ic][15] = TString("VoffsetOp");
        fParNames[ic][16] = TString("VIbiasOp");
        fParNames[ic][17] = TString("VOffsetR0");
        fParNames[ic][18] = TString("VIon");
        fParNames[ic][19] = TString("VIbias_PH");
        fParNames[ic][20] = TString("Ibias_DAC");
        fParNames[ic][21] = TString("VIbias_roc");
        fParNames[ic][22] = TString("VIColOr");
        fParNames[ic][23] = TString("Vnpix");
        fParNames[ic][24] = TString("VSumCol");
        fParNames[ic][25] = TString("Vcal");
        fParNames[ic][26] = TString("CalDel");
        fParNames[ic][27] = TString("RangeTemp");
        fParNames[ic][253] = TString("CtrlReg");
        fParNames[ic][254] = TString("WBC");
    }


    fMain = new TGTab(p, 300, 300);
    fMain->Connect("Selected(Int_t)", "TestDialog", this, "DoTab(Int_t)");

    fL1 = new TGLayoutHints(kLHintsTop | kLHintsLeft | kLHintsExpandX, 2, 2, 2, 2);
    fL2 = new TGLayoutHints(kLHintsBottom | kLHintsRight, 2, 2, 5, 1);
    fL3 = new TGLayoutHints(kLHintsTop | kLHintsLeft, 5, 5, 5, 5);

    TGCompositeFrame * tf;

    for (int i = 0; i < 16; ++i) {
        tf = fMain->AddTab(Form("C%2i", i));
        fF[i] = new TGCompositeFrame(tf, 60, 20, kVerticalFrame);
        fF[i]->AddFrame(new TGTextButton(fF[i], "&Test button", 0), fL3);
        tf->AddFrame(fF[i], fL3);

        TGTextButton * wReadFile = new TGTextButton(fF[i], "Read");
        //    wReadFile->Connect("Clicked()", "daqMultiDACSettings", this, "ReadFile()");
        fF[i]->AddFrame(wReadFile, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));

        TGTextEntry * wFileName = new TGTextEntry(fF[i], fFileNameBuffer[i] = new TGTextBuffer(80));
        wFileName->Resize(200, wFileName->GetDefaultHeight());
        wFileName->SetText("defaultDACParameters.dat");
        fF[i]->AddFrame(wFileName, new TGLayoutHints(kLHintsExpandX, 2, 2, 2, 2));

    }

    // set dialog box title
    fMain->SetWindowName("DAC Parameter Settings");

    // resize & move to center
    fMain->MapSubwindows();
    UInt_t width = fMain->GetDefaultWidth();
    UInt_t height = fMain->GetDefaultHeight();
    fMain->Resize(width, height);
    //  fMain->CenterOnParent();
    // make the message box non-resizable

    fMain->MapWindow();
    gClient->WaitFor(fMain);
}

daqMultiDACSettings::~daqMultiDACSettings() {
    delete fSetButton;
    delete fExitButton;
    delete fL1;
    delete fL2;
    delete fL3;
    delete fMain;
}


// ----------------------------------------------------------------------
void daqMultiDACSettings::CloseWindow() {
    delete this;
}


// ----------------------------------------------------------------------
void daqMultiDACSettings::DoOK() {
    // Handle ok button.
    //  fMain->SendCloseMessage();
}


// ----------------------------------------------------------------------
void daqMultiDACSettings::Set() {
    //   for (int i = 0; i < fParMax; i++) {
    //     cout << fNumericEntries[fParIndex[i]]->GetIntNumber() << endl;
    //   }
}

