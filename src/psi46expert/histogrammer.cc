#include "psi46expert/histogrammer.h"

using namespace std;

// ----------------------------------------------------------------------
histogrammer::histogrammer() {
    fOwner = 0;
    lHistograms = new TList();
}


// ----------------------------------------------------------------------
histogrammer::~histogrammer() {
    cout << "histogrammer dtor" << endl;
    if (fOwner) {
        close();
        delete fRootFile;
    }
}

// ----------------------------------------------------------------------
void histogrammer::close() {

    fRootFile->Write();
    fRootFile->Close();

    lHistograms->Clear();

    if (fOwner) {
        delete fRootFile;
    }
}


// ----------------------------------------------------------------------
void histogrammer::openRootFile(const char * rootfilename) {
    // -- ROOT output file
    fOwner = 1;
    fRootFile = new TFile(rootfilename, "RECREATE");
    fRootFile->cd();
}


// ----------------------------------------------------------------------
void histogrammer::init(const char * dirname) {

    fDir = TString(dirname);

    fRootFile->cd();
    fRootFile->mkdir(dirname);
    fRootFile->cd(dirname);

    TH1D * h1;
    // -- Book Histograms
    h1 = new TH1D("h0", Form("%s Events", dirname), 11, -1., 10.);
    lHistograms->AddLast(h1);
    h1 = new TH1D("h100", Form("%s Levels", dirname), 2000, -2000., 2000.);
    lHistograms->AddLast(h1);
    h1 = new TH1D("h101", Form("%s TBM UB", dirname), 2000, -2000., 2000.);
    lHistograms->AddLast(h1);
    h1 = new TH1D("h102", Form("%s TBM B", dirname), 2000, -2000., 2000.);
    lHistograms->AddLast(h1);
    h1 = new TH1D("h103", Form("%s ROC UB", dirname), 2000, -2000., 2000.);
    lHistograms->AddLast(h1);
    h1 = new TH1D("h104", Form("%s ROC B", dirname), 2000, -2000., 2000.);
    lHistograms->AddLast(h1);


    h1 = new TH1D("h200", Form("%s roLength", dirname), 200, 0., 200.);
    lHistograms->AddLast(h1);

    h1 = new TH1D("h201", Form("%s roLength", dirname), 200, 0., 200.);
    lHistograms->AddLast(h1);

    fpCurrent = 0;

    fRootFile->cd();

}

// ----------------------------------------------------------------------
void histogrammer::reset() {
    TH1 * h = (TH1 *)lHistograms->First();
    while (h) {
        if (h) {
            h->Reset();
            h = (TH1 *)lHistograms->After(h);
        } else {
            h = 0;
        }
    }
}


// ----------------------------------------------------------------------
void histogrammer::setRawData(int (*p)[DecodedReadoutConstants::MAX_PIXELSROC]) {
    cout << "Set fRawData to " << p << endl;
    fRawData = p;
}


// ----------------------------------------------------------------------
TH1 * histogrammer::getNextHistogram() {

    TH1 * h = (TH1 *)lHistograms->Last();
    h->Print();

    if (fpCurrent) fpCurrent->Print();

    if (fpCurrent == (TH1 *)lHistograms->Last()) {
        fpCurrent = 0;
    }

    if (!fpCurrent) {
        fpCurrent = (TH1 *)lHistograms->First();
    }
    else {
        fpCurrent = (TH1 *)lHistograms->After(fpCurrent);
    }
    return fpCurrent;

}


// ----------------------------------------------------------------------
TH1 * histogrammer::getHistogram(const char * hist) {

    TH1 * h = (TH1 *)lHistograms->First();
    while (h) {
        //    cout << h << endl;
        if (!strcmp(h->GetName(), hist)) {
            //      h->Print();
            return h;
        }
        h = (TH1 *)lHistograms->After(h);
    }
    return fpCurrent;

}


// ----------------------------------------------------------------------
void histogrammer::printRawData(int nRawData) {

    cout << " histogrammer::Print(): size = " << nRawData << endl;
    for (int i = 0; i < nRawData; ++i) {
        cout << " " << (*fRawData)[i];
    }
    cout << endl;

}


// ----------------------------------------------------------------------
void histogrammer::fillRawDataHistograms(int nRawData, int header) {
    const char * pwd = gDirectory->GetName();
    fRootFile->cd(fDir.Data());

    int iData;
    TH1D * h1;

    // -- event types: 1 data 4 trig 8 reset
    h1 = (TH1D *)gDirectory->Get("h0");
    h1->Fill(-1.);
    if (header > 0) {
        for (int ibit = 0; ibit < 8; ++ibit) {
            if (header & (0x1 << ibit)) h1->Fill(ibit);
        }
    }

    ((TH1D *)gDirectory->Get("h201"))->Fill(nRawData);
    //cout <<header<<" "<<nRawData<<endl;

    // -- only look at data
    if (header == 1) {

        ((TH1D *)gDirectory->Get("h200"))->Fill(nRawData);

        for (int i = 0; i < nRawData; ++i) {
            iData = (*fRawData)[i];

            ((TH1D *)gDirectory->Get("h100"))->Fill(iData);

            if (i < 3) { // U TBM
                ((TH1D *)gDirectory->Get("h101"))->Fill(iData);
            }
            if (i == 3) { // B TBM
                ((TH1D *)gDirectory->Get("h102"))->Fill(iData);
            }

            if (i == 8) { // U ROC
                ((TH1D *)gDirectory->Get("h103"))->Fill(iData);
            }

            if (i == 9) { // B ROC
                ((TH1D *)gDirectory->Get("h104"))->Fill(iData);
            }

            //      if (i > 10 && i < nRawData-6) {
            //    if ( (*fRawData)[i-3]

        }

    }

    // -- Move back to where you came from
    if (!strcmp(pwd, fRootFile->GetName())) {
        fRootFile->cd();
    } else {
        fRootFile->cd(pwd);
    }
}


// ----------------------------------------------------------------------
void histogrammer::fillPixelHistograms(int nPixel) {
    fRootFile->cd();
    for (int i = 0; i < nPixel; ++i) {
        ((TH1D *)gFile->Get("h0"))->Fill(2.);
    }

}


