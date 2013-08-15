/* geometry definition for ROCs and Modules 



ROC coordinates:
 x-axis runs along the rows, along the readout direction
 y=0 lies in the center of row 0 (closest to the periphery)
 The origin (o) is in the lower left corner of the active region

   Y
   |
   .--x
                    +-------+---+---+---+---+---+--       --+
                    |       |   |   |   |   |   |           |
               80   |       |   |   |   |   |   |           |
                    |       |   |   |   |   |   |           |
                    +-------+---+---+---+---+---+--       --+
               79   |       |   |   |   |   |   |           |
                    +-------+---+---+---+---+---+--       --+
               78   |       |   |   |   |   |   |           |
                    +-------+---+---+---+---+---+--       --+
               77   |       |   |   |   |   |   |           |
                    +-------+---+---+---+---+---+--       --+
		    |       |   |   |   |   |   |           |
		.                                   ...
		.
		    |       |   |   |   |   |   |           |
                    +-------+---+---+---+---+---+-- ...   --+
                3   |       |   |   |   |   |   |           |
                    +-------+---+---+---+---+---+--       --+
                2   |       |   |   |   |   |   |           |
                    +-------+---+---+---+---+---+--       --+
                1   |       |   |   |   |   |   |           |
                    +-------+---+---+---+---+---+--       --+
                0   |       |   |   |   |   |   |           |
                    o-------+---+---+---+---+---+-- ...   --+
                    |                                       |
      column:       |   0     1   2   3   4              51 |
                    |                                       |
                    |     Periphery                         |
                    |                                       |
                    +------------------------------  ...   -+

Lower left corner coordinates of some pixels:
row    col     xLL      yLL
0      0          0u     0
0      1       +300u     0
0      2       +450u     0
0      51      


The coordinates passed from/returned to the user can use a different
origin, defined by rocX0, rocY0 relative to "o" at construction time of this object.
The GetLocal() return the coordinates of the center of the pixel (relative
to the chosen origin).

Module coordinates:
The axes are rotated by 90/270 degrees wrt to ROC coordinates
x-axis runs along the ROCs columns
y-axis runs along the rows
The origin is the corner between rocs 3,4,11,12  (rocs facing "up")

                      Y

                      A
                      |
                      |
                  +---+---+
		  | 8 | 7 |
		  +---+---+
		  | 9 | 6 |
		  +---+---+
		  |10 | 5 |
		  +---+---+
		  |11 | 4 |
		  +---O---+-----> X
		  |12 | 3 |
		  +---+---+
		  |13 | 2 |
		  +---+---+
		  |14 | 1 |
		  +---+---+
		  |15 | 0 |
		  +---+---+


All dimensions are in cm

*/

  // size of the active region in cm
  static const double colWidth=0.0150;
  static const double rowHeight=0.0100;
  static const double rocWidth =54*colWidth;  
  static const double rocHeight=81*rowHeight;
  static const double epsilon=0.0001;
  static const double xActiveMin=-epsilon;
  static const double xActiveMax=54*colWidth+epsilon;
  static const double yActiveMin=-epsilon;
  static const double yActiveMax=81*rowHeight+epsilon;

class RocGeometry{
  
 public:
  double rocX0;   // center of the ROC's coordinate system 
  double rocY0;  
  double modX0;
  double modY0;
  RocGeometry(double x0=0, double y0=0);
  bool getRocColRow(double* xy, int &col, int &row);
  bool getModColRow(double* xy, int &roc, int &col, int &row);
  void getRocLocalLL(int col, int row, double* xy);
  void getRocLocal(int col, int row, double* xy);
  void getRocLocal(int col, int row, float* xy);
  void getModLocal(int roc, int col, int row, double* xy);
  void getModLocal(int roc, int col, int row, float* xy);
};
