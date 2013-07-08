#include <stdio.h>
#include <iostream>
#include <fstream>
#include <math.h>
#include <cstdlib>
#include <cstring>
using namespace std;



int main(int argc, char ** argv)
{
    char buf[100];
    char fileName[200];
    char path[200];
    int run = 0;
    int roc = 0;
    int decode = 0;

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-r")) {
            run = atoi(argv[++i]);
            sprintf(path, "/home/l_tester/log/bt05r%06d", run);
        } else if (!strcmp(argv[i], "-roc")) {
            roc = 1;
        } else if (!strcmp(argv[i], "-d")) {
            decode = 1;
        }
    }


    if (roc) {
        sprintf(fileName, "%s/rtb.bin", path);
    } else {
        sprintf(fileName, "%s/mtb.bin", path);
    }

    ifstream * in = new ifstream(fileName);
    int k = 0;

    if (decode == 0) {
        while (in->is_open() && !in->eof()) {

            unsigned char a = in->get();
            if (in->eof()) break;
            unsigned char b = in->get();
            if (in->eof()) break;
            unsigned short word  = (b << 8) | a;

            if (((word & 0x8000) > 0) && (k > 3)) {
                cout << endl;
                k = 0;
            }

            if (k == 4) cout << ": ";

            if (k > 3) {
                int value = word & 0x0fff;
                if (value & 0x0800) value -= 4096;
                sprintf(buf, "%4d ", value);
            } else {
                sprintf(buf, "%4x ", word);
            }
            cout << buf;
            k++;
        }
        cout << endl;

    } else {

        int data[1001];
        int cor[1000][4];
        int over[1000][4];
        int infro[1000][4];
        unsigned char a = in->get();
        unsigned char b = in->get();
        unsigned short word  = (b << 8) | a;
        unsigned int ndata(0);
        unsigned int ndataetrig(0);
        unsigned int netrig(0);
        unsigned int nitrig(0);
        unsigned int nres(0);
        unsigned int ncal(0);
        unsigned int ntbmres(0);
        unsigned int ninfro(0);
        unsigned int nover(0);
        unsigned int ncor(0);
        int oldtime(0);
        int invtime(0);

        for (int k = 0; k < 1000; k++) {
            for (int l = 0; l < 4; l++) {
                over[k][l] = 0;
                cor[k][l] = 0;
            }
        }

        while (in->is_open() && !in->eof()) {

            int k = 0;
            // header
            for (k = 0; (k < 4) && (!in->eof()) ; k++) {
                data[k] = word;
                //cout << " data[" << k << "]: " << data[k];
                a = in->get();  b =  in->get();
                word  = (b << 8) | a;
            }
            if (oldtime > data[3]) {
                invtime++;
            }

            oldtime = data[3];
            //cout << endl;
            switch (data[0]) {
            case 32769://data 8001
                ndata++;
                break;
            case 32770://ext. trig 8002
                netrig++;
                break;
            case 32772://int. trig 8004
                nitrig++;
                break;
            case 32776://reset 8008
                nres++;
                break;
            case 32784://cal 8010
                ncal++;
                break;
            case 32800://TBMreset 8020
                ntbmres++;
                break;
            case 32832://infreadout 8040
                for (int j = 0; j < 4; j++) {
                    infro[ninfro][j] = data[j];
                }
                ninfro++;
                break;
            case 32896://data overflow 8080
                for (int j = 0; j < 4; j++) {
                    over[nover][j] = data[j];
                }
                nover++;
                break;
            case 32771://data and ext trig 8003
                ndataetrig++;
                break;
            default:
                for (int j = 0; j < 4; j++) {
                    cor[ncor][j] = data[j];
                }
                ncor++;
            }
            sprintf(buf, "%4x %4x %4x %4x :", data[0], data[1], data[2], data[3]);
            unsigned short t0 = data[1];
            unsigned short t1 = data[2];
            unsigned short t2 = data[3];
            unsigned long fUpperTime = t0;
            unsigned long fLowerTime = (t1 << 16) | t2;
            long long int time
                = (((long long int) fUpperTime) << 32) + ((long long int)fLowerTime);
            /*
            long long int time=
                ((long long int)data[1])<<32
              + ((long long int)data[2])<<16
            + ((long long int)data[3]);
            */
            cout << buf << time << endl;

            // event data, if any
            while (((word & 0x8000) == 0) && (!in->eof()) && (k < 1000)) {
                int value = word & 0x0fff;
                if (value & 0x0800) value -= 4096;
                data[k++] = value;
                a = in->get(); b =  in->get();
                word  = (b << 8) | a;
            }

            // pretty print
            if ((data[0] & 0x0001) == 0) {
                // not a data header
                // there should be nothing else, but if there is, just dump it
                for (int i = 4; i < k; i++) {
                    printf(" %4d", data[i]);
                }
                if (k > 4) {cout << endl;}
            } else {
                // data
                sprintf(buf, "                     TBM    %4d %4d %4d %4d %4d %4d %4d %4d",
                        data[4], data[5], data[6], data[7], data[8], data[9], data[10], data[11]);
                cout << buf << endl;

                int roc = 0;
                int ubtbm = (int) round((data[4] + data[5] + data[6]) / 3.*0.8 + 0.2 * data[7]);
                int btbm = (int)  round((data[4] + data[5] + data[6]) / 3.*0.2 + 0.8 * data[7]);
                int ubroc = (int) round(0.8 * data[12] + 0.2 * data[13]);
                int broc = (int)  round(0.2 * data[12] + 0.8 * data[13]);


                int i = 12;
                while (i < k) {
                    if ((data[i] < ubtbm) && (data[i + 1] < ubtbm) && (data[i + 2] > btbm)) {
                        // tbm trailer
                        sprintf(buf, "                     TBM    %4d %4d %4d %4d %4d %4d %4d %4d",
                                data[i], data[i + 1], data[i + 2], data[i + 3], data[i + 4], data[i + 5], data[i + 6], data[i + 7]);
                        i += 8;
                    } else if ((data[i] < ubroc) && (data[i + 1] > broc)) {
                        // roc header
                        sprintf(buf, "                     ROC%2d  %4d %4d %4d",
                                roc, data[i], data[i + 1], data[i + 2]);
                        roc++;
                        i += 3;
                    } else {
                        // hit (I guess)
                        sprintf(buf, "                            %4d %4d %4d %4d %4d %4d",
                                data[i], data[i + 1], data[i + 2], data[i + 3], data[i + 4], data[i + 5]);
                        i += 6;
                    }
                    cout << buf << endl;
                }
            }
        }
        cout << "netrig:" << netrig << " ndata:" << ndata << " nitrig:" << nitrig << " nres:" << nres
             << " ncal:" << ncal << " ntbmres:" << ntbmres << " ninfro:" << ninfro << " nover:" << nover << " ndataetrig:" << ndataetrig << " ncor:" << ncor << " invtime:" << invtime << endl;
        cout << "corrupt headers:" << endl;
        for (int i = 0; i < ncor; i++) {
            cout << i << ": " << cor[i][0] << " " << cor[i][1] << " " << cor[i][2] << " " << cor[i][3];
            unsigned short t0 = cor[i][1];
            unsigned short t1 = cor[i][2];
            unsigned short t2 = cor[i][3];
            unsigned long fUpperTime = t0;
            unsigned long fLowerTime = (t1 << 16) | t2;
            long long int time = (((long long int)fUpperTime) << 32) + ((long long int)fLowerTime);
            cout << ": " << time << endl;
        }
        cout << "overflows:" << endl;
        for (int i = 0; i < nover; i++) {
            cout << i << ": " << over[i][0] << " " << over[i][1] << " " << over[i][2] << " " << over[i][3];
            unsigned short t0 = over[i][1];
            unsigned short t1 = over[i][2];
            unsigned short t2 = over[i][3];
            unsigned long fUpperTime = t0;
            unsigned long fLowerTime = (t1 << 16) | t2;
            long long int time = (((long long int)fUpperTime) << 32) + ((long long int) fLowerTime);
            cout << ": " << time << endl;
        }
        cout << "infinite readouts:" << endl;
        for (int i = 0; i < ninfro; i++) {
            cout << i << ": " << infro[i][0] << " " << infro[i][1] << " " << infro[i][2] << " " << infro[i][3];
            unsigned short t0 = infro[i][1];
            unsigned short t1 = infro[i][2];
            unsigned short t2 = infro[i][3];
            unsigned long fUpperTime = t0;
            unsigned long fLowerTime = (t1 << 16) | t2;
            long long int time = (((long long int)fUpperTime) << 32) + ((long long int) fLowerTime);
            cout << ": " << time << endl;
        }
    }
}
