FILE *file;
int values[26];
char *names[26];
bool produceFiles = false;


Init()
{
  gROOT->SetStyle("Plain");
  gStyle->SetTitleBorderSize(0);
  gStyle->SetPalette(1,0);
  
  gStyle->SetTitleW(0.5);
  gStyle->SetTitleH(0.08);
  
  gStyle->SetOptFit(0111);
  gStyle->SetOptStat(0);
  gStyle->SetStatFormat("g");
  gStyle->SetMarkerStyle(20);
  gStyle->SetMarkerSize(2);
  gStyle->SetHistLineWidth(1);
  gROOT->ForceStyle();
  
  canvas = new TCanvas();
  canvas->SetTickx();
  canvas->SetTicky();
  canvas->SetBottomMargin(.14);
  canvas->SetLeftMargin(.13);

  
  names[0]="Vdig";
  names[1]="Vana";
  names[2]="Vsf";
  names[3]="Vcomp";
  names[4]="Vleak_comp";
  names[5]="VrgPr";
  names[6]="VwllPr";
  names[7]="VrgSh";
  names[8]="VwllSh";
  names[9]="VhldDel";
  names[10]="Vtrim";
  names[11]="VthrComp";
  names[12]="VIBias_Bus";
  names[13]="Vbias_sf";
  names[14]="VoffsetOp";
  names[15]="VIbiasOp";
  names[16]="VOffsetR0";
  names[17]="VIon";
  names[18]="VIbias_PH";
  names[19]="Ibias_DAC";
  names[20]="VIbias_roc";
  names[21]="VIColOr";
  names[22]="Vnpix";
  names[23]="VSumCol";
  names[24]="Vcal";
  names[25]="CalDel";

}

void dacVariation()
  
{

  Init();  

  int oldValue[6][26];
  int newValue[6][26];

  
  ReadDACParameterFile("old0.dat");
  for (int i = 0; i < 26; i++) oldValue[0][i] = values[i];
  ReadDACParameterFile("new0.dat");
  for (int i = 0; i < 26; i++) newValue[0][i] = values[i];
  ReadDACParameterFile("old1.dat");
  for (int i = 0; i < 26; i++) oldValue[1][i] = values[i];
  ReadDACParameterFile("new1.dat");
  for (int i = 0; i < 26; i++) newValue[1][i] = values[i];
  ReadDACParameterFile("old2.dat");
  for (int i = 0; i < 26; i++) oldValue[2][i] = values[i];
  ReadDACParameterFile("new2.dat");
  for (int i = 0; i < 26; i++) newValue[2][i] = values[i];
  ReadDACParameterFile("old3.dat");
  for (int i = 0; i < 26; i++) oldValue[3][i] = values[i];
  ReadDACParameterFile("new3.dat");
  for (int i = 0; i < 26; i++) newValue[3][i] = values[i];
  ReadDACParameterFile("old4.dat");
  for (int i = 0; i < 26; i++) oldValue[4][i] = values[i];
  ReadDACParameterFile("new4.dat");
  for (int i = 0; i < 26; i++) newValue[4][i] = values[i];
  ReadDACParameterFile("old5.dat");
  for (int i = 0; i < 26; i++) oldValue[5][i] = values[i];
  ReadDACParameterFile("new5.dat");
  for (int i = 0; i < 26; i++) newValue[5][i] = values[i];

  
  TCanvas *c[26];
  TLegend *leg[26];
  TH1D *histOld[26];
  TH1D *histNew[26];


  for (int i = 0; i < 26; i++)
    {
      
      histOld[i] = new TH1D(Form("histOld%i",i),"Old Settings",6,0,6);
      histNew[i] = new TH1D(Form("histNew%i",i),"New Settings",6,0,6);
      
      for (int dose = 0; dose < 6; dose++)
	{
	  histOld[i]->SetBinContent(dose+1,oldValue[dose][i]);
	  histNew[i]->SetBinContent(dose+1,newValue[dose][i]);
	}
      
      histOld[i]->SetLineColor(kBlue);
      histNew[i]->SetLineColor(kRed);
      histOld[i]->SetMarkerColor(kBlue);
      histNew[i]->SetMarkerColor(kRed);
      histOld[i]->SetMarkerStyle(20);
      histNew[i]->SetMarkerStyle(29);
      histOld[i]->GetYaxis()->SetRangeUser(0,255);
      histNew[i]->GetYaxis()->SetRangeUser(0,255);
      histOld[i]->GetYaxis()->SetTitle("DAC [DAC units]");
      histNew[i]->GetYaxis()->SetTitle("DAC [DAC units]");
      histOld[i]->GetXaxis()->SetTitle("Dose");
      histNew[i]->GetXaxis()->SetTitle("Dose");



      histOld[i]->SetTitle(names[i]);
      histNew[i]->SetTitle(names[i]);
      histOld[i]->SetName(names[i]);
      histNew[i]->SetName(names[i]);

      c[i] = new TCanvas();
      c[i]->Clear();
      leg[i] = new TLegend(0.65,0.9,0.95,1.);
      leg[i]->AddEntry(histOld[i],"Old Settings","L");
      leg[i]->AddEntry(histNew[i],"New Settings","L");


      histOld[i]->Draw("P0");
      histNew[i]->Draw("sameP0");
      leg[i]->Draw();


      if (produceFiles) c[i]->Print(Form("plots/%i.png",i));

    }

}


bool ReadDACParameterFile(char filename[])
{
  file = fopen(filename, "r");
  if (!file) 
    {
      printf("!!!!!!!!!  ----> Could not open file %s to read DAC parameters\n", filename);
      return false;
    }
  
  int value;
  int reg;
  printf("Reading DAC-Parameters from %s\n", filename);
  
  do
    {
      if (Read_int(reg,value)) values[reg-1] = value ;
      reg++;
    }
  while (reg <= 26);
  fclose(file);
  return true;
}


bool Read_int(int &reg, int &value)
{
  char s[200], string;
  fgets(s, 200, file);
  if (s == NULL) return false;
  if ((s == NULL) || (s[0] == '#') || (s[0] == '-') || (s[0] == '\n')) return false;
  bool isSpace = true;
  for ( int ichar = 0; ichar < strlen(s); ichar++ )
    {
      if ( !isspace(s[ichar]) )
	{
	  isSpace = false;	
	  break;
	}
    }	
  if ( isSpace ) return false;
  
  int v = -1, i = -1;
  sscanf(s,"%i %s %i ",&i, &string, &v);
  value = v;
  reg = i;
  if (value < 0 || value > 256) return false;
  if (reg < 0 || reg > 256) return false;
  return true;
}
