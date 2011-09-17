#include "BasePixel/Pixel.h"
#include "BasePixel/Roc.h"


Pixel::Pixel(Roc* const aRoc, const int columnNumber, const int rowNumber) :
	column(columnNumber), row(rowNumber), roc(aRoc)
{
	trim = 15;
	enabled = false;
	alive = false;
	masked = false;
}


void Pixel::EnablePixel()
{
	if (masked) return;
	roc->PixTrim(column, row, trim);
	enabled = true;
}


void Pixel::DisablePixel()
{
	roc->PixMask(column, row);
	enabled = false;
}


void Pixel::MaskCompletely()
{
	masked = true;
}


const void Pixel::Cal()
{
	roc->PixCal(column, row, 0);
}


const void Pixel::Cals()
{
	roc->PixCal(column, row, 1);
}


void Pixel::ArmPixel()
{
	EnablePixel();
	Cal();
}


void Pixel::DisarmPixel()
{
	DisablePixel();
	roc->ClrCal();
}


void Pixel::SetTrim(int trimBit)
{
	trim = trimBit;
}


int Pixel::GetTrim()
{
	return trim;
}


bool Pixel::IsAlive()
{
	return alive;
}


int Pixel::GetColumn()
{
	return column;
}


int Pixel::GetRow()
{
	return row;
}

void Pixel::SetAlive(bool aBoolean)
{
	alive = aBoolean;
}
