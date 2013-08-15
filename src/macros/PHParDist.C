TCanvas *canvas = new TCanvas();

void Init()
{
    gROOT->SetStyle("Plain");
    gStyle->SetTitleBorderSize(0);
    gStyle->SetPalette(1,0);
    
    gStyle->SetTitleW(0.5);
    gStyle->SetTitleH(0.08);
    
    gStyle->SetOptFit(0);
    gStyle->SetOptStat("emrou");
    //gStyle->SetOptStat(0);
    gStyle->SetStatFormat("g");
    gStyle->SetMarkerStyle(20);
    gStyle->SetMarkerSize(0.5);
    gStyle->SetHistLineWidth(1);
    gROOT->ForceStyle();


    InitCanvas(canvas, "log");
    //InitCanvas(canvas);

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



void PHParDist(const char *filename = "phCalibrationFitTan_C0.dat") {

  Init();

  FILE *inputFile;
  inputFile = fopen(filename, "r");

  TH1D *histPar0 = new TH1D("par0","par0",1000,0,0.01);
  TH1D *histPar1 = new TH1D("par1","par1",1000,-5,5);
  TH1D *histPar2 = new TH1D("par2","par2",1000,500,1500);
  TH1D *histPar3 = new TH1D("par3","par3",1000,-100,100);

  char string[200];
  float par0, par1, par2, par3;
  int a,b;
	

  for (int i = 0; i < 2; i++) fgets(string, 200, inputFile);
  
  for (int icol = 0; icol < 52; ++icol)
    {
      for (int irow = 0; irow < 80; ++irow)
	{
	  fscanf(inputFile, "%e %e %e %e %s %2i %2i", &par0, &par1, &par2, &par3, string, &a, &b);
	  histPar0->Fill(par0);
	  histPar1->Fill(par1);
	  histPar2->Fill(par2);
	  histPar3->Fill(par3);
	}
    }
  canvas->Divide(2,2);
  canvas->cd(1);
  histPar0->Draw();
  canvas->cd(2);
  histPar1->Draw();
  canvas->cd(3);
  histPar2->Draw();
  canvas->cd(4);
  histPar3->Draw();

}
