#include "BasePixel/Keithley.h"
#include <time.h>
#include <stdio.h>
#include <iostream.h>

  


int main(int argc, char* argv[]) {
  
  Keithley k;
  k.Open();

  float v, c;
  k.ReadCurrent(v, c);

  k.GoLocal();
	
  cout << v << ":" << c << endl;

  return 0;
}
