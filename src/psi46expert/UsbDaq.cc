#define DUMMY_READOUT // Only for testing
#define SLOW_READOUT  // read data at 20MHz
#define PRINT 1

#include <cstdlib>
#include <iostream>
#include <TSystem.h>

using namespace std;

#include "psi46expert/UsbDaq.h"

const int LENGTH = 1048576;

UsbDaq::UsbDaq(int mode)
{
  fDoWrite = 1;
  fRunMode = mode;
  fMaxEvent = 9999999;
  fHeader = fNextHeader = -1;
  fEOF = 0;

  cout << " constructed USB DAQ module " << fRunMode << endl;
}

UsbDaq::~UsbDaq()
{
  cout << " delete USB DAQ module "<<endl;
}


//************************************************************
// Module clear after each event. Called after event readout.
int UsbDaq::clear()
{
  int status = 0;
  if(PRINT) cout << " Clear event" << endl;
  return status;
}


// ----------------------------------------------------------------------
int UsbDaq::openBinaryFile()
{
  fInputBinaryFile = new ifstream(fInputFileName);
  if (fInputBinaryFile)
  {
    cout << "--> DAQ will be reading from file " << getInputFileName() << " at " << fInputBinaryFile << endl;
    return 0;
  }
  else
  {
    cout << "--> ERROR: DAQ could NOT open file " << getInputFileName() << endl;
    return 1;
  }
};


// ----------------------------------------------------------------------
void UsbDaq::openOutputFile()
{

  fOutputFile = new ofstream(fOutputFileName);
  if (fOutputFile)
  {
    cout << "--> DAQ will be writing to file " << getOutputFileName() << endl;
  }
  else
  {
    cout << "--> ERROR: DAQ could NOT open output file " << getOutputFileName() << endl;
  }
}

// ----------------------------------------------------------------------
unsigned short UsbDaq::readBinaryWordFromFile()
{

  if (fInputBinaryFile->eof()) fEOF = 1;

  unsigned char a = fInputBinaryFile->get();
  unsigned char b = fInputBinaryFile->get();
  unsigned short word  =  (b << 8) | a;

//    cout << Form("readBinaryWord: %02x %02x word: %04x ", a, b, word) << endl;

  return word;
}


// ----------------------------------------------------------------------
void UsbDaq::setBinaryBuffer(int size, unsigned char *p)
{
  fBinaryBufferCnt = 0;
  fEOF = 0;
  fBinaryBufferSize = size;
  fpBinaryBuffer = p;
}


// ----------------------------------------------------------------------
unsigned short UsbDaq::readBinaryWordFromBuffer()
{

  if (fBinaryBufferCnt >= fBinaryBufferSize)
  {
    fEOF = 1;
    return 0;
  }

/*  unsigned char a = 0x00ff & fpBinaryBuffer[fBinaryBufferCnt];
  unsigned char b = (fpBinaryBuffer[fBinaryBufferCnt] >> 8);*/
  unsigned char a = fpBinaryBuffer[fBinaryBufferCnt];
  unsigned char b = fpBinaryBuffer[fBinaryBufferCnt+1];

  ++fBinaryBufferCnt;
  ++fBinaryBufferCnt;

  unsigned short word  =  (b << 8) | a;

  //  cout << Form("readBinaryWordFromBuffer: %02x %02x word: %04x ", a, b, word) << endl;

  return word;
}

// ----------------------------------------------------------------------
int UsbDaq::nextBinaryHeader()
{

  int header(-1);

  for (int i = 0; i < NUM_DATA; ++i)
  {
    fBuffer[i] = 0;
  }

  fBufferSize = 0;
  unsigned short word(0);

  while (1)
  {
    if (fRunMode == 0)
    {
      word = readBinaryWordFromFile();
    }
    else if (fRunMode > 0)
    {
      word = readBinaryWordFromBuffer();
    }
    else
    {
      cout << "USBDAQ: UNKNOWN RUN MODE!!!!!!!!" << endl;
    }
    
    if (fEOF) break;

    if (word == 0x8000)
    {
      //      cout << "==> end of file " << endl;
      header = 0;
      break;
    }

    if (word == 0x8001)
    {
      header = 1;
      //      cout << " --> data " <<endl;
    }

    if (word == 0x8004)
    {
      header = 4;
      //      cout << " --> trig " << endl;
    }

    if (word == 0x8008)
    {
      header = 8;
      //      cout << " --> reset " << endl;
    }

    if (word == 0x8080)
    {
      header = 80;
      //      cout << " --> overflow " << endl;
    }

    if (header > -1)
    {
//       cout << " --> Found next header " << header << endl;
      break;
    }

    if (0) cout << Form("Adding %04x", word) << " at " << fBufferSize << endl;
    fBuffer[fBufferSize] = word;
    ++fBufferSize;
  }

  fHeader     = fNextHeader;
  fNextHeader = header;

  return header;
}


// ----------------------------------------------------------------------
int UsbDaq::decodeBinaryData() {
  int j(0);

  if (fHeader > 0)  {
    unsigned short t0 = fBuffer[0];
    unsigned short t1 = fBuffer[1];
    unsigned short t2 = fBuffer[2];
    fUpperTime = t0;
    fLowerTime = (t1 << 16) | t2;
    fTime = (((long long int) fUpperTime)<<32) + ((long long int)fLowerTime);
    //    cout << Form(" Event at time  %04x/%08x with Header %d", fUpperTime, fLowerTime, fHeader) << endl;
    //    cout << Form(" Event at time  %04d/%08d with Header %d", fUpperTime, fLowerTime, fHeader) << endl;
  } else  {
    //    cout << "No valid header, skipping this event" << endl;
    return -1;
  }

  int value(0);
  for (int i = 3; i < fBufferSize; ++i)
  {
    value = fBuffer[i] & 0x0fff;
    if (value & 0x0800) value -= 4096;
    fData[i-3] = value;
    ++j;
  }

  fBufferSize -=3;

  if (0)
  {
    for (int i = 0; i < fBufferSize; ++i)
    {
      cout << Form(" %04x ", fData[i]);
    }
    cout << endl;

    for (int i = 0; i < fBufferSize; ++i)
    {
      cout << Form(" %6i ", fData[i]);
    }
    cout << endl;
  }

  return j;

}


// ----------------------------------------------------------------------
int UsbDaq::readBinaryEvent()
{

  cout << "UsbDaq::readBinaryEvent() " << endl;

  int header = nextBinaryHeader(); // search next header
  int words  = decodeBinaryData(); // decode what has been read into buffer so far

  return words;
}

// ----------------------------------------------------------------------
int UsbDaq::writeAscii()
{
  //  cout << "UsbDaq::writeAscii() " << endl;
  char line[200];
  //  sprintf(line, "#time: %04x/%08x, type: %d", fUpperTime, fLowerTime, fHeader);
  sprintf(line, "#time: %04d/%08d, type: %d", fUpperTime, fLowerTime, fHeader);
  (*fOutputFile) << line << endl;
  cout << line << endl;

  int oneLinePerEvent(1), wroteSome(0);
  for (int i = 0; i < fBufferSize; ++i)
  {
    wroteSome = 1;
    sprintf(line, " %6i ", fData[i]);
    if (oneLinePerEvent)
    {
      (*fOutputFile) << line;
      cout << line;
    }
    else
    {
      (*fOutputFile) << line << endl;
    }
  }
  if (oneLinePerEvent && wroteSome)
  {
    (*fOutputFile) << endl;
    cout << endl;
  }
}


// ----------------------------------------------------------------------
int UsbDaq::openTxtFile()
{
  fInputFile = fopen(fInputFileName, "r");
  if (fInputFile)
  {
    cout << "--> DAQ will be reading from file " << getInputFileName() << endl;
    return 0;
  }
  else
  {
    cout << "--> DAQ could NOT open file " << getInputFileName() << endl;
    return 1;
  }
};


// ----------------------------------------------------------------------
int UsbDaq::readLine()
{
  char buffer[2000];
  char number[10];

  cout << "UsbDaq::readLine() " << endl;

  //  fInputFile->getline(buffer, 2000, '\n');
  do
  {
    fgets(buffer, 2000, fInputFile);
  }
  while (buffer[0] == '#');

  int i(0);
  int prev(0);

  int j(0);
  while (buffer[i] != '\0')
  {
    if (buffer[i] == ' ')
    {
      strncpy(number, &buffer[prev], i-prev+1);
      if (i - prev > 1)
      {
        fData[j] = atoi(number);
        //	cout << "read: " << fData[j] << endl;
        ++j;
      }
      prev = i;
    }

    ++i;

  }
  fBufferSize = j;
  //  cout << "Read " << j << " words" << endl;
  return j;
}


// ----------------------------------------------------------------------
void UsbDaq::setHistogrammer(histogrammer *p)
{
  fpHistogrammer = p;
  fpHistogrammer->setRawData(&fData);
}



//***********************************************************
// All modules with data are read here.
// Data is stored in a global array data[].
int UsbDaq::read(unsigned long data[])
{
  int count = 0;  // return  data size
  int halfSpeed=0; // readout at full speed, 0-40MHz, 1-20MHz

#ifdef SLOW_READOUT
  halfSpeed=1;
#endif

#ifdef DUMMY_READOUT
  // Simulated readout
  count = 6;
  for(int i=0;i<count;i++) data[i]=i;

#elif defined(BLOCK_READOUT)
  // Block readout, faster but we need to know the length
  // make it static so we do not use time to create it each time.
  static unsigned int long array[LENGTH];
  //static unsigned int long array24[LENGTH];
  int status = readBlockADC(BlockLength,array);  // Read adc
  //status = readBlockADC24(Blocklength,array24); // Read adc chanel 2&4

  for (int i=0;i<BlockLength;i++)
  {
    if(((i%2)==1)||(halfSpeed==0))
    {
      if ( (array[i]&0x1000) != 0 )
      {  // do until the end marker
        data[count] = (array[i] & 0x00000FFF);  // get channel 1
        //data[count] = (array[i]>>16 & 0x00000FFF);  // get channel 3
        //data[count] = (array24[i] & 0x00000FFF);  // get channel 2
        //data[count] = (array24[i]>>16 & 0x00000FFF); //get chann 4
        count++;
      }
      else
      {
        if(i>3)
          break;
      }
    }//half-speed
  }//loop
#else // Programmed IO (slower!)
  unsigned int long dataWord;
  //unsigned int long dataWord24;
  //int j=0;
  for (int i=0;i<LENGTH;i++)
  {
    dataWord = readADC();  // read channels 1&3
    //dataWord24 = readADC24();	// read channels 2&4
    // skip every other data word in 20MHz readout
    if(((i%2)==1)||(halfSpeed==0))
    {

      // Is the data valid?
      //if (((dataWord&0x1000)!=0)&&((dataWord24&0x1000)!=0)) {//channels 1-4
      if ( (dataWord&0x1000) != 0 )
      {  // channels 1&3
        data[count] = (dataWord & 0x00000FFF);  // Get adc channel 1
        //data[count] = (dataWord>>16 & 0x00000FFF);  // get channel 3
        //data[count] = (dataWord24 & 0x00000FFF);  // get channel 2
        //data[count] = (dataWord24>>16 & 0x00000FFF); //get channel 4
        count++;
      }
      else
      {    // End of data marker found
        if(i>3)  // has to be at least after 3
          break;
      } // end if data valid?
    } // end if halfSpeed
  } // end length loop
#endif

  // The end marker did not arrive. Either make the data buffer larger or
  // reduce the number of ADC clocks/
  if(count >= (LENGTH-1) )
    cout<<" Buffer size too small to read the ADC "
    <<count<<" "<<LENGTH<<endl;
  if(PRINT) cout << "read vme adc, size =  " << count << endl;
  //writeIO(8);  // signal en of readout
  return count;
}


//********** DAQ PRINT ******************************************
void UsbDaq::print(const int size, const unsigned long data[])
{
  cout<<" size = "<<size<<endl;
  for(int i=0;i<size;i++)
  {
    cout<<i<<" "<<hex<<data[i]<<dec<<endl;;
  }
}


// ----------------------------------------------------------------------
void UsbDaq::print()
{
  cout << Form(" UsbDaq::Print/%d:", fBufferSize);
  for (int i = 0; i < fBufferSize; ++i)
  {
    cout << " " << fData[i];
  }
  cout << endl;
}


//*********** DAQ STOP ***********************************************
void UsbDaq::stop()
{
  fRunState = 3;

  if (fInputBinaryFile) fInputBinaryFile->close();
  if (fpHistogrammer) fpHistogrammer->close();
}


//*********** DAQ START ***********************************************
void UsbDaq::start()
{

  cout << "UsbDaq::start() in mode: " << fRunMode << endl;

  fRunState = 1;

  if (fRunMode == -1)
  {
    // -- ASCII text file input
    openTxtFile();
  }
  else if (fRunMode == 0)
  {
    // -- Binary text file input
    openBinaryFile();
    openOutputFile();
    fpHistogrammer->setRawData(&fData);
  }
  else if (fRunMode > 0)
  {
    cout << "would be reading from TB" << endl;
  }
}


// ----------------------------------------------------------------------
void UsbDaq::loop()
{

  int header(-1), readWords(-1);

  if (fRunMode == -1)
  {
    while ((fRunState == 1) && !feof(fInputFile))
    {
      readWords = readLine();
      print();
      fpHistogrammer->printRawData(fBufferSize);
      fpHistogrammer->fillRawDataHistograms(fBufferSize);
      gSystem->ProcessEvents();  // handle GUI events
    }
    fclose(fInputFile);
  }

  if (fRunMode == 0)
  {
    while (fRunState == 1)
    {
      header = nextBinaryHeader();
      readWords  = decodeBinaryData();

      fpHistogrammer->printRawData(fBufferSize);
      fpHistogrammer->fillRawDataHistograms(fBufferSize);
      if (fInputBinaryFile->eof()) break;
    };
  }

}

// ----------------------------------------------------------------------
int UsbDaq::singleStep()
{

  int header(-1), readWords(-1);

  // -- ASCII file input
  if (fRunMode == -1)
  {
    if (feof(fInputFile))
    {
      return 0;
    }
    readWords = readLine();
    if (readWords) ++fEvent;
    print();
    if (fpHistogrammer)
    {
      fpHistogrammer->fillRawDataHistograms(fBufferSize);
    }
  }

  // -- Binary file input
  if (fRunMode == 0)
  {
    if (fInputBinaryFile->eof()) return 0;
    header = nextBinaryHeader();
    readWords  = decodeBinaryData();
    ++fEvent;
    for (int i = 0; i < DecodedReadoutConstants::MAX_PIXELSROC; i++) sData[i] = (short)fData[i];
    fpDecoder->decode(fBufferSize, &sData[0], fPixels, DecodedReadoutConstants::NUM_ROCSMODULE);

    //    print();
    if (fpHistogrammer)
    {
      fpHistogrammer->fillRawDataHistograms(fBufferSize, fHeader);
    }
    if (fDoWrite) writeAscii();
  }

  // -- Binary buffer: Here "single step" means "single shot" over the full buffer
  if (fRunMode > 0)
  {
    while (1)
    {
      if (fEOF) break;
      header = nextBinaryHeader();
      readWords  = decodeBinaryData();
      //     cout << "Read " << readWords << " words " << endl;
      ++fEvent;
      //    int bla = fpDecoder->decode(fBufferSize, &fData[0], fPixels);
      //      print();
      if (fpHistogrammer)
      {
        //	cout << "Header: " << fHeader << endl;
        //	fpHistogrammer->printRawData(fBufferSize);
        fpHistogrammer->fillRawDataHistograms(fBufferSize, fHeader);
      }
      //    writeAscii();
    }
  }

  return fEvent;
}


// ----------------------------------------------------------------------
void UsbDaq::pause()
{
  fRunState = 2;
  loop();
}

// ----------------------------------------------------------------------
void UsbDaq::resume()
{
  fRunState = 1;
  loop();

}
