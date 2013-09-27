#include "TestRange.h"
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <cstring>
#include "interface/Log.h"

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
        psi::LogInfo() << "[TestRange] Could not open file " << fname << " to read pixel mask!" << psi::endl;
        return;
    }

    psi::LogInfo() << "[TestRange] Reading pixel mask from " << fname << " ..." << psi::endl;

    while (maskFile.good()) {
        maskFile >> keyWord;
        if (strcmp(keyWord, "#") == 0) {
            maskFile.getline(line, 60, '\n');
            psi::LogDebug() << "[TestRange] # " << line << psi::endl; // ignore rows starting with "#" = comment
        }
        else if (strcmp(keyWord, "pix") == 0) {
            maskFile >> roc >> col >> row;
            psi::LogInfo() << "[TestRange] Exclude " << keyWord << " " << roc << " " << col << " " << row << psi::endl;
            if ((roc >= 0) && (roc < MODULENUMROCS) && (col >= 0) && (col < ROCNUMCOLS) && (row >= 0) && (row < ROCNUMROWS)) {
                RemovePixel(roc, col, row);
            } else {
                psi::LogInfo() << "[TestRange] Pixel number out of range: " << keyWord << " " << roc << " " << col << " " << row << psi::endl;
            }
        } else if (strcmp(keyWord, "col") == 0) {
            maskFile >> roc >> col;
            psi::LogInfo() << "[TestRange] Exclude " << keyWord << " " << roc << " " << col << psi::endl;
            if ((roc >= 0) && (roc < MODULENUMROCS) && (col >= 0) && (col < ROCNUMCOLS)) {
                ExcludesColumn(roc, col);
            } else {
                psi::LogInfo() << "[TestRange] Pixel number out of range: " << keyWord << " " << roc << " " << col << psi::endl;
            }
        } else if (strcmp(keyWord, "row") == 0) {
            maskFile >> roc >> row;
            psi::LogInfo() << "[TestRange] Exclude " << keyWord << " " << roc << " " << row << psi::endl;
            if ((roc >= 0) && (roc < MODULENUMROCS) && (row >= 0) && (row < ROCNUMROWS)) {
                ExcludesRow(roc, row);
            } else {
                psi::LogInfo() << "[TestRange] Pixel number out of range: " << keyWord << " " << roc << " " << row << psi::endl;
            }
        } else if (strcmp(keyWord, "roc") == 0) {
            maskFile >> roc;
            psi::LogInfo() << "[TestRange] Exclude " << keyWord << " " << roc << psi::endl;
            if ((roc >= 0) && (roc < MODULENUMROCS)) {
                ExcludesRoc(roc);
            } else {
                psi::LogInfo() << "[TestRange] Pixel number out of range: " << keyWord << " " << roc << " " << col << " " << row << psi::endl;
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

/**
    Finds a pixel on the ROC that is in the test range (unmasked). It
    starts by trying pixel 20:20 and if that pixel exits, it is returned.
    If it doesn't exist, all pixels will be tried except for the edge
    pixels until a valid pixel is found.
    @param roc ROC ID
    @param col Integer where the column number (1-50) is returned. Column 0
    and column 51 will never be returned.
    @param row Integer where the row number (1-78) is returned. Row 0 and
    row 79 will never be returned.
    @return 1: Success, 0: failure
 */
int TestRange::GetValidPixel(int roc, int & col, int & row)
{
    /* Check the ROC number */
    if (roc >= MODULENUMROCS)
        return 0;

    /* Check default pixel 20:20 */
    if (pixel[roc][20][20]) {
        col = 20;
        row = 20;
        return 1;
    }

    /* Find any pixel that is not masked (except for edge pixels) */
    for (col = 1; col < ROCNUMCOLS - 1; col++) {
        for (row = 1; row < ROCNUMROWS - 1; row++) {
            if (pixel[roc][col][row])
                return 1;
        }
    }

    /* No pixel was found */
    return 0;
}

/**
    Takes a 2D histogram of size 52x80 (ROC map) and finds the maximum bin while
    avoiding masked pixels.
    @param map A 2D histogram with 52 bins on the x-axis and 80 bins on the
    y-axis (ROC map)
    @param roc ROC ID
    @param col Integer where the column number (starting at zero) is returned
    @param row Integer where the row number (starting at zero) is returned
    @param minimum Optional parameter to search for the minimum (true) instead
    of the maximum (false). Default: false
    @return 1: success, 0: failure
 */
int TestRange::GetMapMaximum(TH2 * map, int roc, int & col, int & row, bool minimum)
{
    /* Check histogram dimensions */
    if (map->GetXaxis()->GetNbins() != ROCNUMCOLS || map->GetYaxis()->GetNbins() != ROCNUMROWS)
        return 0;

    /* Define value to start with */
    double bin_content;
    if (minimum)
        bin_content = 1e300;
    else
        bin_content = -1e300;

    /* Scan through the histogram to find the maximum/minimum */
    int found = 0;
    for (int c = 0; c < ROCNUMCOLS; c++) {
        for (int r = 0; r < ROCNUMROWS; r++) {
            if (minimum && map->GetBinContent(c + 1, r + 1) < bin_content && pixel[roc][c][r]) {
                bin_content = map->GetBinContent(c + 1, r + 1);
                col = c;
                row = r;
                found = 1;
            } else if (!minimum && map->GetBinContent(c + 1, r + 1) > bin_content && pixel[roc][c][r]) {
                bin_content = map->GetBinContent(c + 1, r + 1);
                col = c;
                row = r;
                found = 1;
            }
        }
    }

    return found;
}

/**
    Takes a 2D histogram of size 52x80 (ROC map) and finds the minimum bin while
    avoiding masked pixels.
    @param map A 2D histogram with 52 bins on the x-axis and 80 bins on the
    y-axis (ROC map)
    @param roc ROC ID
    @param col Integer where the column number (starting at zero) is returned
    @param row Integer where the row number (starting at zero) is returned
    @return 1: success, 0: failure
 */
int TestRange::GetMapMinimum(TH2 * map, int roc, int & col, int & row)
{
    /* Use the maximum function with the optional parameter set to true */
    GetMapMaximum(map, roc, col, row, true);
}

/**
    Takes a 2D histogram of size 52x80 (ROC map) and finds the maximum value while
    avoiding masked pixels.
    @param map A 2D histogram with 52 bins on the x-axis and 80 bins on the
    y-axis (ROC map)
    @param roc ROC ID
    @param minimum Optional parameter to search for the minimum (true) instead
    of the maximum (false). Default: false
    @return Maximum of the map, or -1e300 if the maximum cannot be found
 */
float TestRange::GetMapMaximum(TH2 * map, int roc, bool minimum)
{
    int col, row;
    if (GetMapMaximum(map, roc, col, row, minimum))
        return map->GetBinContent(col + 1, row + 1);
    else
        return -1e300;
}

/**
    Takes a 2D histogram of size 52x80 (ROC map) and finds the minimum value while
    avoiding masked pixels.
    @param map A 2D histogram with 52 bins on the x-axis and 80 bins on the
    y-axis (ROC map)
    @param roc ROC ID
    @return Minimum of the map, or -1e300 if the maximum cannot be found
 */
float TestRange::GetMapMinimum(TH2 * map, int roc)
{
    float value = GetMapMaximum(map, roc, true);
    return (value == -1e300) ? -value : value;
}
