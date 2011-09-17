//
// Authors: Sarah Dambach,
//          Christian Veelken
// Modified: Samvel Khalatyan (samvel at cern dot ch)
//
// Pulse height dependency on Vsf and VhldDel DACs
// 


#ifndef VSFOPTIMIZATION
#define VSFOPTIMIZATION

#include "PhDacScan.h"
#include <TH2D.h>
#include <TH1D.h>
#include <TArrayD.h>

class VsfOptimization : public PhDacScan
{
  public:
    VsfOptimization() {};

    VsfOptimization(TestRange *testRange, TestParameters* testParameters, TBInterface *aTBInterface);

    virtual void ReadTestParameters(TestParameters *testParameters);  
    virtual void RocAction();

    void DoDacDacScan();
    void VsfOpt();
    int CurrentOpt();
    int CurrentOpt2();
    int Par1Opt();

    int TestCol();

  private:
    // Group Input parameters
    struct Input {
      int start;
      int stop;
      int steps;
    };

    TArrayI bestVsf_pixel;
    TArrayI bestVhldDel_pixel;
    TArrayF bestQuality_pixel;
    int     bestVsf_ROC;
    int     bestVhldDel_ROC;

    int     par1Vsf;
    int     optVsf;

    Input  vsf;
    Input  vhldDel;

    double goalCurrent;
    double goalPar1;
};

#endif

