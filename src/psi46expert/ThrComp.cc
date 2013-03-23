#include <iostream>
#include <stdio.h>
#include "TF1.h"
#include "TH1D.h"
#include "TH2.h"
#include "interface/Delay.h"
#include "interface/Log.h"
#include "BasePixel/Roc.h"
#include "BasePixel/TBAnalogInterface.h"
#include "BasePixel/ConfigParameters.h"
#include "TestRoc.h"
#include "ThrComp.h"
#include <TMath.h>

ThrComp::ThrComp(TestRange *aTestRange, TestParameters *aTestParameters, TBInterface *aTBInterface)
{
  testParameters = aTestParameters;
  testRange = aTestRange;
  tbInterface = aTBInterface;
  ReadTestParameters(testParameters);
}


void ThrComp::ReadTestParameters(TestParameters *testParameters)
{
  debug = false;
}


void ThrComp::RocAction()
{
  Float_t vcal = 200.;

  double data[ROC_NUMROWS*ROC_NUMCOLS];
  double dataMax[ROC_NUMROWS*ROC_NUMCOLS];
  double efficiency, lastEfficiency = 0.;

  printf("VthrComp roc %i\n", chipId);
  
  SetDAC("Vcal", TMath::Nint(vcal));
  //SetDAC("CtrlReg", 4);
  SetDAC("CtrlReg", 0);

  TGraph* graph = new TGraph();
  TString name = Form("VthrComp_Vcal%i", (int) vcal);
  graph->SetName(name);
  graph->SetLineColor(2);
  graph->SetLineStyle(1);
  graph->SetLineWidth(2);
  graph->SetMarkerColor(2);

  int nPoints = 0;

  for ( Int_t ithrComp = 0; ithrComp < 255; ithrComp += 10 ){
    cout << "VthrComp = " << ithrComp << " : ";

    SetDAC("VthrComp", ithrComp);

    this->RocActionAuxiliary(data, dataMax);

    cout << endl;

    efficiency = 0.;
    for ( int ipixel = 0; ipixel < ROC_NUMROWS*ROC_NUMCOLS; ipixel++ ) efficiency += dataMax[ipixel];
    efficiency /= ROC_NUMROWS*ROC_NUMCOLS;
    cout << " efficiency = " << efficiency << endl;
		
    if ( TMath::Abs(lastEfficiency - efficiency) > 0.1 ){
      for ( int jthrComp = -9; jthrComp <= 0; jthrComp++ ){
	cout << "VthrComp = " << ithrComp + jthrComp << " : ";

	SetDAC("VthrComp", ithrComp + jthrComp);

	this->RocActionAuxiliary(data, dataMax);

	cout << endl;

	efficiency = 0.;
	for ( int ipixel = 0; ipixel < ROC_NUMROWS*ROC_NUMCOLS; ipixel++ ) efficiency += dataMax[ipixel];
	efficiency /= ROC_NUMROWS*ROC_NUMCOLS;
	cout << " efficiency = " << efficiency << endl;
	
	graph->SetPoint(nPoints, ithrComp + jthrComp, efficiency);
	nPoints++;
      }
    } else {
      graph->SetPoint(nPoints, ithrComp, efficiency);
      nPoints++;
    }

    lastEfficiency = efficiency;
  }
  
  histograms->Add(graph);
  graph->Write();
}

void ThrComp::RocActionAuxiliary(double data[], double dataMax[])
{
  for ( int ipixel = 0; ipixel < ROC_NUMROWS*ROC_NUMCOLS; ipixel++ ){
    dataMax[ipixel] = -1e6;
  }

  for ( Int_t icalDel = 0; icalDel < 255; icalDel += 25 ){
    printf(".");
    cout.flush();
    
    SetDAC("CalDel", icalDel);
    Flush();
    roc->ChipEfficiency(10, data);
    
    for ( int ipixel = 0; ipixel < ROC_NUMROWS*ROC_NUMCOLS; ipixel++ ){
      if ( data[ipixel] > dataMax[ipixel] ) dataMax[ipixel] = data[ipixel];
    }
  }

  //for ( int ipixel = 0; ipixel < ROC_NUMROWS*ROC_NUMCOLS; ipixel++ ){
  //  cout << "dataMax[" << ipixel << "] = " << dataMax[ipixel] << endl;
  //}
}


