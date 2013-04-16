#ifndef USBDAQ_H
#define USBDAQ_H

#include <fstream>
#include <iostream>
#include <stdio.h>

#include "BasePixel/DecodedReadout.h"
#include "BasePixel/RawPacketDecoder.h"
#include "psi46expert/histogrammer.h"

class UsbDaq {
 public:
  UsbDaq(int mode = 1);      // mode = 1: File input
  ~UsbDaq();     // Destructor, called at program exit
  void start();  // do for each run start
  void stop();   // do for each run stop
  void pause();  // do for each run pause
  void resume(); // do for each run resume
  int clear();
  int  read(unsigned int data[]); // read data 
  
  void reset();         // send a reset 
  void print(const int size, const unsigned int data[]);//print

  void print();
  void setInputFileName(const char *f) { sprintf(fInputFileName,"%s", f); }
  const char* getInputFileName() {return fInputFileName; }

  int  openTxtFile();
  int  readLine();                            // read ASCII data from file

  int  openBinaryFile();
  int  readBinaryEvent();                     // read one event (=header+data) from file (Beat's format with TB header)
  int  nextBinaryHeader();                    // starting from present position, find next header
  int  decodeBinaryData();                    // called after header has been found => time and fData

  //  unsigned short readBinaryWord();            // read two chars and swap order
  unsigned short readBinaryWordFromFile();    // read two chars and swap order

  void setBinaryBuffer(int size, unsigned char *p);     // provide pointer to an external buffer 
  unsigned short readBinaryWordFromBuffer();             // read two chars and swap order

  void loop();
  int  singleStep();

  void setOutputFileName(const char *f) { sprintf(fOutputFileName,"%s", f); }
  const char* getOutputFileName() {return fOutputFileName; }
  void openOutputFile();
  int  writeAscii();

  void setHistogrammer(histogrammer *p);
  histogrammer * getHistogrammer() {return fpHistogrammer; }

  void setDecoder(RawPacketDecoder *p) {fpDecoder = p; }

  int  getEOF() {return fEOF;}
  int  getRunMode() {return fRunMode;}
  void setRunState(int i) {fRunState = i;}

  //   struct pixel*                  getPixels() {return fPixels;}
  int* getRawData() {return fData;}
  int  getRawDataSize() {return fBufferSize;}
  int  getHeader() {return fHeader;}
  unsigned int getLower() {return fLowerTime;}
  unsigned int getUpper() {return fUpperTime;}
  long long getTime()  {return fTime;}

  void doNotWrite() {fDoWrite = 0;}

 private:
  int        setIOBits; // IO register status
  int        fDoWrite;

  int        fRunState;  // 1 run, 2 pause, 3 stop
  int        fRunMode;   // -1 ASCII, 0 Binary, >0 TB memory
  int        fEvent;     // Event counter
  int        fMaxEvent;  // Event number limitation
  unsigned int fUpperTime, fLowerTime;
  long long fTime;

  int        fHeader, fNextHeader;

  int        fEOF;
  int        fBufferSize;
  static const int NUM_DATA = 1; // Number of ADC channels to read 
  int        fBuffer[NUM_DATA];
  int        fData[DecodedReadoutConstants::MAX_PIXELSROC];
  int short  sData[DecodedReadoutConstants::MAX_PIXELSROC];
  char       fInputFileName[1000];
  FILE       *fInputFile;
  std::ifstream   *fInputBinaryFile;

  // -- Binary buffer input (when spying on TB memory)
  int            fBinaryBufferSize, fBinaryBufferCnt;
  unsigned char* fpBinaryBuffer;


  char            fOutputFileName[1000];
  std::ofstream        *fOutputFile;

  histogrammer    *fpHistogrammer;

  //struct DecodedReadoutPixel fPixels[DecodedReadoutConstants::MAX_PIXELSROC];
  DecodedReadoutModule fPixels;
  RawPacketDecoder* fpDecoder; 

};
#endif
