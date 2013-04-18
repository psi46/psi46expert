#include <cstdlib>
#include "psi46expert/UsbDaq.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include <time.h>
#include <TFile.h>
#include <TString.h>
#include <TApplication.h>
#include <TStyle.h>

using namespace std;
using namespace DecoderCalibrationConstants;
using namespace DecodedReadoutConstants;

// -- Global pointers to raw data and pixels arrays
int * gDataMTB;
int gNMTB;

UsbDaq * daqMTB;


// ----------------------------------------------------------------------
void bookHistograms() {
    cout << "readData: Book user histograms" << endl;

    gFile->cd();
    TH1D * h1;

    h1 = new TH1D("m1", "levels", 2000, -2000., 2000.);
    h1 = new TH1D("m10", "TB UltraBlack level", 2000, -2000., 2000.);
    h1 = new TH1D("m11", "TB Black level", 2000, -2000., 2000.);

    h1 = new TH1D("m20", "ROCs UltraBlack level", 2000, -2000., 2000.);
    h1 = new TH1D("m21", "ROCs Black level", 2000, -2000., 2000.);

    h1 = new TH1D("m30", "r/o length", 500, 0., 500.);
    h1 = new TH1D("m31", "r/o length before overflow", 2500, 0., 2500.);
    h1 = new TH1D("m32", "data r/o length before overflow", 2500, 0., 2500.);
    h1 = new TH1D("m33", "time between trigger and previous trigger", 1000, -20000., 20000.);
    h1 = new TH1D("m34", "time between trigger and previous data r/o", 2000, -1000., 19000.);
    h1 = new TH1D("m35", "time between trigger and (previous data r/o + data r/o length)", 2000, -1000., 19000.);
}


// ----------------------------------------------------------------------
void fillHistograms(int header = 1) {

    static int oldSize(0);
    static int oldDSize(0);

    static unsigned int oldDTime(0);
    static unsigned int oldTTime(0);

    // -- data
    if (header == 1) {
        ((TH1D *)gFile->Get("m30"))->Fill(float(gNMTB));
    }

    // -- overflow
    if (header == 80) {
        ((TH1D *)gFile->Get("m31"))->Fill(oldSize);
        ((TH1D *)gFile->Get("m32"))->Fill(oldDSize);
    }

    // -- trigger time compared to previous event
    if (header == 4) {
        int deltaT = daqMTB->getLower() - oldTTime;
        cout << "..: " << deltaT << endl;
        ((TH1D *)gFile->Get("m33"))->Fill(deltaT);

        deltaT = daqMTB->getLower() - oldDTime;
        ((TH1D *)gFile->Get("m34"))->Fill(deltaT);
        deltaT = daqMTB->getLower() - (oldDTime + oldDSize);

        if (176368 == daqMTB->getLower()) {
            cout << daqMTB->getLower() << "  " << oldDTime << "  " << oldDSize << "  " << deltaT << endl;
        }
        ((TH1D *)gFile->Get("m35"))->Fill(deltaT);

        if (176368 == daqMTB->getLower()) {
            cout << daqMTB->getLower() << "  " << oldDTime << "  " << oldDSize << "  " << deltaT << endl;
        }
    }

    // -- store this event for next event
    oldSize = gNMTB;
    if (header == 1) {
        oldDSize = gNMTB;
        oldDTime = daqMTB->getLower();
    }

    if (header == 4) {
        oldTTime = daqMTB->getLower();
    }

    //  cout << "MTB data " << endl;
    for (int i = 0; i < gNMTB; ++i) {
        //    cout << " " << gDataMTB[i] << endl;
        ((TH1D *)gFile->Get("m1"))->Fill(gDataMTB[i]);

        if (i < 3) {
            ((TH1D *)gFile->Get("m10"))->Fill(gDataMTB[i]);
        }

        if ((i > 2) && (i < 56) && (gDataMTB[i] < -450)) {
            ((TH1D *)gFile->Get("m11"))->Fill(gDataMTB[i]);
        }
    }
}



// ----------------------------------------------------------------------
int main(int argc, char * argv[]) {

    int mode(0);
    char filename[1000];
    sprintf(filename, "adc.txt");
    char rootfilename[1000];
    sprintf(rootfilename, "test.root");
    int noOutput(0);

    // ----------------------------------------------------------------------
    // -- command line arguments
    for (int i = 0; i < argc; i++) {
        if (!strcmp(argv[i], "-a")) {mode = -1; strcpy(filename, argv[++i]); }
        if (!strcmp(argv[i], "-b")) {mode =  0; strcpy(filename, argv[++i]); }
        if (!strcmp(argv[i], "-n")) {noOutput = 1; }
        if (!strcmp(argv[i], "-r")) {strcpy(rootfilename, argv[++i]); }
        if (!strcmp(argv[i], "-o")) {strcpy(rootfilename, argv[++i]); }
    }

    // -- test
    RawPacketDecoder * decoder1 = RawPacketDecoder::Singleton();

    cout << "Manual calibration for modules" << endl;

    TString fileName = TString("addressParameters.dat");
    cout << "Reading Address Level-Parameters from " << fileName << endl;
    DecoderCalibrationModule * decoderCalibrationModule = new DecoderCalibrationModule(fileName, 3, 0, 16);
    decoderCalibrationModule->Print(&cout);
    decoder1->SetCalibration(decoderCalibrationModule);

    TFile * rf = new TFile(rootfilename, "RECREATE");

    // -- default histograms
    rf->cd();
    histogrammer * hMTB = new histogrammer();
    hMTB->setRootFile(rf);
    hMTB->init("MTB");

    // -- Data handler for MTB
    daqMTB = new UsbDaq(mode);
    if (noOutput) daqMTB->doNotWrite();
    cout << "Read MTB data from " << Form("%s/mtb.bin", filename) << endl;
    daqMTB->setInputFileName(Form("%s/mtb.bin", filename));
    daqMTB->setOutputFileName(Form("%s/mtb.txt", filename));
    daqMTB->setHistogrammer(hMTB);
    daqMTB->setDecoder(decoder1);
    daqMTB->start();
    gDataMTB = daqMTB->getRawData();

    // -- Initialize user histograms
    bookHistograms();

    int header(0);
    int mtbEvent(1);
    while (1) {
        mtbEvent = daqMTB->singleStep();
        gNMTB    = daqMTB->getRawDataSize();

        // -- stop if at end of either data source
        if (mtbEvent == 0) break;

        header = daqMTB->getHeader();
        cout << "==> " << header << " " << gNMTB << endl;
        fillHistograms(header);
    }
    // -- Write root file
    rf->Write();
    rf->Close();

    return 0;
}
