const Float_t vCal_rangeConversionFactor = 7.;

static const Int_t vCal_steps        =  51;
static const Int_t vCal_stepWidth    =   5;
Int_t vCal[2*vCal_steps]; 
static const Int_t vsf_0             = 120;
static const Int_t vsf_steps         =  16;
static const Int_t vsf_stepWidth     =   5;
Int_t vsf[vsf_steps]; 
static const Int_t vhldDel_0         =  10;
//static const Int_t vhldDel_0         =  10;
static const Int_t vhldDel_steps     =  50;
static const Int_t vhldDel_stepWidth =  5;
Int_t vhldDel[vhldDel_steps]; 

//static const Int_t nROCs    = 16; // number of ROCs per module
static const Int_t nROCs    =  8; // only process data taken with first ROC
static const Int_t nColumns = 52; // number of columns per ROC
static const Int_t nRows    = 80; // number of rows per column

static const Int_t nVcalGraphs = 5;
Int_t vCal_graphs[nVcalGraphs] = { 100/vCal_stepWidth, 200/vCal_stepWidth, 
				   70/vCal_stepWidth + vCal_steps, 140/vCal_stepWidth + vCal_steps, 210/vCal_stepWidth + vCal_steps };
//const int nPixels = 4160;
const int nPixels = 151;
//const int nPixels = 100;
TArrayF* gArrayPulseHeight_values[nROCs][vsf_steps][vhldDel_steps][nVcalGraphs];
Int_t gArrayPulseHeight_nValues[nROCs][vsf_steps][vhldDel_steps][nVcalGraphs];
TGraph* gGraphPulseHeight1SigmaUp[nROCs][vsf_steps][nVcalGraphs];
TGraph* gGraphPulseHeightMedian[nROCs][vsf_steps][nVcalGraphs];
TGraph* gGraphPulseHeight1SigmaDown[nROCs][vsf_steps][nVcalGraphs];

TH1D* gHistogramDummy = NULL;

TCanvas* gCanvas = NULL; 
TPostScript* gPostScript = NULL; 

//const Bool_t gDebugInformation = true;
const Bool_t gDebugInformation = false;

//const Bool_t gDoubleColumnInformation = true;
const Bool_t gDoubleColumnInformation = false;

Int_t arrayIndex(Int_t row, Int_t column)
{
  return row + column*nRows;
}

void sort(TArrayF* pulseHeight_values, Int_t nMeasuredPulseHeights)
{
  static TArrayI* sortedIndices = new TArrayI(nRows*nColumns);

//--- sort pulse height values in increasing order
//    (lowest pulse height value stored at array index = 0 after sorting)
  TMath::Sort(nMeasuredPulseHeights, pulseHeight_values->GetArray(), sortedIndices->GetArray(), false);

//--- fill in sorted order array elements into temporary data structures
  static TArrayF* sortedPulseHeight_values   = new TArrayF(nRows*nColumns);
  for ( Int_t ipixel = 0; ipixel < nMeasuredPulseHeights; ipixel++ ){
    Int_t sortedIndex = sortedIndices->At(ipixel);
    sortedPulseHeight_values->AddAt(pulseHeight_values->At(sortedIndex), ipixel);
  }

//--- copy sorted array elements into original array
  for ( Int_t ipixel = 0; ipixel < nMeasuredPulseHeights; ipixel++ ){
    pulseHeight_values->AddAt(sortedPulseHeight_values->At(ipixel), ipixel);
  }
}
 
void initialize()
{
  gROOT->SetStyle("Plain");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetPalette(1,0);

  gCanvas = new TCanvas("gCanvas", "gCanvas", 1, 1, 800, 600);
  gCanvas->SetFillColor(10);
  gCanvas->SetBorderSize(5);

  gPostScript = new TPostScript("myVsfVhldDelVariation.ps", 112);

  for ( Int_t ivcalGraph = 0; ivcalGraph < nVcalGraphs; ivcalGraph++ ){
    cout << "vCal_graphs[" << ivcalGraph << "] = " << vCal_graphs[ivcalGraph] << endl;
  }

  for ( Int_t ivcal = 0; ivcal < vCal_steps; ivcal++ ){
    vCal[ivcal]              = ivcal*vCal_stepWidth;
    vCal[ivcal + vCal_steps] = ivcal*vCal_stepWidth*vCal_rangeConversionFactor;
  }

  for ( Int_t ivsf = 0; ivsf < vsf_steps; ivsf++ ){
    vsf[ivsf] = vsf_0 + ivsf*vsf_stepWidth;
  }

  for ( Int_t ivhldDel = 0; ivhldDel < vhldDel_steps; ivhldDel++ ){
    vhldDel[ivhldDel] = vhldDel_0 + ivhldDel*vhldDel_stepWidth;
  }
  
  for ( Int_t iroc = 0; iroc < nROCs; iroc++ ){
    for ( Int_t ivsf = 0; ivsf < vsf_steps; ivsf++ ){
      for ( Int_t ivhldDel = 0; ivhldDel < vhldDel_steps; ivhldDel++ ){
	for ( Int_t ivcalGraph = 0; ivcalGraph < nVcalGraphs; ivcalGraph++ ){
	  gArrayPulseHeight_values[iroc][ivsf][ivhldDel][ivcalGraph] = new TArrayF(nPixels);
	}
      }
    }
  }

  for ( Int_t iroc = 0; iroc < nROCs; iroc++ ){
    for ( Int_t ivsf = 0; ivsf < vsf_steps; ivsf++ ){
      for ( Int_t ivcalGraph = 0; ivcalGraph < nVcalGraphs; ivcalGraph++ ){
	gGraphPulseHeight1SigmaUp[iroc][ivsf][ivcalGraph]   = new TGraph();
	gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]     = new TGraph();
	gGraphPulseHeight1SigmaDown[iroc][ivsf][ivcalGraph] = new TGraph();
      }
    }
  }

  gHistogramDummy = new TH1D("gHistogramDummy", "Pulse Height as function of V_{hldDel}", 256, -0.5, 255.5);
  gHistogramDummy->SetStats(false);
  gHistogramDummy->SetMinimum(-1000);
  gHistogramDummy->SetMaximum(2000);
  gHistogramDummy->GetXaxis()->SetTitle("V_{hldDel} [DAC units]");
  gHistogramDummy->SetTitleOffset(1.05, "X");
  gHistogramDummy->GetYaxis()->SetTitle("PH [ADC units]");
  gHistogramDummy->SetTitleOffset(1.25, "Y");
}

void processAllCurves(const char* directoryName)
{
//--- initialize internal data-structures	
  initialize();

//--- read pulse height information from file
  for ( Int_t iroc = 0; iroc < nROCs; iroc++ ){
    printf("Loading pulse height measurements for ROC %i\n", iroc);
    
    for ( Int_t ivsf = 0; ivsf < vsf_steps; ivsf++ ){
      for ( Int_t ivhldDel = 0; ivhldDel < vhldDel_steps; ivhldDel++ ){
	char inputFileName[255];
	sprintf(inputFileName, "%s/vsfFiles/phCalibration_Vsf%i_VhldDel%i_C%i.dat", directoryName, vsf[ivsf], vhldDel[ivhldDel], iroc);

	cout << "Opening file " << inputFileName << " for reading" << endl;

	FILE* inputFile = fopen(inputFileName, "r");
	if ( !inputFile ){
	  printf("!!!!!!!!!  ----> myPulseHeightVariation: Could not open file %s to read pulse height information\n", inputFileName);
	  return;
	}

//--- skip input file header (three lines of descriptions followed by empty line)
	char line[255];
	for ( int iline = 0; iline < 4; iline++ ) fgets(line, 255, inputFile);

	Int_t pixel = 0;
	while ( !feof(inputFile) ){
	  for ( Int_t ivcal = 0; ivcal < 2*vCal_steps; ivcal++ ){
	    char value[255];
	    fscanf(inputFile, "%s", value);

	    for ( Int_t ivcalGraph = 0; ivcalGraph < nVcalGraphs; ivcalGraph++ ){
	      if ( ivcal == vCal_graphs[ivcalGraph] ){
		if ( strcmp(value, "N/A") != 0 ){
		  Int_t ipixel = gArrayPulseHeight_nValues[iroc][ivsf][ivhldDel][ivcalGraph];
		  gArrayPulseHeight_values[iroc][ivsf][ivhldDel][ivcalGraph]->AddAt(atoi(value), ipixel);
		  gArrayPulseHeight_nValues[iroc][ivsf][ivhldDel][ivcalGraph]++;
		}
	      }
	    }
	  }

//--- skip row and column numbers terminating each line of the input file (format is "Pix column# row#")
	  pixel++;
	  for ( Int_t ivalue = 0; ivalue < 3; ivalue++ ){
	    char value[255];
	    fscanf(inputFile, "%s", value);
	  }
	}

	fclose(inputFile);
      }
    }
  }

//--- process the information that has been read from file
  gCanvas->Clear();
  gCanvas->Divide(1, 1);
  gCanvas->cd(1);

  for ( Int_t iroc = 0; iroc < nROCs; iroc++ ){
    printf("Processing pulse height measurements for ROC %i\n", iroc);
    
    for ( Int_t ivsf = 0; ivsf < vsf_steps; ivsf++ ){
      for ( Int_t ivhldDel = 0; ivhldDel < vhldDel_steps; ivhldDel++ ){
	for ( Int_t ivcalGraph = 0; ivcalGraph < nVcalGraphs; ivcalGraph++ ){
	  TArrayF* pulseHeight_values    = gArrayPulseHeight_values[iroc][ivsf][ivhldDel][ivcalGraph];
	  Int_t    nMeasuredPulseHeights = gArrayPulseHeight_nValues[iroc][ivsf][ivhldDel][ivcalGraph];

//--- sort calibration voltages in ascending order
	  sort(pulseHeight_values, nMeasuredPulseHeights);

	  Float_t pulseHeight1SigmaUp   = pulseHeight_values->At(TMath::Nint(0.84 * nMeasuredPulseHeights));
	  Float_t pulseHeightMedian     = pulseHeight_values->At(TMath::Nint(0.50 * nMeasuredPulseHeights));
	  Float_t pulseHeight1SigmaDown = pulseHeight_values->At(TMath::Nint(0.16 * nMeasuredPulseHeights));

	  gGraphPulseHeight1SigmaUp[iroc][ivsf][ivcalGraph]->SetPoint(ivhldDel, vhldDel[ivhldDel], pulseHeight1SigmaUp);
	  gGraphPulseHeight1SigmaUp[iroc][ivsf][ivcalGraph]->SetPoint(2*vhldDel_steps - (ivhldDel + 1), vhldDel[ivhldDel], pulseHeightMedian);
	  gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]->SetPoint(ivhldDel, vhldDel[ivhldDel], pulseHeightMedian);
	  gGraphPulseHeight1SigmaDown[iroc][ivsf][ivcalGraph]->SetPoint(ivhldDel, vhldDel[ivhldDel], pulseHeightMedian);
	  gGraphPulseHeight1SigmaDown[iroc][ivsf][ivcalGraph]->SetPoint(2*vhldDel_steps - (ivhldDel + 1), vhldDel[ivhldDel], pulseHeight1SigmaDown);
	}
      }
    
      for ( Int_t ivcalGraph = 0; ivcalGraph < nVcalGraphs; ivcalGraph++ ){
	Int_t fillColour = 14 + nVcalGraphs - (ivcalGraph + 1);
	gGraphPulseHeight1SigmaUp[iroc][ivsf][ivcalGraph]->SetFillColor(fillColour);
	gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]->SetLineColor(1);
	gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]->SetLineWidth(2);
	gGraphPulseHeight1SigmaDown[iroc][ivsf][ivcalGraph]->SetFillColor(fillColour);
      }

      char rocNumber[3] = {0};
      sprintf(rocNumber, "%d", iroc);
      char vsfValue[4] = {0};
      sprintf(vsfValue, "%d", vsf[ivsf]);
      gHistogramDummy->SetTitle(TString("Pulse Height as function of V_{hldDel} for ROC").Append(rocNumber).Append(" and V_{sf} = ").Append(vsfValue));
      gHistogramDummy->Draw();

      for ( Int_t ivcalGraph = 0; ivcalGraph < nVcalGraphs; ivcalGraph++ ){
	gGraphPulseHeight1SigmaUp[iroc][ivsf][ivcalGraph]->Draw("F");
	gGraphPulseHeight1SigmaDown[iroc][ivsf][ivcalGraph]->Draw("F");
	gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]->Draw("L");
      }

      gHistogramDummy->Draw("AXISsame");

      TLegend* legend = new TLegend(0.89, 0.27, 1.01, 0.80, NULL, "brNDC");
      for ( Int_t ivcalGraph = nVcalGraphs - 1; ivcalGraph >= 0; ivcalGraph-- ){
	char vCalValue[5] = {0};
	sprintf(vCalValue, "%d", vCal[vCal_graphs[ivcalGraph]]);
	legend->AddEntry(gGraphPulseHeight1SigmaUp[iroc][ivsf][ivcalGraph], TString("V_{cal} = ").Append(vCalValue), "f");
      }
      legend->SetBorderSize(0);
      legend->SetFillColor(10);
      legend->SetTextSize(0.028);
      legend->Draw();

      gCanvas->cd();

      gCanvas->Update();
      gPostScript->NewPage();
    }
  }

  for ( Int_t ivsf = 0; ivsf < vsf_steps; ivsf++ ){

    TLegend* legend_allROCs = new TLegend(0.89, 0.27, 1.01, 0.80, NULL, "brNDC");
    legend_allROCs->SetBorderSize(0);
    legend_allROCs->SetFillColor(10);
    legend_allROCs->SetTextSize(0.028);

    char rocNumber[3] = {0};
    sprintf(rocNumber, "%d", iroc);
    char vsfValue[4] = {0};
    sprintf(vsfValue, "%d", vsf[ivsf]);
    gHistogramDummy->SetTitle(TString("Pulse Height as function of V_{hldDel} for ROC").Append(rocNumber).Append(" and V_{sf} = ").Append(vsfValue));
    gHistogramDummy->Draw();

    for ( Int_t ivcalGraph = 0; ivcalGraph < nVcalGraphs; ivcalGraph++ ){
      for ( Int_t iroc = 0; iroc < nROCs; iroc++ ){
	gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]->SetLineColor((iroc % 8) + 1);
	gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]->SetLineStyle((iroc / 8) + 1);
	gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]->SetLineWidth(1);

	char rocNumber[3] = {0};
	sprintf(rocNumber, "%d", iroc);
	if ( ivcalGraph == 0 ) legend_allROCs->AddEntry(gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph], TString("ROC").Append(rocNumber), "l");

	gGraphPulseHeightMedian[iroc][ivsf][ivcalGraph]->Draw("L");
      }
    }

    gHistogramDummy->Draw("AXISsame");

    legend_allROCs->Draw();

    gCanvas->Update();
    gPostScript->NewPage();
  }

  delete gCanvas;
  delete gPostScript;
}
