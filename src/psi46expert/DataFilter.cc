#include "DataFilter.h"
#include <iostream>
#include <cstdio>
#include <cstring>
#include <TMath.h>
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DigitalReadoutDecoder.h"
#include "BasePixel/GlobalConstants.h"

#include "TLinearFitter.h"

using namespace std;

#ifdef _WIN32
#define QWFMT "%15I64u"  // !!! only for windows
#else
#define QWFMT "%15llu"   // !!! probably works for Linux
#endif

// === CRawEvent ============================================================

void CRawEvent::PrintEventFlags()
{
    char c[] = "[VIrCRTtD]";
    unsigned int i, mask;


    for (i = 1, mask = 0x80; i <= 8; i++, mask >>= 1)
        if (!(flags & mask)) c[i] = '.';
    cout << c;
}


void CRawEvent::PrintDataFlags(unsigned int index)
{
    switch (dflag[index])
    {
    case 0: cout << " "; break;
    case 4: cout << "H"; break;
    case 1: cout << "T"; break;
    case 2: cout << "R"; break;
    case 3: cout << "E"; break;
    default: cout << "?";
    }
}


void CRawEvent::PrintData()
{
    unsigned int i, nPix;
    if (length < 64) nPix = 0; else nPix = (length - 64) / 6;
    printf("<%4u/%3u>", length, nPix);
    for (i = 0; i < length; i++)
    {
        printf("%6i", data[i]);
        PrintDataFlags(i);
    }
    cout << "\n";
}


void CRawEvent::Print()
{
    printf("%02X", flags >> 8);
    PrintEventFlags();

    printf("(" QWFMT ")", time);

    PrintData();
}

// === CEvent ===============================================================

void CEvent::print()
{
    cout << (isReset ? 'R' : '.');
    cout << (isCalibrate ? 'C' : '.');
    cout << (isTrigger ? 'T' : '.');
    cout << (isData ? 'D' : '.');
    cout << (isOverflow ? 'V' : '.');
    cout << " " << timestamp;
    cout << endl;

    if (nHits > 0) {
        for (int r = 0; r < nRocs; r++) {
            int h = hits.roc[r].numPixelHits;
            for (int i = 0; i < h; i++) {
                cout << "ROC " << r << " hit: "
                     << hits.roc[r].pixelHit[i].columnROC
                     << " "
                     << hits.roc[r].pixelHit[i].rowROC << endl;
            }
        }
    }
}

/* Pipe which reads short integers from the testboard RAM. Does not read from any previous pipe. -------------- */

#define BLOCKSIZE 32768 /* Must be an even number! */

RAMRawDataReader::RAMRawDataReader(CTestboard * b, unsigned int ramstart, unsigned int ramend, unsigned int length)
{
    buffer = new unsigned short [(BLOCKSIZE + 1) / 2];
    buffersize = 0;
    bufferpos = 0;

    board = b;
    datastart = ramstart;
    databuffersize = ramend - ramstart;
    dataend = datastart + length;
    dataptr = datastart;
}

RAMRawDataReader::~RAMRawDataReader()
{
    if (buffer)
        delete buffer;
}

PipeObjectShort * RAMRawDataReader::Write()
{
    if (buffersize > 0 && bufferpos < buffersize) {
        /* there is unread data in the buffer */
        s.s = buffer[bufferpos++];
        return &s;
    } else if (buffersize > 0 && bufferpos >= buffersize || buffersize == 0) {
        /* All data was read from the buffer or buffer empty. Get new data from the RAM */

        cout << "> Megabytes left to read: " << (dataend - dataptr) / 1024 / 1024 << " \r";
        cout.flush();

        /* Check whether there is data left to read */
        if (dataptr >= dataend) {
            cout << "                              \r";
            cout.flush();
            return NULL;
        }

        /* Read blocks of BLOCKSIZE bytes. */
        unsigned int bytes = (dataend - dataptr < BLOCKSIZE) ? (dataend - dataptr) : BLOCKSIZE;

        /* Reading smaller amounts leads to corrupt data for some reason. */
        unsigned int offset = 0;
        if (bytes < BLOCKSIZE && databuffersize > BLOCKSIZE) {
            if (dataptr != datastart)
                offset = BLOCKSIZE - bytes;
            bytes = BLOCKSIZE;
        }

        /* Read the data */
        board->Flush();
        board->Clear();
        memset(buffer, BLOCKSIZE, '0');
        board->MemRead(dataptr - offset, bytes, (unsigned char *) buffer);
        board->Flush();
        board->Clear();
        dataptr += bytes - offset;
        if ((bytes % 2) == 1)
            cout << "reading odd number of bytes!" << endl;
        if ((offset % 2) == 1)
            cout << "offset is odd number!" << endl;
        buffersize = bytes / 2;
        bufferpos = offset / 2;
        s.s = buffer[bufferpos++];
        return &s;
    }
}

/* Pipe that brakes raw data into raw events --------------------------------- */

PipeObjectShort * RawData2RawEvent::Read()
{
    return static_cast<PipeObjectShort *>(source->Write());
}

CRawEvent * RawData2RawEvent::Write()
{
    /* This variable keeps its value between calls to the function. In the beginning it has the value NULL. */
    static PipeObjectShort * last = NULL;

    /* Fetch a value for the first time */
    if (!last)
        last = Read();

    /* If GetNext() returns NULL, it's the last entry */
    if (!last)
        return NULL;

    /* Read the event header */
    rawevent.flags = last->s;

    /* Check header validity */
    if (!rawevent.IsHeaderOk()) {
        cout << "RawData2RawEvent: Header expected, but no header found!" << endl;
        return NULL;
    }

    /* Read the timestamp (number of clockcycles, 48 bit) */
    if (!(last = Read()))
        return NULL;
    rawevent.time = (unsigned short) last->s;
    if (!(last = Read()))
        return NULL;
    rawevent.time = (rawevent.time << 16) | (unsigned short) last->s;
    if (!(last = Read()))
        return NULL;
    rawevent.time = (rawevent.time << 16) | (unsigned short) last->s;

    /* Read data */
    if (!(last = Read()))
        return NULL;
    int d = last->s;
    rawevent.length = 0;
    while (!(d & 0x8000)) {
        if (rawevent.length < MAXDATASIZE) {
            /* remove the data header and extend the sign */
            rawevent.data[rawevent.length]
                = (d & 0x0800) ? (d & 0x0fff) - 4096 : (d & 0xfff);
            rawevent.dflag[rawevent.length] = (d >> 12) & 7;
            rawevent.length++;
        } else {
            /* Buffer overflow */
            cout << endl << "RawData2RawEvent: Buffer overflow" << endl;
            return NULL;
        }
        /* If this is the last entry in the buffer, then complete the raw event and return it. The
           next call to this function will attempt to get the next event, and then will fail. */
        if (!(last = Read()))
            break;
        d = last->s;
    }

    return &rawevent;
}

/* Pipe that decodes the analog readout from raw events ------------------------------------------------- */

RawEventDecoder::RawEventDecoder(unsigned int n, bool analog, bool row_address_inverted)
{
    nROCs = n;
    this->analog = analog;
    this->row_address_inverted = row_address_inverted;
    decoding_errors = 0;
}

CRawEvent * RawEventDecoder::Read()
{
    return static_cast<CRawEvent *>(source->Write());
}

CEvent * RawEventDecoder::Write()
{
    CRawEvent * rawevent;
    rawevent = Read();
    if (!rawevent) {
        return NULL;
    }
    decoded_event.isData = rawevent->IsData();
    decoded_event.isTrigger = rawevent->IsTrigger();
    decoded_event.isReset = rawevent->IsRocReset();
    decoded_event.isCalibrate = rawevent->IsCalibrate();
    decoded_event.isOverflow = rawevent->IsOverflow();
    decoded_event.timestamp = rawevent->time;
    decoded_event.nRocs = nROCs;
    decoded_event.nHits = 0;

    /* Decode the analog data, if available */
    if (decoded_event.isData && rawevent->length > 0) {
        if (analog) {
            RawPacketDecoder * decoder = RawPacketDecoder::Singleton();
            decoded_event.nHits = decoder->decode(rawevent->length, (short *) rawevent->data, decoded_event.hits, nROCs);
        } else {
            int ret;
            int flags = this->row_address_inverted ? DRO_INVERT_ROW_ADDRESS : 0;
            ret = decode_digital_readout(&(decoded_event.hits), (short *) rawevent->data, rawevent->length, nROCs, flags);
            decoded_event.nHits = (ret >= 0) ? decoded_event.hits.roc[0].numPixelHits : ret;
        }
        /* go through the hits to find address decoding errors */
        if (decoded_event.nHits < 0) {
            for (int q = 0; q < rawevent->length; q++) {
                if (analog) {
                    cout << rawevent->data[q] << " ";
                } else {
                    for (int r = 3; r >= 0; r--) {
                        cout << ((rawevent->data[q] & (1 << r)) ? 1 : 0);
                    }
                    cout << "|";
                }
            }
            cout << " (" << decoded_event.nHits << ")";
            cout << endl;
            decoding_errors++;
        }
    }

    return &decoded_event;
}

unsigned int RawEventDecoder::GetDecodingErrors()
{
    return decoding_errors;
}

/* Filter pipe that stores hits in a 2D histogram ----------------------------------------------------------- */

/**
    Constructor of the HitMapper data filter. It sets up all the histograms.
    \param nroc Number of ROCs to be mapped
    \param measurement_time Time in seconds that the test is running
 */
HitMapper::HitMapper(unsigned int nroc, float measurement_time)
{
    int rows, cols;
    if (nroc % 2 != 0) {
        /* odd number of ROCs, make one row */
        rows = 80;
        cols = nroc * 52;
    } else if (nroc == 2) {
        rows = 80;
        cols = 2 * 52;
    } else {
        /* even number of ROCs, make two rows */
        rows = 2 * 80;
        cols = nroc / 2 * 52;
    }
    hitmap_module = new TH2I("hitmap_module", "Pixel hit map module", cols, 0, cols, rows, 0, rows);
    double * xbins = new double [cols + 1];
    double * ybins = new double [rows + 1];
    double pos = 0;
    for (int i = 0; i <= cols; i++) {
        xbins[i] = pos;
        pos += ((i % 52) == 0 || (i % 52) == 51) ? 2 : 1;
    }
    pos = 0;
    for (int i = 0; i <= rows; i++) {
        ybins[i] = pos;
        pos += (i == 79 || i == 80) ? 2 : 1;
    }
    hitmap_module2 = new TH2F("hitmap_module2", "Pixel hit map module (double edge)", cols, xbins, rows, ybins);
    delete xbins;
    delete ybins;

    hitmap_roc = new TH2I * [nroc];
    for (int i = 0; i < nroc; i++) {
        hitmap_roc[i] = new TH2I(Form("hitmap_C%i", i), Form("Pixel hit map ROC %i", i), 52, 0, 52, 80, 0, 80);
        hitmap_roc[i]->SetTitle(Form("Pixel hit map ROC %i;Column;Row;Hits per pixel", i));
    }

    /* Make sure to not create problems with bad inputs and to allow for some space (time?) in the end */
    time = TMath::Abs(measurement_time) * 1.05;
    if (time == 0)
        time = 1.0;
    hits_vs_time_dcol = new TH2I("hitmap_hits_vs_time_dcol", "Hitmap DCol hits vs time", 100, 0, time, ROCNUMDCOLS * nroc, 0, ROCNUMDCOLS * nroc);
    hits_vs_time_dcol->SetTitle("Hitmap DCol hits vs time;Time [s];DCol Number;Hits per time bin");
    hits_vs_time_roc = new TH2I("hitmap_hits_vs_time_roc", "Hitmap ROC hits vs time", 100, 0, time, nroc, 0, nroc);
    hits_vs_time_roc->SetTitle("Hitmap ROC hits vs time;Time [s];ROC Number;Hits per time bin");

    last_timestamp = 0;
    first_timestamp = 0;

    this->nroc = nroc;
}

HitMapper::~HitMapper()
{
    delete hitmap_module;
    delete hitmap_module2;
    for (int i = 0; i < nroc; i++) {
        delete hitmap_roc[i];
    }
}

CEvent * HitMapper::Read()
{
    return static_cast<CEvent *>(source->Write());
}

CEvent * HitMapper::Write()
{
    CEvent * event = Read();
    if (event) {
        if (first_timestamp == 0) {
            /* This is the first event */
            first_timestamp = event->timestamp;
        }
        /* Test whether the timestamp on the testboard was reset */
        if (last_timestamp > 0 && event->timestamp < last_timestamp)
            first_timestamp = event->timestamp - (last_timestamp - first_timestamp);

        if (event->nHits > 0) {
            for (int r = 0; r < event->nRocs; r++) {
                if (r >= nroc) {
                    cout << "Warning: Event with more ROCs than expected from HitMapper" << endl;
                    continue;
                }
                int h = event->hits.roc[r].numPixelHits;
                for (int i = 0; i < h; i++) {
                    int col, row;
                    col = event->hits.roc[r].pixelHit[i].columnROC;
                    row = event->hits.roc[r].pixelHit[i].rowROC;
                    hitmap_roc[r]->Fill(col, row);

                    bool edge, corner;
                    edge = (row == 79 || col == 0 || col == 51);
                    corner = (row == 79 && (col == 0 || col == 51));
                    if (nroc % 2 != 0) {
                        /* odd number of ROCs */
                        col += r * 52;
                    } else if (nroc == 2) {
                        /* two ROCs (smallest plaquette) */
                        col += (2 - r - 1) * 52;
                    } else {
                        /* even number of ROCs */
                        if (r < nroc / 2) {
                            col = (nroc / 2 - r) * 52 - col - 1;
                            row = 2 * 80 - row - 1;
                        } else {
                            col += (r - nroc / 2) * 52;
                        }
                    }
                    hitmap_module->Fill(col, row);
                    float hits = hitmap_module2->GetBinContent(col + 1, row + 1);
                    if (corner)
                        hits += 0.25;
                    else if (edge)
                        hits += 0.5;
                    else
                        hits += 1.0;
                    hitmap_module2->SetBinContent(col + 1, row + 1, hits);

                    hits_vs_time_dcol->Fill((event->timestamp - first_timestamp) / 40e6, r * ROCNUMDCOLS + col / 2);
                    hits_vs_time_roc->Fill((event->timestamp - first_timestamp) / 40e6, r);
                }
            }
        }
        if (event->timestamp < (1ul << 48))
            last_timestamp = event->timestamp;
    }
    return event;
}

TH2 * HitMapper::getHitMap(int iroc)
{
    if (iroc == -1)
        return hitmap_module;
    else if (iroc == -2)
        return hitmap_module2;
    else if (iroc >= 0 && iroc < nroc)
        return hitmap_roc[iroc];
    else
        return NULL;
}

TH2I * HitMapper::getHitsVsTimeDcol()
{
    return hits_vs_time_dcol;
}

TH2I * HitMapper::getHitsVsTimeRoc()
{
    return hits_vs_time_roc;
}

/* Filter pipe that stores hits in a 2D histogram ------------------------------------------------------------ */

EfficiencyMapper::EfficiencyMapper(unsigned int nroc, unsigned int ntrig)
{
    effmap_roc = new TH2I * [nroc];
    bkgmap_roc = new TH2I * [nroc];
    effdist_roc = new TH1I * [nroc];
    for (int i = 0; i < nroc; i++) {
        effmap_roc[i] = new TH2I(Form("effmap_C%i", i), Form("Pixel efficiency map ROC %i", i), 52, 0, 52, 80, 0, 80);
        bkgmap_roc[i] = new TH2I(Form("bkgmap_C%i", i), Form("Pixel efficiency background map ROC %i", i), 52, 0, 52, 80, 0, 80);
        effdist_roc[i] = new TH1I(Form("effdist_C%i", i), Form("Pixel efficiency distribution ROC %i", i), 101, 0, 101);
    }
    int rows, cols;
    if (nroc % 2 != 0) {
        /* odd number of ROCs, make one row */
        rows = 80;
        cols = nroc * 52;
    } else if (nroc == 2) {
        rows = 80;
        cols = 2 * 52;
    } else {
        /* even number of ROCs, make two rows */
        rows = 2 * 80;
        cols = nroc / 2 * 52;
    }
    effmap_module = new TH2I("effmap_module", "Pixel efficiency map module", cols, 0, cols, rows, 0, rows);
    bkgmap_module = new TH2I("bkgmap_module", "Pixel efficiency background map module", cols, 0, cols, rows, 0, rows);
    effdist_module = new TH1I("effdist_module", "Pixel efficiency distribution module", 101, 0, 101);

    TrigPerPixel = ntrig;
    EventCounter = 0;
    NRoc = nroc;
}

EfficiencyMapper::~EfficiencyMapper()
{
    for (int i = 0; i < NRoc; i++) {
        delete effmap_roc[i];
        delete bkgmap_roc[i];
        delete effdist_roc[i];
    }
    delete effmap_roc;
    delete bkgmap_roc;
    delete effdist_roc;
    delete effmap_module;
    delete bkgmap_module;
    delete effdist_module;
}

CEvent * EfficiencyMapper::Read()
{
    return static_cast<CEvent *>(source->Write());
}

CEvent * EfficiencyMapper::Write()
{
    CEvent * event = Read();
    if (event) {
        int testcol = -1, testrow = -1;
        if (TrigPerPixel > 0 && event->isData) {
            testcol = EventCounter / (TrigPerPixel * 80);
            testrow = (EventCounter % (TrigPerPixel * 80)) / TrigPerPixel;
            EventCounter++;
        }
        if (event->nHits > 0) {
            for (int r = 0; r < event->nRocs; r++) {
                if (r > NRoc) {
                    cout << "Warning: Event with more ROCs than expected from EfficiencyMapper" << endl;
                    continue;
                }
                int h = event->hits.roc[r].numPixelHits;
                for (int i = 0; i < h; i++) {
                    /* calculate module coordinates */
                    int x, y;
                    x = event->hits.roc[r].pixelHit[i].columnROC;
                    y = event->hits.roc[r].pixelHit[i].rowROC;
                    if (NRoc % 2 != 0) {
                        /* odd number of ROCs */
                        x += r * 52;
                    } else if (NRoc == 2) {
                        /* two ROCs (smallest plaquette) */
                        x += (2 - r - 1) * 52;
                    } else {
                        /* even number of ROCs */
                        if (r < NRoc / 2) {
                            x = (NRoc / 2 - r) * 52 - x - 1;
                            y = 2 * 80 - y - 1;
                        } else {
                            x += (r - NRoc / 2) * 52;
                        }
                    }

                    /* Fill histograms */
                    if (event->hits.roc[r].pixelHit[i].columnROC == testcol && event->hits.roc[r].pixelHit[i].rowROC == testrow) {
                        effmap_roc[r]->Fill(event->hits.roc[r].pixelHit[i].columnROC, event->hits.roc[r].pixelHit[i].rowROC);
                        effmap_module->Fill(x, y);
                    } else if (event->hits.roc[r].pixelHit[i].columnROC / 2 != testcol / 2) {
                        /* Exclude double column under test because it will show a higher rate */
                        bkgmap_roc[r]->Fill(event->hits.roc[r].pixelHit[i].columnROC, event->hits.roc[r].pixelHit[i].rowROC);
                        bkgmap_module->Fill(x, y);
                    }
                }
            }
        }
    }
    return event;
}

TH2I * EfficiencyMapper::getEfficiencyMap(int iroc)
{
    if (iroc >= 0 && iroc < NRoc)
        return effmap_roc[iroc];
    else if (iroc == -1)
        return effmap_module;
    else
        return NULL;
}

TH1I * EfficiencyMapper::getEfficiencyDist(int iroc)
{
    if (iroc < -1 || iroc >= (signed int) NRoc)
        return NULL;

    if (iroc == -1) {
        /* module */
        effdist_module->Clear();
        effdist_module->SetNameTitle("effdist_module", "Pixel efficiency distribution module");

        int rows, cols;
        if (NRoc % 2 != 0) {
            /* odd number of ROCs, make one row */
            rows = 80;
            cols = NRoc * 52;
        } else if (NRoc == 2) {
            rows = 80;
            cols = 2 * 52;
        } else {
            /* even number of ROCs, make two rows */
            rows = 2 * 80;
            cols = NRoc / 2 * 52;
        }

        for (int x = 0; x < cols; x++) {
            for (int y = 0; y < rows; y++) {
                effdist_module->Fill(effmap_module->GetBinContent(x + 1, y + 1) * 100 / TrigPerPixel);
            }
        }
        return effdist_module;
    } else {
        effdist_roc[iroc]->Clear();
        effdist_roc[iroc]->SetNameTitle(Form("effdist_C%i", iroc), Form("Pixel efficiency distribution ROC %i", iroc));
        for (int col = 0; col < 52; col++) {
            for (int row = 0; row < 80; row ++) {
                effdist_roc[iroc]->Fill(effmap_roc[iroc]->GetBinContent(col + 1, row + 1) * 100.0 / TrigPerPixel);
            }
        }
        return effdist_roc[iroc];
    }
}

TH2I * EfficiencyMapper::getBackgroundMap(int iroc)
{
    if (iroc >= 0 && iroc < NRoc)
        return bkgmap_roc[iroc];
    else if (iroc == -1)
        return bkgmap_module;
    else
        return NULL;
}

/* Filter pipe which counts things related to events --------------------------------------------------------------- */

EventCounter::EventCounter()
{
    haveTrigger = false;
    TriggerCounter = 0;
    RocSequenceErrorCounter = 0;
}

CEvent * EventCounter::Read()
{
    return static_cast<CEvent *>(source->Write());
}

CEvent * EventCounter::Write()
{
    CEvent * ev = Read();
    if (!ev)
        return ev;

    if (ev->nHits == -4)
        RocSequenceErrorCounter++;
    if (ev->isTrigger)
        haveTrigger = true;
    if (ev->isData) {
        if (haveTrigger)
            TriggerCounter++;
        else
            cout << "EventCounter: Data event without preceding trigger!" << endl;
        haveTrigger = false;
    }

    return ev;
}

/* Filter pipe that histograms the number of hits per trigger ------------------------------------------------------- */

MultiplicityHistogrammer::MultiplicityHistogrammer()
{
    ModuleMultiplicity = new TH1I("module_multiplicity", "Module hit multiplicity", 500, 0, 500);
    nRocs = -1;
    for (int roc = 0; roc < 16; roc++) {
        RocMultiplicity[roc] = NULL;
        for (int dcol = 0; dcol < 26; dcol++) {
            DColMultiplicity[roc][dcol] = NULL;
        }
    }
}

MultiplicityHistogrammer::~MultiplicityHistogrammer()
{
    delete ModuleMultiplicity;
    for (int roc = 0; roc < nRocs; roc++) {
        delete RocMultiplicity[roc];
        for (int dcol = 0; dcol < 26; dcol++)
            delete DColMultiplicity[roc][dcol];
    }
}

CEvent * MultiplicityHistogrammer::Read()
{
    return static_cast<CEvent *>(source->Write());
}

CEvent * MultiplicityHistogrammer::Write()
{
    CEvent * ev = Read();
    if (!ev)
        return NULL;

    /* initialise histograms if not already done (depends on number of ROCs) */
    if (nRocs == -1) {
        nRocs = ev->nRocs;
        for (int roc = 0; roc < ev->nRocs; roc++) {
            RocMultiplicity[roc] = new TH1I(Form("roc_multiplicity_%i", roc), Form("ROC %i hit multiplicity", roc), 40, 0, 40);
            for (int dcol = 0; dcol < 26; dcol++)
                DColMultiplicity[roc][dcol] = new TH1I(Form("dcol_multiplicity_%i_%i", roc, dcol), Form("DCol %i hit multiplicity (ROC %i)", dcol, roc), 15, 0, 15);
        }
    }

    if (!(ev->isData && ev->nHits >= 0))
        return ev;

    ModuleMultiplicity->Fill(ev->nHits);
    for (int roc = 0; roc < ev->nRocs && roc < nRocs; roc++) {
        int h = ev->hits.roc[roc].numPixelHits;
        RocMultiplicity[roc]->Fill(h);
        int dcolhits [26] = {0};
        for (int i = 0; i < h; i++) {
            dcolhits[ev->hits.roc[roc].pixelHit[i].columnROC / 2]++;
        }
        for (int dcol = 0; dcol < 26; dcol++) {
            DColMultiplicity[roc][dcol]->Fill(dcolhits[dcol]);
        }
    }

    return ev;
}

TH1I * MultiplicityHistogrammer::getModuleMultiplicity()
{
    return ModuleMultiplicity;
}

TH1I * MultiplicityHistogrammer::getRocMultiplicity(int roc)
{
    if (roc < nRocs)
        return RocMultiplicity[roc];

    return NULL;
}

TH1I * MultiplicityHistogrammer::getDColMultiplicity(int roc, int dcol)
{
    if (roc < nRocs)
        if (dcol >= 0 && dcol < 26)
            return DColMultiplicity[roc][dcol];

    return NULL;
}

/* Pipe that histograms the pulse height ------------------------------------------------------------------- */

PulseHeightHistogrammer::PulseHeightHistogrammer()
{
    pulse_height_dist = new TH1F("pulse_height_dist", "Pulse height", 2048, -1024, 1024);
    pulse_height_dist_cal = new TH1F("pulseheight_cal", "Pulse height (VCal units)", 255, 0, 255);
    pulse_height_map = NULL;
    pulse_height_map_cal = NULL;
    pulse_height_width_map = NULL;
    pulse_height_width_map_cal = NULL;
    ph_map_w = new TH2F("ph_map_w", "ph_map_w", 52, 0, 52, 80, 0, 80);
    ph_map_w_cal = new TH2F("ph_map_w_cal", "ph_map_w_cal", 52, 0, 52, 80, 0, 80);
    ph_map_w2 = new TH2F("ph_map_w2", "ph_map_w2", 52, 0, 52, 80, 0, 80);
    ph_map_w2_cal = new TH2F("ph_map_w2_cal", "ph_map_w2_cal", 52, 0, 52, 80, 0, 80);
    ph_map_n = new TH2F("ph_map_n", "ph_map_n", 52, 0, 52, 80, 0, 80);
    calibration = NULL;
}

PulseHeightHistogrammer::~PulseHeightHistogrammer()
{
    delete pulse_height_dist;
    delete pulse_height_dist_cal;
    delete ph_map_w;
    delete ph_map_w_cal;
    delete ph_map_w2;
    delete ph_map_w2_cal;
    delete ph_map_n;
    if (pulse_height_map)
        delete pulse_height_map;
    if (pulse_height_map_cal)
        delete pulse_height_map_cal;
    if (pulse_height_width_map)
        delete pulse_height_width_map;
    if (pulse_height_width_map_cal)
        delete pulse_height_width_map_cal;
}

CEvent * PulseHeightHistogrammer::Read()
{
    return static_cast<CEvent *>(source->Write());
}

CEvent * PulseHeightHistogrammer::Write()
{
    CEvent * ev = Read();
    if (!ev)
        return NULL;

    if (!(ev->isData && ev->nHits >= 0))
        return ev;

    for (int roc = 0; roc < ev->nRocs; roc++) {
        int h = ev->hits.roc[roc].numPixelHits;
        for (int i = 0; i < h; i++) {
            int ph = ev->hits.roc[roc].pixelHit[i].analogPulseHeight;
            int col = ev->hits.roc[roc].pixelHit[i].columnROC;
            int row = ev->hits.roc[roc].pixelHit[i].rowROC;

            float w, w2;
            w = ph_map_w->GetBinContent(col + 1, row + 1) + ph;
            w2 = ph_map_w2->GetBinContent(col + 1, row + 1) + ph * ph;
            ph_map_w->SetBinContent(col + 1, row + 1, w);
            ph_map_w2->SetBinContent(col + 1, row + 1, w2);
            ph_map_n->Fill(col, row);
            pulse_height_dist->Fill(ph);

            if (calibration && calibration[roc] && calibration[roc][col] && calibration[roc][col][row]) {
                float ph_cal = (ph - calibration[roc][col][row][0]) / calibration[roc][col][row][1];
                w = ph_map_w_cal->GetBinContent(col + 1, row + 1) + ph_cal;
                w2 = ph_map_w2_cal->GetBinContent(col + 1, row + 1) + ph_cal * ph_cal;
                ph_map_w_cal->SetBinContent(col + 1, row + 1, w);
                ph_map_w2_cal->SetBinContent(col + 1, row + 1, w2);
                pulse_height_dist_cal->Fill(ph_cal);
            }
        }
    }

    return ev;
}

TH1F * PulseHeightHistogrammer::getPulseHeightDistribution()
{
    return pulse_height_dist;
}

TH1F * PulseHeightHistogrammer::getCalPulseHeightDistribution()
{
    return pulse_height_dist_cal;
}

TH2F * PulseHeightHistogrammer::getPulseHeightMap()
{
    if (pulse_height_map)
        return pulse_height_map;

    pulse_height_map = new TH2F("pulse_height_map", "Pulse height map", 52, 0, 52, 80, 0, 80);

    float w, n;
    for (int col = 0; col < 52; col++) {
        for (int row = 0; row < 80; row++) {
            n = ph_map_n->GetBinContent(col + 1, row + 1);
            if (n < 1)
                continue;
            w = ph_map_w->GetBinContent(col + 1, row + 1);
            pulse_height_map->SetBinContent(col + 1, row + 1, w / (float) n);
        }
    }

    return pulse_height_map;
}

TH2F * PulseHeightHistogrammer::getCalPulseHeightMap()
{
    if (pulse_height_map_cal)
        return pulse_height_map_cal;

    pulse_height_map_cal = new TH2F("pulse_height_map_cal", "Pulse height map (Vcal units)", 52, 0, 52, 80, 0, 80);

    float w, n;
    for (int col = 0; col < 52; col++) {
        for (int row = 0; row < 80; row++) {
            n = ph_map_n->GetBinContent(col + 1, row + 1);
            if (n < 1)
                continue;
            w = ph_map_w_cal->GetBinContent(col + 1, row + 1);
            pulse_height_map_cal->SetBinContent(col + 1, row + 1, w / (float) n);
        }
    }

    return pulse_height_map_cal;
}

TH2F * PulseHeightHistogrammer::getPulseHeightWidthMap()
{
    if (pulse_height_width_map)
        return pulse_height_width_map;

    pulse_height_width_map = new TH2F("pulse_height_width_map", "Pulse height width map", 52, 0, 52, 80, 0, 80);

    float s0, s1, s2, err;
    for (int col = 0; col < 52; col++) {
        for (int row = 0; row < 80; row++) {
            s0 = ph_map_n->GetBinContent(col + 1, row + 1);
            if (s0 < 2)
                continue;
            s1 = ph_map_w->GetBinContent(col + 1, row + 1);
            s2 = ph_map_w2->GetBinContent(col + 1, row + 1);
            float err = TMath::Sqrt((s0 * s2 - s1 * s1) / (s0 * (s0 - 1)));
            pulse_height_width_map->SetBinContent(col + 1, row + 1, err);
        }
    }

    return pulse_height_width_map;
}

TH2F * PulseHeightHistogrammer::getCalPulseHeightWidthMap()
{
    if (pulse_height_width_map_cal)
        return pulse_height_width_map_cal;

    pulse_height_width_map_cal = new TH2F("pulse_height_width_map_cal", "Pulse height width map (Vcal units)", 52, 0, 52, 80, 0, 80);

    float s0, s1, s2, err;
    for (int col = 0; col < 52; col++) {
        for (int row = 0; row < 80; row++) {
            s0 = ph_map_n->GetBinContent(col + 1, row + 1);
            if (s0 < 2)
                continue;
            s1 = ph_map_w_cal->GetBinContent(col + 1, row + 1);
            s2 = ph_map_w2_cal->GetBinContent(col + 1, row + 1);
            float err = TMath::Sqrt((s0 * s2 - s1 * s1) / (s0 * (s0 - 1)));
            pulse_height_width_map_cal->SetBinContent(col + 1, row + 1, err);
        }
    }

    return pulse_height_width_map_cal;
}

void PulseHeightHistogrammer::LoadCalibration(int nRoc, const char * dirfilebase)
{
    char tmp [100];
    double y [5];
    double x [5] = {50, 100, 150, 200, 250};
    calibration = new float ** * [nRoc];
    for (int iroc = 0; iroc < nRoc; iroc++) {
        /* open calibration file */
        FILE * f = fopen(Form("%s/phCalibration_C0.dat", dirfilebase), "r");
        if (!f) {
            calibration[iroc] = NULL;
            continue;
        }

        /* allocate space for calibration data */
        calibration[iroc] = new float ** [52];
        for (int col = 0; col < 52; col++) {
            for (int row = 0; row < 80; row++) {
                /* store two parameters per calibration */
            }
        }

        /* skip 4 lines */
        fgets(tmp, 100, f);
        fgets(tmp, 100, f);
        fgets(tmp, 100, f);
        fgets(tmp, 100, f);

        /* read calibration for each pixel */
        for (int col = 0; col < 52; col++) {
            calibration[iroc][col] = new float * [80];
            for (int row = 0; row < 80; row++) {
                calibration[iroc][col][row] = NULL;

                fgets(tmp, 100, f);
                int n = sscanf(tmp, "%lf %lf %lf %lf %lf", &(y[0]), &(y[1]), &(y[2]), &(y[3]), &(y[4]));
                if (n != 5)
                    continue;

                /* fit the data with a line */
                TLinearFitter lf;
                lf.SetFormula("pol1");
                lf.AssignData(5, 1, x, y);
                int ret = lf.Eval();

                if (ret != 0)
                    continue;

                calibration[iroc][col][row] = new float [2];
                calibration[iroc][col][row][0] = lf.GetParameter(0);
                calibration[iroc][col][row][1] = lf.GetParameter(1);
            }
        }
        fclose(f);
    }
}
