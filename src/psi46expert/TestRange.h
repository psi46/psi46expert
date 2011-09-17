// Defines for which entities a test should be performed

#ifndef TESTRANGE
#define TESTRANGE


#include "BasePixel/GlobalConstants.h"
#include <TObject.h>

class TestRange : public TObject
{

public:
	TestRange();
	
	void AddPixel(int roc, int col, int row);
	void RemovePixel(int roc, int col, int row);
	bool ExcludesColumn(int roc, int col);
	bool ExcludesRow(int roc, int row);
	bool ExcludesRoc(int roc);

	void CompleteRange();
	void CompleteRoc(int iRoc);
	
	bool IncludesPixel(int roc, int col, int row);
	bool IncludesRoc(int roc);
	bool IncludesDoubleColumn(int roc, int doubleColumn);
	bool IncludesColumn(int roc, int column);
	bool IncludesColumn(int column);
	
	void ApplyMaskFile( const char *fileName);
	
	void Print();
		
protected:
	
	bool pixel[MODULENUMROCS][ROCNUMCOLS][ROCNUMROWS];

	ClassDef(TestRange, 1) 
};


#endif

