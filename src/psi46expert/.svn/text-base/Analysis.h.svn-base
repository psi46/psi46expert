// Utilities to analyse histograms

#ifndef ANALYSIS
#define ANALYSIS

#include <TH2D.h>
#include <TH1D.h>
#include "BasePixel/GlobalConstants.h"

class Analysis
{

public:
    Analysis();

    TH1D * TrimBitTest(TH2D * calMap, TH2D * trimMap, char * histoName);
    TH2D * DifferenceMap(TH2D * map1, TH2D * map2, char * mapName);
    TH2D * SumVthrVcal(TH2D * map1, TH2D * map2, TH2D * map3, char * mapName);
    TH1D * Distribution(TH2D * map, int nBins, double lowerEdge, double upperEdge);
    TH1D * Distribution(TH2D * map);

protected:


};

static Analysis * gAnalysis = new Analysis();


#endif

