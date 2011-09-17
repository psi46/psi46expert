#include "BasePixel/DoubleColumn.h"
#include "BasePixel/Roc.h"


DoubleColumn::DoubleColumn(Roc* aRoc, int dColumn) {
	roc = aRoc;
	doubleColumn = dColumn;
	for (int i = 0; i<ROCNUMROWS; i++) {
		pixel[i] = new Pixel(roc,doubleColumn*2,i);
		pixel[i+ROCNUMROWS] = new Pixel(roc,doubleColumn*2+1,i);
	}
}


DoubleColumn::~DoubleColumn() {
	for (int i = 0; i<NPixels; i++) {
		delete pixel[i];
	}
}


void DoubleColumn::EnableDoubleColumn() {
	roc->ColEnable(doubleColumn * 2, 1);
}


int DoubleColumn::DoubleColumnNumber()
{
	return doubleColumn;
}

void DoubleColumn::DisableDoubleColumn() {
	roc->ColEnable(doubleColumn * 2, 0);
}


void DoubleColumn::Mask() {
	DisableDoubleColumn();
	for (int i = 0; i<ROCNUMROWS; i++) {
		pixel[i]->DisablePixel();
		pixel[i+ROCNUMROWS]->DisablePixel();
	}
}

Pixel* DoubleColumn::GetPixel(int column, int row) {
	int n = (column % 2) * ROCNUMROWS + row;
	return pixel[n];
}


void DoubleColumn::EnablePixel(int col, int row)
{
	EnableDoubleColumn();
	GetPixel(col,row)->EnablePixel();
}

void DoubleColumn::DisablePixel(int col, int row)
{
	GetPixel(col,row)->DisablePixel();
}

void DoubleColumn::Cal(int col, int row)
{
	GetPixel(col,row)->Cal();
}

void DoubleColumn::Cals(int col, int row)
{
	GetPixel(col,row)->Cals();
}

// == Tests ===========================================

void DoubleColumn::ArmPixel(int column, int row) {
	EnableDoubleColumn();
	GetPixel(column,row)->ArmPixel();
}


void DoubleColumn::DisarmPixel(int column, int row) {
	DisableDoubleColumn();
	GetPixel(column,row)->DisarmPixel();
}
