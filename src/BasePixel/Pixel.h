// Class provides basic pixel functionalities, contains the trim information

#ifndef PIXEL
#define PIXEL

class Roc;

class Pixel
{


public:
	Pixel(Roc * const roc, const int columnNumber, const int rowNumber);
	inline virtual ~Pixel() {};

	// == Actions ==========================================================

	void EnablePixel();
	void DisablePixel();
	void MaskCompletely();  //to be distinguished from DisablePixel
	const void Cal();
	const void Cals();
	void ArmPixel();
	void DisarmPixel();
	void SetTrim(int trimBit);
	int GetTrim();
	bool IsAlive();
	int GetColumn();
	int GetRow();
	void SetAlive(bool aBoolean);


protected:

	const int column, row;
	int trim;
	bool enabled, alive, masked;

	Roc* const roc;
};

#endif

