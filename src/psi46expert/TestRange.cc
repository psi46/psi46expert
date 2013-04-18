#include "TestRange.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;

ClassImp(TestRange)

TestRange::TestRange()
{
    for (int iRoc = 0; iRoc < MODULENUMROCS; iRoc++)
    {
        for (int iCol = 0; iCol < ROCNUMCOLS; iCol++)
        {
            for (int iRow = 0; iRow < ROCNUMROWS; iRow++) pixel[iRoc][iCol][iRow] = false;
        }
    }
}


void TestRange::CompleteRange()
{
    for (int iRoc = 0; iRoc < MODULENUMROCS; iRoc++)
    {
        CompleteRoc(iRoc);
    }
}


void TestRange::CompleteRoc(int iRoc)
{
    for (int iCol = 0; iCol < ROCNUMCOLS; iCol++)
    {
        for (int iRow = 0; iRow < ROCNUMROWS; iRow++) pixel[iRoc][iCol][iRow] = true;
    }
}


void TestRange::AddPixel(int iRoc, int col, int row)
{
    pixel[iRoc][col][row] = true;
}


void TestRange::RemovePixel(int iRoc, int col, int row)
{
    pixel[iRoc][col][row] = false;
}


bool TestRange::IncludesPixel(int iRoc, int col, int row)
{
    return pixel[iRoc][col][row];
}


bool TestRange::IncludesRoc(int iRoc)
{
    bool result = false;
    for (int k = 0; k < ROCNUMCOLS; k++)
    {
        for (int l = 0; l < ROCNUMROWS; l++)
        {
            if (pixel[iRoc][k][l]) result = true;
        }
    }
    return result;
}


bool TestRange::IncludesDoubleColumn(int iRoc, int doubleColumn)
{
    bool result = false;
    for (int k = doubleColumn * 2; k < doubleColumn * 2 + 2; k++)
    {
        for (int l = 0; l < ROCNUMROWS; l++)
        {
            if (pixel[iRoc][k][l]) result = true;
        }
    }
    return result;
}


bool TestRange::IncludesColumn(int column)
{
    bool result = false;
    for (int iRoc = 0; iRoc < MODULENUMROCS; iRoc++)
    {
        for (int l = 0; l < ROCNUMROWS; l++)
        {
            if (pixel[iRoc][column][l]) result = true;
        }
    }
    return result;
}


bool TestRange::IncludesColumn(int iRoc, int column)
{
    bool result = false;
    for (int l = 0; l < ROCNUMROWS; l++)
    {
        if (pixel[iRoc][column][l]) result = true;
    }
    return result;
}

bool TestRange::ExcludesColumn(int iRoc, int column)
{
    bool result = false;
    for (int l = 0; l < ROCNUMROWS; l++)
    {
        pixel[iRoc][column][l] = false;
    }
    return result;
}

bool TestRange::ExcludesRow(int iRoc, int row)
{
    bool result = false;
    for (int l = 0; l < ROCNUMCOLS; l++)
    {
        pixel[iRoc][l][row] = false;
    }
    return result;
}

bool TestRange::ExcludesRoc(int iRoc) {

    bool result = false;
    for (int l = 0; l < ROCNUMROWS; l++) {
        for (int m = 0; m < ROCNUMCOLS; m++) {
            pixel[iRoc][m][l] = false;
        }
    }
    return result;
}

void TestRange::ApplyMaskFile(const char * fileName) {
    char fname[1000];
    sprintf(fname, "%s", fileName);

    int roc, col, row;
    char keyWord[100], line[1000];

    ifstream maskFile;
    maskFile.open(fname);

    if (maskFile.bad())
    {
        cout << "!!!!!!!!!  ----> Could not open file " << fname << " to read pixel mask\n";
        return;
    }

    cout << "Reading pixel mask from " << fname << endl;

    while (maskFile.good()) {
        maskFile >> keyWord;
        if (strcmp(keyWord, "#") == 0) {
            maskFile.getline(line, 60, '\n');
            cout << "# " << line << endl; // ignore rows starting with "#" = comment
        }
        else if (strcmp(keyWord, "pix") == 0) {
            maskFile >> roc >> col >> row;
            cout << "Exclude " << keyWord << " " << roc << " " << col << " " << row << endl;
            if ((roc >= 0) && (roc < MODULENUMROCS) && (col >= 0) && (col < ROCNUMCOLS) && (row >= 0) && (row < ROCNUMROWS)) {
                RemovePixel(roc, col, row);
            } else {
                cout << "!!!!!!!!!  ----> Pixel number out of range: " << keyWord << " " << roc << " " << col << " " << row << endl;
            }
        } else if (strcmp(keyWord, "col") == 0) {
            maskFile >> roc >> col;
            cout << "Exclude " << keyWord << " " << roc << " " << col << endl;
            if ((roc >= 0) && (roc < MODULENUMROCS) && (col >= 0) && (col < ROCNUMCOLS)) {
                ExcludesColumn(roc, col);
            } else {
                cout << "!!!!!!!!!  ----> Pixel number out of range: " << keyWord << " " << roc << " " << col << endl;
            }
        } else if (strcmp(keyWord, "row") == 0) {
            maskFile >> roc >> row;
            cout << "Exclude " << keyWord << " " << roc << " " << row << endl;
            if ((roc >= 0) && (roc < MODULENUMROCS) && (row >= 0) && (row < ROCNUMROWS)) {
                ExcludesRow(roc, row);
            } else {
                cout << "!!!!!!!!!  ----> Pixel number out of range: " << keyWord << " " << roc << " " << row << endl;
            }
        } else if (strcmp(keyWord, "roc") == 0) {
            maskFile >> roc;
            cout << "Exclude " << keyWord << " " << roc << endl;
            if ((roc >= 0) && (roc < MODULENUMROCS)) {
                ExcludesRoc(roc);
            } else {
                cout << "!!!!!!!!!  ----> Pixel number out of range: " << keyWord << " " << roc << " " << col << " " << row << endl;
            }
        }
        strcpy(keyWord, "\0");

    }

    maskFile.close();

    return;

}

void TestRange::Print()
{
    for (int i = 0; i < MODULENUMROCS; i++)
    {
        for (int k = 0; k < ROCNUMCOLS; k++)
        {
            for (int l = 0; l < ROCNUMROWS; l++)
            {
                if (pixel[i][k][l]) printf("pixel %i %i %i\n", i, k, l);
            }
        }
    }
}
