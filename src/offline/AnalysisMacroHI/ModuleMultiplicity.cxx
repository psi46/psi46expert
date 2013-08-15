#include "ModuleMultiplicity.h"

ModuleMultiplicity::ModuleMultiplicity(){
   ROCMultip = new int[NRocs];
   ROCPh = new int[NRocs];
   ROCPhPuls = new int[NRocs];
   PulseNr = new int[NRocs];
   ActivCol = new int[NRocs* Ncols];
   ActivRow = new int[NRocs * Nrows];                                      
   ModuleMultiplicity::Clean();
}

ModuleMultiplicity::~ModuleMultiplicity(){
  //delete [] ROCMultip;
  //delete [] ROCPh;
  //delete [] ActivCol;
  //delete [] ActivRow;
}

void ModuleMultiplicity::Clean(){
     int i,j;
     for(i=0; i < NRocs; ++i){
       ROCMultip[i] =0;
       ROCPh[i] =0;
       ROCPhPuls[i] = 0;
       PulseNr[i] = 0;
       for(j=0; j < Ncols; ++j) ActivCol[i * Ncols+ j] =0;
       for(j=0; j < Nrows; ++j) ActivRow[i * Nrows + j] =0;
     }
     ModuleMultip =0;
}


void ModuleMultiplicity::Add(int roc, int row, int col, int Ph, bool SelPixel){
     ModuleMultip++;
     ROCMultip[roc]++;
     ROCPh[roc]+=Ph;
     if(SelPixel){
       PulseNr[roc]++;
       ROCPhPuls[roc]+=Ph;
     }
     ActivCol[roc * Ncols + col]++;
     ActivRow[roc * Nrows + row]++;
}

int ModuleMultiplicity::NumberOfColumns(int roc){
      int NActCol =0;
      for(int i =0; i < Ncols; ++i) if(ActivCol[roc * Ncols+ i] >0) ++NActCol;
      return NActCol;
}

int ModuleMultiplicity::NumberOfRows(int roc){
      int NActRow =0;
      for(int i =0; i < Nrows; ++i) if(ActivRow[roc * Nrows+ i] >0) ++NActRow;
      return NActRow;
}

float ModuleMultiplicity::MeanPh(int roc){
	  if(ROCMultip[roc] == 0){ 
        if(ROCPh[roc] >0) cout <<"ERROR ROC " << roc << ": Multiplicity =0 with Ph !=0" << endl;  
        return -10000;
      }
      return ROCPh[roc]/ROCMultip[roc];
}

float ModuleMultiplicity::MeanPhPuls(int roc){
      
	  if(PulseNr[roc] == 0){ 
        if(ROCPhPuls[roc] >0) cout <<"ERROR ROC " << roc << ": Multiplicity =0 with PhPuls !=0" << endl;
        return -10000;
      }
      return ROCPhPuls[roc]/PulseNr[roc];
}
