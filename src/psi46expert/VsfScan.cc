#include <cmath>

#include <algorithm>
#include <sstream>

#include "TF1.h"
#include "TH1D.h"

#include "interface/Log.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/GlobalConstants.h"
#include "TestRoc.h"

#include "VsfScan.h"

VsfScan::VsfScan( TestRange      *_range,
                            TestParameters *_parameters,
                            TBInterface    *_interface)
  : PhDacScan( _range, _parameters, _interface),
    VSF_DAC_REGISTER( 3),
    PH_VCAL_RANGE( 25, 256)
{
  // Extract Relevant Input Test Parameters
  vsf.start = _parameters->vsfStart;
  vsf.stop  = _parameters->vsfStop;
  vsf.steps = _parameters->vsfSteps;
}

void VsfScan::RocAction()
{
  // Cache DAC Parameters
  SaveDacParameters();

  scan();

  // Restore DAC Parameters
  RestoreDacParameters();
}

void VsfScan::scan()
{
  psi::LogInfo() << "[VsfScan] Scan Vsf for PH Linearity parameter and Digital Current "
                 << "Plots. /this is a time consuming test/" << psi::endl;

  // Fitting Function
  std::ostringstream _name;
  _name << "VsfScan:Scan:PHFit:ROC" << chipId; // Note: Name should be unuque
  TF1 *_fit = new TF1( _name.str().c_str(), "[3] + [2] * TMath::TanH( [0] * x - [1] )",
                      50, 1500);
  _fit->SetNpx      ( 1000); // Set # of points used to draw the function
  _fit->SetParameter( 0, 0.013);
  _fit->SetParameter( 1, 1);
  _fit->SetParameter( 2, 800);
  _fit->SetParameter( 3, 150);
  _fit->SetParLimits( 2, 0, 10000);

  struct Pixel {
    int column;
    int row;
  };

  // Book Linearity Parameter vs Vsf Histogram
  _name.str( "");
  _name << "Linearity vs Vsf: ROC" << chipId;
  TH1D *_linearityHist = new TH1D( _name.str().c_str(),
                                   _name.str().c_str(),
                                   vsf.steps, vsf.start, vsf.stop);

  // Book Digital Current vs Vsf Plot
  _name.str( "");
  _name << "Digital Current vs Vsf: ROC" << chipId;
  TH1D *_dcHist = new TH1D( _name.str().c_str(),
                            _name.str().c_str(),
                            vsf.steps, vsf.start, vsf.stop);

  SetDAC( "CtrlReg", 4);

  // Get Column # that will be used for testing. That is the one having the
  // smallest deviation of linearity parameter from its mean measured for 5
  // columns in a Chip for deafult value of Vsf
  Pixel _pixel = { getTestColumn(), 5 };

  // Lock Pixel
  roc->ArmPixel( _pixel.column, _pixel.row);

  TBAnalogInterface *_interface = dynamic_cast<TBAnalogInterface *>( tbInterface);

  // Scan Vsf range specified in input test parameters file and get plots.
  for( int _dacValue = vsf.start,
           _step     = ( vsf.stop - vsf.start) / vsf.steps,
           _offset   = _interface->TBMPresent() ? 16 : 9,
           _vsfBin   = 1;
       _dacValue < vsf.stop;
       _dacValue += _step, ++_vsfBin)
  {
    // Apply new Vsf to Pixel
    SetDAC( VSF_DAC_REGISTER, _dacValue);

    tbInterface->Flush();

    // Delay is needed for Digital Current and Voltages applied to chip/module
    // to stabilyze.
    sleep( 2);

    // Extract Digital Current and add it's value to Histogram
    _dcHist->SetBinContent( _vsfBin, _interface->GetID() );

    // Scan Vcal in range defined below. Note PH values are not nulled (!)
    // even though not whole range of Vcal is scanned: PH_VCAL_RANGE.first 
    // might be NON-ZERO. The same remark is applicable to the top edge of
    // scanned range.
    short _pulseHeights[psi::DAC8];
    _interface->PHDac( PH_VCAL_RANGE.first, PH_VCAL_RANGE.second, nTrig, 
                       _offset + aoutChipPosition * 3, _pulseHeights);

    // Create PH vs Vcal plot and fit it to extract Linearity Parameter
    _name.str( "");
    _name << "PH vs Vcal: ROC" << chipId << " Vsf" << _dacValue;
    TH1D *_hist = new TH1D( _name.str().c_str(),
                            _name.str().c_str(),
                            256, 0, 256);

    // Fill Histogram with data
    for( int _bin = 0; psi::DAC8 > _bin; ++_bin)
    {
      _hist->SetBinContent( _bin + 1, _bin < PH_VCAL_RANGE.first || 
                                      _bin > PH_VCAL_RANGE.second 
                                      ? 1000
                                      : _pulseHeights[_bin]);
    }

    //Find Bin with MINIMUM
    double minFit = _hist->GetBinCenter( _hist->GetMinimumBin() );

    // you also need to cut off the upper part!
    float delta;
    int bin;
    for( bin = _hist->GetMinimumBin();bin<255;bin++)
    {
    delta = _hist->GetBinContent(bin+1)-_hist->GetBinContent(bin);
    if (delta > 1000) break;
    }

    _fit->SetRange(minFit+10, _hist->GetBinCenter(bin)-1);

    _hist->Fit( _fit, "RQ", ""); 
     //,_hist->GetBinCenter( _hist->GetMinimumBin() ) + 10, 255);

    // Add new Linearity Parameter into Histogram
    _linearityHist->SetBinContent( _vsfBin, _fit->GetParameter( 1) );

    // Make Histogram available in output ROOT file
    _hist->GetYaxis()->SetRangeUser( -1000, 1500);
    histograms->Add( _hist);
  }

  delete _fit;

  // Unlock Pixel
  roc->DisarmPixel( _pixel.column, _pixel.row);

  histograms->Add( _linearityHist);
  histograms->Add( _dcHist);

  psi::LogInfo() << "[VsfScan] Scan is done." << psi::endl;
}

class Deviate {
  private:
    const double MEAN;

    double minDiff;
    int    minKey;
    int    key;

  public:
    Deviate( const double &_mean)
      : MEAN   ( _mean), 
        minDiff(  999999999), 
        minKey ( 0), 
        key    ( 0) {}

    int getMinKey() const { return minKey; }

    // WARNING: Argument gets changed that is deviated from mean.
    void operator() ( double &_val)
    {
      _val = std::abs( _val - MEAN);
      if( _val < minDiff) {
        minDiff = _val;
        minKey  = key;
      }

      ++key;
    }
};

int VsfScan::getTestColumn()
{
  // Fitting Function
  std::ostringstream _name;
  _name << "VsfScan:GetTestColumn:PHFit:ROC" << chipId; // Note: Name should be unuque
  TF1 *_fit = new TF1( _name.str().c_str(), "[3] + [2] * TMath::TanH( [0] * x - [1] )",
                      50, 1500);
  _fit->SetNpx      ( 1000); // Set # of points used to draw the function
  _fit->SetParameter( 0, .00382);
  _fit->SetParameter( 1, .886);
  _fit->SetParameter( 2, 112.7);
  _fit->SetParameter( 3, 113);

  TBAnalogInterface *_interface = dynamic_cast<TBAnalogInterface *>( tbInterface);

  std::vector<double> _linearities;

  // Try 5 pixels in different columns but same row and extract Linearity
  // Parameter for each of them
  for( int _col    = 5,
           _offset = _interface->TBMPresent() ? 16 : 9; 
       psi::DCOLS > _col; 
       _col += 5)
  {
    roc->ArmPixel( _col, 5);
    _interface->Flush();

    short _pulseHeights[psi::DAC8];
    _interface->PHDac( PH_VCAL_RANGE.first, PH_VCAL_RANGE.second, nTrig, 
                       _offset + aoutChipPosition * 3, _pulseHeights);

    _name.str( "");
    _name << "GetTestColumn:PHvsVcal:ROC" << chipId << ":Col" << _col;
    TH1D *_hist = new TH1D( _name.str().c_str(),
                            _name.str().c_str(),
                            256, 0, 256);

    // Fill Histogram with data
    for( int _bin = 0; psi::DAC8 > _bin; ++_bin)
    {
      _hist->SetBinContent( _bin + 1, _bin < PH_VCAL_RANGE.first || 
                                      _bin > PH_VCAL_RANGE.second 
                                      ? 1000
                                      : _pulseHeights[_bin]);
    }


    //Find Bin with MINIMUM
    double minFit = _hist->GetBinCenter( _hist->GetMinimumBin() );

    // you also need to cut off the upper part!
    float delta;
    int bin;
    for( bin = _hist->GetMinimumBin();bin<255;bin++)
    {
    delta = _hist->GetBinContent(bin+1)-_hist->GetBinContent(bin);
    if (delta > 1000) break;
    }

    _fit->SetRange(minFit+10, _hist->GetBinCenter(bin)-1);

    _hist->Fit( _fit, "RQ", ""); 


    //_hist->Fit( _fit, "RQ", "", _hist->GetBinCenter( _hist->GetMinimumBin() ) + 10, 255);
    _linearities.push_back( _fit->GetParameter( 1) );

    delete _hist;

    roc->DisarmPixel( _col, 5);
  }

  delete _fit;

  // Calculate Mean Linearity Parameter
  double _meanLinearity = 0;

  for( std::vector<double>::const_iterator _iter = _linearities.begin();
       _linearities.end() != _iter;
       ++_iter)
  {
    _meanLinearity += *_iter;
  }

  Deviate _deviate( _meanLinearity / _linearities.size() );

  // Find Columnt that has deviation of its Linearity parameter from mean
  std::for_each( _linearities.begin(), _linearities.end(), _deviate);

  return 5 + _deviate.getMinKey() * 5;
}
