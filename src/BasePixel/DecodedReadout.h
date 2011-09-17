#ifndef DECODEDREADOUT_H
#define DECODEDREADOUT_H

//////////////////////////////////////////////////////////////////////////
//
// Structures to store the data decoded from a raw pixel data packet 
// (Modules, [Plaquettes, to be implemented ?], TBMs and ROCs)
//
// Christian Veelken (UC Davis), 06/01/06
// 
/////////////////////////////////////////////////////////////////////////

//--- these typedefs allow the DecodeRawPacket class 
//    to be used in different contexts:
//   --> short for use with PSI46 testboard
//       long for use with spy-data of the Front-End-Driver of the final CMS Pixel system
typedef short ADCword;
//typedef long  ADCword;

namespace DecodedReadoutConstants
{
  const int NUM_ROCSMODULE =   16; // number of ROCs on a module
  const int MAX_PIXELSROC  = 1000; // maximum number of pixel hits in one and the same ROC
}

struct DecodedReadoutTBM
{
  int  tbmEventCounter;
  bool tbmErrorStatus[8];

  ADCword rawTBMheader[4]; 
  ADCword rawTBMtrailer[4]; 
};

struct DecodedReadoutPixel
{
  int rocId;

  int columnROC;
  int rowROC;
  
  int analogPulseHeight;

  int columnModule;
  int rowModule;

  ADCword rawADC[6];
};

struct DecodedReadoutROC
{
  int lastDac;

  struct DecodedReadoutPixel pixelHit[DecodedReadoutConstants::MAX_PIXELSROC];
  int numPixelHits;
};

struct DecodedReadoutModule
{
  struct DecodedReadoutTBM tbm;
  struct DecodedReadoutROC roc[DecodedReadoutConstants::NUM_ROCSMODULE];
};

#endif // DECODEDREADOUT_H

