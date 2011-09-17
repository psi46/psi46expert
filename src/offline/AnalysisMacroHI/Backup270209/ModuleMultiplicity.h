#ifndef MODULEMULTIPLICITY_H
#define MODULEMULTIPLICITY_H

#include <iostream>

#define Ncols  52
#define Nrows  80
#define NRocs  16

using namespace std;

class ModuleMultiplicity{
 private:
  int * ROCMultip; //[NRocs];
  int * ROCPh; //[NRocs];            //total Ph
  int * ROCPhPuls;
  int * PulseNr;
  int ModuleMultip;
  int * ActivCol;   //[NRocs][NCols];
  int * ActivRow;   //[NRocs][Nrows];
 public:
  ModuleMultiplicity();
  ~ModuleMultiplicity();
  
  void Clean();
  void Add(int roc, int row, int col, int Ph, int PhPuls);
  int NumberOfColumns(int roc);
  int NumberOfRows(int roc); 
  int ColumnMultiplicity(int roc, int col){return ActivCol[roc * Ncols + col];};
  int RowMultiplicity(int roc, int row){return ActivRow[roc * Nrows + row];};
  int ROCMultiplicity(int rocN){return ROCMultip[rocN];};
  int ModMultiplicity(){return ModuleMultip;};
  float MeanPh(int roc);
  float MeanPhPuls(int roc);
};

#endif
