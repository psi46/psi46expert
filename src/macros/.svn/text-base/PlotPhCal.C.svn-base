// This is a macro to plot a PH curve for a spezific pixel
// from a phCalibration_Cx.dat file

void Init()
{
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1,0);
    
    gStyle->SetTitleW(0.5);
    gStyle->SetTitleH(0.08);
    
    gStyle->SetOptFit(00000);
    gStyle->SetOptStat(111);
    gStyle->SetStatFormat("g");
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(0.5);
    gStyle->SetHistLineWidth(1);
    gROOT->ForceStyle();

    canvas = new TCanvas();
    InitCanvas(canvas);

}

InitCanvas(TPad *aPad, char* option = 0)
{
  if (!option) option = "";
  aPad->SetTickx();
  aPad->SetTicky();
  aPad->SetBottomMargin(.14);
  aPad->SetLeftMargin(.15);
  if (strcmp(option, "log") == 0) aPad->SetLogy();
  if (strcmp(option, "logz") == 0) aPad->SetLogz();
}


InitPad(char* option = 0)
{
  InitCanvas(gPad, option);
}

TLatex* NewLatex()
{
  TLatex *latex = new TLatex;
  latex->SetNDC(kTRUE);
  latex->SetTextSize(0.06);
  latex->SetTextColor(kRed);
  return latex;
}

Double_t Fitfcn( Double_t *x, Double_t *par)
{
	return par[3] + par[2] * TMath::TanH(par[0]*x[0] - par[1]);
}


void PlotPhCal(const char *filename = "phCalibration_C0.dat", int col, int row)
{

  Init();

  int value, a, b, n;
  double x[10], y[10];
  char string[200];
  double aovergain, gain, par1;


  TF1 *pol2Fit = new TF1("pol2Fit", "pol2");
  pol2Fit->SetRange(50,500);
  TF1 *linFit = new TF1("linFit", "pol1");
  linFit->SetRange(150,600);
  TF1 *ursFit = new TF1("ursFit", Fitfcn, 40., 1500., 4);
  ursFit->SetParameter(0,0.00382);
  ursFit->SetParameter(1,0.886);
  ursFit->SetParameter(2,112.7);
  ursFit->SetParameter(3,113.0);


  TGraphErrors *graph = new TGraphErrors();
  TGraph *gSlope = new TGraph();

  FILE *inputFile = fopen(filename, "r");

  for (int i = 0; i < 4; i++) fgets(string, 200, inputFile);
  
  for (int icol = 0; icol < 52; ++icol)
    {
      for (int irow = 0; irow < 80; ++irow)
	{
	  n = 0;
	  for (int point = 0; point < 10; point++)
	    {
	      fscanf(inputFile, "%s", string);
	      
	      if (strcmp(string, "N/A") == 0)  value = 7777;
	      else 
		{
		  value  = atoi(string);
		}		      
	      if ((icol == col) && (irow == row))
		{

		  if (point == 0) x[n] = 50;
		  else if (point == 1) x[n] = 100;
		  else if (point == 2) x[n] = 150;
		  else if (point == 3) x[n] = 200;
		  else if (point == 4) x[n] = 250;
		  else if (point == 5) x[n] = 210;
		  else if (point == 6) x[n] = 350;
		  else if (point == 7) x[n] = 490;
		  else if (point == 8) x[n] = 560;
		  else if (point == 9) x[n] = 1400;
		  
		  if (value != 7777)
		    {
		      //  y[n] = value;  // in testboard adc range
		      y[n] = (value + 400) / 7;  // in 8 bit experiment adc range
		      graph->SetPoint(n, x[n], y[n]);	
		      //  graph->SetPointError(n, 2, 10);   // in testboard adc range
		      graph->SetPointError(n, 2, 10/6);   // in 8 bit experiment adc range

		      n++;
		    }
		}
	    }
	  fscanf(inputFile, "%s %2i %2i", string, &a, &b);  //comment
	}
    }
  graph->GetXaxis()->SetTitle("Vcal [low range DAC units]");
  graph->GetYaxis()->SetTitle("PH [ADC units]");
  graph->GetYaxis()->SetTitleOffset(1.2);


  graph->GetXaxis()->SetTitleSize(0.055);
  graph->GetYaxis()->SetTitleSize(0.055);
  graph->GetXaxis()->SetLabelSize(0.04);
  graph->GetYaxis()->SetLabelSize(0.05);
  graph->GetXaxis()->SetTitleOffset(1.15);
  graph->GetYaxis()->SetTitleOffset(1.2);
  graph->SetTitle("");
  graph->SetLineStyle(1);
  graph->SetLineWidth(2);

  //   graph->Fit("linFit","R");
//   gain = linFit->GetParameter(1);


  graph->Fit("ursFit","R");
  graph->Draw("A*");
  par1 = ursFit->GetParameter(1);

  
  NewLatex()->DrawLatex(0.55, 0.5, Form("p_{1} = %.1f ", par1));
  //  NewLatex()->DrawLatex(0.45, 0.4, "NON-LINEAR pixel");       // PlotPhCal("/home/l_tester/ptr/moduleDB/M0567-070115.08:23/T-10a/phCalibration_C12.dat",7,23)
  NewLatex()->DrawLatex(0.5, 0.4, "LINEAR pixel");                // PlotPhCal("/home/l_tester/ptr/moduleDB/M0493-061206.10:45/T-10a/phCalibration_C1.dat",7,23)
}
