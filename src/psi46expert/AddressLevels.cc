#include "interface/Log.h"

#include "AddressLevels.h"
#include "TestRoc.h"
#include "TestModule.h"
#include "BasePixel/GlobalConstants.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/RawPacketDecoder.h"
#include "BasePixel/DecoderCalibration.h"
#include <TLine.h>
#include <TMath.h>

using namespace DecoderCalibrationConstants;

bool AddressLevels::fPrintDebug   = false;
//bool AddressLevels::fPrintDebug   = true;

AddressLevels::AddressLevels(TestRange *aTestRange, TestParameters *testParameters, TBInterface *aTBInterface)
{
  psi::LogDebug() << "[AddressLevels] Initialization." << psi::endl;

  testRange = aTestRange;
  tbInterface = aTBInterface;
}

void AddressLevels::ModuleAction()
{
  for ( int iroc = 0; iroc < module->NRocs(); iroc++ ) fTestedROC[iroc] = false;

  TestTBM();
  Test::ModuleAction(); // This is where RocAction will be called from

//---  write address level limits into ASCII file
//     (only if the address levels have been determined for all ROCs)
  
  bool allROCsTested = true;
  for ( int iroc = 0; iroc < module->NRocs(); iroc++ )
  {
    if ( !fTestedROC[iroc] ) allROCsTested = false;
  }

  if ( allROCsTested )
  {
    DecoderCalibrationModule* decoderCalibrationModule = new DecoderCalibrationModule(fLimitsTBM, fLimitsROC, module->NRocs());
    RawPacketDecoder::Singleton()->SetCalibration(decoderCalibrationModule);
    
    ConfigParameters* configParameters = ConfigParameters::Singleton();
    TString fileName = TString(configParameters->directory).Append("/addressParameters.dat");
    ofstream* file = new ofstream(fileName, ios::out);

    psi::LogInfo() << "[AddressLevels] Writing decoder levels to '"
                   << fileName.Data() << "'." << psi::endl;

    decoderCalibrationModule->Print(file);
    delete file;
  }   
}


void AddressLevels::RocAction()
{
  TestROC();
}

AddressLevels::~AddressLevels()
{
  delete adcHistogramTBM;
  delete adcHistogramROC;
}


void AddressLevels::TestTBM()
{
        adcHistogramTBM = new TH1D("TBMAddressLevels", "TBMAddressLevels", 4000, -2000, 2000);
  
  int data[4000];
  ((TBAnalogInterface*)tbInterface)->TBMAddressLevels(data);
  for (int i = 0; i < 4000; i++) adcHistogramTBM->SetBinContent(i+1, data[i]);
  histograms->AddLast(adcHistogramTBM);
        
  int numLimitsTBM = 0;
        FindDecoderLevels(adcHistogramTBM, numLimitsTBM, fLimitsTBM, NUM_LEVELSTBM + 1, 30);

  if ( fPrintDebug ){
    cout << "TBM address level limits = { ";
    for ( int ilevel = 0; ilevel < (numLimitsTBM + 1); ilevel++ ){
      cout << fLimitsTBM[ilevel] << " ";
    }
    cout << "}" << endl;
  }

  if ( numLimitsTBM != 4 )
  {
    psi::LogInfo() << "[AddressLevels] Error: Can not calibrate decoder. "
                   << ( numLimitsTBM + 1) << " peaks were found in TBM ADC "
                   << "spectrum." << psi::endl;

    return;
  }
}

void AddressLevels::TestROC()
{
  psi::LogInfo() << "[AddressLevels] Chip #" << roc->GetChipId() << '.'
                 << psi::endl;
  adcHistogramROC = new TH1D(Form("AddressLevels_C%d", roc->GetChipId()), Form("AddressLevels_C%d", roc->GetChipId()), 4000, -2000, 2000);

  int data[4000];
  roc->AddressLevelsTest(data);
  for (int i = 0; i < 4000; i++) adcHistogramROC->SetBinContent(i+1, data[i]);
  histograms->AddLast(adcHistogramROC);
  
  int numLimitsROC = 0;
        FindDecoderLevels(adcHistogramROC, numLimitsROC, fLimitsROC[roc->GetAoutChipPosition()], NUM_LEVELSROC + 1, 50);

  fTestedROC[roc->GetAoutChipPosition()] = true;

  if ( fPrintDebug ){
    cout << "ROC (" << roc->GetChipId() << ") address level limits = { ";
    for ( int ilevel = 0; ilevel < (numLimitsROC + 1); ilevel++ ){
      cout << fLimitsROC[roc->GetAoutChipPosition()][ilevel] << " ";
    }
    cout << "}" << endl;
  }

  if ( numLimitsROC != 6 )
  {
    psi::LogInfo() << "[AddressLevels] Error: Can not calibrate decoder. "
                       << ( numLimitsROC + 1) << " peaks were found in ADC "
                       << "spectrum of ROC #" << roc->GetChipId() << '.'
                       << psi::endl;

//--- in case the ROC address levels cannot be calibrated
//    (e.g. if the ROC does not yield hits)
//    set the ROC UltraBlack level to the level of the TBM UltraBlack,
//    such that the address decoding finds at least the ROC header !
    psi::LogDebug() << "[AddressLevels] Setting ROC UltraBlack level to level "
                    << "of TBM UltraBlack." << psi::endl;

    fLimitsROC[roc->GetAoutChipPosition()][0] = fLimitsTBM[0];
    return;
  }
}

void AddressLevels::FindDecoderLevels(TH1* adcHistogram, int& numLimits, short limits[], int maxLimits, int integralLimit)
{
  int peakStart[7], peakStop[7], peakMean[7], peak = 0;
  bool zeroZone = true, endZone;
  
  for (int i = -2000; i < 2000; i++)
  {
//          if ( fPrintDebug ){
//            cout << "bin = " << i << " : bin-content = " << adcHistogram->GetBinContent(i+2000+1) << endl;
//      if ( zeroZone ) cout << " zero zone" << endl;
//      else cout << " not zero zone" << endl;
//          }

    if (zeroZone) 
    {
      int integral = 0;
      for (int k = 0; k < 10; k++)
      {
              if ( (i+k+2000+1 < 4000) ) integral += TMath::Nint(adcHistogram->GetBinContent(i+k+2000+1));
      }

      endZone = (adcHistogram->GetBinContent(i+2000+1) > 0 && integral > integralLimit) ? true : false;

      if (endZone)
      {
              if ( fPrintDebug ) printf("end zero zone %i\n", i);
        peakStart[peak] = i;
        zeroZone = false;
      }
    }
    else
    {
      int integral = 0;
      for (int k = 0; k < 15; k++)
      {
              if ( (i+k+2000+1 < 4000) ) integral += TMath::Nint(adcHistogram->GetBinContent(i+k+2000+1));
      }

      endZone = (adcHistogram->GetBinContent(i+2000+1) == 0 && integral < integralLimit) ? true : false;

                        //printf("peak maxL %i %i\n", peak, maxLimits);
      if (endZone)
      {
              if ( peak < maxLimits ){
//                if ( fPrintDebug ) printf("end not zero zone %i\n", i);
                peakStop[peak] = i;
          peak++;
          zeroZone = true;
        }
        else printf("Error in <AddressLevels::FindDecoderLevels>: too many address levels found !\n");
      }
    }
  }
    
  for (int i = 0; i < peak; i++) peakMean[i] = (peakStop[i] + peakStart[i])/2;
  for (int i = 0; i < (peak - 1); i++) limits[i] = peakMean[i] + (peakMean[i+1] - peakMean[i])/2;
  limits[peak - 1] = 2000;

  numLimits = peak - 1;
}
