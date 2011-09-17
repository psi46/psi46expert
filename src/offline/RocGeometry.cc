#include "RocGeometry.h"

RocGeometry::RocGeometry(double x0, double y0){
  rocX0=x0;
  rocY0=y0;
  modX0=0;
  modY0=0;
}


bool RocGeometry::getRocColRow(double* xy, int &col, int &row){
  // convert coordinates into column/row
  // returns true if the pixel is inside the active area of the roc
  // for coordinates outside of the active area, the row/column
  // values are clipped and the return value is false

  bool inside=true;
  double dx= xy[0]+rocX0;
  col= int((dx-colWidth)/colWidth);  // note that int(-0.9999) is 0
  if( col< 0){
	 col=0; 
	 if (dx<xActiveMin) {inside=false;}
  }else if(col>51){
	 col=51;
	 if(dx>xActiveMax){inside=false;}
  }

  double dy = xy[1]+rocY0; 
  if(dy<yActiveMin){
	 row=0;
	 inside=false;
  }else{
	 row=int(dy/rowHeight);
	 if    (row>79){row=79;}
	 if(dy>yActiveMax){inside=false;}
  }
  return inside;
}


bool RocGeometry::getModColRow(double* xy, int &roc, int &col, int &row){
  // get row column from module coordinates
  bool inside=true;
  double dx=xy[0]+modX0;
  double dy=xy[1]+modY0;
  double xr[2];
  
  if(dx>0){  // 0..7
	 roc= int(4+dy/rocWidth);
	 if     (roc<0){ roc =0; } else if(roc>7){ roc=7;}
	 xr[0]=dy+4*rocWidth-roc*rocWidth;
	 xr[1]=rocHeight-dx;
  }else{   // 8..15
	 roc=8+int( (4*rocWidth-dy)/rocWidth); 
	 if     (roc<8){ roc =8; } else if(roc>15){ roc=15;}
	 xr[0]=(12-roc)*rocWidth -dy;
	 xr[1]=dx+rocHeight                     ;
  }
  // xr is relative to the LL corner of the active region by now
  // shift to the system expected by getRocColRow
  xr[0]-=rocX0;
  xr[1]-=rocY0;
  inside = getRocColRow(xr,  col, row);
  return inside;

}


void RocGeometry::getRocLocalLL(int col, int row, double* xy){
  /* get local roc coordinates of the center of a pixel 
	  relative to the Lower left corner of the
	  active region of the ROC
	  the x-centers are
	  col=0       150
	  col=1       375
	  col=2       525
  */
  xy[0]= 1.5*colWidth + colWidth* col;   // valid for col =1,2,..,50
  xy[1]= 0.5*rowHeight+ rowHeight* row;   // valid for row=0,..,78
  // correction for wider pixels
  if( col== 0){
	 xy[0]-= 0.5*colWidth;   // i.e. 150
  }else if( col==51 ){
	 xy[0]+= 0.5*colWidth;
  }
  if( row==79){ xy[1]+= 0.5*rowHeight; }
}

void RocGeometry::getRocLocal(int col, int row, double* xy){
  getRocLocalLL(col,row,xy);
  xy[0]-=rocX0;  // return user coordinates
  xy[1]-=rocY0; 
}

void RocGeometry::getRocLocal(int col, int row, float* xy){
  double dxy[2];
  getRocLocal(col, row, dxy);
  xy[0]=dxy[0];
  xy[1]=dxy[1];
}


void RocGeometry::getModLocal(int roc, int col, int row, double* xy){
  double xrll[3];
  getRocLocalLL(col, row, xrll);
  if(roc<8){
	 xy[0]=rocHeight-xrll[1];
	 xy[1]=(roc-4)*rocWidth+xrll[0];
  }else{
	 xy[0]=-rocHeight+xrll[1];
	 xy[1]=(12-roc)*rocWidth-xrll[0];
  }
}

void RocGeometry::getModLocal(int roc, int col, int row, float* xy){
  double dxy[2];
  getModLocal(roc,col,row,dxy);
  xy[0]=dxy[0];
  xy[1]=dxy[1];
}


