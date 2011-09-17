// Test Scans a whole range of Vsf and plots
//    * PulseHeight linearity parameter in low Vcal range vs Vsf
//    * Digital Current vs Vsf

#ifndef VSF_SCAN_H
#define VSF_SCAN_H

#include <utility>

#include "PhDacScan.h"

// Forward declarations
class TestRange;
class TestParameters;
class TBInterface;

class VsfScan: public PhDacScan
{
  public:
    VsfScan( TestRange      *_range, 
             TestParameters *_parameters,
             TBInterface    *_interface);
    virtual ~VsfScan() {}

    virtual void RocAction();

  private:
    void scan();
    int  getTestColumn();

    // Group Input Parameters
    struct Input {
      int start;
      int stop;
      int steps;
    };

    Input vsf;

    const int VSF_DAC_REGISTER;

                          // Vcal range used for PH scan
    const std::pair<const int, const int> PH_VCAL_RANGE; 
};

#endif  // VSF_SCAN_H
